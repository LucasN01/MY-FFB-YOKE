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
    SerialTx=12, Between=14, TestHang=15 };
struct StageTimer {
    uint8_t current=Idle, worst=Idle; uint32_t since=0, worstUs=0;
    void begin(uint32_t us) { current=Idle; since=us; worst=Idle; worstUs=0; }
    void enter(uint8_t stage,uint32_t us) {
        const uint32_t d=us-since;
        if(d>worstUs) { worstUs=d; worst=current; }
        current=stage; since=us;
    }
};
struct SlowEvent { uint32_t atMs, lpUs, idleUs, stageUs; uint8_t stage, flags; };
inline bool slowBetween(uint32_t gapUs,uint32_t prevElapsedUs,uint32_t thresholdUs) {
    return gapUs>=thresholdUs && prevElapsedUs<thresholdUs;
}
struct SlowLog {
    SlowEvent e[4]; uint8_t head=0, count=0; uint16_t total=0, lost=0;
    void push(const SlowEvent& event) {
        if(total<65535) ++total;
        if(count==4) { head=(head+1)%4; --count; if(lost<65535) ++lost; }
        e[(head+count)%4]=event; ++count;
    }
    bool front(SlowEvent& event) const { if(!count) return false; event=e[head]; return true; }
    void pop() { if(count) { head=(head+1)%4; --count; } }
};
inline int formatSlow(char* out,size_t size,uint16_t seq,uint32_t ageMs,const SlowEvent& e,uint16_t total,uint16_t lost) {
    const int n=snprintf(out,size,"\nW,%u,%lu,%lu,%lu,%u,%lu,%u,%u,%u\n",unsigned(seq),
        (unsigned long)ageMs,(unsigned long)e.lpUs,(unsigned long)e.idleUs,unsigned(e.stage),
        (unsigned long)e.stageUs,unsigned(e.flags),unsigned(total),unsigned(lost));
    return n>=0 && size_t(n)<size ? n : -1;
}
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
    case Between: return "ENTRE_CICLOS";
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
