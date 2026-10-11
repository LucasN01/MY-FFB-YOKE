// SimConnect position remains on the velocity friction law even with model flag enabled.
#define YokeGains YokeSimPositionGains
#define YokeSC YokeSimPositionSC
#define YOKE_USE_SIMULATION 1
#define YOKE_FRICTION_MODEL 1
#include "../SimEffects.h"
#include <cstdio>
#include <cstdlib>
int simPositionFrictionChecks() {
    static_assert(!YOKE_FRICTION_POSITION_MODEL,"Simulated position must use velocity friction");
    YokeSC::Effects fx; YokeSC::Receiver rx; uint8_t bytes[26]={0xa5,0x5a,6,1};
    YokeSC::put16(bytes+4,1); YokeSC::put16(bytes+6,10000);
    YokeSC::put16(bytes+24,YokeSC::crc(bytes,24));
    for(int i=0;i<26;++i) rx.feed(bytes[i],0);
    fx.update(rx,0,true); fx.update(rx,2,true);
    const float force=fx.command(rx,1,1,YokeSC::Friction,0,-1);
    const float expected=YokeSC::motionAdjustedForce(0,-1,1,false,true);
    if(force!=expected || force<=0 || fx.command(rx,1,1,YokeSC::Friction,.05f,0)!=0) {
        std::printf("FAIL simulated-position friction\n"); std::exit(1);
    }
    return 3;
}
