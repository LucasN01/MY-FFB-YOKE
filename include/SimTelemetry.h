#ifndef SIM_TELEMETRY_H
#define SIM_TELEMETRY_H
#include <stdint.h>
#include <math.h>
#include "RawTelemetry.h"

// USB CDC: A5 5A version flags seqLE qLE gustRollLE gustPitchLE crcLE (14 bytes).
// v2: same bytes 0..11 + stall/gear/spoiler uint16 at 12/14/16 + CRC at 18 (20 bytes).
// v3: v2 payload + ground envelope at 18 + CRC at 20 (22 bytes).
// v4: v3 payload + signed yoke roll/pitch at 20/22 (-10000..10000), CRC at 24.
// v5: v4 layout, flags bit1=explicit simulator pause (mutually exclusive with armed).
// v6: same layout, q=0..40000 (q/qRef up to 4); old versions retain q<=10000.
// v7: v6 payload + signed ground roll/pitch at 24/26, CRC at 28 (30 bytes).
// Ground forces already include the speed envelope; do not multiply it again.
// envelopes=0..10000, gust=-10000..10000. Flags bit0=armed/running.
// v8: raw telemetry only, 113 bytes. Timestamp at 6; 22 float32 at 10;
// three config float32 at 98, options at 110, CRC at 111. See RawTelemetry.h.
namespace YokeSC {
#ifdef __AVR__
__attribute__((noinline))
#endif
inline float clip(float x, float lo, float hi) { return x < lo ? lo : (x > hi ? hi : x); }
inline uint16_t read16(const uint8_t* p) { return uint16_t(p[0]) | (uint16_t(p[1]) << 8); }
inline void put16(uint8_t* p, uint16_t n) { p[0]=uint8_t(n); p[1]=uint8_t(n>>8); }
inline uint16_t crc(const uint8_t* p, uint8_t n) {
    uint16_t c=0xffff;
    while(n--) { c ^= uint16_t(*p++) << 8; for(uint8_t i=0;i<8;++i) c=(c&0x8000)?uint16_t((c<<1)^0x1021):uint16_t(c<<1); }
    return c;
}
class Receiver {
public:
    YokeRaw::Sample raw;
    bool rawAvailable=false;
    uint32_t revision=0;
    float pressure=0, gust[2]={0,0}, buffet[3]={0,0,0};
    float groundVibration=0;
    float groundForce[2]={0,0};
    bool groundForceAvailable=false;
    float yoke[2]={0,0};
    bool yokeAvailable=false;
    bool centeringAvailable=false; // Only SC3 explicitly opts into centering.
    bool armed=false;
    uint32_t last=0;
    bool fresh(uint32_t now) const { return valid && armed && uint32_t(now-last)<300; }
    bool isPaused() const { return paused; }
    bool calibrationAllowed(uint32_t now,bool connected) const {
        // Offline calibration uses the same timeout as the force watchdog.
        // No link is not proof that the host simulator process has closed.
        if(!connected || !valid || uint32_t(now-last)>=300) return true;
        // A live unpaused bridge stays locked even in monitor/disarmed mode.
        return paused && !armed;
    }
    bool idle() const { return count==0; }
    void feed(uint8_t b, uint32_t now) {
        if (count && uint32_t(now-byteTime)>40) count=0;
        byteTime=now;
        if (!count && b!=0xa5) return;
        if (count==1 && b!=0x5a) { count=(b==0xa5)?1:0; return; }
        buf[count++]=b;
        if (count==3 && (buf[2]<1 || buf[2]>8)) { count=0; return; }
        if (count<3) return;
        const uint8_t length=buf[2]==8 ? YokeRaw::packetSize : (buf[2]>=7 ? 30 : (buf[2]>=4 ? 26 : (buf[2]==3 ? 22 : (buf[2]==2 ? 20 : 14))));
        if (count<length) return;
        count=0;
        if((buf[3] & (buf[2]>=5 ? 0xfc : 0xfe)) || (buf[3]&3)==3 || read16(buf+length-2)!=crc(buf,length-2)) return;
        if(buf[2]==8) {
            const uint16_t s=read16(buf+4);
            const YokeRaw::Sample incoming=YokeRaw::decodePayload(buf);
            if(!YokeRaw::valid(incoming)) return;
            if(valid && uint32_t(now-last)<300 && int16_t(s-sequence)<=0) return;
            // Repeated or backwards capture times cannot keep an old force alive.
            // Disarm/pause frames always pass this clock guard, even at the same time.
            if((buf[3]&1) && rawAvailable && valid && uint32_t(now-last)<300 &&
               int32_t(incoming.timeMs-raw.timeMs)<=0) return;
            raw=incoming; rawAvailable=true; ++revision;
            pressure=groundVibration=0; groundForceAvailable=false;
            for(uint8_t i=0;i<2;++i) gust[i]=groundForce[i]=0;
            for(uint8_t i=0;i<3;++i) buffet[i]=0;
            yoke[0]=raw.value[YokeRaw::YokeRoll]; yoke[1]=raw.value[YokeRaw::YokePitch];
            yokeAvailable=centeringAvailable=true;
            paused=(buf[3]&2)!=0; armed=(buf[3]&1)!=0 && raw.value[YokeRaw::Slew]==0;
            sequence=s; valid=true; last=now; return;
        }
        uint16_t amplitudes[3]={0,0,0};
        if (buf[2]>=2) for (uint8_t i=0;i<3;++i) {
            amplitudes[i]=read16(buf+12+2*i);
            if(amplitudes[i]>10000) return;
        }
        const uint16_t s=read16(buf+4), q=read16(buf+6);
        const uint16_t ground=buf[2]>=3 ? read16(buf+18) : 0;
        const int16_t yokeRoll=buf[2]>=4 ? int16_t(read16(buf+20)) : 0;
        const int16_t yokePitch=buf[2]>=4 ? int16_t(read16(buf+22)) : 0;
        if(yokeRoll < -10000 || yokeRoll > 10000 || yokePitch < -10000 || yokePitch > 10000) return;
        if(ground>10000) return;
        const int16_t gr=buf[2]>=7 ? int16_t(read16(buf+24)) : 0;
        const int16_t gp=buf[2]>=7 ? int16_t(read16(buf+26)) : 0;
        if(gr < -10000 || gr > 10000 || gp < -10000 || gp > 10000) return;
        const int16_t r=int16_t(read16(buf+8)), p=int16_t(read16(buf+10));
        if(q>(buf[2]>=6 ? 40000 : 10000) || r < -10000 || r>10000 || p < -10000 || p>10000) return;
        // Duplicates do not refresh watchdog. New sessions allowed after timeout.
        if(valid && uint32_t(now-last)<300 && int16_t(s-sequence)<=0) return;
        sequence=s; pressure=q*.0001f; gust[0]=r*.0001f; gust[1]=p*.0001f;
        rawAvailable=false; ++revision;
        groundVibration=ground*.0001f; centeringAvailable=buf[2]>=3;
        groundForceAvailable=buf[2]>=7;
        groundForce[0]=ground ? gr*.0001f : 0;
        groundForce[1]=ground ? gp*.0001f : 0;
        yoke[0]=yokeRoll*.0001f; yoke[1]=yokePitch*.0001f; yokeAvailable=buf[2]>=4;
        for (uint8_t i=0;i<3;++i) buffet[i]=amplitudes[i]*.0001f;
        paused=buf[2]>=5 && (buf[3]&2)!=0;
        armed=(buf[3]&1)!=0; valid=true; last=now;
    }
private:
    uint8_t buf[YokeRaw::packetSize]={0}, count=0;
    uint16_t sequence=0;
    uint32_t byteTime=0;
    bool valid=false,paused=false;
};
// Perturbacion de viento, sin centrado ni amortiguacion local.
inline float gustCommand(float gust, float coefficient, float gain,
                         bool fresh, bool connected) {
    if (!fresh || !connected || !isfinite(gust) || !isfinite(coefficient) ||
        !isfinite(gain) || coefficient < 0 || gain < 0) return 0;
    // Continuous dead zone: no force step at the former 10% threshold.
    const float magnitude = fabsf(clip(gust,-1,1));
    if (magnitude <= 0.02f) return 0;
    const float shaped = (magnitude-0.02f) / 0.98f;
    return (gust < 0 ? -shaped : shaped) * clip(coefficient,0,1) * clip(gain,0,1);
}
}
#endif





