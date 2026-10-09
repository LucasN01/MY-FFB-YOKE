// Included after the existing packet helpers and CHECK macro.
static void groundChecks() {
    using YokeGround::Model;
    Model half, full, fast;
    for(int i=0;i<200;++i) {
        const double x=i<10 ? 0 : 3*std::sin(i*.3);
        const double y=i<10 ? 32 : 32+6*std::sin(i*.7);
        const auto h=half.update(x,y,.02,1,20.25,true);
        const auto f=full.update(x,y,.02,1,40,true);
        const auto z=fast.update(x,y,.02,1,100,true);
        CHECK(std::fabs(h.roll-.5*f.roll)<1e-12);
        CHECK(std::fabs(h.pitch-.5*f.pitch)<1e-12);
        CHECK(f.roll==z.roll && f.pitch==z.pitch);
        CHECK(std::fabs(f.roll)<=1 && std::fabs(f.pitch)<=1);
        if(i<10) CHECK(f.roll==0 && f.pitch==0); // static load is silent
    }
    Model m;
    CHECK(m.update(0,32,.02,1,40,true).roll==0);
    CHECK(m.update(0,48,.02,1,40,true).roll>0); // actual impulse
    for(int i=0;i<150;++i) m.update(0,48,.02,1,40,true);
    CHECK(std::fabs(m.update(0,48,.02,1,40,true).pitch)<1e-9);
    for(int mode=0;mode<6;++mode) {
        m.reset(); m.update(0,32,.02,1,40,true);
        m.update(0,48,.02,1,40,true);
        const auto f=m.update(mode==5?std::numeric_limits<double>::quiet_NaN():0,32,
            mode==4?.3:.02,mode==2?0:1,mode==0?0:(mode==1?.5:40),mode!=3);
        CHECK(f.roll==0 && f.pitch==0);
        CHECK(m.update(0,32,.02,1,40,true).pitch==0); // reseed after each interruption
    }
    uint8_t packet[30];
    auto make=[&](uint16_t seq,int16_t r,int16_t p,uint16_t level=5000) {
        frame4(packet,seq,10000,0,0); packet[2]=7;
        YokeSC::put16(packet+18,level);
        YokeSC::put16(packet+24,uint16_t(r)); YokeSC::put16(packet+26,uint16_t(p));
        YokeSC::put16(packet+28,YokeSC::crc(packet,28));
    };
    auto send=[&](YokeSC::Receiver& rx,uint32_t now) {
        for(int j=0;j<30;++j) rx.feed(packet[j],now);
    };
    for(int bit=0;bit<240;++bit) {
        YokeSC::Receiver bad; make(1,3000,-4000);
        packet[bit/8]^=uint8_t(1<<(bit%8)); send(bad,100);
        CHECK(!bad.fresh(100));
    }
    YokeSC::Receiver rx; YokeSC::Effects fx;
    make(1,3000,-4000); send(rx,100); fx.update(rx,100,true);
    CHECK(rx.groundForceAvailable && near(rx.groundForce[1],-.4f));
    CHECK(fx.command(rx,0,1,YokeSC::Ground)==0); // startup guard
    fx.update(rx,102,true);
    const float roll=fx.command(rx,0,1,YokeSC::Ground);
    CHECK(near(roll,.3f*YokeGains::defaults().effective(YokeGains::Ground)));
    CHECK(fx.command(rx,1,1,YokeSC::Ground)<0); // independent signed axes
    for(uint32_t t=104;t<180;t+=2) {
        fx.update(rx,t,true);
        CHECK(near(fx.command(rx,0,1,YokeSC::Ground),roll)); // no 8-Hz carrier
    }
    make(2,10001,0); send(rx,180); CHECK(rx.last==100);
    make(2,0,-10001); send(rx,180); CHECK(rx.last==100);
    make(2,10000,-10000); send(rx,180); fx.update(rx,180,true);
    CHECK(std::fabs(fx.command(rx,0,1,YokeSC::Ground))<=SIMCONNECT_GROUND_MAX_PWM/255.f);
    CHECK(std::fabs(fx.command(rx,1,1,YokeSC::Ground))<=SIMCONNECT_GROUND_MAX_PWM/255.f/SIMCONNECT_GROUND_ROLL_PITCH_RATIO);
    make(3,5000,-5000,0); send(rx,200); fx.update(rx,200,true);
    CHECK(fx.command(rx,0,1,YokeSC::Ground)==0); // stop gate, no tail
    make(4,5000,-5000); send(rx,220); fx.update(rx,220,true);
    fx.update(rx,520,true); CHECK(fx.command(rx,0,1,YokeSC::Ground)==0);
    make(5,5000,-5000); send(rx,540); fx.update(rx,540,true); fx.update(rx,542,true);
    CHECK(fx.command(rx,0,1,YokeSC::Ground)>0);
    fx.update(rx,544,false); CHECK(fx.command(rx,0,1,YokeSC::Ground)==0);
    make(6,5000,-5000); packet[3]=2;
    YokeSC::put16(packet+28,YokeSC::crc(packet,28)); send(rx,560); fx.update(rx,560,true);
    CHECK(!rx.fresh(560) && fx.command(rx,0,1,YokeSC::Ground)==0);
    frame4(packet,7,10000,0,0); feed4(rx,packet,580);
    CHECK(!rx.groundForceAvailable && rx.groundForce[0]==0 && rx.groundForce[1]==0);
}
