// Captura rapida de corriente y tension ante un escalon de tension en el motor 775.
// Solo para ensayos en banco: no forma parte del firmware del yoke.
//
// Que hace: aplica un escalon de ciclo de trabajo con el BTS7960 y guarda en RAM
// la corriente (ACS712) y la tension de la fuente (B+) muestreadas a 10 kHz,
// sincronizadas con el PWM. Al terminar, manda todo por USB como CSV para MATLAB
// (script Capturar_Escalon.m en Modelado_Motor_DC).
//
// Placa: Arduino Pro Micro 5 V / 16 MHz (ATmega32U4). En el IDE: "Arduino Leonardo".
//
// Conexiones (iguales al Ensayo 1, mas el divisor de tension en A1):
//   BTS7960 (IBT-2) -> Pro Micro
//     VCC -> VCC (5 V)      GND -> GND
//     RPWM -> pin 9         LPWM -> pin 10 (queda siempre en LOW)
//     R_EN y L_EN (unidos) -> pin 7, con 10 k de pin 7 a GND
//     R_IS, L_IS -> sin conectar
//   BTS7960 potencia: B+ -> +24 V con FUSIBLE en serie, B- -> GND de la fuente
//                     M+ y M- -> motor, con el ACS712 en serie en uno de los dos
//   ACS712 20 A (100 mV/A): VCC -> VCC (5 V), GND -> GND, OUT -> A0
//   Divisor de tension de la fuente: B+ -> 47 k -> A1 -> 10 k -> GND,
//                                    y 10 nF de A1 a GND (cerca del Pro Micro)
//   Masa comun: GND del Pro Micro, del BTS7960 y negativo de la fuente.
//
// Como se mide:
//   - PWM de 20 kHz (Timer1). El ADC lo dispara el Timer1 (Compare Match B) en la
//     MITAD del tiempo de encendido de cada periodo: ahi la corriente de rizado
//     pasa por su valor medio, asi que la muestra no tiene el rizado del PWM.
//   - Los disparos alternan canales: corriente, tension, corriente, ... Cada
//     par corriente-tension dura 100 us (10 kHz por canal).
//   - Con decimacion N, cada muestra guardada es el promedio de N pares.
//   - La tension media en bornes del motor es duty * Vbus (Vbus = B+ medida).
//
// Comandos (monitor serie o MATLAB, terminados en nueva linea):
//   z            calibra el cero del ACS712 (motor apagado). Hacerlo con la
//                fuente encendida y el motor quieto, antes de cada tanda.
//   u 24.1       calibra el divisor: escribir la tension de B+ medida con
//                multimetro (motor apagado). Se guarda en la EEPROM.
//   e D N M E    M escalones iguales: duty D en % (1 a 100), decimacion N (1 a 50),
//                M repeticiones (1 a 100, por defecto 2) y E segundos de espera
//                entre escalones para que el motor se detenga (0,5 a 30, por
//                defecto 3). Ejemplo: e 40 5 2 3. Cada escalon se manda apenas
//                termina, cerrado con FIN; al terminar todos se manda LISTO.
//                MATLAB promedia los M escalones (como el modo Average de un
//                osciloscopio). Si salta el corte, no se repite.
//                Ventana = 400 muestras * N * 0,1 ms (N=1: 40 ms, N=5: 200 ms,
//                N=20: 800 ms). Las primeras 20 muestras son con el motor apagado.
//                Al final de la ventana el driver se deshabilita y el motor
//                gira libre (sin frenado).
//   c D T [I]    corriente sostenida: duty D (1 a 60 %) durante T s (1 a 15) con el
//                eje trabado; cada 0,5 s informa la corriente del ACS712 y B+, y al
//                final el promedio de la segunda mitad. Para comparar el ACS712 con
//                un multimetro en serie con el motor. Corte en I A (0,5 a 12; por defecto
//                2,8 A, por el fusible de 3 A del multimetro).
//   ?            ayuda y estado.
//
// Seguridad:
//   - Corte inmediato (dentro de la interrupcion) si |I| > I_DISPARO.
//   - El ACS712 de 20 A satura a ~20 A. En vacio, el pico de arranque es
//     ~ D/100 * 24 V / 0,79 ohm: con 24 V usar D <= 50 (pico ~14 A).
//   - Con eje trabado la corriente final es D/100 * 24 / 0,79: usar D <= 15
//     (~4,5 A) y ventanas cortas.
//   - Encender primero el USB (driver apagado) y despues la fuente.
//
// Encoder magnetico AS5600 (opcional): poner USAR_AS5600 en 1 y, en MATLAB,
// usar_as5600 = true en Capturar_Escalon.m.
//   AS5600 -> Pro Micro: VCC -> VCC (5 V; en modulos de 3,3 V usar 3,3 V y
//   adaptador de nivel I2C), GND -> GND, SDA -> pin 2, SCL -> pin 3,
//   DIR -> GND (sentido horario = cuentas crecientes). Iman diametral en el
//   extremo del eje, a 0,5-3 mm del chip.
//   El angulo se lee por I2C (400 kHz) en el lazo principal mientras dura la
//   captura y se guarda el ultimo valor leido en cada muestra (cuentas 0-4095).
//   Cada lectura tarda ~0,15 ms: con N = 1 quedan muestras sin angulo (65535);
//   para la velocidad usar N >= 5. La memoria baja a 250 muestras
//   (N = 5: 125 ms; N = 10: 250 ms).
#define USAR_AS5600 0
//
// Tacometro LM393 (opcional, USAR_TACOMETRO 1): salida digital D0 -> pin 4,
// VCC -> 5 V, GND -> GND. El pin 4 (PD4) no tiene interrupcion externa y su
// captura por hardware (ICP1) es del Timer1, que genera el PWM; por eso se lee
// dentro de la interrupcion del ADC, cada 50 us, y se guarda el instante de cada
// flanco de bajada (resolucion 50 us: 0,8 % del periodo a 10000 rpm con 1 ranura).
// Se rechazan flancos mas cercanos que el periodo a TACO_RPM_MAX (rebotes).
// Despues de las muestras se mandan lineas "F,<t_s>" con el tiempo de cada flanco
// respecto del escalon; MATLAB calcula la velocidad (Capturar_Escalon.m).
#define USAR_TACOMETRO 1

#include <EEPROM.h>
#if USAR_AS5600
#include <Wire.h>
const uint8_t AS5600_DIR = 0x36;
const uint8_t AS5600_CONF = 0x07;      // CONF (0x07-0x08)
const uint8_t AS5600_STATUS = 0x0B;
const uint8_t AS5600_RAW = 0x0C;       // RAW ANGLE (0x0C-0x0D), 12 bits
#endif

const uint8_t PIN_RPWM = 9;   // OC1A
const uint8_t PIN_LPWM = 10;  // OC1B: no se usa como PWM, el canal B dispara el ADC
const uint8_t PIN_EN = 7;     // R_EN y L_EN

// Canales del ADC del ATmega32U4: A0 = ADC7, A1 = ADC6
const uint8_t MUX_I = 7;
const uint8_t MUX_V = 6;

const uint16_t PWM_TOP = 799;          // 16 MHz / 800 = 20 kHz
#if USAR_AS5600
const uint16_t N_MUESTRAS = 250;       // 250 * 6 bytes: deja ~640 bytes libres con Wire
#else
const uint16_t N_MUESTRAS = 400;
#endif
const uint16_t N_PRE = 20;             // muestras previas al escalon
const float SENS_V_A = 0.100f;         // ACS712 20 A
const float V_LSB = 5.0f / 1023.0f;    // referencia AVCC (USB)
const float I_DISPARO = 18.0f;         // A
const float KV_DEF = (57.0f / 10.0f) * V_LSB;  // divisor 47 k / 10 k

const int DIR_EEPROM = 0;
const uint32_t MARCA = 0x56425553UL;   // "VBUS"

// Buffers: sumas de N lecturas de 10 bits (N <= 50 -> < 65535)
uint16_t bufI[N_MUESTRAS];
uint16_t bufV[N_MUESTRAS];
#if USAR_AS5600
uint16_t bufA[N_MUESTRAS];    // angulo crudo del AS5600; 65535 = sin lectura

// Lee un registro de 16 bits del AS5600; devuelve 65535 si falla
uint16_t leer16(uint8_t reg) {
  Wire.beginTransmission(AS5600_DIR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return 65535;
  if (Wire.requestFrom(AS5600_DIR, (uint8_t)2) != 2) return 65535;
  uint16_t a = (uint16_t)Wire.read() << 8;
  return a | Wire.read();
}

uint8_t leerEstado() {
  Wire.beginTransmission(AS5600_DIR);
  Wire.write(AS5600_STATUS);
  if (Wire.endTransmission(false) != 0) return 0xFF;
  if (Wire.requestFrom(AS5600_DIR, (uint8_t)1) != 1) return 0xFF;
  return Wire.read();
}

void configurarAS5600() {
  Wire.begin();
  Wire.setClock(400000);
  // CONF: filtro lento SF = 11 (el mas rapido, ~0,29 ms de respuesta; el valor
  // de fabrica 00 tarda ~2,2 ms) y el resto en 0 (sin histeresis, filtro rapido
  // apagado). Se escribe en RAM, no se graba en el chip (no usa BURN).
  Wire.beginTransmission(AS5600_DIR);
  Wire.write(AS5600_CONF);
  Wire.write(0x03);   // CONF[13:8]: WD=0, FTH=000, SF=11
  Wire.write(0x00);   // CONF[7:0]: PWMF, OUTS, HYST, PM = 0
  Wire.endTransmission();
}
#endif

volatile uint16_t idx;        // muestra que se esta llenando
volatile uint8_t cuenta;      // pares acumulados en la muestra actual
volatile uint8_t decim;       // N
volatile bool canalV;         // false: la proxima conversion es corriente
volatile bool capturando;
volatile bool disparo;
volatile uint16_t ocrEscalon; // OCR1A del escalon
volatile int16_t limDisparo;  // cuentas del ADC respecto del cero
volatile int16_t ceroCuentas;
uint16_t sumI, sumV;          // acumuladores (solo en la ISR)
volatile uint16_t tick;       // interrupciones del ADC desde el inicio (50 us cada una)
volatile uint16_t tickEscalon;
// Modo "c" (corriente sostenida): sumas de 32 bits en lugar de los buffers
volatile bool sostener;
volatile uint32_t accI, accV;
volatile uint16_t nAcc;
volatile uint8_t nSobre;      // muestras seguidas sobre el limite (modo c)
volatile int16_t picoCuentas; // mayor |I| leida (modo c), en cuentas
const float I_MAX_SOSTENIDA = 2.8f;    // A: el multimetro tiene fusible de 3 A

#if USAR_TACOMETRO
const uint8_t PIN_TACO = 4;            // PD4
const uint8_t RANURAS_TACO = 1;        // marcas del disco por vuelta (solo para el rechazo)
const uint16_t TACO_RPM_MAX = 15000;
// flancos mas cercanos que esto (en ticks de 50 us) se toman como rebote
const uint16_t TACO_MIN_TICKS = (uint16_t)(60.0 / (TACO_RPM_MAX * (float)RANURAS_TACO) / 50e-6);
#if USAR_AS5600
const uint8_t N_FLANCOS = 64;
#else
const uint8_t N_FLANCOS = 96;
#endif
uint16_t flancos[N_FLANCOS];
volatile uint8_t nFlancos;
volatile uint16_t rechazados;
volatile uint16_t tickUltimo;
volatile uint8_t tacoPrev;
#endif

float cero = 511.5f;          // cuentas del ACS712 con 0 A
float kV = KV_DEF;            // V por cuenta en A1
float dutyCap = 0;
uint8_t decimCap = 1;

// El flag de Compare Match B tiene que limpiarse para que haya un nuevo disparo.
EMPTY_INTERRUPT(TIMER1_COMPB_vect);

void driverApagado() {
  digitalWrite(PIN_EN, LOW);
  TCCR1A &= ~_BV(COM1A1);      // desconecta el PWM del pin 9
  digitalWrite(PIN_RPWM, LOW);
}

void configurarTimer1() {
  // Fast PWM modo 14, TOP = ICR1, sin prescaler. COM1A se conecta en el escalon.
  TCCR1A = _BV(WGM11);
  TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10);
  ICR1 = PWM_TOP;
  OCR1A = 0;
  OCR1B = PWM_TOP / 2;
}

void setup() {
  pinMode(PIN_EN, OUTPUT);
  digitalWrite(PIN_EN, LOW);
  pinMode(PIN_RPWM, OUTPUT);
  pinMode(PIN_LPWM, OUTPUT);
#if USAR_TACOMETRO
  pinMode(PIN_TACO, INPUT_PULLUP);   // sin sensor conectado lee alto: no hay flancos
#endif
  digitalWrite(PIN_RPWM, LOW);
  digitalWrite(PIN_LPWM, LOW);
  configurarTimer1();
  driverApagado();

  uint32_t marca;
  EEPROM.get(DIR_EEPROM, marca);
  if (marca == MARCA) {
    float k;
    EEPROM.get(DIR_EEPROM + 4, k);
    if (k > 0.5f * KV_DEF && k < 1.5f * KV_DEF) kV = k;
  }
#if USAR_AS5600
  configurarAS5600();
#endif
  Serial.begin(115200);
}

// Lectura lenta con analogRead (fuera de la captura)
float promedio(uint8_t pin, uint16_t n) {
  uint32_t s = 0;
  for (uint16_t k = 0; k < n; k++) s += analogRead(pin);
  return (float)s / n;
}

ISR(ADC_vect) {
  uint16_t x = ADC;
#if USAR_TACOMETRO
  uint8_t p = PIND & _BV(4);
  if (!p && tacoPrev) {                         // flanco de bajada
    if (nFlancos == 0 || (uint16_t)(tick - tickUltimo) >= TACO_MIN_TICKS) {
      if (nFlancos < N_FLANCOS) flancos[nFlancos++] = tick;
      tickUltimo = tick;
    } else {
      rechazados++;
    }
  }
  tacoPrev = p;
#endif
  tick++;
  if (!canalV) {
    // corriente
    int16_t d = (int16_t)x - ceroCuentas;
    if (sostener) {
      // modo c: corte con 3 muestras seguidas (300 us) sobre el limite, para que un
      // pico de conmutacion aislado no lo dispare (tau electrico ~0,7 ms)
      int16_t a = d < 0 ? -d : d;
      if (a > picoCuentas) picoCuentas = a;
      if (a > limDisparo) {
        if (++nSobre >= 3) { digitalWrite(PIN_EN, LOW); disparo = true; }
      } else {
        nSobre = 0;
      }
    } else if (d > limDisparo || d < -limDisparo) {
      digitalWrite(PIN_EN, LOW);
      disparo = true;
    }
    if (sostener) accI += x; else sumI += x;
    ADMUX = _BV(REFS0) | MUX_V;   // la proxima conversion es tension
    canalV = true;
    return;
  }
  // tension
  ADMUX = _BV(REFS0) | MUX_I;
  canalV = false;
  if (sostener) { accV += x; nAcc++; return; }
  sumV += x;
  if (++cuenta < decim) return;
  bufI[idx] = sumI;
  bufV[idx] = sumV;
  sumI = 0; sumV = 0; cuenta = 0;
  idx++;
  if (idx == N_PRE && !disparo) {
    // escalon: duty en el pin 9 y disparo del ADC en la mitad del encendido
    OCR1A = ocrEscalon;
    OCR1B = ocrEscalon / 2;
    TCCR1A |= _BV(COM1A1);
    digitalWrite(PIN_EN, HIGH);
    tickEscalon = tick;
  }
  if (idx >= N_MUESTRAS) {
    digitalWrite(PIN_EN, LOW);
    ADCSRA &= ~(_BV(ADATE) | _BV(ADIE));
    TIMSK1 &= ~_BV(OCIE1B);
    capturando = false;
  }
}

void capturar(float duty, uint8_t n, int rep, int reps) {
  uint8_t adcsraPrev = ADCSRA, adcsrbPrev = ADCSRB;
  dutyCap = duty;
  decimCap = n;

  // En fast PWM la salida esta en alto OCR1A + 1 ciclos de los TOP + 1 del periodo.
  int32_t o = (int32_t)(duty / 100.0f * (PWM_TOP + 1) + 0.5f) - 1;
  if (o > PWM_TOP) o = PWM_TOP;       // OCR1A = TOP -> salida siempre alta (100 %)
  if (o < 2) o = 2;
  uint16_t ocr = (uint16_t)o;

  noInterrupts();
  idx = 0; cuenta = 0; decim = n; canalV = false; disparo = false;
  sumI = 0; sumV = 0;
  tick = 0; tickEscalon = 0;
#if USAR_TACOMETRO
  nFlancos = 0; rechazados = 0; tickUltimo = 0;
  tacoPrev = PIND & _BV(4);
#endif
  ocrEscalon = ocr;
  ceroCuentas = (int16_t)(cero + 0.5f);
  limDisparo = (int16_t)(I_DISPARO * SENS_V_A / V_LSB);
  OCR1B = PWM_TOP / 2;
  TCNT1 = 0;
  ADMUX = _BV(REFS0) | MUX_I;
  // ADC: disparo por Timer1 Compare Match B (ADTS = 0101), prescaler 32 (500 kHz,
  // 26 us por conversion: entra en el periodo de 50 us del PWM)
  ADCSRB = (ADCSRB & ~0x0F) | 0x05;
  ADCSRA = _BV(ADEN) | _BV(ADATE) | _BV(ADIE) | _BV(ADPS2) | _BV(ADPS0);
  TIFR1 = _BV(OCF1B);
  TIMSK1 |= _BV(OCIE1B);
  capturando = true;
  interrupts();

  uint32_t t0 = millis();
  uint32_t limite = (uint32_t)N_MUESTRAS * n / 10 + 500;   // ms
#if USAR_AS5600
  for (uint16_t k = 0; k < N_MUESTRAS; k++) bufA[k] = 65535;
  while (capturando && millis() - t0 < limite) {
    uint16_t a = leer16(AS5600_RAW);
    noInterrupts();
    uint16_t k = idx;             // muestra en curso al terminar la lectura
    interrupts();
    if (k < N_MUESTRAS && a != 65535) bufA[k] = a & 0x0FFF;
  }
#else
  while (capturando && millis() - t0 < limite) { }
#endif

  noInterrupts();
  driverApagado();
  ADCSRA = adcsraPrev & ~(_BV(ADATE) | _BV(ADIE) | _BV(ADIF));
  ADCSRB = adcsrbPrev;
  TIMSK1 &= ~_BV(OCIE1B);
  OCR1B = PWM_TOP / 2;
  bool completo = !capturando;
  capturando = false;
  interrupts();

  // Salida CSV
  float dt = n * 1e-4f;
  float dutyReal = (ocr >= PWM_TOP) ? 100.0f : 100.0f * (ocr + 1) / (PWM_TOP + 1);
  Serial.print(F("# captura rep=")); Serial.print(rep);
  Serial.print('/'); Serial.print(reps);
  Serial.print(F(" duty_pct=")); Serial.print(dutyReal, 2);
  Serial.print(F(" N=")); Serial.print(n);
  Serial.print(F(" dt_s=")); Serial.print(dt, 6);
  Serial.print(F(" pre=")); Serial.print(N_PRE);
  Serial.print(F(" cero=")); Serial.print(cero, 2);
  Serial.print(F(" kV=")); Serial.print(kV, 6);
  Serial.print(F(" disparo=")); Serial.print(disparo ? 1 : 0);
#if USAR_TACOMETRO
  Serial.print(F(" flancos=")); Serial.print(nFlancos);
  Serial.print(F(" rechazados=")); Serial.print(rechazados);
#endif
  Serial.print(F(" completa=")); Serial.println(completo ? 1 : 0);
#if USAR_AS5600
  Serial.println(F("t_s,i_A,vbus_V,v_motor_V,ang_cuentas"));
#else
  Serial.println(F("t_s,i_A,vbus_V,v_motor_V"));
#endif
  uint16_t total = completo ? N_MUESTRAS : idx;
  for (uint16_t k = 0; k < total; k++) {
    float ci = (float)bufI[k] / n;
    float cv = (float)bufV[k] / n;
    float i = (ci - cero) * V_LSB / SENS_V_A;
    float vb = cv * kV;
    float vm = (k >= N_PRE) ? vb * dutyReal / 100.0f : 0.0f;
    // tiempo del centro de los N pares promediados (t = 0: primer periodo del escalon)
    float t = (float)((int16_t)k - (int16_t)N_PRE) * dt + (n - 1) * 0.5e-4f;
    Serial.print(t, 5); Serial.print(',');
    Serial.print(i, 3); Serial.print(',');
    Serial.print(vb, 3); Serial.print(',');
#if USAR_AS5600
    Serial.print(vm, 3); Serial.print(',');
    Serial.println(bufA[k]);
#else
    Serial.println(vm, 3);
#endif
  }
#if USAR_TACOMETRO
  // Flancos del tacometro: tiempo respecto del escalon (50 us por tick)
  for (uint8_t k = 0; k < nFlancos; k++) {
    Serial.print(F("F,"));
    Serial.println((float)((int32_t)flancos[k] - (int32_t)tickEscalon) * 50e-6f, 5);
  }
#endif
  Serial.println(F("FIN"));
}

// c D T: duty D fijo durante T segundos (eje trabado) y cada 0,5 s informa la
// corriente media leida por el ACS712 en la mitad del encendido (= valor medio
// de la corriente del motor) y la tension de B+. Sirve para comparar el ACS712
// con un multimetro en serie con el motor. Corte a I_MAX_SOSTENIDA.
void sostenida(float duty, float segundos, float iMax) {
  uint8_t adcsraPrev = ADCSRA, adcsrbPrev = ADCSRB;
  int32_t o = (int32_t)(duty / 100.0f * (PWM_TOP + 1) + 0.5f) - 1;
  if (o < 2) o = 2;
  uint16_t ocr = (uint16_t)o;
  float dutyReal = 100.0f * (ocr + 1) / (PWM_TOP + 1);

  noInterrupts();
  canalV = false; disparo = false; sostener = true;
  accI = 0; accV = 0; nAcc = 0; nSobre = 0; picoCuentas = 0;
  ceroCuentas = (int16_t)(cero + 0.5f);
  limDisparo = (int16_t)(iMax * SENS_V_A / V_LSB);
  TCNT1 = 0;
  OCR1A = ocr;
  OCR1B = ocr / 2;               // ADC en la mitad del encendido
  ADMUX = _BV(REFS0) | MUX_I;
  ADCSRB = (ADCSRB & ~0x0F) | 0x05;
  ADCSRA = _BV(ADEN) | _BV(ADATE) | _BV(ADIE) | _BV(ADPS2) | _BV(ADPS0);
  TIFR1 = _BV(OCF1B);
  TIMSK1 |= _BV(OCIE1B);
  TCCR1A |= _BV(COM1A1);
  digitalWrite(PIN_EN, HIGH);
  interrupts();

  Serial.print(F("# sostenida duty_pct=")); Serial.print(dutyReal, 2);
  Serial.print(F(" T_s=")); Serial.println(segundos, 1);
  Serial.println(F("t_s,i_ACS_A,vbus_V"));
  uint32_t t0 = millis(), tBloque = t0;
  uint32_t durMs = (uint32_t)(segundos * 1000.0f);
  float sI2 = 0, sV2 = 0; uint32_t n2 = 0;   // segunda mitad: promedio final
  while (millis() - t0 < durMs && !disparo) {
    if (millis() - tBloque < 500) continue;
    tBloque += 500;
    noInterrupts();
    uint32_t aI = accI, aV = accV; uint16_t na = nAcc;
    accI = 0; accV = 0; nAcc = 0;
    interrupts();
    if (na == 0) continue;
    float i = ((float)aI / na - cero) * V_LSB / SENS_V_A;
    float vb = (float)aV / na * kV;
    Serial.print((millis() - t0) * 1e-3f, 1); Serial.print(',');
    Serial.print(i, 3); Serial.print(',');
    Serial.println(vb, 2);
    if (millis() - t0 > durMs / 2) { sI2 += (float)aI; sV2 += (float)aV; n2 += na; }
  }

  noInterrupts();
  sostener = false;
  driverApagado();
  ADCSRA = adcsraPrev & ~(_BV(ADATE) | _BV(ADIE) | _BV(ADIF));
  ADCSRB = adcsrbPrev;
  TIMSK1 &= ~_BV(OCIE1B);
  OCR1B = PWM_TOP / 2;
  interrupts();

  if (disparo) {
    Serial.print(F("# CORTE: |I| > ")); Serial.print(iMax, 1);
    Serial.println(F(" A durante 300 us (bajar el duty)"));
  }
  Serial.print(F("# pico de una muestra: ")); Serial.print(picoCuentas * V_LSB / SENS_V_A, 2);
  Serial.println(F(" A"));
  if (!disparo && n2 > 0) {
    float i = (sI2 / n2 - cero) * V_LSB / SENS_V_A;
    Serial.print(F("# PROMEDIO 2a mitad: I_ACS=")); Serial.print(i, 3);
    Serial.print(F(" A  Vbus=")); Serial.print(sV2 / n2 * kV, 2);
    Serial.print(F(" V  v_motor~")); Serial.print(sV2 / n2 * kV * dutyReal / 100.0f, 2);
    Serial.println(F(" V"));
  }
  Serial.println(F("LISTO"));
}

void ayuda() {
  Serial.println(F("# captura_escalon: z (cero ACS), u <V> (calibrar B+), e <duty%> <N>, c <duty%> <s>, ?"));
  Serial.print(F("# cero=")); Serial.print(cero, 2);
  Serial.print(F(" kV=")); Serial.print(kV, 6);
  Serial.print(F(" Vbus=")); Serial.print(promedio(A1, 200) * kV, 2);
  Serial.print(F(" I=")); Serial.println((promedio(A0, 200) - cero) * V_LSB / SENS_V_A, 3);
#if USAR_AS5600
  // STATUS: bit 5 MD = iman detectado, bit 4 ML = campo debil, bit 3 MH = campo fuerte
  uint8_t st = leerEstado();
  Serial.print(F("# AS5600: angulo=")); Serial.print(leer16(AS5600_RAW) & 0x0FFF);
  Serial.print(F(" conf=0x")); Serial.print(leer16(AS5600_CONF), HEX);
  if (st == 0xFF) Serial.println(F(" SIN RESPUESTA (revisar SDA/SCL/VCC)"));
  else {
    Serial.print(F(" iman=")); Serial.print((st & 0x20) ? F("si") : F("NO"));
    if (st & 0x10) Serial.print(F(" (debil: acercar)"));
    if (st & 0x08) Serial.print(F(" (fuerte: alejar)"));
    Serial.println();
  }
#endif
}

void loop() {
  if (!Serial.available()) return;
  String linea = Serial.readStringUntil('\n');
  linea.trim();
  if (linea.length() == 0) return;
  char c = linea.charAt(0);
  String resto = linea.substring(1);
  resto.trim();

  if (c == 'z') {
    driverApagado();
    delay(200);
    cero = promedio(A0, 4000);
    Serial.print(F("# cero=")); Serial.print(cero, 2);
    // Sin corriente el ACS712 da VCC/2 (~512 cuentas). Lejos de eso: sin alimentacion
    // o sin el cable OUT en A0.
    if (cero < 410.0f || cero > 615.0f)
      Serial.println(F(" ERROR: el ACS712 no da ~2,5 V en A0 (revisar VCC, GND y OUT)"));
    else
      Serial.println();
  } else if (c == 'u') {
    float vreal = resto.toFloat();
    float cuentas = promedio(A1, 2000);
    if (vreal > 1.0f && cuentas > 10.0f) {
      kV = vreal / cuentas;
      EEPROM.put(DIR_EEPROM, MARCA);
      EEPROM.put(DIR_EEPROM + 4, kV);
      Serial.print(F("# kV=")); Serial.println(kV, 6);
    } else {
      Serial.println(F("# ERROR: tension invalida o B+ sin tension"));
    }
  } else if (c == 'e') {
    // e D N M E: duty, decimacion, repeticiones y espera entre escalones [s]
    float val[4] = {0, 1, 2, 3};
    uint8_t nv = 0;
    while (resto.length() > 0 && nv < 4) {
      int esp = resto.indexOf(' ');
      String tok = (esp < 0) ? resto : resto.substring(0, esp);
      val[nv++] = tok.toFloat();
      resto = (esp < 0) ? String() : resto.substring(esp + 1);
      resto.trim();
    }
    float d = val[0];
    int n = (int)val[1], m = (int)val[2];
    float espera = val[3];
    if (d < 1.0f || d > 100.0f || n < 1 || n > 50 || m < 1 || m > 100 ||
        espera < 0.5f || espera > 30.0f) {
      Serial.println(F("# ERROR: usar e <duty 1..100> <N 1..50> <reps 1..100> <espera 0.5..30 s>"));
      return;
    }
    for (int r = 1; r <= m; r++) {
      uint32_t t0 = millis();
      capturar(d, (uint8_t)n, r, m);
      if (disparo) break;                       // no repetir si salto el corte
      if (r < m) {
        // espera a que el motor se detenga (incluye el tiempo de envio)
        while (millis() - t0 < (uint32_t)(espera * 1000.0f) + (uint32_t)N_MUESTRAS * n / 10) { }
      }
    }
    Serial.println(F("LISTO"));
  } else if (c == 'c') {
    // c D T I: duty D (1 a 60 %) sostenido T segundos (1 a 15), corte en I amperes
    // (0,5 a 12; por defecto 2,8 por el fusible del multimetro)
    float val[3] = {0, 5, I_MAX_SOSTENIDA};
    uint8_t nv = 0;
    while (resto.length() > 0 && nv < 3) {
      int esp = resto.indexOf(' ');
      val[nv++] = ((esp < 0) ? resto : resto.substring(0, esp)).toFloat();
      resto = (esp < 0) ? String() : resto.substring(esp + 1);
      resto.trim();
    }
    if (val[0] < 1.0f || val[0] > 60.0f || val[1] < 1.0f || val[1] > 15.0f ||
        val[2] < 0.5f || val[2] > 12.0f) {
      Serial.println(F("# ERROR: usar c <duty 1..60> <segundos 1..15> [corte 0.5..12 A]"));
      return;
    }
    sostenida(val[0], val[1], val[2]);
  } else if (c == '?') {
    ayuda();
  } else {
    Serial.println(F("# comando desconocido (?: ayuda)"));
  }
}
