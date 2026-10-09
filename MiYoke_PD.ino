#include "mis_defines.h"
#if YOKE_USE_SIMULATION == 0
#include "AxisEncoders.h"
#elif YOKE_USE_SIMULATION == 2
#include "AxisPotentiometers.h"
#endif
#include "YokeMotion.h"
#include "MotorDriver.h"
#include "GainSettings.h"
#include "CalibrationControls.h"
#if SIMCONNECT_ADDITIONS_ENABLED
#include "SimEffects.h"
#endif
#if defined(ARDUINO_ARCH_RP2040) && defined(USE_TINYUSB)
// USB CDC (Serial) para SimConnect y diagnostico; no se registra joystick HID.
#include <Adafruit_TinyUSB.h>
#endif
#include "LcdTelemetry.h"
#include <stdio.h>
#if defined(ARDUINO_ARCH_AVR)
#include <avr/pgmspace.h>
#else
#include <pgmspace.h>
#endif

// Fuerzas exclusivamente desde SimConnect. La otra placa envia los ejes al simulador.
YokeGains::Settings gainSettings;
YokePots::Settings potSettings;
#if YOKE_CALIBRATION_ENABLED
YokeCalibration::Controls calibration(gainSettings,potSettings);
#endif
#if SIMCONNECT_ADDITIONS_ENABLED
YokeSC::Receiver simTelemetry;
YokeSC::Effects simEffects(gainSettings);
#endif
#if YOKE_USE_SIMULATION == 0
AxisEncoders positionInput;
#elif YOKE_USE_SIMULATION == 2
AxisPotentiometers positionInput(potSettings);
#endif
#if YOKE_USE_SIMULATION != 1
YokeMotion localMotion[2];
#endif
float yokePosition[2]={0,0}, yokeVelocity[2]={0,0};
LcdTelemetry lcd;

MotorDriver rollMotor(ROLL_PWM_LEFT, ROLL_PWM_RIGHT, ROLL_EN_PIN);
MotorDriver pitchMotor(PITCH_PWM_LEFT, PITCH_PWM_RIGHT, PITCH_EN_PIN);


uint32_t lastControlLoopUs = 0;
uint32_t effectsWorstUs = 0;

void debugPrintMotion(unsigned long now, const float* motorCommands) {
#if DEBUG_MOTION_TELEMETRY
    static unsigned long lastPrint = 0;
    static bool printPitch = true; // cada linea empieza por Pitch
    if (now - lastPrint < MOTION_TELEMETRY_MS) return;
    lastPrint = now;
    // En AVR, operator bool de Serial introduce delay(10); dtr() no espera.
    if (!Serial.dtr()) return;


    const uint8_t axis = printPitch ? 1 : 0;
    const int16_t pwm = printPitch ? pitchMotor.getLastAppliedPWM() : rollMotor.getLastAppliedPWM();
    char line[64];
#if YOKE_USE_SIMULATION == 2
    const int length = snprintf_P(
        line, sizeof(line), PSTR("%c a=%u x=%ld v=%ld f=%d p=%d%s"),
        printPitch ? 'P' : 'R', (unsigned)positionInput.raw(axis),
        (long)(yokePosition[axis] * 10000.0f),
        (long)(yokeVelocity[axis] * 1000.0f),
        (int)(motorCommands[axis] * 10000.0f), (int)pwm,
        printPitch ? " | " : "\n");
#else
    // v: velocidad en milesimas/s. f: comando SimConnect en 1/10000.
    // p: PWM final con polaridad.
    // Linea compacta para caber en el buffer USB sin bloquear.
    const int length = snprintf_P(
        line, sizeof(line), PSTR("%c x=%ld v=%ld f=%d p=%d%s"),
        printPitch ? 'P' : 'R',
        (long)(yokePosition[axis] * 10000.0f),
        (long)(yokeVelocity[axis] * 1000.0f),
        (int)(motorCommands[axis] * 10000.0f),
        (int)pwm,
        printPitch ? " | " : "\n"
    );
#endif
    // Enviar un eje por vez: Pitch agrega separador, Roll cierra la linea.
    // Cada fragmento cabe en un paquete USB. No esperar si el host no recibe.
    if (length > 0 && length < (int)sizeof(line) && Serial.availableForWrite() >= length) {
        Serial.write((const uint8_t*)line, length);
        printPitch = !printPitch;
    }
#endif
}

void setup() {
#if defined(ARDUINO_ARCH_RP2040)
    analogWriteRange(255);
    analogWriteFreq(MOTOR_PWM_HZ);
#endif
#if DEBUG_MOTION_TELEMETRY || SIMCONNECT_ADDITIONS_ENABLED
    Serial.begin(115200); // USB CDC: no esperar la conexion del monitor
#endif
    lcd.begin(); // programar inicio; LCD se inicializa por etapas desde update()
#if YOKE_USE_SIMULATION != 1
    positionInput.begin(); // encoders: centrar al iniciar; potes: centro calibrado
#endif
    lastControlLoopUs = micros();
    rollMotor.begin();
    pitchMotor.begin();
#if YOKE_CALIBRATION_ENABLED
    calibration.begin(); // cargar ganancias guardadas; arrancar siempre en modo normal
#endif

}

void loop() {
    const uint32_t nowUs = micros();
    if (nowUs - lastControlLoopUs < CONTROL_LOOP_MS * 1000UL) return;
    lastControlLoopUs = nowUs;
    const unsigned long now = millis();

#if YOKE_USE_SIMULATION == 2
    positionInput.update(); // muestra actual ANTES de procesar OK de calibracion
#if YOKE_CALIBRATION_ENABLED
    calibration.menu.potReadings(positionInput.raw(0),positionInput.raw(1));
#endif
#endif

#if SIMCONNECT_ADDITIONS_ENABLED
    // USB CDC del mismo cable; no usa los pines de Serial1 de los encoders.
    // Limitar trabajo por ciclo y nunca esperar a que llegue un paquete.
    for (uint8_t n = 0; n < 128 && Serial.available(); ++n) {
        const uint8_t b = Serial.read();
        if (b == '?' && simTelemetry.idle()) {
            if (Serial.availableForWrite() >= 10) Serial.write("YOKE17SC8\n", 10);
        } else simTelemetry.feed(b, now);
    }
#endif

#if SIMCONNECT_ADDITIONS_ENABLED
#if YOKE_CALIBRATION_ENABLED
    calibration.update(now,simTelemetry.calibrationAllowed(now,Serial.dtr()));
    if(calibration.menu.savePending()) {
        // La escritura flash bloquea brevemente CPU/interrupciones: PWM cero antes.
        rollMotor.stop(); pitchMotor.stop();
        calibration.save();
#if YOKE_USE_SIMULATION != 1
        localMotion[0].reset(); localMotion[1].reset();
#endif
        simEffects.reset();
        return; // recalcular fuerza con hora/telemetria actuales en el proximo ciclo
    }
#endif
    const uint32_t effectsStartUs=micros();
    simEffects.update(simTelemetry, now, Serial.dtr());
    const uint32_t effectsElapsedUs=micros()-effectsStartUs;
    if(effectsElapsedUs>effectsWorstUs) effectsWorstUs=effectsElapsedUs;
#endif

#if YOKE_CALIBRATION_ENABLED && !SIMCONNECT_ADDITIONS_ENABLED
    calibration.update(now,true); // standalone configuration; simulator effects compiled out
    if(calibration.menu.savePending()) {
        rollMotor.stop(); pitchMotor.stop(); calibration.save();
#if YOKE_USE_SIMULATION != 1
        localMotion[0].reset(); localMotion[1].reset();
#endif
        return;
    }
#endif

    // Una sola fuente para posicion y velocidad de TODOS los efectos.
#if YOKE_USE_SIMULATION == 1
    for(uint8_t axis=0;axis<2;++axis) {
#if SIMCONNECT_ADDITIONS_ENABLED
        yokePosition[axis]=simTelemetry.yoke[axis];
        yokeVelocity[axis]=simEffects.simulationVelocity(axis);
#else
        yokePosition[axis]=yokeVelocity[axis]=0;
#endif
    }
#else
#if YOKE_USE_SIMULATION == 0
    const long counts[2]={positionInput.readRollRaw(),positionInput.readPitchRaw()};
    const float negative[2]={-float(ROLL_POS_MIN),-float(PITCH_POS_MIN)};
    const float positive[2]={float(ROLL_POS_MAX),float(PITCH_POS_MAX)};
    const float positions[2]={counts[0]/(counts[0]<0 ? negative[0] : positive[0]),
                              counts[1]/(counts[1]<0 ? negative[1] : positive[1])};
#else
    const float positions[2]={positionInput.position(0),positionInput.position(1)};
#endif
    for(uint8_t axis=0;axis<2;++axis) {
        localMotion[axis].update(positions[axis],nowUs);
        yokePosition[axis]=localMotion[axis].position;
        yokeVelocity[axis]=localMotion[axis].velocity;
    }
#endif
    // 2) Unica fuente de fuerza: telemetria SimConnect valida y armada.
    float motorCommands[2] = {0.0f, 0.0f};
#if SIMCONNECT_ADDITIONS_ENABLED
    for (uint8_t axis = 0; axis < 2; ++axis) {
        motorCommands[axis] = simEffects.commandWithPositionSource(simTelemetry, axis,
            gainSettings.get(YokeGains::Total),
            yokePosition[axis], YokeSC::configuredEffects, YOKE_USE_SIMULATION == 1, yokeVelocity[axis]);
    }
#endif
#if YOKE_CALIBRATION_ENABLED
    if(calibration.menu.active()) motorCommands[0]=motorCommands[1]=0;
#endif
    const float motorCommandRoll = motorCommands[0];
    const float motorCommandPitch = motorCommands[1];

    // 3) Aplicar polaridad y convertir -1..1 a PWM, respetando PWM_MAX.
    rollMotor.setCommand(ROLL_MOTOR_DIRECTION * motorCommandRoll, PWM_MAX);
    pitchMotor.setCommand(PITCH_MOTOR_DIRECTION * motorCommandPitch, PWM_MAX);
    const int16_t pwmRoll = rollMotor.getLastAppliedPWM();
    const int16_t pwmPitch = pitchMotor.getLastAppliedPWM();

#if SIMCONNECT_ADDITIONS_ENABLED
    // Nonblocking feedback from the actual Pico, recorded by the bridge in pico.log.
    // gr/gp are calculated signals x10000 before gains; R/P are final applied PWM.
    static uint32_t lastModelLog=0;
    if(simTelemetry.rawAvailable && Serial.dtr() && uint32_t(now-lastModelLog)>=100) {
        char modelLine[96];
        const auto& model=simEffects.calculated();
        const int n=snprintf(modelLine,sizeof(modelLine),"\nSC8 t=%lu gr=%d gp=%d R=%d P=%d us=%lu\n",
            (unsigned long)simTelemetry.raw.timeMs,int(model.ground[0]*10000),
            int(model.ground[1]*10000),int(pwmRoll),int(pwmPitch),(unsigned long)effectsWorstUs);
        if(n>0 && n<int(sizeof(modelLine)) && Serial.availableForWrite()>=n) {
            Serial.write((const uint8_t*)modelLine,n); lastModelLog=now;
        }
    }
#endif

    // 4) Diagnostico serie y pantalla locales; no se envian ejes a Prepar3D.
    debugPrintMotion(now, motorCommands);
#if YOKE_CALIBRATION_ENABLED
    char menuTop[17], menuBottom[17];
    calibration.menu.rows(menuTop,menuBottom);
    lcd.showMenu(calibration.menu.displayActive(),menuTop,menuBottom);
#endif
    lcd.update(now, motorCommandRoll, motorCommandPitch, pwmRoll, pwmPitch);
    
}
