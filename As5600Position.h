#pragma once
#include <stdint.h>
#include "mis_defines.h"

namespace YokeAs5600 {
constexpr uint8_t StatusMH=0x08, StatusML=0x10, StatusMD=0x20;
struct Frame { uint16_t raw; uint8_t status; };
inline Frame decode(uint8_t status,uint8_t high,uint8_t low) {
    return {uint16_t(((high & 0x0F) << 8) | low),uint8_t(status & 0x38)};
}
inline int wrapDelta(int delta) {
    while(delta > 2047) delta -= 4096;
    while(delta < -2048) delta += 4096;
    return delta;
}
class Axis {
public:
    Axis(int centerRaw,int countsNeg,int countsPos,int direction)
        : centerRaw(centerRaw), countsNeg(countsNeg), countsPos(countsPos), direction(direction) {}
    void start(uint32_t nowMs) { startMs=nowMs; }
    void update(bool ok,uint16_t raw,uint32_t nowMs) {
        if(!ok) {
            if(failures < 255) ++failures;
            goods=0;
            if(failures >= YOKE_AS5600_FAULT_CYCLES) faulted=true;
            return;
        }
        if(isLocked) return;
        if(!initialized) {
            if(centerRaw < 0 && uint32_t(nowMs-startMs) > YOKE_AS5600_START_WINDOW_MS) {
                isLocked=true;
                return;
            }
            center=centerRaw < 0 ? raw : centerRaw;
            accum=wrapDelta(int(raw)-center);
            held=accum;
            lastRaw=raw;
            lastGoodMs=nowMs;
            failures=0;
            faulted=false;
            initialized=true;
            updatePosition();
            return;
        }
        if(centerRaw >= 0) accum=wrapDelta(int(raw)-center);
        else {
            const bool gapTooLong=uint32_t(nowMs-lastGoodMs) > YOKE_AS5600_MAX_GAP_MS;
            const bool excused=gapExcused;
            gapExcused=false;
            if(gapTooLong && !excused) { isLocked=true; return; }
            accum+=wrapDelta(int(raw)-int(lastRaw));
            if(accum < -YOKE_AS5600_OVERTRAVEL*countsNeg ||
               accum > YOKE_AS5600_OVERTRAVEL*countsPos) { isLocked=true; return; }
        }
        failures=0;
        lastRaw=raw;
        lastGoodMs=nowMs;
        if(faulted && ++goods >= YOKE_AS5600_RECOVER_CYCLES) { faulted=false; goods=0; }
        updatePosition();
    }
    void excuseGap() { gapExcused=true; }
    bool healthy() const { return initialized && !faulted && !isLocked; }
    bool locked() const { return isLocked; }
    float position() const { return value; }
    long counts() const { return direction*held; }
    uint16_t raw() const { return lastRaw; }
private:
    void updatePosition() {
        if(accum > held+YOKE_AS5600_HYSTERESIS_COUNTS) held=accum-YOKE_AS5600_HYSTERESIS_COUNTS;
        if(accum < held-YOKE_AS5600_HYSTERESIS_COUNTS) held=accum+YOKE_AS5600_HYSTERESIS_COUNTS;
        const long signedCounts=direction*held;
        value=float(signedCounts)/(signedCounts < 0 ? countsNeg : countsPos);
        if(value < -1) value=-1;
        if(value > 1) value=1;
    }
    int centerRaw, countsNeg, countsPos, direction, center=0;
    long accum=0, held=0;
    uint32_t startMs=0, lastGoodMs=0;
    uint16_t lastRaw=0, goods=0;
    uint8_t failures=0;
    bool initialized=false, faulted=false, isLocked=false, gapExcused=false;
    float value=0;
};
}
