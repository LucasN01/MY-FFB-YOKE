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
#include "PicoLog.h"
#endif
#if defined(ARDUINO_ARCH_RP2040) && defined(USE_TINYUSB)
// USB CDC (Serial) para SimConnect y diagnostico; no se registra joystick HID.
#include <Adafruit_TinyUSB.h>
#endif
#include "LcdTelemetry.h"
#include <stdio.h>
#include "UsbGuard.h"
#include "LoopSupervisor.h"
#if defined(ARDUINO_ARCH_RP2040)
#if defined(PICO_RP2350) && PICO_RP2350
#include <hardware/structs/powman.h>
#endif
#if defined(__ARM_ARCH_8M_MAIN__)
#include <hardware/structs/io_bank0.h>
#include <hardware/structs/sio.h>
extern "C" uint32_t __StackBottom, __StackTop;
extern "C" __attribute__((used, noinline, noreturn)) void __not_in_flash_func(hardfaultCapture)(uint32_t* frame) {
    // Seguridad primero: EN y luego los 4 PWM como GPIO SIO en 0.
    const uint32_t en = (1u<<ROLL_EN_PIN) | (1u<<PITCH_EN_PIN);
    const uint32_t pwm = (1u<<ROLL_PWM_LEFT) | (1u<<ROLL_PWM_RIGHT) | (1u<<PITCH_PWM_LEFT) | (1u<<PITCH_PWM_RIGHT);
    sio_hw->gpio_clr = en; sio_hw->gpio_oe_set = en;
    sio_hw->gpio_clr = pwm; sio_hw->gpio_oe_set = pwm;
    io_bank0_hw->io[ROLL_PWM_LEFT].ctrl = GPIO_FUNC_SIO; io_bank0_hw->io[ROLL_PWM_RIGHT].ctrl = GPIO_FUNC_SIO;
    io_bank0_hw->io[PITCH_PWM_LEFT].ctrl = GPIO_FUNC_SIO; io_bank0_hw->io[PITCH_PWM_RIGHT].ctrl = GPIO_FUNC_SIO;
    watchdog_hw->scratch[1] = frame[6];
    watchdog_hw->scratch[2] = frame[5];
    watchdog_hw->scratch[3] = *(volatile uint32_t*)0xE000ED28u;
    const uint32_t exc = frame[7] & 0xFFu;
    watchdog_hw->scratch[0] = (watchdog_hw->scratch[0] & 0xFFFF00FFu) | (exc<<8) | YokeHealth::kFaultBit;
    for(;;){}
}
extern "C" __attribute__((naked)) void __not_in_flash_func(isr_hardfault)(void) {
    __asm volatile("tst lr, #4\n ite eq\n mrseq r0, msp\n mrsne r0, psp\n b hardfaultCapture\n");
}
#else
extern "C" void isr_hardfault(void) {
    watchdog_hw->scratch[0] |= YokeHealth::kFaultBit;
    for(;;){}
}
#endif
#endif
#if defined(ARDUINO_ARCH_RP2040) && defined(USE_TINYUSB) && YOKE_USB_GUARD
extern "C" void yield() {
    if (mutex_try_enter(&__usb_mutex, nullptr)) {
        tud_task();
        TinyUSB_Device_FlushCDC();
        mutex_exit(&__usb_mutex);
    }
}
#endif
#if defined(ARDUINO_ARCH_AVR)
#include <avr/pgmspace.h>
#else
#include <pgmspace.h>
#endif

// Fuerzas exclusivamente desde SimConnect. La otra placa envia los ejes al simulador.
YokeGains::Settings gainSettings;
YokePots::Settings potSettings;
#if YOKE_RECORD_OPTION
YokeRecord::Settings recordSettings;
#endif
#if YOKE_CALIBRATION_ENABLED
#if YOKE_RECORD_OPTION
YokeCalibration::Controls calibration(gainSettings,potSettings,&recordSettings);
#else
YokeCalibration::Controls calibration(gainSettings,potSettings);
#endif
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
uint32_t loopWorstUs = 0, gapWorstUs = 0;
int bootReason = 0;
uint32_t bootWd = 0, bootPm = 0;
uint32_t bootPc = 0, bootLr = 0, bootCfsr = 0;
uint8_t bootExc = 0;
uint8_t bootStage = 0;
bool bootStageValid = false, bootFault = false, bootSent = false;

#if YOKE_RECORD_OPTION
YokeHealth::StageTimer stageTimer;
YokeHealth::SlowLog slowLog;
uint32_t prevElapsedUs=0;
inline void markStage(uint8_t s) { YokeHealth::mark(s); stageTimer.enter(s,micros()); }
uint8_t slowFlags() {
    uint8_t flags;
    { UsbLock usb; flags=Serial.dtr() ? 1 : 0; }
    if(simTelemetry.rawAvailable) flags|=2;
#if YOKE_CALIBRATION_ENABLED
    if(calibration.menu.active()) flags|=4;
#endif
    if(simTelemetry.armed) flags|=8;
    return flags;
}
#else
inline void markStage(uint8_t s) { YokeHealth::mark(s); }
#endif
void finishLoop(uint32_t nowUs) {
    const uint32_t elapsedUs=micros()-nowUs;
    if(elapsedUs>loopWorstUs) loopWorstUs=elapsedUs;
#if YOKE_RECORD_OPTION
    stageTimer.enter(YokeHealth::Idle,micros());
    if(elapsedUs>=YOKE_SLOW_LOOP_US)
        slowLog.push({millis(),elapsedUs,0,stageTimer.worstUs,stageTimer.worst,slowFlags()});
    prevElapsedUs=elapsedUs;
#endif
    YokeHealth::mark(YokeHealth::Idle);
}

void debugPrintMotion(unsigned long now, const float* motorCommands) {
#if DEBUG_MOTION_TELEMETRY
    static unsigned long lastPrint = 0;
    static bool printPitch = true; // cada linea empieza por Pitch
    if (now - lastPrint < MOTION_TELEMETRY_MS) return;
    lastPrint = now;
    // En AVR, operator bool de Serial introduce delay(10); dtr() no espera.
    { UsbLock usb; if (!Serial.dtr()) return; }


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
    {
        UsbLock usb;
        if (length > 0 && length < (int)sizeof(line) && Serial.availableForWrite() >= length) {
            YokeHealth::mark(YokeHealth::SerialTx);
            Serial.write((const uint8_t*)line, length);
            printPitch = !printPitch;
        }
    }
#endif
}

void setup() {
    rollMotor.begin();
    pitchMotor.begin();
#if defined(ARDUINO_ARCH_RP2040) && defined(__ARM_ARCH_8M_MAIN__)
    uint32_t* stackCurrent;
    __asm volatile("mrs %0, msp" : "=r"(stackCurrent));
    YokeHealth::paintStack(&__StackBottom,stackCurrent,0xA5A5A5A5u);
#endif
#if defined(ARDUINO_ARCH_RP2040)
    bootReason=rp2040.getResetReason();
    bootWd=watchdog_hw->reason;
#if defined(PICO_RP2350) && PICO_RP2350
    bootPm=powman_hw->chip_reset;
#endif
    bootStageValid=YokeHealth::decode(YokeHealth::readScratch(),bootStage,bootFault);
    if(bootFault) {
        bootPc=watchdog_hw->scratch[1]; bootLr=watchdog_hw->scratch[2]; bootCfsr=watchdog_hw->scratch[3];
        bootExc=YokeHealth::decodeException(YokeHealth::readScratch());
    }
    watchdog_hw->scratch[1]=watchdog_hw->scratch[2]=watchdog_hw->scratch[3]=0;
    YokeHealth::mark(YokeHealth::Idle);
    analogWriteRange(255);
    analogWriteFreq(MOTOR_PWM_HZ);
#endif
#if defined(ARDUINO_ARCH_RP2040) && defined(USE_TINYUSB)
    // TinyUSB usa 239A:CAFE por defecto; el lanzador busca la Pico 2 como 2E8A:000F.
    TinyUSBDevice.setID(0x2E8A, 0x000F);
    if (TinyUSBDevice.mounted()) { // ya enumerada con el ID anterior: re-enumerar
        TinyUSBDevice.detach();
        delay(10);
        TinyUSBDevice.attach();
    }
#endif
#if DEBUG_MOTION_TELEMETRY || SIMCONNECT_ADDITIONS_ENABLED
    Serial.begin(115200); // USB CDC: no esperar la conexion del monitor
#endif
    lcd.begin(); // programar inicio; LCD se inicializa por etapas desde update()
#if YOKE_USE_SIMULATION != 1
    positionInput.begin(); // encoders: centrar al iniciar; potes: centro calibrado
#endif
    lastControlLoopUs = micros();
#if YOKE_CALIBRATION_ENABLED
    calibration.begin(); // cargar ganancias guardadas; arrancar siempre en modo normal
#endif
#if defined(ARDUINO_ARCH_RP2040)
    rp2040.wdt_begin(YOKE_WDT_MS);
#endif
}

void loop() {
#if defined(ARDUINO_ARCH_RP2040)
    rp2040.wdt_reset();
#endif
    const uint32_t nowUs = micros();
    if (nowUs - lastControlLoopUs < CONTROL_LOOP_MS * 1000UL) return;
    const uint32_t gapUs=nowUs-lastControlLoopUs;
    if(gapUs>gapWorstUs) gapWorstUs=gapUs;
    lastControlLoopUs = nowUs;
#if YOKE_RECORD_OPTION
    const uint32_t idleUs=gapUs-prevElapsedUs;
    if(YokeHealth::slowBetween(gapUs,prevElapsedUs,YOKE_SLOW_LOOP_US))
        slowLog.push({millis(),0,idleUs,idleUs,YokeHealth::Between,slowFlags()});
    stageTimer.begin(nowUs);
#endif
    const unsigned long now = millis();

#if YOKE_USE_SIMULATION == 2
    markStage(YokeHealth::PotRead);
    positionInput.update(); // muestra actual ANTES de procesar OK de calibracion
#if YOKE_CALIBRATION_ENABLED
    calibration.menu.potReadings(positionInput.raw(0),positionInput.raw(1));
#endif
#endif

#if SIMCONNECT_ADDITIONS_ENABLED
    // USB CDC del mismo cable; no usa los pines de Serial1 de los encoders.
    // Limitar trabajo por ciclo y nunca esperar a que llegue un paquete.
    markStage(YokeHealth::SerialRx);
    {
        UsbLock usb;
        for (uint8_t n = 0; n < 128 && Serial.available(); ++n) {
            const uint8_t b = Serial.read();
            if (b == '?' && simTelemetry.idle()) {
                if (Serial.availableForWrite() >= 10) {
                    markStage(YokeHealth::SerialTx);
                    Serial.write("YOKE17SC8\n", 10);
                    markStage(YokeHealth::SerialRx);
                }
            } else simTelemetry.feed(b, now);
        }
    }
    bool dtr;
    { UsbLock usb; dtr=Serial.dtr(); }
#endif

#if SIMCONNECT_ADDITIONS_ENABLED
#if YOKE_CALIBRATION_ENABLED
    markStage(YokeHealth::Calibration);
    calibration.update(now,simTelemetry.calibrationAllowed(now,dtr));
    if(calibration.menu.savePending()) {
        // La escritura flash bloquea brevemente CPU/interrupciones: PWM cero antes.
        rollMotor.stop(); pitchMotor.stop();
        markStage(YokeHealth::FlashSave);
#if defined(ARDUINO_ARCH_RP2040)
        rp2040.wdt_begin(YOKE_WDT_FLASH_MS);
#endif
        calibration.save();
#if defined(ARDUINO_ARCH_RP2040)
        rp2040.wdt_begin(YOKE_WDT_MS);
#endif
#if YOKE_USE_SIMULATION != 1
        localMotion[0].reset(); localMotion[1].reset();
#endif
        simEffects.reset();
        finishLoop(nowUs);
        return; // recalcular fuerza con hora/telemetria actuales en el proximo ciclo
    }
#endif
    const uint32_t effectsStartUs=micros();
    markStage(YokeHealth::Effects);
    simEffects.update(simTelemetry, now, dtr);
    const uint32_t effectsElapsedUs=micros()-effectsStartUs;
    if(effectsElapsedUs>effectsWorstUs) effectsWorstUs=effectsElapsedUs;
#endif

#if YOKE_CALIBRATION_ENABLED && !SIMCONNECT_ADDITIONS_ENABLED
    markStage(YokeHealth::Calibration);
    calibration.update(now,true); // standalone configuration; simulator effects compiled out
    if(calibration.menu.savePending()) {
        rollMotor.stop(); pitchMotor.stop();
        markStage(YokeHealth::FlashSave);
#if defined(ARDUINO_ARCH_RP2040)
        rp2040.wdt_begin(YOKE_WDT_FLASH_MS);
#endif
        calibration.save();
#if defined(ARDUINO_ARCH_RP2040)
        rp2040.wdt_begin(YOKE_WDT_MS);
#endif
#if YOKE_USE_SIMULATION != 1
        localMotion[0].reset(); localMotion[1].reset();
#endif
        finishLoop(nowUs);
        return;
    }
#endif

    // Una sola fuente para posicion y velocidad de TODOS los efectos.
    markStage(YokeHealth::Motion);
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
    markStage(YokeHealth::Force);
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
    markStage(YokeHealth::Motor);
    rollMotor.setCommand(ROLL_MOTOR_DIRECTION * motorCommandRoll, PWM_MAX);
    pitchMotor.setCommand(PITCH_MOTOR_DIRECTION * motorCommandPitch, PWM_MAX);
    const int16_t pwmRoll = rollMotor.getLastAppliedPWM();
    const int16_t pwmPitch = pitchMotor.getLastAppliedPWM();

#if SIMCONNECT_ADDITIONS_ENABLED
    if(!bootSent && simTelemetry.rawAvailable && dtr) {
        char bootLine[160]; bootLine[0]='\n';
        const int n=YokeHealth::formatBoot(bootLine+1,sizeof(bootLine)-1,bootReason,
            bootWd,bootPm,bootStageValid,bootStage,bootFault,bootExc,bootPc,bootLr,bootCfsr);
        UsbLock usb;
        if(n>0 && Serial.availableForWrite()>=n+1) {
            markStage(YokeHealth::SerialTx);
            Serial.write((const uint8_t*)bootLine,n+1); markStage(YokeHealth::ModelLog); bootSent=true;
        }
    }
    markStage(YokeHealth::ModelLog);
#if YOKE_MODEL_LOG
    static uint16_t seq=0, drop=0, gainSeq=0, lastBits=0, lastSkip=0;
    static uint32_t lastLoggedRev=0, lastLineMs=0, lastGainsMs=0;
    static bool logSynced=false, lastDtr=false, gainsSent=false;
    static bool recordSent=false, lastRecordSent=true;
    static uint32_t lastRecordMs=0, lastSlowMs=0;
    static uint16_t slowSeq=0;
#if defined(ARDUINO_ARCH_RP2040) && defined(__ARM_ARCH_8M_MAIN__)
    static uint32_t lastStackCheckMs=0, lastStackMs=0, lastStackBytes=0xFFFFFFFFu, stackBytes=0xFFFFFFFFu;
    if(uint32_t(now-lastStackCheckMs)>=1000) {
        stackBytes=YokeHealth::freeStackBytes(&__StackBottom,&__StackTop,0xA5A5A5A5u);
        lastStackCheckMs=now;
    }
#endif
    static int16_t peak[2]={0,0};
    static uint16_t lastGains[YokeGains::Count]={0};
    const int16_t pwm[2]={pwmRoll,pwmPitch};
    for(uint8_t i=0;i<2;++i) {
        const int16_t magnitude=pwm[i]<0 ? -pwm[i] : pwm[i];
        if(magnitude>peak[i]) peak[i]=magnitude;
    }
    if(dtr && !lastDtr) {
        logSynced=false; recordSent=false; gainsSent=false;
#if defined(ARDUINO_ARCH_RP2040) && defined(__ARM_ARCH_8M_MAIN__)
        lastStackBytes=0xFFFFFFFFu;
#endif
    }
    if(recordSettings.enabled && !lastRecordSent) { gainsSent=false; logSynced=false; peak[0]=peak[1]=0; }
    lastDtr=dtr;
    const uint32_t r=simEffects.modeledRevision();
    if(r==0) lastLoggedRev=0;
    if(!logSynced) { lastLoggedRev=r; lastSkip=simEffects.skipped(); logSynced=true; }
    bool modelWritten=false;
    const bool logReady=dtr && simTelemetry.rawAvailable && uint32_t(now-simTelemetry.last)<300;
    if(logReady) {
        UsbLock usb;
        if(Serial.availableForWrite()>=YOKE_LOG_M_MAX) {
            char line[YOKE_LOG_M_MAX]; YokeHealth::SlowEvent event;
            if(slowLog.front(event) && uint32_t(now-lastSlowMs)>=20) {
                const int n=YokeHealth::formatSlow(line,sizeof(line),slowSeq,millis()-event.atMs,event,slowLog.total,slowLog.lost);
                if(n>0) {
                    markStage(YokeHealth::SerialTx); Serial.write((const uint8_t*)line,n);
                    markStage(YokeHealth::ModelLog);
                    slowLog.pop(); ++slowSeq; lastSlowMs=now; modelWritten=true;
                }
#if defined(ARDUINO_ARCH_RP2040) && defined(__ARM_ARCH_8M_MAIN__)
            } else if(stackBytes!=0xFFFFFFFFu && (stackBytes<lastStackBytes || uint32_t(now-lastStackMs)>=60000)) {
                const int n=snprintf(line,sizeof(line),"\nS,%lu\n",(unsigned long)stackBytes);
                if(n>0 && n<int(sizeof(line))) {
                    markStage(YokeHealth::SerialTx); Serial.write((const uint8_t*)line,n);
                    markStage(YokeHealth::ModelLog);
                    lastStackBytes=stackBytes; lastStackMs=now; modelWritten=true;
                }
#endif
            } else if(!recordSent || recordSettings.enabled!=lastRecordSent || uint32_t(now-lastRecordMs)>=5000) {
                const int n=YokeLog::formatRecord(line,sizeof(line),recordSettings.enabled);
                if(n>0) {
                    markStage(YokeHealth::SerialTx); Serial.write((const uint8_t*)line,n);
                    markStage(YokeHealth::ModelLog);
                    recordSent=true; lastRecordSent=recordSettings.enabled; lastRecordMs=now; modelWritten=true;
                }
            }
        }
    }
    if(!modelWritten && logReady && recordSettings.enabled && recordSent && (r!=lastLoggedRev || uint32_t(now-lastLineMs)>=100)) {
        UsbLock usb;
        if(Serial.availableForWrite()>=YOKE_LOG_M_MAX) {
            const uint16_t nextDrop=YokeLog::nextDrop(drop,r,lastLoggedRev,simEffects.skipped(),lastSkip);
            YokeLog::Frame f{};
            f.seq=seq; f.t=simTelemetry.raw.timeMs; f.rev=r;
            f.flags=uint8_t((simEffects.active() ? YokeLog::Active : 0) |
                (simTelemetry.armed ? YokeLog::Armed : 0) | (simTelemetry.isPaused() ? YokeLog::Paused : 0) |
                (dtr ? YokeLog::Dtr : 0) | (simTelemetry.rawAvailable ? YokeLog::Raw : 0) |
                (r!=0 ? YokeLog::Modeled : 0));
#if YOKE_CALIBRATION_ENABLED
            if(calibration.menu.active()) f.flags|=YokeLog::Menu;
#endif
            const auto& model=simEffects.calculated();
            f.q=model.pressure; f.env=model.groundEnvelope; f.kc=simEffects.centering();
            for(uint8_t i=0;i<3;++i) f.buffet[i]=model.buffet[i];
            for(uint8_t i=0;i<2;++i) {
                f.gust[i]=model.gust[i]; f.ground[i]=model.ground[i];
                f.pos[i]=yokePosition[i]; f.vel[i]=yokeVelocity[i]; f.cmd[i]=motorCommands[i];
                f.pwm[i]=pwm[i]; f.peak[i]=peak[i];
            }
            f.dropped=nextDrop; f.skip=simEffects.skipped();
            char line[YOKE_LOG_M_MAX];
            const int n=YokeLog::formatModel(line,sizeof(line),f);
            if(n>0) {
                markStage(YokeHealth::SerialTx);
                Serial.write((const uint8_t*)line,n);
                markStage(YokeHealth::ModelLog);
                ++seq; drop=nextDrop; if(r!=0) lastLoggedRev=r;
                lastSkip=simEffects.skipped();
                lastLineMs=now; peak[0]=peak[1]=0; modelWritten=true;
            }
        }
    }
    if(!modelWritten && logReady && recordSettings.enabled && recordSent) {
        uint16_t raw[YokeGains::Count];
        const uint16_t bits=gainSettings.effects();
        bool changed=bits!=lastBits;
        for(uint8_t i=0;i<YokeGains::Count;++i) {
            raw[i]=gainSettings.raw(YokeGains::Id(i));
            if(raw[i]!=lastGains[i]) changed=true;
        }
        if(!gainsSent || changed || uint32_t(now-lastGainsMs)>=5000) {
            UsbLock usb;
            if(Serial.availableForWrite()>=YOKE_LOG_M_MAX) {
                char line[YOKE_LOG_M_MAX];
                const int n=YokeLog::formatGains(line,sizeof(line),gainSeq,bits,raw,YokeGains::Count);
                if(n>0) {
                    markStage(YokeHealth::SerialTx);
                    Serial.write((const uint8_t*)line,n);
                    markStage(YokeHealth::ModelLog);
                    ++gainSeq; gainsSent=true; lastGainsMs=now; lastBits=bits;
                    for(uint8_t i=0;i<YokeGains::Count;++i) lastGains[i]=raw[i];
                }
            }
        }
    }
#endif
    // Nonblocking feedback from the actual Pico, recorded by the bridge in pico.log.
    // gr/gp are calculated signals x10000 before gains; R/P are final applied PWM.
    static uint32_t lastModelLog=0;
    if(simTelemetry.rawAvailable && dtr && uint32_t(now-lastModelLog)>=100) {
        char modelLine[128];
        const auto& model=simEffects.calculated();
        const int n=snprintf(modelLine,sizeof(modelLine),"\nSC8 t=%lu gr=%d gp=%d R=%d P=%d us=%lu lp=%lu gap=%lu\n",
            (unsigned long)simTelemetry.raw.timeMs,int(model.ground[0]*10000),
            int(model.ground[1]*10000),int(pwmRoll),int(pwmPitch),(unsigned long)effectsWorstUs,
            (unsigned long)loopWorstUs,(unsigned long)gapWorstUs);
        UsbLock usb;
        if(n>0 && n<int(sizeof(modelLine)) && Serial.availableForWrite()>=n) {
            markStage(YokeHealth::SerialTx);
            Serial.write((const uint8_t*)modelLine,n); markStage(YokeHealth::ModelLog); lastModelLog=now;
        }
    }
#endif

    // 4) Diagnostico serie y pantalla locales; no se envian ejes a Prepar3D.
    markStage(YokeHealth::DebugLog);
    debugPrintMotion(now, motorCommands);
    markStage(YokeHealth::Lcd);
#if YOKE_CALIBRATION_ENABLED
    char menuTop[17], menuBottom[17];
    calibration.menu.rows(menuTop,menuBottom);
    lcd.showMenu(calibration.menu.displayActive(),menuTop,menuBottom);
#endif
    lcd.update(now, motorCommandRoll, motorCommandPitch, pwmRoll, pwmPitch);
#if YOKE_USB_STRESS > 0
    {
        UsbLock usb;
        if(Serial.availableForWrite()>=2) {
            markStage(YokeHealth::SerialTx);
            Serial.write("S\n",2);
        }
    }
#endif
#if defined(ARDUINO_ARCH_RP2040) && YOKE_TEST_HANG > 0
    static bool testHung=false;
    if(!testHung && now>=YOKE_TEST_HANG_MS && bootReason!=RP2040::WDT_RESET) {
        testHung=true;
        markStage(YokeHealth::TestHang);
#if YOKE_TEST_HANG == 1
        for(;;){}
#elif YOKE_TEST_HANG == 2
        noInterrupts(); for(;;){}
#elif YOKE_TEST_HANG == 3
        __builtin_trap();
#endif
    }
#endif
    finishLoop(nowUs);
}
