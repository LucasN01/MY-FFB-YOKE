#ifndef YOKE_RAW_TELEMETRY_H
#define YOKE_RAW_TELEMETRY_H
#include <stdint.h>
#include <string.h>
#include <math.h>

// SC8 transport only: original SimConnect units, IEEE754 binary32, little endian.
// No force/envelope/pressure-ratio calculations in the Windows bridge.
namespace YokeRaw {
enum Field { PressurePSF, IAS, WindX, WindY, WindZ, OnGround, Slew,
    AoA, StallAlpha, FlapLeft, FlapRight, VS0, VS1, Gear, Retractable,
    SpoilerLeft, SpoilerRight, GroundSpeed, YokeRoll, YokePitch, AccelX, AccelY, Count };
enum Option { Gusts=1, Buffets=2 };
static const uint8_t packetSize=113;
struct Sample {
    uint32_t timeMs=0; // capture time of latest sample, not time each USB byte arrived
    float value[Count]={};
    float referenceKnots=120, gustGain=1, gustLongitudinal=.25f;
    uint8_t options=Buffets;
};
inline uint32_t read32(const uint8_t* p) {
    return uint32_t(p[0]) | (uint32_t(p[1])<<8) | (uint32_t(p[2])<<16) | (uint32_t(p[3])<<24);
}
inline void put32(uint8_t* p,uint32_t n) {
    for(uint8_t i=0;i<4;++i) p[i]=uint8_t(n>>(8*i));
}
inline float readFloat(const uint8_t* p) {
    static_assert(sizeof(float)==4,"SC8 needs binary32 float");
    uint32_t bits=read32(p); float v; memcpy(&v,&bits,4); return v;
}
inline void putFloat(uint8_t* p,float v) {
    uint32_t bits; memcpy(&bits,&v,4); put32(p,bits);
}
inline void encodePayload(uint8_t* p,const Sample& s) {
    put32(p+6,s.timeMs);
    for(uint8_t i=0;i<Count;++i) putFloat(p+10+4*i,s.value[i]);
    putFloat(p+98,s.referenceKnots); putFloat(p+102,s.gustGain);
    putFloat(p+106,s.gustLongitudinal); p[110]=s.options;
}
inline Sample decodePayload(const uint8_t* p) {
    Sample s; s.timeMs=read32(p+6);
    for(uint8_t i=0;i<Count;++i) s.value[i]=readFloat(p+10+4*i);
    s.referenceKnots=readFloat(p+98); s.gustGain=readFloat(p+102);
    s.gustLongitudinal=readFloat(p+106); s.options=p[110]; return s;
}
inline bool range(float v,float low,float high) { return isfinite(v) && v>=low && v<=high; }
inline bool valid(const Sample& s) {
    // Optional aircraft data (including NaN spoilers) is validated per effect on Pico.
    return range(s.value[PressurePSF],0,5000) && isfinite(s.value[IAS]) &&
        isfinite(s.value[WindX]) && isfinite(s.value[WindY]) && isfinite(s.value[WindZ]) &&
        (s.value[OnGround]==0 || s.value[OnGround]==1) && isfinite(s.value[Slew]) &&
        range(s.value[YokeRoll],-1,1) && range(s.value[YokePitch],-1,1) &&
        range(s.referenceKnots,40,250) && range(s.gustGain,0,10) &&
        range(s.gustLongitudinal,-1,1) && (s.options & ~3)==0;
}
}
#endif
