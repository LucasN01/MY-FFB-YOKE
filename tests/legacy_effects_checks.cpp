// Separate namespace prevents differing compile-time Effects variants from sharing inline definitions.
#define YokeGains YokeLegacyGains
#define YokeSC YokeLegacySC
#define YOKE_DAMPER_STIFFNESS_SCALING 0
#define YOKE_FRICTION_MODEL 0
#define SIMCONNECT_BUFFET_NARROWBAND 0
#define SIMCONNECT_GROUND_NARROWBAND_TEXTURE 0
#include "../SimEffects.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
static int checks=0;
#define CHECK(x) do { ++checks; if(!(x)) { std::printf("FAIL legacy line %d: %s\n",__LINE__,#x); std::exit(1); } } while(0)
static bool bits(float a,float b) { return std::memcmp(&a,&b,sizeof(a))==0; }
static void feed(YokeSC::Receiver& rx,uint32_t t) {
    uint8_t bytes[26]={0xa5,0x5a,6,1};
    YokeSC::put16(bytes+4,uint16_t(t/20+1)); YokeSC::put16(bytes+6,10000);
    for(int offset=12;offset<=18;offset+=2) YokeSC::put16(bytes+offset,10000);
    YokeSC::put16(bytes+24,YokeSC::crc(bytes,24));
    for(int i=0;i<26;++i) rx.feed(bytes[i],t);
}
int legacyEffectsChecks() {
    YokeSC::Effects fx; YokeSC::Receiver rx;
    float amplitude=0;
    for(uint32_t t=0;t<=200;t+=2) {
        if(t%20==0) feed(rx,t);
        fx.update(rx,t,true);
        if(t) amplitude+=2.0f/(100.0f+2)*(1-amplitude);
        const float wave=amplitude*YokeSC::triangle(uint16_t((250+t*13)%1000));
        CHECK(bits(fx.command(rx,1,1,YokeSC::Stall),wave*YokeGains::defaults().effective(YokeGains::Stall)));
    }
    const float speed=1-YOKE_MOTION_DEAD_SPEED;
    const float damper=-YokeGains::defaults().effective(YokeGains::Damper)*speed;
    const float friction=YokeGains::defaults().effective(YokeGains::Friction)*speed/(YOKE_FRICTION_SPEED+speed);
    CHECK(bits(fx.command(rx,1,1,YokeSC::Centering|YokeSC::Damper,0,1),damper));
    CHECK(bits(fx.command(rx,1,1,YokeSC::Friction,0,-1),friction));
    YokeSC::Effects road; YokeSC::Receiver roadRx; YokeRaw::Sample sample;
    sample.value[YokeRaw::OnGround]=1; sample.value[YokeRaw::GroundSpeed]=40;
    for(uint32_t now=100;now<300;now+=2) {
        if(now%20==0) {
            sample.timeMs=now; uint8_t bytes[113]={0xa5,0x5a,8,1};
            YokeSC::put16(bytes+4,uint16_t(now)); YokeRaw::encodePayload(bytes,sample);
            YokeSC::put16(bytes+111,YokeSC::crc(bytes,111));
            for(int i=0;i<113;++i) roadRx.feed(bytes[i],now);
        }
        road.update(roadRx,now,true,YokeSC::Ground);
        const float t=(now-100)*.001f,level=road.calculated().groundEnvelope;
        const float texture=level*.12f*(.65f*sinf(6.2831853f*7*t)+.35f*sinf(6.2831853f*11.3f*t));
        const float expected=YokeSC::clip(texture,-level,level)*YokeGains::defaults().effective(YokeGains::Ground);
        CHECK(bits(road.command(roadRx,0,1,YokeSC::Ground),expected));
    }
    return checks;
}
