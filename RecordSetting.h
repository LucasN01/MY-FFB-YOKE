#ifndef YOKE_RECORD_SETTING_H
#define YOKE_RECORD_SETTING_H
#include "PotCalibration.h"
#include "SimTelemetry.h"
namespace YokeRecord {
struct Settings { bool enabled=true; };
constexpr uint16_t recordOffset=96;
constexpr uint8_t recordSize=8;
static_assert(YokePots::recordOffset+YokePots::recordSize<=recordOffset && recordOffset+recordSize<=256,
    "Registro fuera de EEPROM");
inline void encode(const Settings& settings,uint8_t* out) {
    out[0]='Y'; out[1]='R'; out[2]='E'; out[3]='C'; out[4]=1; out[5]=settings.enabled ? 1 : 0;
    YokeSC::put16(out+6,YokeSC::crc(out,6));
}
inline bool decode(const uint8_t* in,Settings& out) {
    if(in[0]!='Y' || in[1]!='R' || in[2]!='E' || in[3]!='C' || in[4]!=1 || in[5]>1 ||
       YokeSC::read16(in+6)!=YokeSC::crc(in,6)) return false;
    out.enabled=in[5]!=0; return true;
}
}
#endif
