#ifndef YOKE_GAIN_SETTINGS_H
#define YOKE_GAIN_SETTINGS_H
#include "mis_defines.h"
#include "SimTelemetry.h"

namespace YokeGains {
enum Id : uint8_t { Total, CenterPitch, CenterRest, GustRoll, GustPitch,
                   Ground, Stall, Gear, Spoiler, Damper, Friction, CenterRoll, Count };
struct Spec { const char* label; uint16_t step; float initial; };
// Values are stored as integer ten-thousandths: deterministic button steps,
// no accumulated float error. Labels fit after '* ' in a 16-column LCD.
inline const Spec& spec(Id id) {
    static const Spec table[Count]={
        {"Ganancia total",500,SIMCONNECT_TOTAL_GAIN*.01f},
        {"Centrado pitch",500,SIMCONNECT_CENTERING_PITCH_GAIN},
        {"Centro reposo",500,SIMCONNECT_CENTERING_MIN_STIFFNESS},
        {"Turbul. roll",500,SIMCONNECT_GUST_ROLL},
        {"Turbul. pitch",500,SIMCONNECT_GUST_PITCH},
        {"Vib. suelo",100,SIMCONNECT_GROUND_VIBRATION_GAIN},
        {"Buffet perdida",100,SIMCONNECT_STALL_BUFFET_GAIN},
        {"Buffet tren",100,SIMCONNECT_GEAR_BUFFET_GAIN},
        {"Buffet spoiler",100,SIMCONNECT_SPOILER_BUFFET_GAIN},
        {"Damper",100,YOKE_DAMPER_GAIN},
        {"Friccion",100,YOKE_FRICTION_GAIN},
        {"Centrado roll",500,SIMCONNECT_CENTERING_ROLL_GAIN}
    };
    return table[id<Count ? id : Total];
}
class Settings {
public:
    Settings() { for(uint8_t i=0;i<Count;++i) values[i]=uint16_t(spec(Id(i)).initial*10000+.5f); }
    uint16_t raw(Id id) const { return id<Count ? values[id] : 0; }
    float get(Id id) const { return raw(id)*.0001f; }
    static constexpr uint16_t effectBits=(uint16_t(1)<<Count)-2; // Total is not an effect
    bool enabled(Id id) const { return id==Total || (id<Count && (enabledBits & (uint16_t(1)<<id))); }
    float effective(Id id) const { return enabled(id) ? get(id) : 0; }
    bool setEnabled(Id id,bool on) {
        if(id==Total || id>=Count) return false;
        if(on) enabledBits|=uint16_t(1)<<id;
        else enabledBits&=~(uint16_t(1)<<id);
        return true;
    }
    uint16_t effects() const { return enabledBits; }
    bool setEffects(uint16_t bits) {
        if(bits & ~effectBits) return false;
        enabledBits=bits; return true;
    }
    bool set(Id id,uint16_t value) {
        if(id>=Count || value>10000) return false;
        values[id]=value; return true;
    }
private:
    uint16_t values[Count];
    uint16_t enabledBits=effectBits;
};
inline const Settings& defaults() { static const Settings settings; return settings; }

// CenterRoll is appended to preserve existing EEPROM IDs.
// Portable explicit layout, independent of compiler padding or float format.
// magic[4], version u16, count, format (0=legacy,1=effect bits), values, bits u16, CRC16.
static const uint8_t recordSize=12+2*Count;
inline void encode(const Settings& settings,uint8_t* bytes) {
    bytes[0]='Y'; bytes[1]='K'; bytes[2]='G'; bytes[3]='N';
    YokeSC::put16(bytes+4,YOKE_SETTINGS_VERSION); bytes[6]=Count; bytes[7]=1;
    for(uint8_t i=0;i<Count;++i) YokeSC::put16(bytes+8+2*i,settings.raw(Id(i)));
    YokeSC::put16(bytes+8+2*Count,settings.effects());
    YokeSC::put16(bytes+recordSize-2,YokeSC::crc(bytes,recordSize-2));
}
inline bool decode(const uint8_t* bytes,Settings& settings) {
    const uint8_t storedCount=bytes[6];
    if(storedCount!=Count && storedCount!=Count-1) return false;
    if(bytes[7]>1 || (bytes[7]==1 && storedCount!=Count)) return false;
    const uint8_t storedSize=(bytes[7]==1 ? 12 : 10)+2*storedCount;
    if(bytes[0]!='Y'||bytes[1]!='K'||bytes[2]!='G'||bytes[3]!='N'||
       YokeSC::read16(bytes+4)!=YOKE_SETTINGS_VERSION||
       YokeSC::read16(bytes+storedSize-2)!=YokeSC::crc(bytes,storedSize-2)) return false;
    Settings candidate;
    for(uint8_t i=0;i<storedCount;++i)
        if(!candidate.set(Id(i),YokeSC::read16(bytes+8+2*i))) return false;
    if(storedCount==Count-1) candidate.set(CenterRoll,candidate.raw(CenterPitch)/2);
    if(bytes[7]==1 && !candidate.setEffects(YokeSC::read16(bytes+8+2*storedCount))) return false;
    settings=candidate; return true; // only mutate after validating the whole record
}
}
#endif
