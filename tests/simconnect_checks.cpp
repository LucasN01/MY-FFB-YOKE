#include "../SimTelemetry.h"
#include "../SimConnectBridge/GustGain.h"
#include "../SimConnectBridge/GustModel.h"
#include "../SimConnectBridge/BuffetModel.h"
#include "../SimEffects.h"
#include "../PotentiometerPosition.h"
#include "../SimConnectBridge/GroundModel.h"
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <cmath>

static int checks = 0;
#define CHECK(x) do { ++checks; if (!(x)) { std::printf("FAIL line %d: %s\n", __LINE__, #x); std::exit(1); } } while (0)
static void frame(uint8_t* f, uint16_t seq, bool armed = true,
                  uint16_t q = 5000, int16_t r = 10000, int16_t p = -5000) {
    f[0]=0xa5; f[1]=0x5a; f[2]=1; f[3]=armed?1:0;
    YokeSC::put16(f+4,seq); YokeSC::put16(f+6,q);
    YokeSC::put16(f+8,uint16_t(r)); YokeSC::put16(f+10,uint16_t(p));
    YokeSC::put16(f+12,YokeSC::crc(f,12));
}
static void feed(YokeSC::Receiver& rx, const uint8_t* f, uint32_t time) {
    for (int i=0;i<14;++i) rx.feed(f[i],time);
}
static void frame2(uint8_t* f, uint16_t seq, bool armed=true,
                   uint16_t stall=10000, uint16_t gear=10000, uint16_t spoiler=10000) {
    frame(f,seq,armed); f[2]=2;
    YokeSC::put16(f+12,stall); YokeSC::put16(f+14,gear); YokeSC::put16(f+16,spoiler);
    YokeSC::put16(f+18,YokeSC::crc(f,18));
}
static void feed2(YokeSC::Receiver& rx, const uint8_t* f, uint32_t time) {
    for(int i=0;i<20;++i) rx.feed(f[i],time);
}
static bool near(float a,float b) { return std::fabs(a-b)<.000001f; }
static void frame3(uint8_t* f, uint16_t seq, uint16_t q, uint16_t ground, bool armed=true) {
    frame2(f,seq,armed,0,0,0); f[2]=3;
    YokeSC::put16(f+6,q); YokeSC::put16(f+8,0); YokeSC::put16(f+10,0);
    YokeSC::put16(f+18,ground); YokeSC::put16(f+20,YokeSC::crc(f,20));
}
static void feed3(YokeSC::Receiver& rx, const uint8_t* f, uint32_t time) {
    for(int i=0;i<22;++i) rx.feed(f[i],time);
}
static void frame4(uint8_t* f, uint16_t seq, uint16_t q, int16_t roll, int16_t pitch, bool armed=true) {
    frame3(f,seq,q,0,armed); f[2]=4;
    YokeSC::put16(f+20,uint16_t(roll)); YokeSC::put16(f+22,uint16_t(pitch));
    YokeSC::put16(f+24,YokeSC::crc(f,24));
}
static void feed4(YokeSC::Receiver& rx, const uint8_t* f, uint32_t time) {
    for(int i=0;i<26;++i) rx.feed(f[i],time);
}
#include "calibration_checks.h"
#include "ground_checks.h"
#include "raw_checks.h"
#include "realism_checks.h"
#include "link_checks.h"
int legacyEffectsChecks();
int simPositionFrictionChecks();
int main(int argc,char** argv) {
    linkChecks();
    rawChecks(); if(argc>1) replayRaw(argv[1]);
    groundChecks();
    realismChecks(); checks+=legacyEffectsChecks()+simPositionFrictionChecks();
    CHECK(validPotCalibration(100, 1900, 3800));
    CHECK(validPotCalibration(3800, 1900, 100));
    CHECK(!validPotCalibration(0, 0, 4095));
    CHECK(!validPotCalibration(0, 2048, 4096));
    for (int reversed = 0; reversed < 2; ++reversed) {
        const int neg = reversed ? 3800 : 100, pos = reversed ? 100 : 3800;
        PotentiometerPosition low(neg,1900,pos), center(neg,1900,pos), high(neg,1900,pos);
        CHECK(near(low.update(neg),-1));
        CHECK(near(center.update(1900),0));
        CHECK(near(high.update(pos),1));
        PotentiometerPosition sensor(neg,1900,pos);
        YokeMotion motion;
        motion.update(sensor.update(1900),0);
        CHECK(motion.velocity==0);
        for (int i=1; i<=100; ++i) {
            motion.update(sensor.update(1900+(i%2 ? 3 : -3)),i*2000);
            CHECK(motion.position==0 && motion.velocity==0);
        }
        for (int i=1; i<=200; ++i)
            motion.update(sensor.update(1900+(reversed ? -i : i)),200000+i*2000);
        CHECK(motion.position>0 && motion.velocity>0); // desplazamiento lento acumulado
        for (int i=1; i<=50; ++i)
            motion.update(sensor.update(1900+(reversed ? -200 : 200)),600000+i*2000);
        CHECK(motion.velocity==0);
        for (int i=1; i<=100; ++i)
            motion.update(sensor.update(1900+(reversed ? -200+i*10 : 200-i*10)),700000+i*2000);
        CHECK(motion.velocity<0);
        CHECK(sensor.update(reversed ? 4095 : 0)==-1);
        CHECK(sensor.update(reversed ? 0 : 4095)==1);
    }
    calibrationChecks();
    {
    // SC6 extends q/qRef without changing legacy interpretation or watchdog.
    YokeSC::Receiver extended;
    YokeSC::Effects extendedFx;
    uint8_t extendedFrame[26];
    float atReference=0, aboveReference=0;
    for(uint32_t t=0;t<=4000;t+=2) {
        if(t%20==0) {
            frame4(extendedFrame,uint16_t(t/20+1),t<2000?10000:22500,0,2000);
            extendedFrame[2]=6;
            YokeSC::put16(extendedFrame+24,YokeSC::crc(extendedFrame,24));
            feed4(extended,extendedFrame,t);
        }
        extendedFx.update(extended,t,true);
        if(t==1998) atReference=extendedFx.command(extended,1,1,YokeSC::Centering,.2f);
        if(t==4000) aboveReference=extendedFx.command(extended,1,1,YokeSC::Centering,.2f);
    }
    CHECK(near(extended.pressure,2.25f));
    CHECK(aboveReference<atReference*1.9f); // 180 vs 120 EAS: no early stiffness plateau
    CHECK(std::fabs(aboveReference)<SIMCONNECT_EXTRA_LIMIT);
    CHECK(extendedFx.command(extended,1,1,YokeSC::Damper,0,1)<0);
#if YOKE_FRICTION_POSITION_MODEL
    extendedFx.command(extended,1,1,YokeSC::Friction,0,0);
    CHECK(extendedFx.command(extended,1,1,YokeSC::Friction,-.05f,0)>0);
#else
    CHECK(extendedFx.command(extended,1,1,YokeSC::Friction,0,-1)>0);
#endif
#if YOKE_FRICTION_POSITION_MODEL
    // Start the rest check without presliding displacement from the preceding case.
    extendedFx.command(extended,1,1,YokeSC::Damper,0,0);
#endif
    CHECK(extendedFx.command(extended,1,1,YokeSC::Damper|YokeSC::Friction,0,0)==0);
    for(int version=5;version<=6;++version) {
        frame4(extendedFrame,202,version==5?10001:40001,0,0);
        extendedFrame[2]=uint8_t(version);
        YokeSC::put16(extendedFrame+24,YokeSC::crc(extendedFrame,24));
        feed4(extended,extendedFrame,4020);
        CHECK(extended.last==4000); // invalid pressure never refreshes watchdog
    }
    extendedFx.update(extended,4300,true);
    CHECK(extendedFx.command(extended,1,1,YokeSC::Damper,0,1)==0);

    }
    // The factory spring curve is unchanged; rest stiffness adds equally at every Q.
    for(float pressure : {0.0f,.25f,1.0f,4.0f,2.0f}) {
        YokeGains::Settings springSettings;
        const float rest=pressure==2 ? .30f : .15f;
        CHECK(springSettings.set(YokeGains::CenterRest,pressure==2 ? 3000 : 1500));
        YokeSC::Effects spring(springSettings);
        YokeSC::Receiver springRx;
        uint8_t springPacket[26];
        for(uint32_t now=0;now<=3000;now+=2) {
            if(now%20==0) {
                frame4(springPacket,uint16_t(now/20+1),uint16_t(pressure*10000),0,0);
                springPacket[2]=6;
                YokeSC::put16(springPacket+24,YokeSC::crc(springPacket,24));
                feed4(springRx,springPacket,now);
            }
            spring.update(springRx,now,true,YokeSC::Centering);
        }
        const float expected=pressure==2 ? 2.0f : rest+(1-rest)*pressure;
        CHECK(near(spring.command(springRx,1,1,YokeSC::Centering,.1f),
                   -.1f*expected*springSettings.effective(YokeGains::CenterPitch)));
    }
    // Passive velocity loads must exist at neutral and may overcome the spring.
    CHECK(YokeSC::motionAdjustedForce(-.2f,2,1)<-.2f);
    CHECK(YokeSC::motionAdjustedForce(-.2f,-2,1)>-.2f);
    CHECK(YokeSC::motionAdjustedForce(-.2f,-10,1)>0);
    CHECK(YokeSC::motionAdjustedForce(.2f,10,1)<0);
    CHECK(YokeSC::motionAdjustedForce(0,10,1)<0);
    CHECK(near(YokeSC::motionAdjustedForce(.2f,0,1),.2f));
    CHECK(near(YokeSC::motionAdjustedForce(.2f,3,1,false,false),.2f));
    CHECK(YokeSC::motionAdjustedForce(-.2f,2,1,true,false)<-.2f);
    CHECK(YokeSC::motionAdjustedForce(-.2f,2,1,false,true)<-.2f);
    CHECK(YokeSC::motionAdjustedForce(.2f,std::numeric_limits<float>::quiet_NaN(),1)==0);
    for(int b=-20;b<=20;++b) for(int v=-100;v<=100;++v) {
        const float base=b*.01f, velocity=v*.1f;
        const float adjusted=YokeSC::motionAdjustedForce(base,velocity,.9f);
        CHECK((adjusted-base)*velocity<=.000001f);
        CHECK(near(adjusted,-YokeSC::motionAdjustedForce(-base,-velocity,.9f)));
        CHECK(near(adjusted-base,YokeSC::motionAdjustedForce(0,velocity,.9f)));
        if(base*velocity<0) CHECK(std::fabs(adjusted)>=std::fabs(base));
    }
    YokeMotion measured;
    measured.update(.2f,1000); CHECK(measured.velocity==0);
    measured.update(.24f,21000); CHECK(measured.velocity>0 && measured.velocity<2.01f);
    const float heldVelocity=measured.velocity;
    for(int n=0;n<100;++n) measured.update(.24f,21000);
    CHECK(measured.velocity==heldVelocity); // held SC sample is not 100 new samples
    measured.update(.20f,41000); CHECK(measured.velocity<0); // reversal, no old-direction tail
    measured.update(.20f,61000); measured.update(.20f,81000); CHECK(measured.velocity==0);
    measured.update(-.8f,400000); CHECK(measured.velocity==0); // gap/reconnect, no derivative kick
    measured.update(std::numeric_limits<float>::quiet_NaN(),402000); CHECK(measured.velocity==0);
    measured.update(.8f,404000); CHECK(measured.velocity==0);
    measured.reset(); measured.update(0,0xfffff000UL); measured.update(.02f,uint32_t(0xfffff000UL+20000));
    CHECK(measured.velocity>0 && measured.velocity<=1);
    float rates[3];
    for(int rate=0;rate<3;++rate) {
        YokeMotion tracker; const uint32_t step=rate==0?2000:(rate==1?10000:20000);
        tracker.update(0,0);
        for(uint32_t t=step;t<=500000;t+=step) tracker.update(t*.000001f,t);
        rates[rate]=tracker.velocity;
    }
    CHECK(std::fabs(rates[0]-rates[2])<.002f && std::fabs(rates[1]-rates[2])<.002f);
    uint8_t sc4[26]; YokeSC::Receiver simRx; YokeSC::Effects simFx;
    for(int bit=0;bit<208;++bit) {
        YokeSC::Receiver bad; frame4(sc4,1,5000,-5000,5000); sc4[bit/8]^=uint8_t(1<<(bit%8));
        feed4(bad,sc4,100); CHECK(!bad.fresh(100));
    }
    for(uint32_t t=0;t<=1000;t+=2) {
        if(t%20==0) { frame4(sc4,uint16_t(t/20+1),0,10000,10000); feed4(simRx,sc4,t); }
        simFx.update(simRx,t,true);
    }
    const auto force=[&](int axis,float local=0) {
        return simFx.commandWithPositionSource(simRx,uint8_t(axis),1,local,YokeSC::Centering,true);
    };
    CHECK(force(1)<-.119f); // stopped aircraft: increased baseline stiffness
    CHECK(near(force(1),2*force(0)));
    CHECK(near(force(1,-1),force(1,1))); // local sensors cannot affect sim mode
    frame4(sc4,52,10000,0,0); feed4(simRx,sc4,1020); simFx.update(simRx,1020,true);
    CHECK(force(0,1)==0 && force(1,-1)==0); // exact zero immediately on return to center
    frame4(sc4,53,10000,-10000,10000); feed4(simRx,sc4,1040); simFx.update(simRx,1040,true);
    CHECK(force(0)>0 && force(1)<0); CHECK(near(force(1),-2*force(0)));
    CHECK(simFx.commandWithPositionSource(simRx,1,1,-1,YokeSC::Centering,false)>0); // future local mode
    CHECK(near(simFx.commandWithPositionSource(simRx,1,1,-1,YokeSC::Centering),
        simFx.commandWithPositionSource(simRx,1,1,-1,YokeSC::Centering,YOKE_USE_SIMULATION==1)));
    for(int axis=0;axis<2;++axis) {
        frame4(sc4,54,10000,axis==0?10001:0,axis==1?-10001:0);
        feed4(simRx,sc4,1060); CHECK(simRx.last==1040);
    }
    simFx.update(simRx,1340,true); CHECK(force(0)==0 && force(1)==0);
    frame4(sc4,55,10000,10000,10000,false); feed4(simRx,sc4,1360); simFx.update(simRx,1360,true);
    CHECK(force(0)==0 && force(1)==0);
    frame3(sc4,56,10000,0); feed3(simRx,sc4,1380); simFx.update(simRx,1380,true);
    simFx.update(simRx,1382,true);
    CHECK(!simRx.yokeAvailable && simRx.yoke[0]==0 && simRx.yoke[1]==0);
    CHECK(force(0,1)==0 && force(1,1)==0); // old packets never fall back to local input
    CHECK(YokeGround::envelope(1,0,true)==0);
    CHECK(YokeGround::envelope(1,.5,true)==0);
    CHECK(YokeGround::envelope(1,.5001,true)>0);
    CHECK(YokeGround::envelope(1,20.25,true)==.5);
    CHECK(YokeGround::envelope(1,40,true)==1);
    CHECK(YokeGround::envelope(1,100,true)==1);
    CHECK(YokeGround::envelope(0,100,true)==0);
    CHECK(YokeGround::envelope(1,100,false)==0);
    CHECK(YokeGround::envelope(1,std::numeric_limits<double>::quiet_NaN(),true)==0);
    uint8_t sc3[22];
    for(int bit=0;bit<176;++bit) {
        YokeSC::Receiver bad; frame3(sc3,1,5000,5000); sc3[bit/8]^=uint8_t(1<<(bit%8));
        feed3(bad,sc3,100); CHECK(!bad.fresh(100));
    }
    YokeSC::Receiver groundRx; YokeSC::Effects groundFx;
    float lo=1, hi=-1;
    for(uint32_t t=0;t<=1000;t+=2) {
        if(t%20==0) { frame3(sc3,uint16_t(t/20+1),10000,10000); feed3(groundRx,sc3,t); }
        groundFx.update(groundRx,t,true);
        float command=groundFx.command(groundRx,0,1,YokeSC::Ground);
        for(int g=0;g<=10;++g) {
            const float gain=g*.1f;
            const float roll=groundFx.command(groundRx,0,gain,YokeSC::Ground);
            const float pitch=groundFx.command(groundRx,1,gain,YokeSC::Ground);
            CHECK(near(roll,pitch*10));
            CHECK(int(std::fabs(roll)*PWM_MAX+.5f)<=40);
            CHECK(int(std::fabs(pitch)*PWM_MAX+.5f)<=4);
            CHECK(near(roll,groundFx.command(groundRx,0,gain,YokeSC::Ground,1,10)));
            CHECK(near(pitch,groundFx.command(groundRx,1,gain,YokeSC::Ground,-1,-10)));
            for(int axis=0;axis<2;++axis) {
                const float base=groundFx.command(groundRx,uint8_t(axis),gain,YokeSC::Centering,.5f,2);
                const float mixed=groundFx.command(groundRx,uint8_t(axis),gain,YokeSC::Centering|YokeSC::Ground,.5f,2);
                CHECK(std::fabs(mixed-base)*255 <= (axis==0?40.0001f:4.0001f));
            }
        }
        if(command<lo) lo=command;
        if(command>hi) hi=command;
        CHECK(groundFx.command(groundRx,1,1,0,1)==0);
    }
    CHECK(lo<-.02f && hi>.02f);
    CHECK(int(-lo*255+.5f)==40 && int(hi*255+.5f)==40);
    const float pitch=groundFx.command(groundRx,1,1,YokeSC::Centering,1);
    CHECK(pitch<-.79f);
    CHECK(near(groundFx.command(groundRx,1,1,YokeSC::Centering|YokeSC::Damper|YokeSC::Friction,1,10),-SIMCONNECT_EXTRA_LIMIT));
    CHECK(near(groundFx.command(groundRx,1,1,YokeSC::Centering|YokeSC::Damper|YokeSC::Friction,-1,-10),SIMCONNECT_EXTRA_LIMIT));
    CHECK(int(SIMCONNECT_EXTRA_LIMIT*PWM_MAX+.5f)==220);
    CHECK(groundFx.command(groundRx,1,1,YokeSC::Centering,0,10)==0);
    CHECK(groundFx.command(groundRx,1,0,YokeSC::Centering,1,10)==0);
    CHECK(groundFx.commandWithPositionSource(groundRx,1,1,1,YokeSC::Centering|YokeSC::Damper|YokeSC::Friction,false,2)<pitch);
    CHECK(groundFx.commandWithPositionSource(groundRx,1,1,1,YokeSC::Centering|YokeSC::Damper|YokeSC::Friction,false,-2)>pitch);
    for(int x=-10;x<=10;++x) for(int v=-10;v<=10;++v) {
        const float value=groundFx.command(groundRx,1,1,YokeSC::configuredEffects,x*.1f,float(v));
        CHECK(std::fabs(value)<=SIMCONNECT_EXTRA_LIMIT);
        CHECK(int(std::fabs(value)*PWM_MAX+.5f)<=220);
    }
    CHECK(near(pitch,2*groundFx.command(groundRx,0,1,YokeSC::Centering,1)));
    CHECK(near(-pitch,groundFx.command(groundRx,1,1,YokeSC::Centering,-1)));
    CHECK(groundFx.command(groundRx,1,1,YokeSC::Centering,0)==0);
    CHECK(near(pitch*.5f,groundFx.command(groundRx,1,1,YokeSC::Centering,.5f)));
    CHECK(groundFx.command(groundRx,1,0,YokeSC::Centering,1)==0);
    frame3(sc3,52,5000,0); feed3(groundRx,sc3,1020); groundFx.update(groundRx,1020,true);
    CHECK(groundFx.command(groundRx,1,1,YokeSC::Ground)==0); // no airborne/stopped tail
    CHECK(groundFx.command(groundRx,1,1,YokeSC::Centering,1)>pitch);
    frame3(sc3,53,5000,10001); feed3(groundRx,sc3,1040); CHECK(groundRx.last==1020);
    groundFx.update(groundRx,1320,true);
    CHECK(groundFx.command(groundRx,1,1,YokeSC::Centering,1)==0);
    CHECK(groundFx.command(groundRx,1,1,YokeSC::Ground)==0);
    frame3(sc3,54,5000,10000); feed3(groundRx,sc3,1340); groundFx.update(groundRx,1340,true);
    CHECK(groundFx.command(groundRx,1,1,YokeSC::Centering,1)==0); // smooth restart
    groundFx.update(groundRx,1342,true); CHECK(groundFx.command(groundRx,1,1,YokeSC::Centering,1)<0);
    groundFx.update(groundRx,1344,false); CHECK(groundFx.command(groundRx,1,1,YokeSC::Centering,1)==0);
    frame3(sc3,55,5000,10000,false); feed3(groundRx,sc3,1360); groundFx.update(groundRx,1360,true);
    CHECK(groundFx.command(groundRx,1,1,YokeSC::Ground|YokeSC::Centering,1)==0);
    frame2(sc3,56); feed2(groundRx,sc3,1380);
    CHECK(!groundRx.centeringAvailable && groundRx.groundVibration==0);
    uint8_t f[14]; YokeSC::Receiver rx;
    CHECK(!rx.fresh(0));
    const uint8_t known[]={'1','2','3','4','5','6','7','8','9'};
    CHECK(YokeSC::crc(known,9)==0x29b1);
    frame(f,100); feed(rx,f,1000);
    CHECK(rx.fresh(1000)); CHECK(rx.fresh(1299)); CHECK(!rx.fresh(1300));
    CHECK(near(rx.pressure,.5f)); CHECK(near(rx.gust[0],1)); CHECK(near(rx.gust[1],-.5f));
    feed(rx,f,1200); CHECK(rx.last==1000); // Duplicate cannot extend watchdog.
    frame(f,99); feed(rx,f,1201); CHECK(rx.last==1000);
    frame(f,101,false); feed(rx,f,1202); CHECK(!rx.fresh(1202));
    frame(f,102); feed(rx,f,1203); CHECK(rx.fresh(1203));
    for (int bit=0;bit<112;++bit) { // Every single bit error rejected, including CRC/header.
        YokeSC::Receiver bad; frame(f,1); f[bit/8]^=uint8_t(1<<(bit%8));
        feed(bad,f,10); CHECK(!bad.fresh(10));
    }
    for (int type=0;type<4;++type) {
        YokeSC::Receiver bad;
        frame(f,1,true,type==0?10001:5000,type==1?10001:0,type==2?-10001:0);
        if(type==3) { f[3]=3; YokeSC::put16(f+12,YokeSC::crc(f,12)); }
        feed(bad,f,10); CHECK(!bad.fresh(10));
    }
    YokeSC::Receiver wrap; frame(f,65535); feed(wrap,f,0xfffffff0UL);
    frame(f,0); feed(wrap,f,10); CHECK(wrap.fresh(10)); CHECK(!wrap.fresh(310));
    frame(f,50000); feed(wrap,f,400); CHECK(wrap.fresh(400)); // New session after timeout.
    YokeSC::Receiver partial; frame(f,1);
    for(int i=0;i<5;++i) partial.feed(f[i],1);
    feed(partial,f,50); CHECK(partial.fresh(50)); // Partial-frame timeout.
    CHECK(near(YokeSC::gustCommand(1,.08f,.9f,true,true),.072f));
    CHECK(near(YokeSC::gustCommand(-1,.10f,.9f,true,true),-.09f));
    CHECK(YokeSC::gustCommand(1,.08f,.9f,false,true)==0);
    CHECK(YokeSC::gustCommand(1,.08f,.9f,true,false)==0);
    CHECK(YokeSC::gustCommand(0,.08f,.9f,true,true)==0);
    CHECK(YokeSC::gustCommand(1,.08f,0,true,true)==0);
    CHECK(YokeSC::gustCommand(std::numeric_limits<float>::quiet_NaN(),.08f,.9f,true,true)==0);
    for(int j=-100;j<=100;++j) {
        const float command=YokeSC::gustCommand(j*.01f,.08f,.9f,true,true);
        CHECK(std::fabs(command)<=.072001f);
        CHECK(YokeSC::gustCommand(j*.01f,.08f,.9f,false,true)==0);
    }
    CHECK(scaleGust(.1,3)>.299 && scaleGust(.1,3)<.301);
    CHECK(scaleGust(1,10)==1); CHECK(scaleGust(-1,10)==-1);
    CHECK(scaleGust(.1,-1)==0); CHECK(scaleGust(.1,11)==0);
    // Continuity, odd symmetry and full-scale preservation of firmware shaping.
    CHECK(YokeSC::gustCommand(.02f,1,1,true,true)==0);
    CHECK(YokeSC::gustCommand(.020001f,1,1,true,true)<.000002f);
    CHECK(std::fabs(YokeSC::gustCommand(.100001f,1,1,true,true)-
                    YokeSC::gustCommand(.099999f,1,1,true,true))<.000003f);
    CHECK(near(YokeSC::gustCommand(-.4f,1,1,true,true),
              -YokeSC::gustCommand(.4f,1,1,true,true)));
    YokeGust::Model model;
    auto f0=model.update(20,30,40,.02,1,1);
    CHECK(f0.roll==0 && f0.pitch==0); // startup at nonzero wind
    for(int i=0;i<100;++i) {
        f0=model.update(20,30,40,.02,1,1);
        CHECK(f0.roll==0 && f0.pitch==0); // steady wind is not turbulence
    }
    model.reset(); model.update(0,0,0,.02,1,1);
    f0=model.update(5,5,0,.02,1,10);
    CHECK(f0.roll>0 && f0.pitch>0);
    CHECK(std::fabs(f0.roll/f0.pitch-.7)<1e-10); // saturation preserves direction
    for(int i=0;i<1000;++i) {
        f0=model.update(100*std::sin(i*.1),100*std::cos(i*.13),30,.02,1,10);
        CHECK(std::hypot(f0.roll,f0.pitch)<=1.000000001);
    }
    for(int i=0;i<1500;++i) f0=model.update(0,0,0,.02,1,1);
    CHECK(std::hypot(f0.roll,f0.pitch)<1e-9);
    model.reset(); model.update(0,0,0,.02,1,1);
    f0=model.update(0,0,5,.02,1,1);
    CHECK(f0.roll==0 && f0.pitch>0);
    model.reset(); model.update(0,0,0,.02,1,1,0);
    f0=model.update(0,0,5,.02,1,1,0);
    CHECK(f0.roll==0 && f0.pitch==0);
    f0=model.update(5,5,5,.02,1,1,.25,false);
    CHECK(f0.roll==0 && f0.pitch==0); // pause/ground/disarm immediately clears
    f0=model.update(80,90,100,.02,1,1);
    CHECK(f0.roll==0 && f0.pitch==0); // reconnect primes, no kick
    f0=model.update(0,0,0,.3,1,1);
    CHECK(f0.roll==0 && f0.pitch==0); // stale interval
    f0=model.update(0,0,0,.02,1,1);
    CHECK(f0.roll==0 && f0.pitch==0);
    f0=model.update(std::numeric_limits<double>::quiet_NaN(),0,0,.02,1,1);
    CHECK(f0.roll==0 && f0.pitch==0);
    model.update(0,0,0,.02,0,1);
    f0=model.update(5,5,5,.02,0,1);
    CHECK(f0.roll==0 && f0.pitch==0);
    // Gust speed factor accepts 2; invalid speed/tau clear and re-prime the model.
    model.reset(); model.update(0,0,0,.02,2,1);
    CHECK(model.update(0,1,0,.02,2,1).pitch>0);
    CHECK(model.update(0,1,0,.02,2.01,1).pitch==0);
    CHECK(model.update(0,1,0,.02,2,1).pitch==0);
    for(double tau : {0.0,std::numeric_limits<double>::quiet_NaN()}) {
        CHECK(model.update(0,2,0,.02,1,1,.25,true,tau).pitch==0);
        CHECK(model.update(0,1,0,.02,1,1).pitch==0);
    }
    YokeGust::Model shortTau,longTau;
    shortTau.update(0,0,0,.02,1,1,.25,true,.5);
    longTau.update(0,0,0,.02,1,1,.25,true,1);
    double shortH=0,longH=0,shortPrevious=0,longPrevious=0;
    const double blendGust=-std::expm1(-.02/.08);
    for(int i=0;i<50;++i) {
        const double a=shortTau.update(0,5,0,.02,1,1,.25,true,.5).pitch;
        const double b=longTau.update(0,5,0,.02,1,1,.25,true,1).pitch;
        shortH=5*(shortPrevious+(a-shortPrevious)/blendGust);
        longH=5*(longPrevious+(b-longPrevious)/blendGust);
        shortPrevious=a; longPrevious=b;
    }
    CHECK(shortH<longH);
    CHECK(std::fabs(shortH-5*std::exp(-2.0))<1e-10);
    CHECK(std::fabs(longH-5*std::exp(-1.0))<1e-10);
    // Compare same continuous wind at 30/60/120 Hz (discretization tolerance).
    double results[3];
    for(int j=0;j<3;++j) {
        const int hz=30*(1<<j);
        YokeGust::Model rate;
        rate.update(0,0,0,0,1,1);
        for(int i=1;i<=hz*2;++i)
            f0=rate.update(0,std::sin(2.0*i/hz),0,1.0/hz,1,1);
        results[j]=f0.pitch;
    }
    CHECK(std::fabs(results[0]-results[2])<.005);
    CHECK(std::fabs(results[1]-results[2])<.003);
    // Solo SimConnect: no hacen falta efectos ni habilitacion HID del host.
    YokeSC::Receiver integrated;
    YokeSC::Effects onlyTelemetry;
    CHECK(onlyTelemetry.command(integrated,0,1)==0);
    onlyTelemetry.update(integrated,0,true);
    CHECK(onlyTelemetry.command(integrated,0,1)==0);
    frame(f,1,true,5000,3000,4000); feed(integrated,f,100);
    onlyTelemetry.update(integrated,100,true);
    CHECK(onlyTelemetry.command(integrated,0,1)>0);
    CHECK(onlyTelemetry.command(integrated,1,1)>0);
    CHECK(onlyTelemetry.command(integrated,0,0)==0);
    onlyTelemetry.update(integrated,400,true);
    CHECK(onlyTelemetry.command(integrated,0,1)==0);
    CHECK(onlyTelemetry.command(integrated,1,1)==0);
    // v2 envelopes: CRC, ranges, sequence, fallback to v1, stream recovery.
    uint8_t packet[20]; YokeSC::Receiver extended;
    frame2(packet,1); feed2(extended,packet,100);
    CHECK(extended.fresh(100));
    for(int i=0;i<3;++i) CHECK(near(extended.buffet[i],1));
    feed2(extended,packet,200); CHECK(extended.last==100);
    for(int bit=0;bit<160;++bit) {
        YokeSC::Receiver bad; frame2(packet,1); packet[bit/8]^=uint8_t(1<<(bit%8));
        feed2(bad,packet,100); CHECK(!bad.fresh(100));
    }
    for(int i=0;i<3;++i) {
        YokeSC::Receiver bad; frame2(packet,1);
        YokeSC::put16(packet+12+2*i,10001); YokeSC::put16(packet+18,YokeSC::crc(packet,18));
        feed2(bad,packet,100); CHECK(!bad.fresh(100));
    }
    frame(f,2); feed(extended,f,210);
    for(int i=0;i<3;++i) CHECK(extended.buffet[i]==0);
    YokeSC::Receiver cut; frame2(packet,1);
    for(int i=0;i<12;++i) cut.feed(packet[i],0);
    feed2(cut,packet,50); CHECK(cut.fresh(50));
    frame2(packet,2,false); feed2(cut,packet,60); CHECK(!cut.fresh(60));
    // Model: onset, full stall, deployment/speed response, optional invalid fields.
    YokeBuffet::Input input={150,15,15,0,0,45,60,1,1,1,1};
    auto b=YokeBuffet::calculate(input,true);
    CHECK(b.stall==1 && b.gear==1 && b.spoiler==1);
    b=YokeBuffet::calculate(input,false);
    CHECK(b.stall==0 && b.gear==0 && b.spoiler==0);
    input.aoa=10.9; b=YokeBuffet::calculate(input,true); CHECK(b.stall==0);
    input.aoa=13; b=YokeBuffet::calculate(input,true); CHECK(b.stall>0 && b.stall<1);
    input.aoa=10; input.flapLeft=input.flapRight=1; b=YokeBuffet::calculate(input,true); CHECK(b.stall==0);
    input.aoa=10.5; input.flapLeft=input.flapRight=0;
    CHECK(YokeBuffet::calculate(input,true).stall==0);
    input.flapLeft=input.flapRight=1;
    CHECK(YokeBuffet::calculate(input,true).stall>0);
    input.aoa=15; input.stallAlpha=0; b=YokeBuffet::calculate(input,true);
    CHECK(b.stall==0 && b.gear==1 && b.spoiler==1);
    input.retractable=0; b=YokeBuffet::calculate(input,true); CHECK(b.gear==0);
    input.retractable=1; input.gear=.5; input.spoilerLeft=0;
    b=YokeBuffet::calculate(input,true); CHECK(b.gear==.5 && b.spoiler==.5);
    input.ias=0; b=YokeBuffet::calculate(input,true);
    CHECK(b.stall==0 && b.gear==0 && b.spoiler==0);
    input.ias=150; input.spoilerRight=std::numeric_limits<double>::quiet_NaN();
    b=YokeBuffet::calculate(input,true); CHECK(b.spoiler==0 && b.gear==.5);
    // Each enabled effect produces a signal; other flags can independently disable it.
    for(uint8_t effect=1;effect<=8;effect<<=1) {
        YokeSC::Receiver signal; frame2(packet,1); feed2(signal,packet,100);
        YokeSC::Effects renderer; renderer.update(signal,100,true,effect);
        float peak=0;
        for(uint32_t now=102;now<390;now+=2) {
            renderer.update(signal,now,true,effect);
            const float command=renderer.command(signal,1,1,effect);
            if(std::fabs(command)>peak) peak=std::fabs(command);
            CHECK(std::fabs(command)<=SIMCONNECT_EXTRA_LIMIT);
            CHECK(renderer.command(signal,1,1,0)==0);
        }
        CHECK(peak>0);
        renderer.update(signal,400,true,effect);
        CHECK(renderer.command(signal,1,1,effect)==0); // 300 ms watchdog
        frame2(packet,2); feed2(signal,packet,410);
        renderer.update(signal,410,true,effect);
        renderer.update(signal,412,false,effect);
        CHECK(renderer.command(signal,1,1,effect)==0); // DTR
        frame2(packet,3,false); feed2(signal,packet,414);
        renderer.update(signal,414,true,effect);
        CHECK(renderer.command(signal,1,1,effect)==0); // Desarmado por pausa/suelo/slew
    }
    // Gear load grows with excess IAS^2 above 100 kt, full at 150 kt.
    input.retractable=1; input.gear=1;
    input.ias=100; CHECK(YokeBuffet::calculate(input,true).gear==0);
    input.ias=110; CHECK(near(float(YokeBuffet::calculate(input,true).gear),.168f));
    input.ias=125; CHECK(near(float(YokeBuffet::calculate(input,true).gear),.45f));
    input.ias=150; CHECK(YokeBuffet::calculate(input,true).gear==1);
    input.spoilerLeft=input.spoilerRight=1;
    input.ias=60; CHECK(YokeBuffet::calculate(input,true).spoiler==0);
    input.ias=80; CHECK(YokeBuffet::calculate(input,true).spoiler==0);
    input.ias=110; CHECK(near(float(YokeBuffet::calculate(input,true).spoiler),5700.0f/13200));
    input.ias=140; CHECK(YokeBuffet::calculate(input,true).spoiler==1);

    // Sustained gear-only packets through the real receiver and oscillator.
    // Check both 500 Hz motor commands and the 100 ms LCD sampling cadence.
    YokeSC::Receiver gearSignal;
    YokeSC::Effects gearRenderer;
    float motorLow=1, motorHigh=-1, lcdLow=1, lcdHigh=-1;
    uint16_t gearSequence=0;
    for(uint32_t now=0;now<=3000;now+=2) {
        if(now%20==0) {
            frame2(packet,++gearSequence,true,0,10000,0);
            feed2(gearSignal,packet,now);
        }
        gearRenderer.update(gearSignal,now,true,YokeSC::Gear);
        const float pitch=gearRenderer.command(gearSignal,1,.9f,YokeSC::Gear);
        CHECK(near(gearRenderer.command(gearSignal,0,.9f,YokeSC::Gear),pitch*.5f));
        CHECK(std::fabs(pitch)<=SIMCONNECT_GEAR_BUFFET_GAIN*.9f+.000001f);
        if(now>=1000) {
            if(pitch<motorLow) motorLow=pitch;
            if(pitch>motorHigh) motorHigh=pitch;
            if(now%100==0) {
                if(pitch<lcdLow) lcdLow=pitch;
                if(pitch>lcdHigh) lcdHigh=pitch;
            }
        }
    }
    CHECK(motorLow<-.01f && motorHigh>.01f);
#if SIMCONNECT_GEAR_BUFFET_HZ == 2
    CHECK(lcdLow<-.01f && lcdHigh>.01f);
#endif
    // Document why the previous 10 Hz carrier can look stationary on the LCD.
    for(uint32_t now=0;now<=1000;now+=100)
        CHECK(near(YokeSC::triangle(uint16_t((250+now*10)%1000)),0));
    std::printf("Gear at %d Hz: pitch PWM-equivalent range %.2f..%.2f; LCD samples %.2f..%.2f\n",
        SIMCONNECT_GEAR_BUFFET_HZ,motorLow*255,motorHigh*255,lcdLow*255,lcdHigh*255);

    for(int phase=0;phase<1000;++phase) {
        CHECK(std::fabs(YokeSC::triangle(uint16_t(phase)))<=1.000001f);
        CHECK(std::fabs(YokeSC::triangle(uint16_t(phase))+
              YokeSC::triangle(uint16_t((phase+500)%1000)))<.000001f);
    }
    std::printf("PASS: %d checks (SC1-SC8, watchdog, effects, motion sources, passive loads, PWM limit).\n",checks);
}
