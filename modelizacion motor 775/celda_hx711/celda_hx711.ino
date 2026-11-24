// Lectura de una celda de carga con el amplificador HX711.
// Solo para ensayos en banco: no forma parte del firmware del yoke.
//
// Placa: Arduino UNO. Requiere la biblioteca "HX711" de Bogdan Necula
// (gestor de bibliotecas del IDE de Arduino).
//
// Conexiones (las mismas del programa de banco de la caracterizacion del 775):
//   Celda -> HX711: rojo E+, negro E-, verde A+, blanco A-
//                   (si la lectura baja al cargar, invertir A+ y A-)
//   HX711 -> UNO:   VCC -> 5V, GND -> GND, DT -> pin 4, SCK -> pin 5
//
// Salida: monitor serie a 115200, una linea CSV por cada muestra del HX711
// (10 por segundo con RATE a GND, 80 con RATE a VCC):
//   muestra,tiempo_ms,cuentas,masa_kg,fuerza_N
// "cuentas" es la lectura cruda menos la tara; masa_kg = cuentas / factor;
// fuerza_N = masa_kg * g.
//
// Comandos por el monitor serie (terminar con "nueva linea"):
//   t        tara: pone a cero con la celda sin carga (promedio de 20 lecturas)
//   c1 1.0   registra el patron 1: masa conocida en kg apoyada en la celda
//   c2 2.5   registra el patron 2
//   c3 5.0   registra el patron 3; con los tres registrados calibra sola,
//            informa el error de cada patron y guarda el factor en la EEPROM
//   r        borra los patrones registrados (para empezar de nuevo)
//   f 12345  fija el factor a mano (cuentas por kg) y lo guarda
//   d        vuelve a la calibracion por defecto (borra la guardada)
//   ?        muestra factor, tara y patrones registrados
//
// Calibracion por defecto: si la EEPROM no tiene una calibracion propia, el
// factor sale de los patrones PATRON_DEF_* (los de la calibracion del
// 17/11/2026), con el mismo ajuste por minimos cuadrados. Una calibracion
// nueva (c1/c2/c3 o f) queda guardada y tiene prioridad hasta enviar "d".
//
// Calibracion con tres patrones:
//   1) sin carga enviar "t";
//   2) apoyar la primera masa (pesada antes en una balanza) y enviar
//      "c1 <masa_kg>"; repetir con la segunda ("c2 ...") y la tercera ("c3 ...").
//      Conviene que sean distintas y cubran el rango de uso. Un patron se puede
//      repetir enviando de nuevo su comando: se reemplaza y se recalcula.
//   3) el factor sale del ajuste por minimos cuadrados de una recta por el
//      origen (la tara ya fija el cero): factor = suma(cuentas*m) / suma(m^2).
//      Es el factor que hace minimo el error cuadratico de los tres patrones,
//      en lugar de depender de uno solo. Se informa el error de cada patron
//      (kg y %) y el error medio absoluto.
// El factor queda guardado y se usa en los arranques siguientes; la tara se
// hace sola en cada arranque (por eso hay que arrancar sin carga en la celda).

#include "HX711.h"
#include <EEPROM.h>

const uint8_t PIN_DT = 4;
const uint8_t PIN_SCK = 5;

const float G = 9.80665f;            // aceleracion de la gravedad estandar, m/s^2
const uint8_t LECTURAS_TARA = 20;    // promedio para la tara
const uint8_t LECTURAS_CALIB = 20;   // promedio para la calibracion
// Calibracion por defecto (patrones medidos el 17/11/2026: masa en kg y
// cuentas sin tara). Factor resultante ~106733 cuentas/kg, error <= 0,93 %.
const uint8_t N_PATRONES_DEF = 3;
const float PATRON_DEF_MASA[N_PATRONES_DEF] = {0.187f, 0.345f, 0.187f};
const float PATRON_DEF_CUENTAS[N_PATRONES_DEF] = {20144.0f, 36653.0f, 20088.0f};

// Factor guardado en la EEPROM: marca + valor, para distinguir una EEPROM
// virgen (0xFF) de un factor valido. La marca cambio de "HX71" a "HX72" al
// agregar la calibracion por defecto: un factor viejo no la pisa.
const int DIR_EEPROM = 0;
const uint32_t MARCA = 0x48583732UL;  // "HX72"
struct Guardado {
  uint32_t marca;
  float factor;
};

HX711 celda;
float factor = 1.0f;  // cuentas por kg (se carga en setup)
bool factorPropio = false;  // true: calibracion guardada en la EEPROM
uint32_t muestra = 0;

// Patrones de calibracion: masa conocida y cuentas medidas (sin tara).
const uint8_t N_PATRONES = 3;
float patronMasa[N_PATRONES];
float patronCuentas[N_PATRONES];
bool patronOk[N_PATRONES] = {false, false, false};

bool factorValido(float f) {
  return !isnan(f) && !isinf(f) && fabs(f) > 1e-6f;
}

// Minimos cuadrados de cuentas = factor * masa con los patrones por defecto.
float factorPorDefecto() {
  float sumaCM = 0.0f, sumaMM = 0.0f;
  for (uint8_t i = 0; i < N_PATRONES_DEF; i++) {
    sumaCM += PATRON_DEF_CUENTAS[i] * PATRON_DEF_MASA[i];
    sumaMM += PATRON_DEF_MASA[i] * PATRON_DEF_MASA[i];
  }
  return sumaCM / sumaMM;
}

void cargarFactor() {
  Guardado g;
  EEPROM.get(DIR_EEPROM, g);
  if (g.marca == MARCA && factorValido(g.factor)) {
    factor = g.factor;
    factorPropio = true;
  } else {
    factor = factorPorDefecto();
    factorPropio = false;
  }
}

void guardarFactor() {
  Guardado g = {MARCA, factor};
  EEPROM.put(DIR_EEPROM, g);
  factorPropio = true;
}

// Invalida la calibracion guardada y vuelve a la de los patrones por defecto.
void restaurarDefecto() {
  Guardado g = {0xFFFFFFFFUL, 0.0f};
  EEPROM.put(DIR_EEPROM, g);
  factor = factorPorDefecto();
  factorPropio = false;
  celda.set_scale(factor);
  Serial.println(F("# calibracion por defecto restaurada"));
}

void informarEstado() {
  Serial.print(F("# factor="));
  Serial.print(factor, 3);
  Serial.print(factorPropio ? F(" cuentas/kg (calibracion guardada)") : F(" cuentas/kg (por defecto)"));
  Serial.print(F(", tara="));
  Serial.println(celda.get_offset());
  if (!factorPropio) {
    for (uint8_t i = 0; i < N_PATRONES_DEF; i++) {
      Serial.print(F("# patron por defecto "));
      Serial.print(i + 1);
      Serial.print(F(": "));
      Serial.print(PATRON_DEF_MASA[i], 3);
      Serial.print(F(" kg, "));
      Serial.print(PATRON_DEF_CUENTAS[i], 0);
      Serial.println(F(" cuentas"));
    }
  }
  for (uint8_t i = 0; i < N_PATRONES; i++) {
    Serial.print(F("# patron "));
    Serial.print(i + 1);
    if (patronOk[i]) {
      Serial.print(F(": "));
      Serial.print(patronMasa[i], 3);
      Serial.print(F(" kg, "));
      Serial.print(patronCuentas[i], 0);
      Serial.println(F(" cuentas"));
    } else {
      Serial.println(F(": sin registrar"));
    }
  }
}

// Ajuste por minimos cuadrados de cuentas = factor * masa (recta por el
// origen) con los tres patrones; informa el error de cada uno y lo guarda.
void calibrarConPatrones() {
  float sumaCM = 0.0f, sumaMM = 0.0f;
  for (uint8_t i = 0; i < N_PATRONES; i++) {
    sumaCM += patronCuentas[i] * patronMasa[i];
    sumaMM += patronMasa[i] * patronMasa[i];
  }
  float nuevo = sumaCM / sumaMM;
  if (!factorValido(nuevo)) {
    Serial.println(F("# error: factor nulo; revisar tara y conexion"));
    return;
  }

  Serial.println(F("# patron,masa_kg,medido_kg,error_kg,error_%"));
  float sumaErrAbs = 0.0f, sumaErrPct = 0.0f;
  for (uint8_t i = 0; i < N_PATRONES; i++) {
    float medido = patronCuentas[i] / nuevo;
    float err = medido - patronMasa[i];
    float pct = 100.0f * err / patronMasa[i];
    sumaErrAbs += fabs(err);
    sumaErrPct += fabs(pct);
    Serial.print(F("# "));
    Serial.print(i + 1);
    Serial.print(',');
    Serial.print(patronMasa[i], 3);
    Serial.print(',');
    Serial.print(medido, 3);
    Serial.print(',');
    Serial.print(err, 4);
    Serial.print(',');
    Serial.println(pct, 2);
  }
  Serial.print(F("# error medio absoluto: "));
  Serial.print(sumaErrAbs / N_PATRONES, 4);
  Serial.print(F(" kg ("));
  Serial.print(sumaErrPct / N_PATRONES, 2);
  Serial.println(F(" %)"));

  factor = nuevo;
  celda.set_scale(factor);
  guardarFactor();
  Serial.println(F("# calibrado y guardado"));
  informarEstado();
}

void procesarComando() {
  char c = Serial.read();
  switch (c) {
    case 't':
    case 'T':
      celda.tare(LECTURAS_TARA);
      Serial.println(F("# tara hecha"));
      informarEstado();
      break;

    case 'c':
    case 'C': {
      long n = Serial.parseInt();      // numero de patron: 1, 2 o 3
      float masa = Serial.parseFloat();
      if (n < 1 || n > N_PATRONES || masa <= 0.0f) {
        Serial.println(F("# error: usar c1/c2/c3 <masa_kg>, con masa > 0"));
        break;
      }
      uint8_t i = n - 1;
      // get_value devuelve lectura cruda menos tara, sin dividir por el factor.
      patronCuentas[i] = celda.get_value(LECTURAS_CALIB);
      patronMasa[i] = masa;
      patronOk[i] = true;
      Serial.print(F("# patron "));
      Serial.print(n);
      Serial.print(F(" registrado: "));
      Serial.print(masa, 3);
      Serial.print(F(" kg, "));
      Serial.print(patronCuentas[i], 0);
      Serial.println(F(" cuentas"));

      uint8_t faltan = 0;
      for (uint8_t k = 0; k < N_PATRONES; k++) {
        if (!patronOk[k]) faltan++;
      }
      if (faltan == 0) {
        calibrarConPatrones();
      } else {
        Serial.print(F("# faltan "));
        Serial.print(faltan);
        Serial.println(F(" patron(es) para calibrar"));
      }
      break;
    }

    case 'r':
    case 'R':
      for (uint8_t k = 0; k < N_PATRONES; k++) patronOk[k] = false;
      Serial.println(F("# patrones borrados"));
      break;

    case 'f':
    case 'F': {
      float nuevo = Serial.parseFloat();
      if (!factorValido(nuevo)) {
        Serial.println(F("# error: usar f <cuentas_por_kg>, distinto de 0"));
        break;
      }
      factor = nuevo;
      celda.set_scale(factor);
      guardarFactor();
      informarEstado();
      break;
    }

    case 'd':
    case 'D':
      restaurarDefecto();
      informarEstado();
      break;

    case '?':
      informarEstado();
      break;

    default:
      break;  // fin de linea u otro caracter: se ignora
  }
}

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(200);  // para parseFloat en los comandos c y f

  cargarFactor();
  celda.begin(PIN_DT, PIN_SCK);

  Serial.println(F("# Celda de carga HX711 - comandos: t, c1/c2/c3 <kg>, r, f <cuentas/kg>, d, ?"));
  if (!celda.wait_ready_timeout(1000)) {
    Serial.println(F("# error: el HX711 no responde; revisar DT, SCK y alimentacion"));
  }
  celda.set_scale(factor);
  celda.tare(LECTURAS_TARA);  // arrancar sin carga sobre la celda
  informarEstado();
  Serial.println(F("muestra,tiempo_ms,cuentas,masa_kg,fuerza_N"));
}

void loop() {
  if (Serial.available()) {
    procesarComando();
  }

  // Una linea por cada conversion nueva del HX711 (no bloquea esperando).
  if (celda.is_ready()) {
    long cuentas = celda.read() - celda.get_offset();
    float masaKg = cuentas / factor;
    float fuerzaN = masaKg * G;

    Serial.print(++muestra);
    Serial.print(',');
    Serial.print(millis());
    Serial.print(',');
    Serial.print(cuentas);
    Serial.print(',');
    Serial.print(masaKg, 3);
    Serial.print(',');
    Serial.println(fuerzaN, 2);
  }
}
