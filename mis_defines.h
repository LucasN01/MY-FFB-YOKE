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
#if defined(ARDUINO)
#include <Arduino.h>
#endif

// LCD 16x2 I2C: Pico 2 GP4/GP5; Pro Micro D2/D3.
// Migracion y conexion electrica: LEEME_PICO2.md.
#define LCD_I2C_ADDRESS 0x27 // tambien prueba 0x3F si 0x27 no responde
#define LCD_REFRESH_MS 100  // nueva lectura cada 100 ms
#define LCD_POWER_ON_WAIT_MS 100 // asentamiento de alimentacion antes de inicializar
#define LCD_RETRY_MS 500        // reintentar si no responde o falla I2C
#define LCD_RESYNC_MS 5000      // recuperar tambien reset del LCD con PCF8574 aun activo
// Menu de ganancias en Pico 2. GPIO (no numeros fisicos del conector).
#ifndef YOKE_CALIBRATION_ENABLED
#if defined(ARDUINO_ARCH_RP2040) || !defined(ARDUINO)
#define YOKE_CALIBRATION_ENABLED 1
#else
#define YOKE_CALIBRATION_ENABLED 0
#endif
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
// 0 = encoders AB; 1 = SimConnect; 2 = potenciometros ADC0/ADC1 de Pico.
#ifndef YOKE_USE_SIMULATION
#define YOKE_USE_SIMULATION 2
#endif
static_assert(YOKE_USE_SIMULATION >= 0 && YOKE_USE_SIMULATION <= 2,
              "YOKE_USE_SIMULATION debe ser 0, 1 o 2");
#if YOKE_USE_SIMULATION == 2 && defined(ARDUINO) && !defined(ARDUINO_ARCH_RP2040)
#error "El modo potenciometros requiere Raspberry Pi Pico/Pico 2 (Arduino-Pico)"
#endif

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
// PINES - ENCODERS (cuadratura, uno por eje)
// ============================================================
// Solo para el cableado AVR descrito en la rama #else:
// Roll usa dos interrupciones; Pitch usa interrupcion en A y lectura de B.
// D2/D3 quedan para el LCD I2C. No utilizar Serial1 en D0/D1.
// Pico 2 usa entradas a 3.3 V e interrupciones en ambos canales de cada eje.
#if defined(ARDUINO_ARCH_RP2040)
// Numeros GPIO, no numeros fisicos del conector. Solo logica de 3.3 V.
#define ROLL_ENC_A 0
#define ROLL_ENC_B 1
#define PITCH_ENC_A 2
#define PITCH_ENC_B 3
#else
#define ROLL_ENC_A   0     // A del encoder Roll -> D0 (RX)
#define ROLL_ENC_B   1     // B del encoder Roll -> D1 (TX)
#define PITCH_ENC_A  7     // A del encoder Pitch -> D7
#define PITCH_ENC_B  4     // B del encoder Pitch -> D4

#endif

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
// (en Leonardo/Micro: 3,5,6,9,10,11,13)
#if defined(ARDUINO_ARCH_RP2040)
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
#else
#define ROLL_PWM_LEFT    9    // TODO-USUARIO
#define ROLL_PWM_RIGHT   10   // TODO-USUARIO
#define PITCH_PWM_LEFT   5    // TODO-USUARIO
#define PITCH_PWM_RIGHT  6    // TODO-USUARIO

// Habilitación del driver, si tu puente H la usa (ej. BTS7960 R_EN/L_EN
// unidos a un solo pin, o un EN general). Si no la usás, dejalo en -1.
#define ROLL_EN_PIN   A2   // LCD I2C: mover habilitacion de roll de D7 a A2
#define PITCH_EN_PIN  8    // TODO-USUARIO (-1 si no aplica)

#define LCD_SDA_PIN SDA
#define LCD_SCL_PIN SCL
#endif

// Límite de PWM real que le mandás al driver (0-255 típico en Arduino).
// Podés limitarlo por debajo de 255 si tu driver/motor lo requiere.
#define MOTOR_PWM_HARD_LIMIT 220 // techo absoluto no modificable desde el menu; no limita corriente
static_assert(MOTOR_PWM_HARD_LIMIT >= 0 && MOTOR_PWM_HARD_LIMIT <= 220, "Techo PWM invalido");
#define PWM_MAX  255   // TODO-USUARIO si tu driver necesita otro techo

// Tiempo de ciclo de control, en milisegundos.
#define CONTROL_LOOP_MS  2   // 2ms = 500Hz, igual orden que el proyecto original
#define YOKE_WDT_MS        250   // reinicio por hardware si loop() no corre en 250 ms
#define YOKE_WDT_FLASH_MS  3000  // durante EEPROM.commit (borrado de sector, max ~400 ms)
#define YOKE_USB_GUARD     1     // serializar CDC con __usb_mutex (correccion H5)
#define YOKE_TEST_HANG     0     // SOLO ENSAYO: 0=no, 1=for(;;), 2=noInterrupts+for(;;), 3=__builtin_trap
#define YOKE_TEST_HANG_MS  60000 // instante del ensayo de cuelgue
#define YOKE_USB_STRESS    0     // SOLO ENSAYO: 1 = escribir linea corta en cada ciclo de 2 ms
static_assert(YOKE_WDT_MS >= 50*CONTROL_LOOP_MS, "WDT demasiado corto para el lazo");

// Velocidad: filtro de primer orden, con el dt real medido en microsegundos.
// 0.018 segundos equivale a alpha=0.90 cuando dt=0.002 segundos.
#define VELOCITY_FILTER_TAU_S 0.018f
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
#define MOTION_TELEMETRY_MS 100

static_assert(ROLL_POS_MIN < 0 && ROLL_POS_MAX > 0, "Revisar limites Roll");
static_assert(PITCH_POS_MIN < 0 && PITCH_POS_MAX > 0, "Revisar limites Pitch");
static_assert(ROLL_POS_MIN >= -32767L && ROLL_POS_MAX <= 32767L &&
              PITCH_POS_MIN >= -32767L && PITCH_POS_MAX <= 32767L,
              "Los limites deben caber en el rango de posicion HID de 16 bits");
static_assert(VELOCITY_FILTER_TAU_S >= 0.0f, "Tau debe ser >= 0");
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
