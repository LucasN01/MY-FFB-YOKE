#pragma once
#include "PotentiometerPosition.h"
#include "SimTelemetry.h"

namespace YokePots {
struct Axis { uint16_t negative, center, positive; };
struct Settings {
    Axis axes[2] = {
        {ROLL_POT_ADC_NEG, ROLL_POT_ADC_CENTER, ROLL_POT_ADC_POS},
        {PITCH_POT_ADC_NEG, PITCH_POT_ADC_CENTER, PITCH_POT_ADC_POS}
    };
};
// Rechazar extremos iguales o un recorrido comparable al ruido del ADC.
constexpr int minimumSpan = 64;
inline bool valid(const Axis& axis) {
    const int span = int(axis.positive) - int(axis.negative);
    return validPotCalibration(axis.negative,axis.center,axis.positive) &&
        (span >= minimumSpan || span <= -minimumSpan);
}
inline bool valid(const Settings& settings) { return valid(settings.axes[0]) && valid(settings.axes[1]); }
inline Axis endpoints(uint16_t negative,uint16_t positive) {
    return {negative,uint16_t((uint32_t(negative)+positive)/2),positive};
}
// Registro independiente: conservar ganancias y efectos de firmwares anteriores.
constexpr uint16_t recordOffset = 64;
constexpr uint8_t recordSize = 20;
inline void encode(const Settings& settings,uint8_t* bytes) {
    bytes[0]='Y'; bytes[1]='K'; bytes[2]='P'; bytes[3]='T';
    YokeSC::put16(bytes+4,1);
    for(uint8_t axis=0;axis<2;++axis) {
        const Axis& value=settings.axes[axis];
        YokeSC::put16(bytes+6+axis*6,value.negative);
        YokeSC::put16(bytes+8+axis*6,value.center);
        YokeSC::put16(bytes+10+axis*6,value.positive);
    }
    YokeSC::put16(bytes+18,YokeSC::crc(bytes,18));
}
inline bool decode(const uint8_t* bytes,Settings& settings) {
    if(bytes[0]!='Y'||bytes[1]!='K'||bytes[2]!='P'||bytes[3]!='T'||
       YokeSC::read16(bytes+4)!=1||YokeSC::read16(bytes+18)!=YokeSC::crc(bytes,18)) return false;
    Settings candidate;
    for(uint8_t axis=0;axis<2;++axis)
        candidate.axes[axis]={YokeSC::read16(bytes+6+axis*6),YokeSC::read16(bytes+8+axis*6),
                              YokeSC::read16(bytes+10+axis*6)};
    if(!valid(candidate)) return false;
    settings=candidate; return true;
}
}
