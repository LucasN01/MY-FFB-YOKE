// Ensayo 1 de la caracterizacion del motor 775 (Etapa 1, motor solo):
// rotor bloqueado contra la celda para medir R (resistencia del bobinado),
// Vesc (caida en escobillas) y Kt (constante de torque).
// Solo para ensayos en banco: no forma parte del firmware del yoke.
//
// Por que hace falta este programa: el procedimiento pide una fuente regulable
// en modo limite de corriente (1, 2, 3, 4 y 5 A). Con una fuente FIJA de 24 V
// y el eje trabado, conectar el motor directo daria unos 45 A. Aca el limite
// lo hace el Pro Micro: regula la corriente con el PWM del BTS7960 y la mide
// con el ACS712, igual que haria la fuente en modo CC.
//
// Placa: Arduino Pro Micro 5 V / 16 MHz (ATmega32U4). En el IDE elegir
// "Arduino Leonardo" (o "SparkFun Pro Micro 5V/16MHz" si esta instalado).
// No usar la version de 3,3 V / 8 MHz: el ACS712 da 0-5 V.
//
// Conexiones:
//   BTS7960 (modulo IBT-2) -> Pro Micro
//     VCC -> VCC (5 V)      GND -> GND
//     RPWM -> pin 9         LPWM -> pin 10
//     R_EN y L_EN (unidos) -> pin 7, con 10 k de pin 7 a GND (asi el driver
//                             queda apagado mientras el Pro Micro arranca)
//     R_IS, L_IS -> sin conectar
//   BTS7960 potencia: B+ -> +24 V con FUSIBLE de 10 A en serie, B- -> -24 V
//                     M+ y M- -> motor, con el ACS712 en serie en uno de los dos
//   ACS712 (version 20 A, 100 mV/A) -> Pro Micro
//     VCC -> VCC (5 V)      GND -> GND      OUT -> A0
//   Masa comun: GND del Pro Micro unido al GND del BTS7960 (ya lo hace el modulo).
//   Multimetro en VCC (tension continua) directo en los bornes del motor.
//   La celda y su Arduino UNO quedan como estan (celda_hx711.ino).
//
// Orden de encendido: primero el USB del Pro Micro (driver apagado), despues
// la fuente de 24 V. Para desconectar: primero la fuente.
//
// Salida: monitor serie a 115200 (con "nueva linea"), CSV cada 0,1 s durante
// un punto y cada 1 s en reposo:
//   tiempo_ms,estado,i_obj_A,i_med_A,duty_pct,v_est_V,t_punto_s  Imed="i_med_A"
//   ejemplo: 562060,R,0.00,0.05,0.00,0.00,0.0  Imed="0.05"
//   (Imed repite i_med_A a pedido del usuario, para leerla de un vistazo)
// estado: R reposo, S subiendo la corriente, M midiendo (corriente estable),
//         P pausa obligatoria entre puntos, V modo manual de PWM,
//         F falla (driver apagado).
// i_med_A es el promedio del ACS712 en los ultimos 0,1 s. v_est_V = duty*Vfuente
// es solo una estimacion; la tension que vale es la del multimetro en bornes.
// Las lineas que empiezan con # son mensajes (no son datos).
//
// Comandos (terminar con "nueva linea"):
//   i 2       hace un punto a 2 A en sentido +; "i -2" en sentido contrario.
//             Sube en rampa, se mantiene hasta completar el tiempo del punto
//             y apaga solo. Al terminar informa la corriente media del tramo
//             estable (estado M).
//   v 123     MODO MANUAL de PWM, para probar el driver con el motor SIN CARGA
//             (eje libre, sin brazo). Valor de 0 a 255 como analogWrite
//             (255 = 100 % = tension de la fuente); "v -123" gira al reves.
//             El PWM sube y baja en rampa (RAMPA_MANUAL). Se puede cambiar el
//             valor mientras gira; "v 0" baja a cero y sale del modo manual.
//             Si el motor tiene carga o el eje esta trabado, la corriente sube
//             mucho mas que la de vacio: pasa a falla y apaga el driver (ver
//             "Proteccion del modo manual" abajo).
//   s         para ya y apaga el driver (en modo manual el motor queda libre)
//   z         cero del ACS712 (con el driver apagado, sin corriente)
//   t 5       tiempo maximo de cada punto en s (1 a 8; el procedimiento pide <= 5)
//   e 10      pausa minima entre puntos en s (0 a 120; el procedimiento pide 10-20)
//   f 24.1    tension real de la fuente medida con el multimetro (para v_est)
//   m 8       zona muerta del driver en % de duty (se guarda). Medirla en modo
//             manual con el motor libre: con "v 60" (23,5 %) y "v 120" (47,1 %)
//             leer el multimetro en bornes y calcular en cada uno
//             m = duty% - 100 * V_multimetro / Vfuente; usar el promedio.
//             Ej.: 23,5 % y 3,6 V a 24 V -> m = 23,5 - 15,0 = 8,5.
//   (antes de cada punto y de cada arranque manual se hace el cero solo)
//   k 100     sensibilidad del ACS712 en mV/A (se guarda en la EEPROM)
//   x         invierte el signo de la lectura del ACS712 (se guarda)
//   r         rearma despues de una falla
//   ?         estado y parametros
//
// Uso en cada punto (procedimiento 6.1 del documento):
//   1) Con "i 0.5" verificar primero que el motor empuje el brazo CONTRA la
//      celda. Si lo aleja, invertir los cables del motor (o usar "i -0.5").
//   2) Tarar la celda en el UNO ("t") con el brazo apenas apoyado.
//   3) "i 1": cuando el estado pasa a M, esperar 2-3 s y anotar V (multimetro)
//      y F (celda). La corriente la informa el resumen "# fin de punto".
//   4) Repetir con 2, 3, 4 y 5 A. La pausa entre puntos la impone el programa.
//
// Protecciones (cualquiera apaga el driver y pasa a F hasta enviar "r"):
//   - corriente instantanea mayor que I_DISPARO (3 muestras seguidas, ~0,3 ms);
//   - duty en su limite sin alcanzar la corriente pedida durante 0,2 s
//     (rotor que se libero y gira, R mucho mayor que la esperada o cable cortado);
//   - duty mayor que DUTY_CHEQUEO sin que el sensor vea corriente en el sentido
//     pedido (ACS712 desconectado o con el signo invertido: comando "x");
//   - fin del tiempo del punto (no es falla: apaga y pasa a P).
// El duty tiene un techo que depende de la corriente pedida, para que si el
// rotor se libera no pueda girar rapido: V_max = V_LIM_BASE + R_LIM * |I|,
// y nunca mas que DUTY_MAX * Vfuente.
//
// Proteccion del modo manual (motor sin carga):
//   Un 775 libre consume poca corriente (vacio ~1 A a 24 V segun el vendedor).
//   Con el eje trabado la corriente es (V - Vesc)/R: con R ~0,5 ohm supera
//   I_LIBRE_MAX con solo ~2,5 V aplicados. Como el PWM sube en rampa desde 0,
//   un eje trabado se detecta al principio de la rampa, con poca tension:
//   el motor llega a recibir ~3-4 A durante menos de medio segundo y se corta.
//   No se puede detectar la carga sin energizar nada, pero si con un pulso
//   corto y de baja tension como este.
//   - corriente filtrada mayor que I_LIBRE_MAX durante T_ALTA_MS -> falla;
//   - corriente instantanea mayor que I_DISPARO_MANUAL -> falla (por si el
//     motor se frena de golpe girando rapido);
//   - duty mayor que DUTY_CHEQUEO_MANUAL sin corriente -> falla (sin sensor la
//     proteccion anterior quedaria ciega).
//
// MODO SOSTENIDO (Ensayo 6, limite termico): "c 2" mantiene 2 A con el rotor
// trabado hasta que llegue "s", sin el tope de t (lo maneja registrar_ensayo.py
// con el comando "termico"). Igual que un punto "i", con estas diferencias:
//   - la corriente se mantiene sin cortes hasta "s" (el cero del ACS712 se
//     hace solo al arrancar); tope de seguridad T_SOST_MAX_MS (60 min);
//   - VIGILANCIA: la PC tiene que mandar "w" al menos cada T_VIGILANCIA_MS
//     (10 s). Si el script se cuelga o se desconecta el USB, se apaga solo;
//   - el cero puede derivar en un ensayo largo: tras "s", con el driver
//     apagado, la corriente que marca la linea es esa deriva;
//   - techo de duty: al arrancar hasta DUTY_MAX_SOST (la R sube ~30-40 % al
//     calentarse); 2 s despues de estabilizar baja a duty estable + MARGEN_SOST,
//     asi que si el rotor se libera sigue sin poder girar rapido.
//   w         senal de vida de la PC (solo en modo sostenido; no responde)

#include <EEPROM.h>

// ---------------------------------------------------------------- pines
const uint8_t PIN_RPWM = 9;   // OC1A del Timer1
const uint8_t PIN_LPWM = 10;  // OC1B del Timer1
const uint8_t PIN_EN = 7;     // R_EN y L_EN del BTS7960
const uint8_t PIN_ACS = A0;

// ---------------------------------------------------------------- PWM
// Timer1 en fast PWM con TOP = ICR1: f = 16 MHz / (TOP + 1) = 20 kHz
// (el BTS7960 admite hasta 25 kHz). 800 pasos de duty: 0,03 V por paso a 24 V.
const uint16_t PWM_TOP = 799;

// ---------------------------------------------------------------- limites
const float VFUENTE_DEF = 24.0f;  // V, se corrige con el comando v
const float I_CMD_MAX = 6.0f;     // A, mayor corriente que se acepta pedir
const float I_DISPARO = 7.5f;     // A, corte inmediato
const float DUTY_MAX = 0.20f;     // techo absoluto: 4,8 V a 24 V
const float V_LIM_BASE = 2.0f;    // V, techo de tension = V_LIM_BASE + R_LIM*|I|
const float R_LIM = 1.0f;         // ohm (cubre R hasta ~1 ohm y Vesc hasta ~1 V)
const float DUTY_CHEQUEO = 0.08f; // con mas duty que esto tiene que haber corriente
const float I_MIN_CHEQUEO = 0.3f; // A, corriente minima esperada en ese caso
const uint16_t T_SATURADO_MS = 200;
// 400 ms (antes 100): con 100 ms el motor de roll fallaba en sentido
// antihorario (17/11/2026). Ese sentido empieza a conducir ~2,5 % de duty mas
// tarde (~0,6 V) y a los 100 ms de pasar el umbral iba por 0,24 A (< 0,3 A).
// Si el ACS712 realmente no mide, el duty queda frenado en el techo y ademas
// actua T_SATURADO_MS, asi que la proteccion se mantiene.
const uint16_t T_SIN_CORRIENTE_MS = 400;

// ---------------------------------------------------------------- modo manual
const float RAMPA_MANUAL = 0.25f;         // duty por segundo: de 0 a 100 % en 4 s
const float I_LIBRE_MAX = 3.0f;           // A, mas que esto sin carga = eje trabado
const uint16_t T_ALTA_MS = 100;
const float TAU_FILTRO_S = 0.02f;         // filtro de la corriente para el chequeo
const float I_DISPARO_MANUAL = 5.0f;      // A, corte instantaneo en modo manual
const float DUTY_CHEQUEO_MANUAL = 0.15f;  // con mas duty tiene que haber corriente
const float I_MIN_MANUAL = 0.08f;         // A

// ---------------------------------------------------------------- control
// Lazo de corriente integral puro a 500 Hz. Planta con rotor bloqueado:
// I = (duty*Vfuente - Vesc)/R, ganancia ~48 A por unidad de duty con R = 0,5 ohm,
// constante electrica L/R ~ 1 ms. Con KI = 2,5 el cruce queda en ~19 Hz
// (mucho mas lento que la planta y que el retardo de medicion de 2 ms).
const uint16_t DT_US = 2000;
const float DT_S = DT_US * 1e-6f;
const float KI = 2.5f;            // duty por (A*s)
const float RAMPA_A_S = 5.0f;     // subida de la corriente pedida
// Subida maxima del duty en un punto: 25 %/s = 6 V/s a 24 V. Seguir la rampa
// de 5 A/s con R = 0,55 ohm pide ~11 %/s, asi que en uso normal no limita.
// Sirve si el ACS712 deja de medir (lee 0 o con el signo al reves): el
// integrador pediria duty sin freno; asi, cuando actua el chequeo de "sin
// corriente" (duty > 8 % durante 0,4 s) el duty llego como mucho a ~18 % o al
// techo de la corriente pedida, que casi siempre es menor, en lugar de saltar
// de golpe al techo absoluto.
const float DUTY_SUBIDA_MAX = 0.25f;  // duty por segundo
const uint16_t T_ESTABLE_MS = 300;
const float TOL_ESTABLE_A = 0.05f;
const float TOL_ESTABLE_REL = 0.03f;

// ---------------------------------------------------------------- tiempos
const float T_PUNTO_DEF = 5.0f;   // s
const float T_PUNTO_MAX = 8.0f;
const float PAUSA_DEF = 10.0f;    // s
const uint16_t T_LINEA_PUNTO_MS = 100;
const uint16_t T_LINEA_REPOSO_MS = 1000;

// ---------------------------------------------------------------- modo sostenido
const uint32_t T_SOST_MAX_MS = 60UL * 60UL * 1000UL;
const uint16_t T_VIGILANCIA_MS = 10000;
const uint32_t T_AJUSTE_TECHO_MS = 2000;
const float DUTY_MAX_SOST = 0.30f;  // techo absoluto en tension real: 7,2 V a 24 V
const float MARGEN_SOST = 0.06f;    // ~1,4 V sobre el duty estable

// ---------------------------------------------------------------- ACS712
// Lectura ratiometrica: el ACS712 y la referencia del ADC salen del mismo 5 V,
// asi que las cuentas por amper no dependen de la tension real del USB.
const float SENS_DEF_MV_A = 100.0f;  // version de 20 A (30 A = 66, 5 A = 185)
const uint16_t MUESTRAS_CERO = 4000;

const int DIR_EEPROM = 0;
const uint32_t MARCA = 0x41435331UL;  // "ACS1"
struct Guardado {
  uint32_t marca;
  float sens;
  int8_t signo;
  float muerto;  // zona muerta del driver, fraccion de duty
};

// Zona muerta del BTS7960: a 20 kHz los pulsos muy cortos no llegan al motor
// (retardos de conmutacion del driver), asi que por debajo de cierto duty la
// tension real en bornes es ~0. Se mide en modo manual con el motor libre:
//   m = duty - V_multimetro / Vfuente   (ver comando m).
// Se suma a los techos de duty y a los umbrales de los chequeos, que estan
// pensados en tension real. Por defecto 0 (lo mas seguro); maximo 12 %.
const float MUERTO_MAX = 0.12f;

// ---------------------------------------------------------------- estado
enum Estado : char { REPOSO = 'R', SUBIENDO = 'S', MIDIENDO = 'M', PAUSA = 'P', MANUAL = 'V', FALLA = 'F' };
Estado estado = REPOSO;

bool enPunto() { return estado == SUBIENDO || estado == MIDIENDO; }
bool motorActivo() { return enPunto() || estado == MANUAL; }

// modo manual: duty con signo (+ RPWM, - LPWM)
float dutyManual = 0.0f;
float objetivoManual = 0.0f;
float iFiltrada = 0.0f;
uint16_t msAlta = 0;
int limiteCuentasManual = 0;

float vFuente = VFUENTE_DEF;
float dutyMuerto = 0.0f;

// Tension media estimada en bornes para un duty con signo.
float vEstimada(float d) {
  float a = fabs(d) - dutyMuerto;
  if (a < 0.0f) a = 0.0f;
  return (d < 0.0f ? -a : a) * vFuente;
}
float tPunto = T_PUNTO_DEF;
float pausa = PAUSA_DEF;
float sens = SENS_DEF_MV_A;
int8_t signoSensor = 1;

float offsetCuentas = 512.0f;
int offsetEntero = 512;
float cuentasPorA = 0.0f;
int limiteCuentas = 0;

int8_t dir = 0;           // +1 RPWM, -1 LPWM
float iObj = 0.0f;        // modulo de la corriente pedida
float iRampa = 0.0f;
float duty = 0.0f;
float dutyLim = 0.0f;
uint32_t inicioPuntoMs = 0;
uint32_t finPuntoMs = 0;
uint16_t msEstable = 0, msSaturado = 0, msSinCorriente = 0;
uint8_t sobreSeguidas = 0;

// modo sostenido
bool sostenido = false;
bool techoAjustado = false;
uint32_t ultimoVivoMs = 0;

// tramo estable (estado M) para el resumen
float sumaI = 0.0f, sumaDuty = 0.0f;
uint32_t pasosM = 0;
uint32_t inicioMms = 0;

// acumuladores
uint32_t sumaRaw = 0;
uint16_t nRaw = 0;
float sumaILinea = 0.0f, sumaDutyLinea = 0.0f;
uint16_t pasosLinea = 0;
float ultimaI = 0.0f;

uint32_t tCtrlUs = 0;
uint32_t tLineaMs = 0;
bool avisoReposo = false;
uint16_t msCorrienteReposo = 0;

char linea[32];
uint8_t largo = 0;

// ================================================================ driver
void pwmSalida(int8_t d, float dc) {
  uint16_t c = (uint16_t)(dc * (PWM_TOP + 1) + 0.5f);
  if (c > PWM_TOP) c = PWM_TOP;
  if (d == 0 || c == 0) {
    TCCR1A = (1 << WGM11);  // salidas desconectadas del timer: quedan en LOW
    OCR1A = 0;
    OCR1B = 0;
  } else if (d > 0) {
    OCR1B = 0;
    OCR1A = c;
    TCCR1A = (1 << WGM11) | (1 << COM1A1);
  } else {
    OCR1A = 0;
    OCR1B = c;
    TCCR1A = (1 << WGM11) | (1 << COM1B1);
  }
}

void driverApagado() {
  digitalWrite(PIN_EN, LOW);
  pwmSalida(0, 0.0f);
  duty = 0.0f;
  dutyManual = 0.0f;
  objetivoManual = 0.0f;
}

void configurarPwm() {
  digitalWrite(PIN_RPWM, LOW);
  digitalWrite(PIN_LPWM, LOW);
  pinMode(PIN_RPWM, OUTPUT);
  pinMode(PIN_LPWM, OUTPUT);
  TCCR1B = 0;
  TCCR1A = (1 << WGM11);
  ICR1 = PWM_TOP;
  OCR1A = 0;
  OCR1B = 0;
  TCNT1 = 0;
  TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10);  // modo 14, sin prescaler
}

// ================================================================ sensor
void actualizarEscala() {
  cuentasPorA = sens / 5000.0f * 1024.0f;
  limiteCuentas = (int)(I_DISPARO * cuentasPorA + 0.5f);
  limiteCuentasManual = (int)(I_DISPARO_MANUAL * cuentasPorA + 0.5f);
}

void cargarEeprom() {
  Guardado g;
  EEPROM.get(DIR_EEPROM, g);
  if (g.marca == MARCA && g.sens > 10.0f && g.sens < 400.0f && (g.signo == 1 || g.signo == -1)) {
    sens = g.sens;
    signoSensor = g.signo;
    // EEPROM de una version anterior (sin este campo): queda en 0.
    if (g.muerto >= 0.0f && g.muerto <= MUERTO_MAX) dutyMuerto = g.muerto;
  }
  actualizarEscala();
}

void guardarEeprom() {
  Guardado g = {MARCA, sens, signoSensor, dutyMuerto};
  EEPROM.put(DIR_EEPROM, g);
}

void hacerCero() {
  uint32_t s = 0;
  for (uint16_t k = 0; k < MUESTRAS_CERO; k++) s += analogRead(PIN_ACS);
  offsetCuentas = (float)s / MUESTRAS_CERO;
  offsetEntero = (int)(offsetCuentas + 0.5f);
  Serial.print(F("# cero ACS712: "));
  Serial.print(offsetCuentas, 1);
  Serial.print(F(" cuentas ("));
  Serial.print(offsetCuentas * 5.0f / 1024.0f, 3);
  Serial.println(F(" V)"));
  if (offsetCuentas < 462.0f || offsetCuentas > 562.0f) {
    Serial.println(F("# aviso: el ACS712 deberia dar ~2,5 V sin corriente; revisar VCC, GND y OUT -> A0"));
  }
}

// ================================================================ mensajes
void informarEstado() {
  Serial.print(F("# estado="));
  Serial.print((char)estado);
  Serial.print(F(" Vfuente="));
  Serial.print(vFuente, 2);
  Serial.print(F(" V, t_punto="));
  Serial.print(tPunto, 1);
  Serial.print(F(" s, pausa="));
  Serial.print(pausa, 1);
  Serial.println(F(" s"));
  Serial.print(F("# ACS712: "));
  Serial.print(sens, 2);
  Serial.print(F(" mV/A, signo "));
  Serial.print(signoSensor > 0 ? '+' : '-');
  Serial.print(F(", cero "));
  Serial.print(offsetCuentas, 1);
  Serial.print(F(" cuentas, resolucion "));
  Serial.print(1000.0f / cuentasPorA, 0);
  Serial.println(F(" mA/cuenta (se promedia)"));
  Serial.print(F("# limites: I pedida <= "));
  Serial.print(I_CMD_MAX, 1);
  Serial.print(F(" A, corte "));
  Serial.print(I_DISPARO, 1);
  Serial.print(F(" A, duty <= "));
  Serial.print((dutyMuerto + DUTY_MAX) * 100.0f, 1);
  Serial.print(F(" %, zona muerta m="));
  Serial.print(dutyMuerto * 100.0f, 1);
  Serial.println(F(" %"));
}

void encabezado() {
  Serial.println(F("tiempo_ms,estado,i_obj_A,i_med_A,duty_pct,v_est_V,t_punto_s"));
}

void falla(const __FlashStringHelper *motivo) {
  driverApagado();
  estado = FALLA;
  Serial.print(F("# FALLA: "));
  Serial.println(motivo);
  Serial.print(F("# corriente medida "));
  Serial.print(ultimaI, 2);
  Serial.println(F(" A. Driver apagado. Revisar y enviar r para rearmar."));
}

void resumenPunto(const __FlashStringHelper *motivo) {
  Serial.print(F("# fin de punto ("));
  Serial.print(motivo);
  Serial.print(F("): I_obj="));
  Serial.print(dir * iObj, 2);
  Serial.print(F(" A"));
  if (pasosM > 0) {
    float iM = sumaI / pasosM;
    float dM = sumaDuty / pasosM;
    Serial.print(F(", I_media="));
    Serial.print(iM, 3);
    Serial.print(F(" A, duty_medio="));
    Serial.print(dM * 100.0f, 2);
    Serial.print(F(" %, V_est="));
    Serial.print(vEstimada(dM), 2);
    Serial.print(F(" V, tramo estable="));
    Serial.print((finPuntoMs - inicioMms) / 1000.0f, 1);
    Serial.println(F(" s"));
    if (finPuntoMs - inicioMms < 2000) {
      Serial.println(F("# aviso: tramo estable menor que 2 s; la celda puede no haberse asentado (subir t)"));
    }
  } else {
    Serial.println(F(", sin tramo estable"));
  }
  Serial.print(F("# pausa de "));
  Serial.print(pausa, 0);
  Serial.println(F(" s antes del proximo punto"));
}

// ================================================================ puntos
void iniciarPunto(float i, bool sost) {
  if (estado == FALLA) {
    Serial.println(F("# en falla: enviar r primero"));
    return;
  }
  if (motorActivo()) {
    Serial.println(F("# el motor esta encendido (punto o modo manual): enviar s primero"));
    return;
  }
  if (estado == PAUSA) {
    Serial.print(F("# pausa: faltan "));
    Serial.print((pausa * 1000.0f - (millis() - finPuntoMs)) / 1000.0f, 1);
    Serial.println(F(" s"));
    return;
  }
  float m = fabs(i);
  if (m < 0.1f || m > I_CMD_MAX) {
    Serial.print(F("# error: usar i <A> con 0,1 <= |A| <= "));
    Serial.println(I_CMD_MAX, 1);
    return;
  }
  dir = (i > 0) ? 1 : -1;
  iObj = m;
  iRampa = 0.0f;
  duty = 0.0f;
  float vLim = V_LIM_BASE + R_LIM * iObj;
  dutyLim = vLim / vFuente;
  float techoAbs = sost ? DUTY_MAX_SOST : DUTY_MAX;
  if (dutyLim > techoAbs) dutyLim = techoAbs;
  dutyLim += dutyMuerto;  // los techos estan en tension real
  sostenido = sost;
  techoAjustado = false;
  msEstable = msSaturado = msSinCorriente = 0;
  sobreSeguidas = 0;
  sumaI = sumaDuty = 0.0f;
  pasosM = 0;
  sumaILinea = sumaDutyLinea = 0.0f;
  pasosLinea = 0;

  Serial.print(sost ? F("# sostenido: I=") : F("# punto: I="));
  Serial.print(dir * iObj, 2);
  Serial.print(F(" A, t="));
  Serial.print(sost ? T_SOST_MAX_MS / 1000.0f : tPunto, 1);
  Serial.print(F(" s, techo de duty "));
  Serial.print(dutyLim * 100.0f, 1);
  Serial.println(F(" %"));

  driverApagado();
  hacerCero();  // el cero del ACS712 deriva unos 0,2 A en minutos
  pwmSalida(0, 0.0f);
  digitalWrite(PIN_EN, HIGH);
  estado = SUBIENDO;
  inicioPuntoMs = millis();
  ultimoVivoMs = inicioPuntoMs;
  tLineaMs = inicioPuntoMs;
  tCtrlUs = micros();
  sumaRaw = 0;
  nRaw = 0;
}

// Modo manual: fija el PWM objetivo (-255 a 255). Arranca desde reposo.
void comandoManual(float valor) {
  if (estado == FALLA) {
    Serial.println(F("# en falla: enviar r primero"));
    return;
  }
  if (enPunto()) {
    Serial.println(F("# hay un punto en curso: enviar s primero"));
    return;
  }
  long v = lround(valor);
  if (v < -255 || v > 255) {
    Serial.println(F("# error: usar v <PWM> entre -255 y 255"));
    return;
  }
  objetivoManual = v / 255.0f;
  if (estado != MANUAL) {
    if (v == 0) {
      Serial.println(F("# el motor ya esta apagado"));
      return;
    }
    // Arranque: driver habilitado con duty 0, sube en rampa.
    dutyManual = 0.0f;
    duty = 0.0f;
    iFiltrada = 0.0f;
    msAlta = msSinCorriente = 0;
    sobreSeguidas = 0;
    sumaILinea = sumaDutyLinea = 0.0f;
    pasosLinea = 0;
    Serial.println(F("# modo manual: el motor tiene que estar SIN CARGA (eje libre)"));
    driverApagado();
    objetivoManual = v / 255.0f;  // driverApagado lo borra
    hacerCero();
    pwmSalida(0, 0.0f);
    digitalWrite(PIN_EN, HIGH);
    estado = MANUAL;
    inicioPuntoMs = millis();
    tLineaMs = inicioPuntoMs;
    tCtrlUs = micros();
    sumaRaw = 0;
    nRaw = 0;
  }
  Serial.print(F("# PWM objetivo "));
  Serial.print(v);
  Serial.print(F("/255 = "));
  Serial.print(objetivoManual * 100.0f, 1);
  Serial.print(F(" % = "));
  Serial.print(vEstimada(objetivoManual), 2);
  Serial.println(F(" V (estimado)"));
}

// Un paso del modo manual: rampa del duty y protecciones de motor sin carga.
void pasoManual(float iMed, uint16_t dtMs) {
  iFiltrada += (iMed - iFiltrada) * (DT_S / (TAU_FILTRO_S + DT_S));
  float iAbs = fabs(iFiltrada);

  if (fabs(iMed) > I_DISPARO_MANUAL) {
    falla(F("sobrecorriente en modo manual (motor frenado o con carga)"));
    return;
  }
  if (iAbs > I_LIBRE_MAX) {
    msAlta += dtMs;
    if (msAlta >= T_ALTA_MS) {
      falla(F("corriente alta: el motor tiene carga o el eje esta trabado. Sacar la carga"));
      return;
    }
  } else {
    msAlta = 0;
  }
  if (fabs(dutyManual) > dutyMuerto + DUTY_CHEQUEO_MANUAL && iAbs < I_MIN_MANUAL) {
    msSinCorriente += dtMs;
    if (msSinCorriente >= 300) {
      falla(F("sin corriente en el ACS712 (sensor, motor o fusible): sin sensor no hay proteccion"));
      return;
    }
  } else {
    msSinCorriente = 0;
  }

  float paso = RAMPA_MANUAL * DT_S;
  if (dutyManual < objetivoManual) {
    dutyManual += paso;
    if (dutyManual > objetivoManual) dutyManual = objetivoManual;
  } else if (dutyManual > objetivoManual) {
    dutyManual -= paso;
    if (dutyManual < objetivoManual) dutyManual = objetivoManual;
  }
  duty = fabs(dutyManual);
  pwmSalida(dutyManual >= 0.0f ? 1 : -1, duty);

  sumaILinea += iMed;
  sumaDutyLinea += dutyManual;  // con signo, para ver el sentido en la salida
  pasosLinea++;

  if (objetivoManual == 0.0f && dutyManual == 0.0f) {
    driverApagado();
    estado = REPOSO;
    Serial.println(F("# modo manual terminado, driver apagado"));
  }
}

void terminarPunto(const __FlashStringHelper *motivo) {
  driverApagado();
  finPuntoMs = millis();
  estado = PAUSA;
  sostenido = false;
  resumenPunto(motivo);
}

// Un paso del lazo (cada 2 ms) con el promedio de las muestras del ADC.
void pasoControl(float iMed) {
  ultimaI = iMed;
  float dt = DT_S;
  uint16_t dtMs = DT_US / 1000;

  if (estado == MANUAL) {
    pasoManual(iMed, dtMs);
    return;
  }

  if (estado != SUBIENDO && estado != MIDIENDO) {
    sumaILinea += iMed;
    pasosLinea++;
    // En reposo no deberia circular corriente.
    if (estado != FALLA && fabs(iMed) > 0.5f) {
      if (msCorrienteReposo < 1000) msCorrienteReposo += dtMs;
      if (msCorrienteReposo >= 100 && !avisoReposo) {
        avisoReposo = true;
        Serial.println(F("# aviso: el ACS712 marca corriente con el driver apagado; revisar el cero (z) o el cableado"));
      }
    } else {
      msCorrienteReposo = 0;
    }
    return;
  }

  if (fabs(iMed) > I_DISPARO) {
    falla(F("sobrecorriente"));
    return;
  }

  float iDir = dir * iMed;  // corriente en el sentido pedido

  if (iRampa < iObj) {
    iRampa += RAMPA_A_S * dt;
    if (iRampa > iObj) iRampa = iObj;
  }

  float delta = KI * (iRampa - iDir) * dt;
  if (delta > DUTY_SUBIDA_MAX * dt) delta = DUTY_SUBIDA_MAX * dt;  // bajar no se limita
  duty += delta;
  if (duty < 0.0f) duty = 0.0f;
  bool enTecho = false;
  if (duty >= dutyLim) {
    duty = dutyLim;
    enTecho = true;
  }
  pwmSalida(dir, duty);

  // Duty en el techo sin llegar a la corriente pedida.
  if (enTecho && iDir < 0.9f * iRampa) {
    msSaturado += dtMs;
    if (msSaturado >= T_SATURADO_MS) {
      falla(F("no se alcanza la corriente con el techo de duty (rotor libre girando, R alta o motor desconectado)"));
      return;
    }
  } else {
    msSaturado = 0;
  }

  // Hay tension aplicada pero el sensor no ve corriente en el sentido pedido.
  if (duty > dutyMuerto + DUTY_CHEQUEO && iDir < I_MIN_CHEQUEO) {
    msSinCorriente += dtMs;
    if (msSinCorriente >= T_SIN_CORRIENTE_MS) {
      if (iDir < -I_MIN_CHEQUEO) {
        falla(F("el ACS712 mide en sentido contrario: enviar x para invertir el signo"));
      } else {
        falla(F("sin corriente en el ACS712 (sensor desconectado, motor abierto o fusible)"));
      }
      return;
    }
  } else {
    msSinCorriente = 0;
  }

  // Corriente estable -> estado M.
  float tol = TOL_ESTABLE_REL * iObj;
  if (tol < TOL_ESTABLE_A) tol = TOL_ESTABLE_A;
  if (estado == SUBIENDO) {
    if (iRampa >= iObj && fabs(iDir - iObj) < tol) {
      msEstable += dtMs;
      if (msEstable >= T_ESTABLE_MS) {
        estado = MIDIENDO;
        inicioMms = millis();
      }
    } else {
      msEstable = 0;
    }
  } else {
    sumaI += iMed;
    sumaDuty += duty;
    pasosM++;
  }

  sumaILinea += iMed;
  sumaDutyLinea += duty;
  pasosLinea++;

  uint32_t ahoraMs = millis();
  if (!sostenido) {
    if (ahoraMs - inicioPuntoMs >= (uint32_t)(tPunto * 1000.0f)) {
      terminarPunto(F("tiempo"));
    }
    return;
  }

  // ---- modo sostenido ----
  if (ahoraMs - ultimoVivoMs >= T_VIGILANCIA_MS) {
    terminarPunto(F("sin senal de vida de la PC (w) en 10 s"));
    return;
  }
  if (ahoraMs - inicioPuntoMs >= T_SOST_MAX_MS) {
    terminarPunto(F("tiempo maximo del modo sostenido"));
    return;
  }
  if (estado == MIDIENDO && !techoAjustado && ahoraMs - inicioMms >= T_AJUSTE_TECHO_MS && pasosM > 0) {
    float dEst = sumaDuty / pasosM;
    if (dEst + MARGEN_SOST < dutyLim) dutyLim = dEst + MARGEN_SOST;
    techoAjustado = true;
    Serial.print(F("# techo de duty ajustado a "));
    Serial.print(dutyLim * 100.0f, 1);
    Serial.print(F(" % (estable "));
    Serial.print(dEst * 100.0f, 2);
    Serial.println(F(" %)"));
  }
}

// ================================================================ serie
void imprimirLinea(uint32_t ahora) {
  // No esperar al USB: si el monitor no lee, se saltea la linea.
  if (Serial.availableForWrite() < 60) return;  // la linea ocupa hasta ~60 bytes
  float iL = ultimaI, dL = duty;
  if (pasosLinea > 0) {
    iL = sumaILinea / pasosLinea;
    dL = sumaDutyLinea / pasosLinea;
  }
  sumaILinea = sumaDutyLinea = 0.0f;
  pasosLinea = 0;
  bool activo = motorActivo();
  Serial.print(ahora);
  Serial.print(',');
  Serial.print((char)estado);
  Serial.print(',');
  Serial.print(enPunto() ? dir * iRampa : 0.0f, 2);
  Serial.print(',');
  Serial.print(iL, 2);
  Serial.print(',');
  Serial.print(dL * 100.0f, 2);
  Serial.print(',');
  Serial.print(vEstimada(dL), 2);
  Serial.print(',');
  Serial.print(activo ? (ahora - inicioPuntoMs) / 1000.0f : 0.0f, 1);
  Serial.print(F("  Imed=\""));
  Serial.print(iL, 2);
  Serial.println('"');
}

float numeroDe(const char *p) {
  while (*p == ' ' || *p == '\t') p++;
  if (*p == '\0') return NAN;
  return (float)atof(p);
}

void procesarLinea() {
  linea[largo] = '\0';
  largo = 0;
  char c = linea[0];
  float n = numeroDe(linea + 1);
  bool activo = motorActivo();

  switch (c) {
    case 's':
    case 'S':
      if (enPunto()) {
        terminarPunto(F("parado a mano"));
      } else if (estado == MANUAL) {
        driverApagado();
        estado = REPOSO;
        Serial.println(F("# modo manual parado, driver apagado (el motor sigue por inercia)"));
      } else {
        driverApagado();
        Serial.println(F("# driver apagado"));
      }
      break;
    case 'i':
    case 'I':
      if (isnan(n)) {
        Serial.println(F("# error: usar i <A>, por ejemplo i 2 o i -2"));
      } else {
        iniciarPunto(n, false);
      }
      break;
    case 'c':
    case 'C':
      if (isnan(n)) {
        Serial.println(F("# error: usar c <A>, por ejemplo c 2 o c -2 (modo sostenido)"));
      } else {
        iniciarPunto(n, true);
      }
      break;
    case 'w':
    case 'W':
      ultimoVivoMs = millis();
      break;
    case 'z':
    case 'Z':
      if (activo) {
        Serial.println(F("# no se puede hacer el cero con el motor encendido"));
      } else {
        driverApagado();
        delay(50);
        hacerCero();
        avisoReposo = false;
      }
      break;
    case 't':
    case 'T':
      if (isnan(n) || n < 1.0f || n > T_PUNTO_MAX) {
        Serial.println(F("# error: usar t <s> entre 1 y 8"));
      } else {
        tPunto = n;
        if (tPunto > 5.0f) Serial.println(F("# aviso: el procedimiento pide no mas de 5 s por punto"));
        informarEstado();
      }
      break;
    case 'e':
    case 'E':
      if (isnan(n) || n < 0.0f || n > 120.0f) {
        Serial.println(F("# error: usar e <s> entre 0 y 120"));
      } else {
        pausa = n;
        informarEstado();
      }
      break;
    case 'v':
    case 'V':
      if (isnan(n)) {
        Serial.println(F("# error: usar v <PWM> entre -255 y 255, por ejemplo v 123"));
      } else {
        comandoManual(n);
      }
      break;
    case 'm':
    case 'M':
      if (isnan(n) || n < 0.0f || n > MUERTO_MAX * 100.0f) {
        Serial.println(F("# error: usar m <%> entre 0 y 12 (zona muerta del driver)"));
      } else if (activo) {
        Serial.println(F("# cambiar m con el motor apagado"));
      } else {
        dutyMuerto = n / 100.0f;
        guardarEeprom();
        informarEstado();
      }
      break;
    case 'f':
    case 'F':
      if (isnan(n) || n < 5.0f || n > 30.0f) {
        Serial.println(F("# error: usar f <V> entre 5 y 30"));
      } else if (activo) {
        Serial.println(F("# cambiar f con el motor apagado"));
      } else {
        vFuente = n;
        informarEstado();
      }
      break;
    case 'k':
    case 'K':
      if (isnan(n) || n <= 10.0f || n >= 400.0f) {
        Serial.println(F("# error: usar k <mV/A>, por ejemplo k 100"));
      } else if (activo) {
        Serial.println(F("# cambiar k con el motor apagado"));
      } else {
        sens = n;
        actualizarEscala();
        guardarEeprom();
        informarEstado();
      }
      break;
    case 'x':
    case 'X':
      if (activo) {
        Serial.println(F("# cambiar el signo con el motor apagado"));
      } else {
        signoSensor = -signoSensor;
        guardarEeprom();
        informarEstado();
      }
      break;
    case 'r':
    case 'R':
      if (estado == FALLA) {
        driverApagado();
        estado = REPOSO;
        Serial.println(F("# rearmado"));
      }
      break;
    case '?':
      informarEstado();
      encabezado();
      break;
    default:
      break;
  }
}

void leerSerie() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (largo > 0) procesarLinea();
    } else if (largo < sizeof(linea) - 1) {
      linea[largo++] = c;
    }
  }
}

// ================================================================ principal
void setup() {
  digitalWrite(PIN_EN, LOW);
  pinMode(PIN_EN, OUTPUT);
  configurarPwm();
  driverApagado();

  Serial.begin(115200);
  while (!Serial && millis() < 3000) {
  }
  Serial.println(F("# Ensayo 1 motor 775 - rotor bloqueado con BTS7960 + ACS712"));
  Serial.println(F("# comandos: v <PWM -255..255> (manual, sin carga), i <A>, c <A> (sostenido, con w), s, z, t <s>, e <s>, f <V>, m <%>, k <mV/A>, x, r, ?"));
  cargarEeprom();
  hacerCero();
  informarEstado();
  encabezado();
  tCtrlUs = micros();
  tLineaMs = millis();
}

void loop() {
  int raw = analogRead(PIN_ACS);  // ~110 us por muestra, ~18 por paso de 2 ms
  sumaRaw += raw;
  nRaw++;

  // Corte rapido por muestra, sin esperar el promedio del paso.
  if (motorActivo()) {
    int d = raw - offsetEntero;
    int lim = (estado == MANUAL) ? limiteCuentasManual : limiteCuentas;
    if (d > lim || d < -lim) {
      if (++sobreSeguidas >= 3) {
        ultimaI = signoSensor * d / cuentasPorA;
        falla(F("sobrecorriente instantanea"));
      }
    } else {
      sobreSeguidas = 0;
    }
  }

  leerSerie();

  uint32_t ahoraUs = micros();
  if (ahoraUs - tCtrlUs >= DT_US) {
    tCtrlUs += DT_US;
    if (ahoraUs - tCtrlUs >= DT_US) tCtrlUs = ahoraUs;  // atraso grande: resincronizar
    if (nRaw > 0) {
      float prom = (float)sumaRaw / nRaw;
      sumaRaw = 0;
      nRaw = 0;
      pasoControl(signoSensor * (prom - offsetCuentas) / cuentasPorA);
    }
  }

  uint32_t ahoraMs = millis();
  bool activo = motorActivo();
  uint16_t periodo = activo ? T_LINEA_PUNTO_MS : T_LINEA_REPOSO_MS;
  if (ahoraMs - tLineaMs >= periodo) {
    tLineaMs = ahoraMs;
    imprimirLinea(ahoraMs);
  }

  if (estado == PAUSA && ahoraMs - finPuntoMs >= (uint32_t)(pausa * 1000.0f)) {
    estado = REPOSO;
    Serial.println(F("# listo para el proximo punto"));
  }
}
