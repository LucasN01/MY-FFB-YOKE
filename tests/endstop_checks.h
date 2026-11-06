#pragma once
#include <algorithm>
#include <cstring>
#include <initializer_list>
#include "../include/EndstopCalibration.h"

static void endstopFrame(uint8_t* f, uint16_t seq, int16_t roll, int16_t pitch);

// Runs the calibrator with the given switches held for enough cycles to pass the debounce.
static void calibrationStep(YokeEndstop::Calibration& c, std::initializer_list<int> held,
                            float roll, float pitch, bool valid=true,
                            int cycles=YOKE_ENDSTOP_SWITCH_DEBOUNCE_CYCLES+1) {
    bool pressed[YokeEndstop::StopCount]={false,false,false,false};
    for(int stop : held) pressed[stop]=true;
    const float position[2]={roll,pitch};
    for(int i=0;i<cycles;++i) c.update(pressed,position,valid);
}
// Moves to one stop, presses its switch and comes back to rest.
static void calibrationVisit(YokeEndstop::Calibration& c, int stop, float at) {
    const float roll=stop<2 ? at : 0, pitch=stop<2 ? 0 : at;
    calibrationStep(c,{stop},roll,pitch);
    calibrationStep(c,{},roll,pitch);
    calibrationStep(c,{},0,0);
}

static void endstopCalibrationChecks() {
    using namespace YokeEndstop;
    char top[17], bottom[17];
    const float stops[StopCount]={-.6f,1.f,-1.f,.8f};

    // Any order, asymmetric stops: ready only after the last release.
    int order[StopCount]={0,1,2,3}, orders=0;
    do {
        Calibration c;
        for(int i=0;i<StopCount;++i) {
            CHECK(!c.ready());
            const int stop=order[i];
            const float roll=stop<2 ? stops[stop] : 0, pitch=stop<2 ? 0 : stops[stop];
            calibrationStep(c,{stop},roll,pitch);
            CHECK(!c.ready()); // switch still pressed
            CHECK(c.captured(uint8_t(stop)));
            calibrationStep(c,{},roll,pitch);
            if(i<StopCount-1) calibrationStep(c,{},0,0);
        }
        CHECK(c.ready());
        CHECK(std::fabs(c.normalized(0,-.6f)+1)<.00001f && std::fabs(c.normalized(0,1.f)-1)<.00001f);
        CHECK(std::fabs(c.normalized(0,.2f))<.00001f);
        CHECK(std::fabs(c.normalized(1,-1.f)+1)<.00001f && std::fabs(c.normalized(1,.8f)-1)<.00001f);
        CHECK(std::fabs(c.normalized(1,-.1f))<.00001f);
        CHECK(c.normalized(0,1.2f)>1); // beyond the stop stays beyond, for the limiter
        ++orders;
    } while(std::next_permutation(order,order+StopCount));
    CHECK(orders==24);

    // Not ready: normalized is the identity and the axis guard holds.
    {
        Calibration c;
        CHECK(c.normalized(0,.7f)==.7f && !c.captured(StopCount));
        calibrationStep(c,{},0,0);
        CHECK(!c.ready());
    }

    // Debounce: a short pulse records nothing, a long one does.
    {
        Calibration c;
        calibrationStep(c,{RollNeg},-1,0,true,YOKE_ENDSTOP_SWITCH_DEBOUNCE_CYCLES-1);
        calibrationStep(c,{},-1,0);
        CHECK(!c.captured(RollNeg));
        calibrationStep(c,{RollNeg},-1,0);
        CHECK(c.captured(RollNeg));
        calibrationStep(c,{},-1,0);
        // A short release does not count as released.
        calibrationVisit(c,RollPos,1); calibrationVisit(c,PitchNeg,-1);
        calibrationStep(c,{PitchPos},0,1);
        calibrationStep(c,{},0,1,true,YOKE_ENDSTOP_SWITCH_DEBOUNCE_CYCLES-1);
        CHECK(!c.ready());
        calibrationStep(c,{PitchPos},0,1);
        calibrationStep(c,{},0,1);
        CHECK(c.ready());
    }

    // A switch pressed again during the last release keeps the motors blocked, even in its debounce.
    for(int stop=0;stop<StopCount;++stop) {
        Calibration c;
        const float roll=stop==RollNeg ? -1.f : 1.f, pitch=stop==PitchNeg ? -1.f : 1.f;
        calibrationVisit(c,RollNeg,-1); calibrationVisit(c,RollPos,1); calibrationVisit(c,PitchNeg,-1);
        calibrationStep(c,{PitchPos},1,1);
        calibrationStep(c,{},1,1,true,YOKE_ENDSTOP_SWITCH_DEBOUNCE_CYCLES-1);
        calibrationStep(c,{stop},roll,pitch,true,1); // the release ends here
        CHECK(!c.ready());
        calibrationStep(c,{stop},roll,pitch);
        CHECK(!c.ready());
        calibrationStep(c,{},roll,pitch,true,YOKE_ENDSTOP_SWITCH_DEBOUNCE_CYCLES-1);
        CHECK(!c.ready());
        calibrationStep(c,{},roll,pitch);
        CHECK(c.ready());
    }

    // Invalid or non-finite samples record nothing; valid ones while held do.
    {
        Calibration c;
        const float nan=std::numeric_limits<float>::quiet_NaN();
        calibrationStep(c,{RollNeg},-1,0,false);
        CHECK(!c.captured(RollNeg));
        c.rows(top,bottom); CHECK(std::strcmp(bottom,"SIN POSICION")==0);
        calibrationStep(c,{RollNeg},nan,0);
        CHECK(!c.captured(RollNeg));
        calibrationStep(c,{RollNeg},0,nan);
        CHECK(!c.captured(RollNeg));
        calibrationStep(c,{RollNeg},-1,0);
        CHECK(c.captured(RollNeg));
    }

    // Lagging position: the extreme reached while pressed is kept.
    {
        Calibration c;
        calibrationStep(c,{RollPos},.95f,0);
        calibrationStep(c,{RollPos},1.f,0);
        calibrationStep(c,{RollPos},.98f,0);
        calibrationStep(c,{},.98f,0);
        calibrationVisit(c,RollNeg,-1); calibrationVisit(c,PitchNeg,-1); calibrationVisit(c,PitchPos,1);
        CHECK(c.ready());
        CHECK(std::fabs(c.normalized(0,1.f)-1)<.00001f);
    }

    // Rejections keep the motors blocked and allow a clean retry.
    const auto good=[](Calibration& c) {
        calibrationVisit(c,RollNeg,-1); calibrationVisit(c,RollPos,1);
        calibrationVisit(c,PitchNeg,-1); calibrationVisit(c,PitchPos,1);
    };
    {   // NEG and POS crossed on one axis.
        Calibration c;
        calibrationVisit(c,RollNeg,1); calibrationVisit(c,RollPos,-1);
        calibrationVisit(c,PitchNeg,-1); calibrationVisit(c,PitchPos,1);
        CHECK(!c.ready());
        c.rows(top,bottom); CHECK(std::strcmp(bottom,"REPETIR TOPES")==0);
        good(c); CHECK(c.ready());
    }
    {   // Pitch crossed; roll correct.
        Calibration c;
        calibrationVisit(c,RollNeg,-1); calibrationVisit(c,RollPos,1);
        calibrationVisit(c,PitchNeg,1); calibrationVisit(c,PitchPos,-1);
        CHECK(!c.ready());
        good(c); CHECK(c.ready());
    }
    {   // Short travel.
        Calibration c;
        calibrationVisit(c,RollNeg,-.2f); calibrationVisit(c,RollPos,.2f);
        calibrationVisit(c,PitchNeg,-1); calibrationVisit(c,PitchPos,1);
        CHECK(!c.ready());
        good(c); CHECK(c.ready());
    }
    {   // Sensors of different axes crossed: roll stop reached, "pitch" switch fires, and vice versa.
        Calibration c;
        calibrationStep(c,{PitchNeg},-1,0); calibrationStep(c,{},-1,0); calibrationStep(c,{},0,0);
        calibrationStep(c,{RollNeg},0,-1); calibrationStep(c,{},0,-1); calibrationStep(c,{},0,0);
        calibrationVisit(c,RollPos,1); calibrationVisit(c,PitchPos,1);
        CHECK(!c.ready());
        good(c); CHECK(c.ready());
    }
    {   // Known limit: axes moved together cannot tell crossed sensors apart. Values stay correct.
        Calibration c;
        calibrationStep(c,{RollNeg,PitchNeg},-1,-1); calibrationStep(c,{},-1,-1); calibrationStep(c,{},0,0);
        calibrationStep(c,{RollPos,PitchPos},1,1); calibrationStep(c,{},1,1);
        CHECK(c.ready());
        CHECK(std::fabs(c.normalized(0,-1)+1)<.00001f && std::fabs(c.normalized(1,1)-1)<.00001f);
    }
    {   // Same limit, axis short of its stop: the stop is learned early, never beyond the travel seen.
        Calibration c;
        calibrationStep(c,{RollNeg,PitchNeg},-.7f,-1); calibrationStep(c,{},-.7f,-1); calibrationStep(c,{},0,0);
        calibrationStep(c,{RollPos,PitchPos},1,1); calibrationStep(c,{},1,1);
        CHECK(c.ready());
        CHECK(std::fabs(c.normalized(0,-.7f)+1)<.00001f);
    }
    {   // One sensor inverted: pressed everywhere except at its stop.
        Calibration c;
        calibrationStep(c,{RollNeg},0,0);
        calibrationStep(c,{},-1,0); // at its stop it reads released
        calibrationStep(c,{RollNeg},0,0);
        CHECK(c.captured(RollNeg));
        calibrationStep(c,{RollNeg,RollPos},1,0); // the other stop: both pressed
        CHECK(!c.ready() && !c.captured(RollNeg));
        c.rows(top,bottom); CHECK(std::strcmp(bottom,"REPETIR TOPES")==0);
    }
    {   // Global polarity wrong: all pressed at rest, never ready.
        Calibration c;
        for(int i=0;i<500;++i) calibrationStep(c,{RollNeg,RollPos,PitchNeg,PitchPos},0,0);
        CHECK(!c.ready());
    }
    {   // Lone outward sample while pressed.
        Calibration c;
        calibrationStep(c,{RollPos},.9f,0);
        calibrationStep(c,{RollPos},3.f,0,true,1);
        calibrationStep(c,{RollPos},.9f,0);
        calibrationStep(c,{},.9f,0); calibrationStep(c,{},0,0);
        calibrationVisit(c,RollNeg,-.9f); calibrationVisit(c,PitchNeg,-1); calibrationVisit(c,PitchPos,1);
        CHECK(!c.ready());
    }

    // Once ready, nothing changes it.
    {
        Calibration c;
        good(c); CHECK(c.ready());
        const float before=c.normalized(0,.5f);
        calibrationStep(c,{RollNeg,RollPos,PitchNeg,PitchPos},5,5);
        calibrationStep(c,{RollNeg},-9,0,false);
        CHECK(c.ready() && c.normalized(0,.5f)==before);
    }

    // LCD rows fit and list only what is pending.
    {
        Calibration c;
        calibrationStep(c,{},0,0);
        c.rows(top,bottom);
        CHECK(std::strcmp(top,"CALIBRAR TOPES")==0 && std::strcmp(bottom,"R- R+ P- P+")==0);
        calibrationVisit(c,RollPos,1); calibrationVisit(c,PitchNeg,-1);
        c.rows(top,bottom);
        CHECK(std::strcmp(bottom,"R-       P+")==0);
        calibrationVisit(c,RollNeg,-1);
        calibrationStep(c,{PitchPos},0,1);
        c.rows(top,bottom);
        CHECK(std::strcmp(bottom,"SOLTAR TOPE")==0);
        CHECK(std::strlen(top)<=16 && std::strlen(bottom)<=16);
    }

    // The limiter follows the stop position while forces keep the mode's position.
    {
        YokeSC::Receiver rx; YokeSC::Effects fx; uint8_t f[26];
        for(uint32_t t=0;t<=4000;t+=2) {
            if(t%20==0) { endstopFrame(f,uint16_t(t/20+1),0,0); feed4(rx,f,t); }
            fx.update(rx,t,true);
            const uint8_t effects=YokeSC::Stall|YokeSC::Gear|YokeSC::Spoiler|YokeSC::Turbulence;
            for(uint8_t axis=0;axis<2;++axis) for(float sign : {1.f,-1.f}) {
                // Stop position at the stop: force toward it is gone, at position 0 of the mode.
                CHECK(fx.commandWithStopPosition(rx,axis,1,0,sign,effects,false)*sign<=0);
                // Stop position free: same force as the position source, whatever the mode's position.
                CHECK(near(fx.commandWithStopPosition(rx,axis,1,sign,0,YokeSC::Centering,false),
                    fx.command(rx,axis,1,YokeSC::Centering,sign,0)));
                // Same position in both: identical to the position source entry.
                CHECK(fx.commandWithStopPosition(rx,axis,1,.95f*sign,.95f*sign,effects,false)==
                    fx.commandWithPositionSource(rx,axis,1,.95f*sign,effects,false));
                CHECK(fx.commandWithStopPosition(rx,axis,1,0,std::numeric_limits<float>::quiet_NaN(),effects,false)==0);
            }
        }
    }
}

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
                    CHECK(brake*sign<=0); // 0 when both commands saturate at the same limit
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
