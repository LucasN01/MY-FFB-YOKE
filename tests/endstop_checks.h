#pragma once

// Frame with position plus every effect input at full scale.
static void endstopFrame(uint8_t* f, uint16_t seq, int16_t roll, int16_t pitch) {
    frame4(f,seq,10000,roll,pitch);
    YokeSC::put16(f+8,10000); YokeSC::put16(f+10,10000);
    YokeSC::put16(f+12,10000); YokeSC::put16(f+14,10000); YokeSC::put16(f+16,10000);
    YokeSC::put16(f+18,10000); YokeSC::put16(f+24,YokeSC::crc(f,24));
}

static void endstopChecks() {
    using YokeSC::endstopLimited;
    const float start=.90f, full=.98f;
    const float inf=std::numeric_limits<float>::infinity(), nan=std::numeric_limits<float>::quiet_NaN();

    // Law: both signs, outside, edges, ramp, stop and beyond.
    for(float sign : {1.f,-1.f}) {
        const float into=.5f*sign;
        CHECK(endstopLimited(into,0,start,full)==into);
        CHECK(endstopLimited(into,.5f*sign,start,full)==into);
        CHECK(endstopLimited(into,start*sign,start,full)==into);
        CHECK(std::fabs(endstopLimited(into,.94f*sign,start,full)-.25f*sign)<.0001f);
        CHECK(endstopLimited(into,full*sign,start,full)==0);
        CHECK(endstopLimited(into,sign,start,full)==0);
        CHECK(endstopLimited(into,1.5f*sign,start,full)==0);
        CHECK(endstopLimited(into,inf*sign,start,full)==0);
        CHECK(endstopLimited(-into,sign,start,full)==-into);       // toward center: whole at the stop
        CHECK(endstopLimited(-into,.94f*sign,start,full)==-into);
        CHECK(endstopLimited(inf*sign,.5f,start,full)==0);
    }
    CHECK(endstopLimited(nan,.5f,start,full)==0);
    CHECK(endstopLimited(.5f,nan,start,full)==0);
    CHECK(endstopLimited(0,1,start,full)==0);

    // Sweep: never adds force, never flips sign, continuous in position.
    for(int c=-90;c<=90;++c) {
        const float u=c*.01f;
        float previous=endstopLimited(u,-1.2f,start,full);
        for(int p=-1199;p<=1200;++p) {
            const float x=p*.001f, out=endstopLimited(u,x,start,full);
            CHECK(std::fabs(out)<=std::fabs(u));
            CHECK(out*u>=0);
            if(u*x<=0 || std::fabs(x)<=start) CHECK(out==u);
            if(u*x>0 && std::fabs(x)>=full) CHECK(out==0);
            CHECK(std::fabs(out-previous)<=std::fabs(u)*.001f/(full-start)+.0001f);
            previous=out;
        }
    }

    // Real effects through the firmware entry point, with local and simulator position.
    const uint8_t effects=YokeSC::Turbulence|YokeSC::Stall|YokeSC::Gear|YokeSC::Spoiler|YokeSC::Ground;
    const float rollFull=ROLL_ENDSTOP_ZONE_FULL, pitchFull=PITCH_ENDSTOP_ZONE_FULL;
    uint8_t f[26];
    for(int sim=0;sim<2;++sim) for(int sign=-1;sign<=1;sign+=2) {
        YokeSC::Receiver rx; YokeSC::Effects fx;
        const int16_t held=sim ? int16_t(sign*10000) : 0;
        float largest=0;
        for(uint32_t t=0;t<=4000;t+=2) {
            if(t%20==0) { endstopFrame(f,uint16_t(t/20+1),held,held); feed4(rx,f,t); }
            fx.update(rx,t,true);
            for(uint8_t axis=0;axis<2;++axis) {
                const float atStop=fx.commandWithPositionSource(rx,axis,1,float(sign),effects,sim!=0);
                CHECK(atStop*sign<=0); // nothing may push into the stop
                const float x=(axis==0 ? rollFull : pitchFull)*sign;
                if(!sim) CHECK(fx.commandWithPositionSource(rx,axis,1,x,effects,false)*sign<=0);
                // Away from this stop the same effects still reach the motor.
                const float free=fx.command(rx,axis,1,effects,0,0);
                if(!sim) CHECK(near(fx.commandWithPositionSource(rx,axis,1,0,effects,false),free));
                if(std::fabs(free)>largest) largest=std::fabs(free);
                // At the stop, vibration toward the center passes and the other half is removed.
                if(free*sign<0) CHECK(near(atStop,free));
                if(free*sign>0) CHECK(atStop==0);
            }
        }
        CHECK(largest>.01f); // the checks above saw real force
    }

    // Damping: whole when braking motion into the stop, removed when pushing into it.
    YokeSC::Receiver dampRx; YokeSC::Effects dampFx;
    for(uint32_t t=0;t<=1000;t+=2) {
        if(t%20==0) { frame4(f,uint16_t(t/20+1),10000,0,0); feed4(dampRx,f,t); }
        dampFx.update(dampRx,t,true);
    }
    for(uint8_t axis=0;axis<2;++axis) for(float sign : {1.f,-1.f}) {
        const float braking=dampFx.commandWithPositionSource(dampRx,axis,1,sign,YokeSC::Damper,false,2*sign);
        CHECK(braking*sign<0);
        CHECK(near(braking,dampFx.command(dampRx,axis,1,YokeSC::Damper,sign,2*sign)));
        CHECK(dampFx.commandWithPositionSource(dampRx,axis,1,sign,YokeSC::Damper,false,-2*sign)==0);
        const float inside=dampFx.commandWithPositionSource(dampRx,axis,1,.5f*sign,YokeSC::Damper,false,-2*sign);
        CHECK(inside*sign>0 && near(inside,dampFx.command(dampRx,axis,1,YokeSC::Damper,.5f*sign,-2*sign)));
    }

    // Effects plus damper: into the stop the effects are limited and the brake stays whole.
    const uint8_t damped=effects|YokeSC::Damper;
    for(int sign=-1;sign<=1;sign+=2) {
        YokeSC::Receiver rx; YokeSC::Effects fx;
        int brakeOutweighs=0;
        for(uint32_t t=0;t<=4000;t+=2) {
            if(t%20==0) { endstopFrame(f,uint16_t(t/20+1),0,0); feed4(rx,f,t); }
            fx.update(rx,t,true);
            for(uint8_t axis=0;axis<2;++axis) {
                const float begin=axis==0 ? ROLL_ENDSTOP_ZONE_START : PITCH_ENDSTOP_ZONE_START;
                const float end=axis==0 ? ROLL_ENDSTOP_ZONE_FULL : PITCH_ENDSTOP_ZONE_FULL;
                for(float x : {(begin+end)*.5f*sign,float(sign)}) {
                    const float into=2.f*sign;
                    const float total=fx.command(rx,axis,1,damped,x,into);
                    const float undamped=fx.command(rx,axis,1,effects,x,into);
                    const float brake=total-undamped;
                    CHECK(brake*sign<=0); // 0 when the effects alone saturate EXTRA_LIMIT
                    const float out=fx.commandWithPositionSource(rx,axis,1,x,damped,false,into);
                    CHECK(std::fabs(out-(endstopLimited(undamped,x,begin,end)+brake))<.00001f);
                    if(std::fabs(x)>=end) CHECK(out*sign<=0); // at the stop: nothing pushes into it
                    CHECK(std::fabs(out)<=std::fmax(std::fabs(total),std::fabs(brake))+.00001f);
                    if(undamped*sign>0 && total*sign<0) ++brakeOutweighs;
                    // Moving away, the damper pushes into the stop: limited with everything else.
                    const float away=fx.command(rx,axis,1,damped,x,-into);
                    CHECK(near(fx.commandWithPositionSource(rx,axis,1,x,damped,false,-into),
                        endstopLimited(away,x,begin,end)));
                }
            }
        }
        CHECK(brakeOutweighs>0); // the case found by review really ran
    }

    // Continuity when the sum crosses zero while braking.
    for(int i=-1000;i<=1000;++i) {
        const float x=.94f, brake=-.1f, step=.001f;
        const float a=endstopLimited(i*step,x,start,full)+brake, b=endstopLimited((i+1)*step,x,start,full)+brake;
        CHECK(std::fabs(a-b)<=step+.00001f);
    }

    // Simulator mode without position (frame version 3): no force at all.
    YokeSC::Receiver oldRx; YokeSC::Effects oldFx;
    for(uint32_t t=0;t<=1000;t+=2) {
        if(t%20==0) { frame3(f,uint16_t(t/20+1),10000,10000); feed3(oldRx,f,t); }
        oldFx.update(oldRx,t,true);
        CHECK(!oldRx.yokeAvailable);
        CHECK(oldFx.commandWithPositionSource(oldRx,0,1,0,effects,true)==0);
        CHECK(oldFx.commandWithPositionSource(oldRx,1,1,0,effects,true)==0);
    }
}
