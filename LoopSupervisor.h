#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#if defined(ARDUINO_ARCH_RP2040)
#include <hardware/structs/watchdog.h>
#endif

namespace YokeHealth {
enum Stage : uint8_t { Idle=0, SerialRx=1, Calibration=2, FlashSave=3, Effects=4,
    Motion=5, Force=6, Motor=7, ModelLog=8, DebugLog=9, Lcd=10, PotRead=11,
    SerialTx=12, TestHang=15 };
constexpr uint32_t kMagic = 0x59C80000u;
constexpr uint8_t kFaultBit = 0x80;
inline uint32_t encode(uint8_t stage) { return kMagic | stage; }
inline bool decode(uint32_t v, uint8_t& stage, bool& fault) {
    if ((v & 0xFFFFFF00u) != kMagic) return false;
    stage = uint8_t(v & 0x7F); fault = (v & kFaultBit) != 0; return true;
}
inline const char* stageName(uint8_t s) {
    switch(s) {
    case Idle: return "INACTIVO";
    case SerialRx: return "SERIE_RX";
    case Calibration: return "CALIB";
    case FlashSave: return "FLASH";
    case Effects: return "EFECTOS";
    case Motion: return "MOVIM";
    case Force: return "FUERZA";
    case Motor: return "MOTOR";
    case ModelLog: return "LOGMOD";
    case DebugLog: return "LOGDBG";
    case Lcd: return "LCD";
    case PotRead: return "POTES";
    case SerialTx: return "SERIE_TX";
    case TestHang: return "PRUEBA";
    default: return "?";
    }
}
inline const char* resetName(int r) {
    switch(r) {
    case 1: return "ENCENDIDO";
    case 2: return "PIN_RUN";
    case 3: return "SOFT";
    case 4: return "WDT";
    case 5: return "DEBUG";
    case 6: return "GLITCH";
    case 7: return "BROWNOUT";
    default: return "DESCONOCIDO";
    }
}
inline int formatBoot(char* out, size_t size, int reason, uint32_t wdReason,
                      uint32_t powmanReason, bool stageValid, uint8_t stage, bool fault) {
    const int n=stageValid ?
        snprintf(out,size,"BOOT reset=%s wd=0x%lx pm=0x%lx etapa=%u:%s falla=%u\n",
            resetName(reason),(unsigned long)wdReason,(unsigned long)powmanReason,
            unsigned(stage),stageName(stage),unsigned(fault)) :
        snprintf(out,size,"BOOT reset=%s wd=0x%lx pm=0x%lx\n",
            resetName(reason),(unsigned long)wdReason,(unsigned long)powmanReason);
    return n>=0 && size_t(n)<size ? n : -1;
}
#if defined(ARDUINO_ARCH_RP2040)
inline void mark(uint8_t s) { watchdog_hw->scratch[0] = encode(s); }
inline uint32_t readScratch() { return watchdog_hw->scratch[0]; }
#else
inline void mark(uint8_t) {}
#endif
}
