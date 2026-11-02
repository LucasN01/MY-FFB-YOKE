/*
  mis_defines.h
  ------------------------------------------------------------
  ARCHIVO NUEVO — reemplaza a "defines.h" del proyecto original.
  Acá van TODOS los valores que dependen de TU hardware.
  Buscá "TODO-USUARIO" y completá con tus datos reales.
------------------------------------------------------------
*/

#ifndef MIS_DEFINES_H
#define MIS_DEFINES_H

// LCD 16x2 I2C: Pico 2 GP4/GP5.
// Migracion y conexion electrica: LEEME_PICO2.md.
#define LCD_I2C_ADDRESS 0x27 // tambien prueba 0x3F si 0x27 no responde
#define LCD_REFRESH_MS 100  // nueva lectura cada 100 ms
#define LCD_POWER_ON_WAIT_MS 100 // asentamiento de alimentacion antes de inicializar
#define LCD_RETRY_MS 500        // reintentar si no responde o falla I2C
#define LCD_RESYNC_MS 5000      // recuperar tambien reset del LCD con PCF8574 aun activo
// Menu de ganancias en Pico 2. GPIO (no numeros fisicos del conector).
#ifndef YOKE_CALIBRATION_ENABLED
#define YOKE_CALIBRATION_ENABLED 1
#endif
#define YOKE_LED_NORMAL_PIN 12
#define YOKE_LED_CALIBRATION_PIN 13
#define YOKE_BUTTON_MODE_PIN 20
#define YOKE_BUTTON_BACK_PIN 19
#define YOKE_BUTTON_OK_PIN 18
#define YOKE_BUTTON_MINUS_PIN 17
#define YOKE_BUTTON_PLUS_PIN 16
#define YOKE_BUTTON_DEBOUNCE_MS 30
#define YOKE_BUTTON_REPEAT_DELAY_MS 1000
#define YOKE_BUTTON_REPEAT_INTERVAL_MS 150
static_assert(YOKE_BUTTON_REPEAT_DELAY_MS >= 500 && YOKE_BUTTON_REPEAT_INTERVAL_MS >= 50, "Repeticion demasiado rapida");
// Incrementar para descartar ajustes persistidos y volver a estos defaults.
#define YOKE_SETTINGS_VERSION 1
static_assert(YOKE_CALIBRATION_ENABLED == 0 || YOKE_CALIBRATION_ENABLED == 1,
              "Bandera de menu debe ser 0 o 1");
static_assert(YOKE_BUTTON_DEBOUNCE_MS >= 10 && YOKE_BUTTON_DEBOUNCE_MS <= 200 &&
              YOKE_SETTINGS_VERSION > 0 && YOKE_SETTINGS_VERSION <= 65535,
              "Revisar debounce/version de ajustes");
static_assert(LCD_POWER_ON_WAIT_MS >= 50 && LCD_RETRY_MS >= 100 && LCD_RESYNC_MS >= 1000,
              "Tiempos LCD demasiado cortos");

// Fuente UNICA de posicion y velocidad para todos los efectos.
// 0 = encoders incrementales AB; 
// 1 = SimConnect; 
// 2 = potenciometros ADC0/ADC1;
// 3 = encoders magneticos absolutos AS5600 por I2C (solo Pico).

#ifdef YOKE_USE_SIMULATION
#error "YOKE_USE_SIMULATION se renombro a YOKE_POSITION_SOURCE (0..2 iguales; 3 = AS5600)"
#endif
#ifndef YOKE_POSITION_SOURCE
#define YOKE_POSITION_SOURCE 1
#endif
static_assert(YOKE_POSITION_SOURCE >= 0 && YOKE_POSITION_SOURCE <= 3,
              "YOKE_POSITION_SOURCE debe ser 0, 1, 2 o 3");

// Potenciometros: numeros GPIO, NO numeros fisicos 31/32. Entrada 0..3.3 V.
#define ROLL_POT_PIN 26  // ADC0, pin fisico 31
#define PITCH_POT_PIN 27 // ADC1, pin fisico 32
// TODO-USUARIO: medir extremos mecanicos y centro; ADC de 12 bits (0..4095).
// NEG/POS indican sentido mecanico. Se permite NEG > CENTER > POS.
#define ROLL_POT_ADC_NEG 0
#define ROLL_POT_ADC_CENTER 2048
#define ROLL_POT_ADC_POS 4095
#define PITCH_POT_ADC_NEG 0
#define PITCH_POT_ADC_CENTER 2048
#define PITCH_POT_ADC_POS 4095
// Promedio por ciclo y pequena histeresis para no derivar ruido en reposo.
#define YOKE_POT_SAMPLES 4
#define YOKE_POT_HYSTERESIS_ADC 4
static_assert(YOKE_POT_SAMPLES >= 1 && YOKE_POT_SAMPLES <= 16 &&
              YOKE_POT_HYSTERESIS_ADC >= 0 && YOKE_POT_HYSTERESIS_ADC <= 32,
              "Revisar muestreo/histeresis ADC");
// ============================================================
// ENCODERS MAGNETICOS AS5600 (YOKE_POSITION_SOURCE 3). Ver LEEME_AS5600.md.
// ============================================================
// Direccion I2C fija 0x36: un sensor por bus. Roll: I2C1 propio.
// Pitch: I2C0 compartido con el LCD (LCD_SDA_PIN/LCD_SCL_PIN).
#define AS5600_I2C_ADDRESS 0x36
#define ROLL_AS5600_SDA_PIN 14 // I2C1 SDA, pin fisico 19
#define ROLL_AS5600_SCL_PIN 15 // I2C1 SCL, pin fisico 20
#define YOKE_I2C1_HZ 400000UL  // bus propio del AS5600 de roll
#define YOKE_I2C0_HZ 100000UL  // LCD + AS5600 de pitch; PCF8574: 100 kHz maximo por hoja de datos
// Centro: -1 = la posicion al encender es el centro (como encoders incrementales).
// 0..4095 = valor a= medido con el yugo centrado (centro absoluto, sin centrar al
// encender); solo si el eje del sensor gira menos de media vuelta a cada lado.
#define ROLL_AS5600_CENTER_RAW  -1
#define PITCH_AS5600_CENTER_RAW -1
// Cuentas desde el centro a cada extremo (4096 = 1 vuelta del eje del sensor).
// Roll +-90 grados: 1024. Pitch +-150 mm con polea efectiva de 75 mm en el eje del
// sensor: 150/(pi*75)*4096 = 2608. ESTIMACIONES: reemplazar por cuentas medidas.
#define ROLL_AS5600_COUNTS_NEG  1024
#define ROLL_AS5600_COUNTS_POS  1024
#define PITCH_AS5600_COUNTS_NEG 2608
#define PITCH_AS5600_COUNTS_POS 2608
#define ROLL_AS5600_DIRECTION   1   // +1 o -1 (tambien se invierte con el pin DIR)
#define PITCH_AS5600_DIRECTION  1
#define YOKE_AS5600_HYSTERESIS_COUNTS 1 // banda movil contra el ruido de +-1 cuenta
#define YOKE_AS5600_FAULT_CYCLES 3      // lecturas malas seguidas para cortar fuerza (6 ms)
#define YOKE_AS5600_RECOVER_CYCLES 50   // lecturas buenas seguidas para volver (100 ms)
#define YOKE_AS5600_MAX_GAP_MS 40       // con centro al encender: hueco mas largo = bloqueo hasta reiniciar
#define YOKE_AS5600_START_WINDOW_MS 500 // con centro al encender: primera lectura despues de esto = bloqueo
#define YOKE_AS5600_OVERTRAVEL 1.25f    // |cuentas| > 1,25 * extremo = vuelta perdida (bloqueo)
#define YOKE_AS5600_REQUIRE_MD 1        // 1 = exigir bit MD (iman detectado); 0 si un modulo no lo informa
#define YOKE_AS5600_CONF 0x0500         // SF=8x (1,1 ms), FTH=6 LSB, WD=0, PM=NOM. Volatil, nunca BURN

// ============================================================
// PINES - ENCODERS (cuadratura, uno por eje)
// ============================================================
// Pico 2 usa entradas a 3.3 V e interrupciones en ambos canales de cada eje.
// Numeros GPIO, no numeros fisicos del conector. Solo logica de 3.3 V.
#define ROLL_ENC_A 0
#define ROLL_ENC_B 1
#define PITCH_ENC_A 2
#define PITCH_ENC_B 3


// Etapas 1 y 2: +1 conserva el cableado/comportamiento anterior; -1 invierte.
// Convencion propuesta: roll derecha positivo; pitch tirar positivo.
#define ROLL_ENCODER_DIRECTION   1
#define PITCH_ENCODER_DIRECTION  1
#define ROLL_MOTOR_DIRECTION     1
#define PITCH_MOTOR_DIRECTION    1

// ============================================================
// LIMITES DE POSICION SEGUN EL SENSOR
// ------------------------------------------------------------
// Encoder de 1000 PPR leido x4: 4000 cuentas/vuelta.
// Roll directo +/-90 grados: +/-1000 cuentas.
// Pitch +/-150 mm, polea efectiva de 75 mm, relacion 1:1: +/-2546 cuentas.
// Pitch es una ESTIMACION: reemplazar por las cuentas c= medidas en extremos.
// Estos cuatro valores solo afectan al modo encoder. Centro al encender.
#define ROLL_ENCODER_MIN    -1000
#define ROLL_ENCODER_MAX     1000
#define PITCH_ENCODER_MIN   -2546
#define PITCH_ENCODER_MAX    2546

#define ROLL_POS_MIN   ROLL_ENCODER_MIN
#define ROLL_POS_MAX   ROLL_ENCODER_MAX
#define PITCH_POS_MIN  PITCH_ENCODER_MIN
#define PITCH_POS_MAX  PITCH_ENCODER_MAX

// ============================================================
// PINES - PUENTE H (2 PWM por eje, igual que el proyecto original)
// ============================================================
// TODO-USUARIO: confirmá que sean pines con salida PWM en tu placa
#define ROLL_PWM_LEFT 6
#define ROLL_PWM_RIGHT 7
#define PITCH_PWM_LEFT 8
#define PITCH_PWM_RIGHT 9
#define ROLL_EN_PIN 10
#define PITCH_EN_PIN 11
#define LCD_SDA_PIN 4
#define LCD_SCL_PIN 5
// Frecuencia aproximada del analogWrite AVR original, rango conservado 0..255.
#define MOTOR_PWM_HZ 490


// Límite de PWM real que le mandás al driver (0-255 típico en Arduino).
// Podés limitarlo por debajo de 255 si tu driver/motor lo requiere.
#define MOTOR_PWM_HARD_LIMIT 220 // techo absoluto no modificable desde el menu; no limita corriente
static_assert(MOTOR_PWM_HARD_LIMIT >= 0 && MOTOR_PWM_HARD_LIMIT <= 220, "Techo PWM invalido");
#define PWM_MAX  255   // TODO-USUARIO si tu driver necesita otro techo

// Tiempo de ciclo de control, en milisegundos.
#define CONTROL_LOOP_MS  2   // 2ms = 500Hz, igual orden que el proyecto original
#define YOKE_WDT_MS        250   // reinicio por hardware si loop() no corre en 250 ms
#define YOKE_WDT_FLASH_MS  3000  // durante commit de EEPROM (borrado de sector, max ~400 ms)
#define YOKE_USB_GUARD     1     // serializar CDC con mutex de la HAL (correccion H5)
#ifndef YOKE_TEST_HANG
#define YOKE_TEST_HANG     0     // SOLO ENSAYO: 0=no, 1=for(;;), 2=hal::irqSave+for(;;), 3=__builtin_trap
#endif
#ifndef YOKE_TEST_HANG_MS
#define YOKE_TEST_HANG_MS  60000 // instante del ensayo de cuelgue
#endif
#define YOKE_USB_STRESS    0     // SOLO ENSAYO: 1 = escribir linea corta en cada ciclo de 2 ms
static_assert(YOKE_WDT_MS >= 50*CONTROL_LOOP_MS, "WDT demasiado corto para el lazo");

// Ajustes comunes para velocidad de SimConnect, encoders y potenciometros.
#define YOKE_MOTION_FILTER_TAU_S 0.040f
#define YOKE_MOTION_MAX_SPEED 10.0f // recorridos centro-extremo por segundo
#define YOKE_MOTION_DEAD_SPEED 0.02f
#ifndef YOKE_DAMPER_ENABLED
#define YOKE_DAMPER_ENABLED 1
#endif
#ifndef YOKE_FRICTION_ENABLED
#define YOKE_FRICTION_ENABLED 1
#endif
// Damper: B escala con sqrt(rigidez/rigidez a Q=1) para mantener la razon de
// amortiguamiento zeta=b/(2*sqrt(kJ)). 0 = B constante (comportamiento anterior).
#ifndef YOKE_DAMPER_STIFFNESS_SCALING
#define YOKE_DAMPER_STIFFNESS_SCALING 1
#endif
#define YOKE_DAMPER_SCALE_MIN 0.50f
#define YOKE_DAMPER_SCALE_MAX 2.00f

// Friccion: 0 = Coulomb regularizada por velocidad (anterior);
// 1 = Hayward-Armstrong (adherencia + presliding, solo posicion).
#ifndef YOKE_FRICTION_MODEL
#define YOKE_FRICTION_MODEL 1
#endif
#define YOKE_FRICTION_PRESLIDE 0.020f // desplazamiento normalizado hasta deslizar

#define YOKE_DAMPER_GAIN 0.120f // fuerza adicional por unidad de velocidad
#define YOKE_FRICTION_GAIN 0.030f // correccion suave casi constante al moverse
#define YOKE_FRICTION_SPEED 0.200f // velocidad para alcanzar media correccion
#define YOKE_MOTION_CORRECTION_LIMIT 0.400f
#define YOKE_MOTION_FORCE_BLEND 0.100f // legado: ya no interviene en amortiguacion/friccion
static_assert((YOKE_DAMPER_STIFFNESS_SCALING == 0 || YOKE_DAMPER_STIFFNESS_SCALING == 1) &&
              (YOKE_FRICTION_MODEL == 0 || YOKE_FRICTION_MODEL == 1), "Banderas damper/friccion 0 o 1");
static_assert(YOKE_DAMPER_SCALE_MIN > 0 && YOKE_DAMPER_SCALE_MIN <= 1 &&
              YOKE_DAMPER_SCALE_MAX >= 1 && YOKE_DAMPER_SCALE_MAX <= 4 &&
              YOKE_FRICTION_PRESLIDE > 0 && YOKE_FRICTION_PRESLIDE <= .2f, "Ajustes damper/friccion invalidos");
static_assert((YOKE_DAMPER_ENABLED == 0 || YOKE_DAMPER_ENABLED == 1) &&
              (YOKE_FRICTION_ENABLED == 0 || YOKE_FRICTION_ENABLED == 1), "Banderas movimiento 0 o 1");
static_assert(YOKE_MOTION_FILTER_TAU_S >= 0 && YOKE_MOTION_MAX_SPEED > 0 &&
              YOKE_MOTION_DEAD_SPEED >= 0 && YOKE_MOTION_DEAD_SPEED < YOKE_MOTION_MAX_SPEED &&
              YOKE_DAMPER_GAIN >= 0 && YOKE_DAMPER_GAIN <= 1 &&
              YOKE_FRICTION_GAIN >= 0 && YOKE_FRICTION_GAIN <= 1 && YOKE_FRICTION_SPEED > 0 &&
              YOKE_MOTION_CORRECTION_LIMIT >= 0 && YOKE_MOTION_CORRECTION_LIMIT <= 1 &&
              YOKE_MOTION_FORCE_BLEND > 0, "Ajustes de movimiento invalidos");

// Telemetria de etapas 1/2: una linea cada 100ms, alternando Roll/Pitch.
// No espera a que se abra el monitor; omite la linea si no hay espacio USB.
#define DEBUG_MOTION_TELEMETRY 1
#ifndef YOKE_MODEL_LOG
#define YOKE_MODEL_LOG 1
#endif
#define YOKE_RECORD_OPTION (YOKE_MODEL_LOG && SIMCONNECT_ADDITIONS_ENABLED)
#ifndef YOKE_SLOW_LOOP_US
#define YOKE_SLOW_LOOP_US 3000
#endif
static_assert(YOKE_SLOW_LOOP_US > CONTROL_LOOP_MS*1000, "Umbral de ciclo lento invalido");
#define MOTION_TELEMETRY_MS 100

static_assert(ROLL_AS5600_CENTER_RAW >= -1 && ROLL_AS5600_CENTER_RAW <= 4095, "Centro AS5600 ROLL invalido");
static_assert(ROLL_AS5600_COUNTS_NEG >= 1 && ROLL_AS5600_COUNTS_NEG <= 32767 &&
              (ROLL_AS5600_CENTER_RAW < 0 || ROLL_AS5600_COUNTS_NEG <= 1800), "Recorrido AS5600 ROLL NEG invalido");
static_assert(ROLL_AS5600_COUNTS_POS >= 1 && ROLL_AS5600_COUNTS_POS <= 32767 &&
              (ROLL_AS5600_CENTER_RAW < 0 || ROLL_AS5600_COUNTS_POS <= 1800), "Recorrido AS5600 ROLL POS invalido");
static_assert(ROLL_AS5600_DIRECTION == 1 || ROLL_AS5600_DIRECTION == -1, "Signo AS5600 ROLL invalido");
static_assert(PITCH_AS5600_CENTER_RAW >= -1 && PITCH_AS5600_CENTER_RAW <= 4095, "Centro AS5600 PITCH invalido");
static_assert(PITCH_AS5600_COUNTS_NEG >= 1 && PITCH_AS5600_COUNTS_NEG <= 32767 &&
              (PITCH_AS5600_CENTER_RAW < 0 || PITCH_AS5600_COUNTS_NEG <= 1800), "Recorrido AS5600 PITCH NEG invalido");
static_assert(PITCH_AS5600_COUNTS_POS >= 1 && PITCH_AS5600_COUNTS_POS <= 32767 &&
              (PITCH_AS5600_CENTER_RAW < 0 || PITCH_AS5600_COUNTS_POS <= 1800), "Recorrido AS5600 PITCH POS invalido");
static_assert(PITCH_AS5600_DIRECTION == 1 || PITCH_AS5600_DIRECTION == -1, "Signo AS5600 PITCH invalido");
static_assert(YOKE_AS5600_HYSTERESIS_COUNTS >= 0 && YOKE_AS5600_HYSTERESIS_COUNTS <= 8, "YOKE_AS5600_HYSTERESIS_COUNTS fuera de rango");
static_assert(YOKE_AS5600_FAULT_CYCLES >= 1 && YOKE_AS5600_FAULT_CYCLES <= 50, "YOKE_AS5600_FAULT_CYCLES fuera de rango");
static_assert(YOKE_AS5600_RECOVER_CYCLES >= 1 && YOKE_AS5600_RECOVER_CYCLES <= 500, "YOKE_AS5600_RECOVER_CYCLES fuera de rango");
static_assert(YOKE_AS5600_MAX_GAP_MS >= 10 && YOKE_AS5600_MAX_GAP_MS <= 100, "YOKE_AS5600_MAX_GAP_MS fuera de rango");
static_assert(YOKE_AS5600_START_WINDOW_MS >= 100 && YOKE_AS5600_START_WINDOW_MS <= 2000, "YOKE_AS5600_START_WINDOW_MS fuera de rango");
static_assert(YOKE_AS5600_OVERTRAVEL >= 1.05f && YOKE_AS5600_OVERTRAVEL <= 2, "YOKE_AS5600_OVERTRAVEL fuera de rango");
static_assert(YOKE_AS5600_REQUIRE_MD >= 0 && YOKE_AS5600_REQUIRE_MD <= 1, "YOKE_AS5600_REQUIRE_MD fuera de rango");
static_assert(YOKE_I2C0_HZ >= 100000 && YOKE_I2C0_HZ <= 400000, "YOKE_I2C0_HZ fuera de rango");
static_assert(YOKE_I2C1_HZ >= 100000 && YOKE_I2C1_HZ <= 1000000, "YOKE_I2C1_HZ fuera de rango");
static_assert(ROLL_AS5600_SDA_PIN % 4 == 2 && ROLL_AS5600_SDA_PIN <= 26, "Pin I2C1 SDA invalido");
static_assert(ROLL_AS5600_SCL_PIN % 4 == 3 && ROLL_AS5600_SCL_PIN <= 27, "Pin I2C1 SCL invalido");
#if YOKE_POSITION_SOURCE == 3
static_assert(ROLL_AS5600_SDA_PIN != ROLL_PWM_LEFT &&
              ROLL_AS5600_SDA_PIN != ROLL_PWM_RIGHT &&
              ROLL_AS5600_SDA_PIN != PITCH_PWM_LEFT &&
              ROLL_AS5600_SDA_PIN != PITCH_PWM_RIGHT &&
              ROLL_AS5600_SDA_PIN != ROLL_EN_PIN &&
              ROLL_AS5600_SDA_PIN != PITCH_EN_PIN &&
              ROLL_AS5600_SDA_PIN != LCD_SDA_PIN &&
              ROLL_AS5600_SDA_PIN != LCD_SCL_PIN, "Pin AS5600 SDA en conflicto");
static_assert(ROLL_AS5600_SCL_PIN != ROLL_PWM_LEFT &&
              ROLL_AS5600_SCL_PIN != ROLL_PWM_RIGHT &&
              ROLL_AS5600_SCL_PIN != PITCH_PWM_LEFT &&
              ROLL_AS5600_SCL_PIN != PITCH_PWM_RIGHT &&
              ROLL_AS5600_SCL_PIN != ROLL_EN_PIN &&
              ROLL_AS5600_SCL_PIN != PITCH_EN_PIN &&
              ROLL_AS5600_SCL_PIN != LCD_SDA_PIN &&
              ROLL_AS5600_SCL_PIN != LCD_SCL_PIN, "Pin AS5600 SCL en conflicto");
#endif

static_assert(ROLL_POS_MIN < 0 && ROLL_POS_MAX > 0, "Revisar limites Roll");
static_assert(PITCH_POS_MIN < 0 && PITCH_POS_MAX > 0, "Revisar limites Pitch");
static_assert(ROLL_POS_MIN >= -32767L && ROLL_POS_MAX <= 32767L &&
              PITCH_POS_MIN >= -32767L && PITCH_POS_MAX <= 32767L,
              "Los limites deben caber en el rango de posicion HID de 16 bits");
static_assert(CONTROL_LOOP_MS > 0, "Periodo de control debe ser > 0");
static_assert(PWM_MAX >= 0 && PWM_MAX <= 255, "PWM_MAX fuera de rango");
static_assert(ROLL_ENCODER_DIRECTION == 1 || ROLL_ENCODER_DIRECTION == -1, "Signo encoder Roll");
static_assert(PITCH_ENCODER_DIRECTION == 1 || PITCH_ENCODER_DIRECTION == -1, "Signo encoder Pitch");
static_assert(ROLL_MOTOR_DIRECTION == 1 || ROLL_MOTOR_DIRECTION == -1, "Signo motor Roll");
static_assert(PITCH_MOTOR_DIRECTION == 1 || PITCH_MOTOR_DIRECTION == -1, "Signo motor Pitch");

// Ganancia total de los efectos calculados desde SimConnect (0..100).
#define SIMCONNECT_TOTAL_GAIN 90
static_assert(SIMCONNECT_TOTAL_GAIN >= 0 && SIMCONNECT_TOTAL_GAIN <= 100,
              "Ganancia SimConnect fuera de 0..100");

// Aportes SimConnect independientes del FFB HID. Ver EFECTOS_SIMCONNECT.md.
#ifndef SIMCONNECT_ADDITIONS_ENABLED
#define SIMCONNECT_ADDITIONS_ENABLED 1
#endif
// Centrado aerodinamico y vibracion de rodaje: recompilar firmware al cambiar.
#ifndef SIMCONNECT_DYNAMIC_CENTERING_ENABLED
#define SIMCONNECT_DYNAMIC_CENTERING_ENABLED 1
#endif
#ifndef SIMCONNECT_GROUND_VIBRATION_ENABLED
#define SIMCONNECT_GROUND_VIBRATION_ENABLED 1
#endif
#define SIMCONNECT_CENTERING_PITCH_GAIN 0.800f
// Rigidez mecanica de reposo, sumada a la aerodinamica (0 = solo aerodinamico).
#define SIMCONNECT_CENTERING_MIN_STIFFNESS 0.15f
// Centrado: rigidez = reposo (mecanica) + este coeficiente * Q (aerodinamica).
// 0.85 = 1 - 0.15 conserva identica la curva de fabrica (k=0.15+0.85Q).
#define SIMCONNECT_CENTERING_AERO_STIFFNESS 0.85f

static_assert(SIMCONNECT_CENTERING_AERO_STIFFNESS > 0 && SIMCONNECT_CENTERING_AERO_STIFFNESS <= 4,
              "Rigidez aerodinamica fuera de rango");
static_assert(SIMCONNECT_CENTERING_MIN_STIFFNESS >= 0 && SIMCONNECT_CENTERING_MIN_STIFFNESS <= 1,
              "Rigidez minima fuera de 0..1");
#define SIMCONNECT_CENTERING_ROLL_GAIN (SIMCONNECT_CENTERING_PITCH_GAIN * 0.5f)
#define SIMCONNECT_GROUND_VIBRATION_GAIN 0.180f
// Rodaje: ganancia referida a ROLL; pitch recibe 1/10. Tope propio del efecto.
#define SIMCONNECT_GROUND_ROLL_PITCH_RATIO 10.0f
#define SIMCONNECT_GROUND_MAX_PWM 40
static_assert(SIMCONNECT_GROUND_ROLL_PITCH_RATIO >= 1 &&
              SIMCONNECT_GROUND_MAX_PWM >= 0 && SIMCONNECT_GROUND_MAX_PWM <= 40,
              "Rodaje: ratio >= 1 y limite PWM entre 0 y 40");
static_assert((SIMCONNECT_DYNAMIC_CENTERING_ENABLED == 0 || SIMCONNECT_DYNAMIC_CENTERING_ENABLED == 1) &&
              (SIMCONNECT_GROUND_VIBRATION_ENABLED == 0 || SIMCONNECT_GROUND_VIBRATION_ENABLED == 1),
              "Banderas de centrado/rodaje deben ser 0 o 1");
static_assert(SIMCONNECT_CENTERING_PITCH_GAIN >= 0 && SIMCONNECT_CENTERING_PITCH_GAIN <= 1 &&
              SIMCONNECT_GROUND_VIBRATION_GAIN >= 0 && SIMCONNECT_GROUND_VIBRATION_GAIN <= 1,
              "Ganancias centrado/rodaje fuera de 0..1");
// Efectos SimConnect: 1 activa / 0 desactiva. Recompilar y cargar Arduino.
#ifndef SIMCONNECT_TURBULENCE_ENABLED
#define SIMCONNECT_TURBULENCE_ENABLED 1
#endif
#ifndef SIMCONNECT_STALL_BUFFET_ENABLED
#define SIMCONNECT_STALL_BUFFET_ENABLED 1
#endif
#ifndef SIMCONNECT_GEAR_BUFFET_ENABLED
#define SIMCONNECT_GEAR_BUFFET_ENABLED 1
#endif
#ifndef SIMCONNECT_SPOILER_BUFFET_ENABLED
#define SIMCONNECT_SPOILER_BUFFET_ENABLED 1
#endif
// Buffets: 1 = ruido de banda estrecha; 0 = triangular periodica (anterior).
#ifndef SIMCONNECT_BUFFET_NARROWBAND
#define SIMCONNECT_BUFFET_NARROWBAND 1
#endif
#define SIMCONNECT_BUFFET_DAMPING_RATIO 0.15f // ancho de banda ~2*zeta*f
#define SIMCONNECT_BUFFET_RMS 0.45f           // RMS de la portadora antes de recortar a +-1
// Textura de pista: 1 = ruido de banda estrecha 7/11.3 Hz; 0 = senoides (anterior).
#ifndef SIMCONNECT_GROUND_NARROWBAND_TEXTURE
#define SIMCONNECT_GROUND_NARROWBAND_TEXTURE 1
#endif
// Maximos de cada vibracion (pitch; roll usa la mitad), antes de ganancia total.
#define SIMCONNECT_STALL_BUFFET_GAIN   0.240f
#define SIMCONNECT_GEAR_BUFFET_GAIN    0.160f
#ifndef SIMCONNECT_GEAR_BUFFET_HZ
// Portadora empirica de 10 Hz; el LCD de 10 Hz puede ocultarla por aliasing.
#define SIMCONNECT_GEAR_BUFFET_HZ      10
#endif
static_assert(SIMCONNECT_GEAR_BUFFET_HZ >= 1 && SIMCONNECT_GEAR_BUFFET_HZ <= 20 &&
              SIMCONNECT_GEAR_BUFFET_HZ == (int)SIMCONNECT_GEAR_BUFFET_HZ,
              "Frecuencia del tren: entero entre 1 y 20 Hz");
#define SIMCONNECT_SPOILER_BUFFET_GAIN 0.160f
#define SIMCONNECT_EXTRA_LIMIT        (220.0f / 255.0f) // suma de TODOS los aportes, por eje
static_assert((SIMCONNECT_BUFFET_NARROWBAND == 0 || SIMCONNECT_BUFFET_NARROWBAND == 1) &&
              (SIMCONNECT_GROUND_NARROWBAND_TEXTURE == 0 || SIMCONNECT_GROUND_NARROWBAND_TEXTURE == 1),
              "Banderas ruido 0 o 1");
static_assert(SIMCONNECT_BUFFET_DAMPING_RATIO > 0 && SIMCONNECT_BUFFET_DAMPING_RATIO < 1 &&
              SIMCONNECT_BUFFET_RMS > 0 && SIMCONNECT_BUFFET_RMS <= 1, "Ajustes ruido invalidos");
static_assert((SIMCONNECT_TURBULENCE_ENABLED == 0 || SIMCONNECT_TURBULENCE_ENABLED == 1) &&
              (SIMCONNECT_STALL_BUFFET_ENABLED == 0 || SIMCONNECT_STALL_BUFFET_ENABLED == 1) &&
              (SIMCONNECT_GEAR_BUFFET_ENABLED == 0 || SIMCONNECT_GEAR_BUFFET_ENABLED == 1) &&
              (SIMCONNECT_SPOILER_BUFFET_ENABLED == 0 || SIMCONNECT_SPOILER_BUFFET_ENABLED == 1),
              "Banderas de efectos deben ser 0 o 1");
static_assert(SIMCONNECT_STALL_BUFFET_GAIN >= 0 && SIMCONNECT_STALL_BUFFET_GAIN <= 1 &&
              SIMCONNECT_GEAR_BUFFET_GAIN >= 0 && SIMCONNECT_GEAR_BUFFET_GAIN <= 1 &&
              SIMCONNECT_SPOILER_BUFFET_GAIN >= 0 && SIMCONNECT_SPOILER_BUFFET_GAIN <= 1 &&
              SIMCONNECT_EXTRA_LIMIT >= 0 && SIMCONNECT_EXTRA_LIMIT <= 0.900f,
              "Ganancias de efectos fuera de 0..1");
#define SIMCONNECT_GUST_ROLL  0.350f
#define SIMCONNECT_GUST_PITCH 0.700f
static_assert(SIMCONNECT_ADDITIONS_ENABLED == 0 || SIMCONNECT_ADDITIONS_ENABLED == 1,
              "SimConnect debe ser 0 o 1");
static_assert(SIMCONNECT_GUST_ROLL >= 0 && SIMCONNECT_GUST_ROLL <= 1 &&
              SIMCONNECT_GUST_PITCH >= 0 && SIMCONNECT_GUST_PITCH <= 1,
              "Ganancia de turbulencia fuera de 0..1");

#endif
