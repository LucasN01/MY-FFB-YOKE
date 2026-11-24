// Tacometro para caracterizar el motor 775 con un modulo optico LM393.
// Solo para ensayos en banco: no forma parte del firmware del yoke.
//
// Placa: Arduino UNO. Conexion del modulo:
//   VCC -> 5V, GND -> GND, D0 -> pin 2, A0 -> sin conectar.
//
// Salida: monitor serie a 115200, una linea CSV cada 0,5 s (se puede pegar
// en una planilla): tiempo_ms,rpm,frecuencia_hz,pulsos,rechazados
//
// Mide el tiempo entre flancos en lugar de contar pulsos en una ventana fija:
// asi la resolucion no depende de cuantas ranuras tenga el disco.

const uint8_t PIN_SENSOR = 2;

// Cantidad de ranuras del disco: AJUSTAR antes de medir.
const uint8_t RANURAS = 1;

// Velocidad maxima esperada. Los flancos mas proximos que el periodo a esta
// velocidad son rebote del comparador y se descartan. Si "rechazados" crece y
// las rpm se clavan cerca de este valor, subirlo.
const uint32_t RPM_MAXIMA = 15000;

const uint32_t PERIODO_MIN_US = 60000000UL / (RPM_MAXIMA * RANURAS);
const uint32_t INTERVALO_MS = 500;
// Sin pulsos durante este tiempo se informa 0 rpm.
const uint32_t TIEMPO_DETENIDO_MS = 2000;

volatile uint32_t ultimoFlancoUs = 0;
volatile uint32_t pulsos = 0;
volatile uint32_t rechazados = 0;

void alFlanco() {
  uint32_t ahora = micros();
  if (ahora - ultimoFlancoUs < PERIODO_MIN_US) {
    rechazados++;
    return;
  }
  ultimoFlancoUs = ahora;
  pulsos++;
}

void setup() {
  Serial.begin(115200);
  // Solo espera en placas con USB nativo; en el UNO sigue de inmediato.
  while (!Serial && millis() < 3000) {
  }
  // Con el cable suelto lee 0 rpm en lugar de ruido.
  pinMode(PIN_SENSOR, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_SENSOR), alFlanco, FALLING);
  Serial.println(F("tiempo_ms,rpm,frecuencia_hz,pulsos,rechazados"));
}

void loop() {
  static uint32_t proximaLinea = INTERVALO_MS;
  static uint32_t pulsosRef = 0;
  static uint32_t flancoRef = 0;
  static uint32_t rechazadosRef = 0;
  static uint32_t ultimoCambioMs = 0;
  static bool hayReferencia = false;
  static float frecuencia = 0.0;

  uint32_t ahoraMs = millis();
  if ((int32_t)(ahoraMs - proximaLinea) < 0) {
    return;
  }
  proximaLinea += INTERVALO_MS;

  noInterrupts();
  uint32_t p = pulsos;
  uint32_t u = ultimoFlancoUs;
  uint32_t r = rechazados;
  interrupts();

  uint32_t pulsosVentana = p - pulsosRef;
  if (pulsosVentana > 0) {
    // El primer pulso tras estar detenido solo fija la referencia: el tiempo
    // desde el flanco anterior no representa la velocidad actual.
    if (hayReferencia) {
      frecuencia = pulsosVentana * 1000000.0 / (uint32_t)(u - flancoRef);
    }
    pulsosRef = p;
    flancoRef = u;
    hayReferencia = true;
    ultimoCambioMs = ahoraMs;
  } else if (ahoraMs - ultimoCambioMs > TIEMPO_DETENIDO_MS) {
    frecuencia = 0.0;
    hayReferencia = false;
  }

 // Serial.print(ahoraMs);
  //Serial.print(',');
  Serial.println(frecuencia * 60.0 / RANURAS, 1);
  //Serial.print(',');
  //Serial.print(frecuencia, 2);
  //Serial.print(',');
  //Serial.print(pulsosVentana);
  //Serial.print(',');
  //Serial.println(r - rechazadosRef);

  rechazadosRef = r;
}
