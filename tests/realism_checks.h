static void realismChecks() {
    const float nan=std::numeric_limits<float>::quiet_NaN();
    YokeNoise::Narrowband noise(13,.002f,.15f,.45f,0x9E3779B9u);
    std::vector<float> samples(50000);
    double sum=0,squares=0,cross=0,lagSquares=0,triCross=0,triSquares=0;
    int crossings=0;
    for(int i=0;i<50000;++i) {
        samples[i]=noise.next(); sum+=samples[i]; squares+=double(samples[i])*samples[i];
        if(i && (samples[i]>=0)!=(samples[i-1]>=0)) ++crossings;
        if(i>=38) {
            cross+=double(samples[i])*samples[i-38]; lagSquares+=double(samples[i-38])*samples[i-38];
            const float tri=YokeSC::triangle(uint16_t((250+i*26)%1000));
            const float old=YokeSC::triangle(uint16_t((250+(i-38)*26)%1000));
            triCross+=double(tri)*old; triSquares+=double(old)*old;
        }
    }
    const double rms=std::sqrt(squares/50000),hz=crossings/(2*100.0);
    CHECK(rms>=.405 && rms<=.495); CHECK(std::fabs(sum/50000)<.02);
    CHECK(hz>=10.4 && hz<=15.6); CHECK(cross/lagSquares<.8); CHECK(triCross/triSquares>.99);
    noise.reset(); for(int i=0;i<50000;++i) CHECK(noise.next()==samples[i]);
    YokeNoise::Narrowband invalid[]={
        YokeNoise::Narrowband(0,.002f,.15f,.45f,1),YokeNoise::Narrowband(13,.002f,0,.45f,1),
        YokeNoise::Narrowband(13,.002f,1,.45f,1),YokeNoise::Narrowband(13,.002f,2,.45f,1)};
    for(int i=0;i<4;++i) for(int j=0;j<100;++j) CHECK(invalid[i].next()==0);
    for(int sign=-1;sign<=1;sign+=2) {
        YokeFriction::Model friction;
        CHECK(friction.update(0,.03f,.02f)==0);
        CHECK(near(friction.update(sign*.01f,.03f,.02f),sign*-.015f));
        CHECK(near(friction.update(sign*.05f,.03f,.02f),sign*-.03f));
        CHECK(near(friction.update(sign*.04f,.03f,.02f),sign*-.015f));
        CHECK(near(friction.update(sign*.02f,.03f,.02f),sign*.015f));
        const float held=friction.update(sign*.02f,.03f,.02f);
        CHECK(friction.update(sign*.02f,.03f,.02f)==held);
        CHECK(friction.update(0,0,.02f)==0); CHECK(friction.update(0,nan,.02f)==0);
    }
    CHECK(YokeSC::motionAdjustedForce(.2f,1,1,true,true,YokeGains::defaults(),-1)==.2f);
    CHECK(YokeSC::motionAdjustedForce(.2f,1,1,true,true,YokeGains::defaults(),nan)==.2f);
    // A zero-duration update preserves carriers; reset reproduces their sequence.
    {
        YokeSC::Effects fx; YokeSC::Receiver rx; uint8_t bytes[20]; float first[100];
        for(int pass=0;pass<2;++pass) {
            fx.reset();
            const uint32_t start=uint32_t(pass*1000);
            frame2(bytes,uint16_t(pass*100+1)); feed2(rx,bytes,start); fx.update(rx,start,true);
            for(int i=0;i<100;++i) {
                const uint32_t now=start+uint32_t((i+1)*2);
                fx.update(rx,now,true);
                const float value=fx.command(rx,1,1,YokeSC::Stall);
                if(pass==0) first[i]=value; else CHECK(value==first[i]);
                fx.update(rx,now,true); CHECK(fx.command(rx,1,1,YokeSC::Stall)==value);
            }
        }
    }
    // Check the scale via the damper contribution, with the spring at zero displacement.
    const float pressures[]={0,1,4},scales[]={.5f,1,1.8841444f};
    for(int k=0;k<3;++k) {
        YokeSC::Effects fx; YokeSC::Receiver rx; uint8_t bytes[26];
        for(uint32_t t=0;t<=4000;t+=2) {
            if(t%20==0) {
                frame4(bytes,uint16_t(t/20+1),uint16_t(pressures[k]*10000),0,0);
                bytes[2]=6; YokeSC::put16(bytes+24,YokeSC::crc(bytes,24)); feed4(rx,bytes,t);
            }
            fx.update(rx,t,true);
        }
        const float nominal=YokeSC::motionAdjustedForce(0,1,1,true,false);
#if YOKE_DAMPER_STIFFNESS_SCALING
        CHECK(std::fabs(fx.command(rx,1,1,YokeSC::Centering|YokeSC::Damper,0,1)/nominal-scales[k])<.0001f);
#else
        CHECK(fx.command(rx,1,1,YokeSC::Centering|YokeSC::Damper,0,1)==nominal);
#endif
        CHECK(fx.command(rx,1,1,YokeSC::Damper,0,1)==nominal);
        rx.centeringAvailable=false;
        CHECK(fx.command(rx,1,1,YokeSC::Centering|YokeSC::Damper,0,1)==nominal);
        for(int axis=0;axis<2;++axis) for(int sign=-1;sign<=1;sign+=2) for(int g=1;g<=10;++g) {
            const float gain=g*.1f,ceiling=YOKE_MOTION_CORRECTION_LIMIT*gain*(axis==0?.5f:1);
            fx.command(rx,uint8_t(axis),gain,YokeSC::Damper,0,0); // clears the anchor
            fx.command(rx,uint8_t(axis),gain,YokeSC::Friction,0,0);
            const float resistance=fx.command(rx,uint8_t(axis),gain,YokeSC::Damper|YokeSC::Friction,sign*.05f,sign*10.0f);
            CHECK(std::fabs(resistance)<=ceiling+.000001f); CHECK(resistance*sign<0);
        }
    }
}
