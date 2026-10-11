#include <fstream>
#include <sstream>
#include <string>
#include <vector>
static void rawFrame(uint8_t* p,const YokeRaw::Sample& s,uint16_t seq,uint8_t flags=1) {
    p[0]=0xa5; p[1]=0x5a; p[2]=8; p[3]=flags;
    YokeSC::put16(p+4,seq); YokeRaw::encodePayload(p,s);
    YokeSC::put16(p+111,YokeSC::crc(p,111));
}
static void rawFeed(YokeSC::Receiver& rx,const uint8_t* p,uint32_t now) {
    for(int i=0;i<113;++i) rx.feed(p[i],now);
}
static void rawChecks() {
    // Smooth runway: constant acceleration still yields a small bipolar texture.
    {
        YokeGains::Settings gains; gains.set(YokeGains::Ground,4500);
        YokeSC::Effects f(gains); YokeSC::Receiver r; YokeRaw::Sample sample;
        sample.value[YokeRaw::OnGround]=1; sample.value[YokeRaw::GroundSpeed]=40;
        uint8_t bytes[113]; float low=0,high=0;
        for(uint32_t t=100;t<2100;t+=2) {
            if(t%20==0) { sample.timeMs=t; rawFrame(bytes,sample,uint16_t(t)); rawFeed(r,bytes,t); }
            f.update(r,t,true,YokeSC::Ground);
            const float u=f.command(r,0,.9f,YokeSC::Ground);
            if(u<low) low=u; if(u>high) high=u;
            CHECK(std::fabs(u)<=.450f*.9f*.12f+.000001f);
            CHECK(f.calculated().ground[0]==0); // telemetry bump model stays silent
        }
        CHECK(low*255<-10 && high*255>10);
        sample.timeMs=2100; sample.value[YokeRaw::GroundSpeed]=.5f;
        rawFrame(bytes,sample,2100); rawFeed(r,bytes,2100); f.update(r,2100,true,YokeSC::Ground);
        CHECK(f.command(r,0,.9f,YokeSC::Ground)==0);
        sample.timeMs=2120; sample.value[YokeRaw::GroundSpeed]=60; sample.value[YokeRaw::OnGround]=0;
        rawFrame(bytes,sample,2120); rawFeed(r,bytes,2120); f.update(r,2120,true,YokeSC::Ground);
        CHECK(f.command(r,0,.9f,YokeSC::Ground)==0);
    }
    YokeRaw::Sample s; s.timeMs=100; s.value[YokeRaw::OnGround]=1;
    s.value[YokeRaw::GroundSpeed]=40;
    uint8_t p[113];
    for(int bit=0;bit<904;++bit) {
        YokeSC::Receiver rx; rawFrame(p,s,1); p[bit/8]^=1<<(bit%8);
        rawFeed(rx,p,100); CHECK(!rx.fresh(100));
    }
    YokeSC::Receiver rx; rawFrame(p,s,1); rawFeed(rx,p,100);
    CHECK(rx.fresh(100) && rx.rawAvailable && rx.raw.timeMs==100);
    rawFrame(p,s,2); rawFeed(rx,p,120);
    CHECK(rx.last==100); // repeated capture cannot renew watchdog
    s.timeMs=120; rawFrame(p,s,3); rawFeed(rx,p,120);
    CHECK(rx.last==120 && rx.raw.timeMs==120);
    s.timeMs=140; rawFrame(p,s,4); rawFeed(rx,p,120);
    CHECK(rx.raw.timeMs==140); // batched USB arrival is not the model clock
    s.timeMs=130; rawFrame(p,s,5); rawFeed(rx,p,140);
    CHECK(rx.raw.timeMs==140);
    rawFrame(p,s,6,2); rawFeed(rx,p,140); CHECK(!rx.fresh(140));
    s.timeMs=160; s.value[YokeRaw::Slew]=1;
    rawFrame(p,s,7); rawFeed(rx,p,160); CHECK(!rx.fresh(160));
    s.value[YokeRaw::Slew]=0; s.timeMs=180;
    s.value[YokeRaw::SpoilerLeft]=std::numeric_limits<float>::quiet_NaN();
    rawFrame(p,s,8); rawFeed(rx,p,180); CHECK(rx.fresh(180));
    CHECK(!rx.fresh(480));
    YokeSC::Receiver wrap; s.timeMs=0xfffffff0;
    rawFrame(p,s,65535); rawFeed(wrap,p,100);
    s.timeMs=4; rawFrame(p,s,0); rawFeed(wrap,p,120);
    CHECK(wrap.fresh(120) && wrap.raw.timeMs==4);
    // Every essential field rejects non-finite values; optional bad accelerations silence ground.
    const int essential[]={YokeRaw::PressurePSF,YokeRaw::IAS,YokeRaw::WindX,YokeRaw::WindY,
        YokeRaw::WindZ,YokeRaw::OnGround,YokeRaw::Slew,YokeRaw::YokeRoll,YokeRaw::YokePitch};
    for(int field:essential) {
        auto bad=s; bad.value[field]=std::numeric_limits<float>::quiet_NaN();
        YokeSC::Receiver r; rawFrame(p,bad,1); rawFeed(r,p,100); CHECK(!r.fresh(100));
    }
    YokeSC::PicoForceModels half,full,fast;
    s.value[YokeRaw::PressurePSF]=100;
    for(int i=0;i<200;++i) {
        s.timeMs=100+i*20; s.value[YokeRaw::AccelY]=float(8*std::sin(i*.7));
        s.value[YokeRaw::GroundSpeed]=20.25; half.update(s);
        s.value[YokeRaw::GroundSpeed]=40; full.update(s);
        s.value[YokeRaw::GroundSpeed]=80; fast.update(s);
        CHECK(near(half.ground[0],full.ground[0]*.5f));
        CHECK(near(half.ground[1],full.ground[1]*.5f));
        CHECK(full.ground[0]==fast.ground[0] && full.ground[1]==fast.ground[1]);
        const float previous=full.ground[1]; s.value[YokeRaw::AccelY]=999;
        full.update(s); CHECK(full.ground[1]==previous); // duplicate does not erase filter
    }
    CHECK(std::fabs(full.pressure-2.05121)<.001);
    s.timeMs+=20; s.value[YokeRaw::GroundSpeed]=.5f; full.update(s);
    CHECK(full.ground[0]==0 && full.ground[1]==0);
    YokeGround::Model g; g.update(0,0,.02,1,40,true);
    const auto force=g.update(0,8,.02,1,40,true);
    CHECK(force.pitch>0 && g.update(0,8,0,1,40,true).pitch==force.pitch);
    // Airborne SC8 integrates the same validated models, including runtime options.
    YokeSC::PicoForceModels air; YokeGust::Model referenceWind;
    s=YokeRaw::Sample{}; s.options=YokeRaw::Gusts|YokeRaw::Buffets;
    s.value[YokeRaw::IAS]=125; s.value[YokeRaw::Gear]=1; s.value[YokeRaw::Retractable]=1;
    s.value[YokeRaw::SpoilerLeft]=s.value[YokeRaw::SpoilerRight]=.5f;
    s.value[YokeRaw::AoA]=12; s.value[YokeRaw::StallAlpha]=15;
    s.value[YokeRaw::VS0]=45; s.value[YokeRaw::VS1]=60;
    s.value[YokeRaw::PressurePSF]=2000;
    for(int i=0;i<100;++i) {
        s.timeMs=100+i*20; s.value[YokeRaw::WindX]=float(std::sin(i*.2)*5);
        air.update(s);
        const auto w=referenceWind.update(s.value[YokeRaw::WindX],0,0,.02,2.0,1,.25,true,.25);
        CHECK(near(air.gust[0],float(w.roll)) && near(air.gust[1],float(w.pitch)));
        CHECK(air.pressure==4 && near(air.buffet[1],.45f) && near(air.buffet[2],.5f*9225.0f/13200));
        CHECK(air.buffet[0]>0 && air.ground[0]==0 && air.ground[1]==0);
    }
    s.timeMs+=20; s.options=0; air.update(s);
    CHECK(air.gust[0]==0 && air.gust[1]==0 && air.buffet[0]==0 && air.buffet[1]==0 && air.buffet[2]==0);
    // 120 kt equivalent pressure: unit gust speed and plunge tau about 0.478 s.
    YokeSC::PicoForceModels referenceAir;
    s=YokeRaw::Sample{}; s.options=YokeRaw::Gusts;
    s.value[YokeRaw::PressurePSF]=float(.5*1.225*std::pow(120*.514444444,2)/47.88025898);
    referenceAir.update(s);
    s.timeMs=20; s.value[YokeRaw::WindY]=5; referenceAir.update(s);
    CHECK(near(referenceAir.pressure,1));
    const double firstHigh=referenceAir.gust[1]/(-std::expm1(-.02/.08));
    const double inferredTau=-.02/std::log(firstHigh);
    CHECK(std::fabs(inferredTau-.478)<.005);
    s.value[YokeRaw::OnGround]=1;
    const float speeds[]={10,10.375f,20.25f,40};
    const float textureLevels[]={float(std::sqrt(9.5/39.5)),.5f,float(std::sqrt(.5)),1};
    for(int i=0;i<4;++i) {
        s.timeMs+=20; s.value[YokeRaw::GroundSpeed]=speeds[i]; referenceAir.update(s);
        CHECK(near(referenceAir.groundEnvelope,textureLevels[i]));
    }
    // The fast effects loop must not advance/reset a dynamic filter between packets.
    YokeSC::Receiver live; YokeSC::Effects effects;
    s=YokeRaw::Sample{}; s.value[YokeRaw::OnGround]=1; s.value[YokeRaw::GroundSpeed]=40;
    s.timeMs=100; rawFrame(p,s,1); rawFeed(live,p,100);
    effects.update(live,100,true,YokeSC::Ground); effects.update(live,102,true,YokeSC::Ground);
    for(int t=104;t<120;t+=2) effects.update(live,t,true,YokeSC::Ground);
    s.timeMs=120; s.value[YokeRaw::AccelY]=8; rawFrame(p,s,2); rawFeed(live,p,120);
    effects.update(live,120,true,YokeSC::Ground);
    const float held=effects.calculated().ground[1]; CHECK(held>0);
    for(int t=122;t<420;t+=2) {
        effects.update(live,t,true,YokeSC::Ground); CHECK(effects.calculated().ground[1]==held);
    }
    effects.update(live,420,true,YokeSC::Ground);
    CHECK(effects.command(live,0,.9f,YokeSC::Ground)==0 && effects.calculated().ground[1]==0);
}
// Replay the user's SC7 capture through the actual SC8 codec, receiver and Pico effects.
// SimConnect batches are coalesced to the latest sample; no synthetic dt is introduced.
static void replayRaw(const char* path) {
    std::ifstream file(path); CHECK(file.good()); std::string line;
    std::getline(file,line); std::vector<YokeRaw::Sample> batches;
    int rows=0,oldNonzero=0;
    while(std::getline(file,line)) {
        std::istringstream stream(line); std::string cell; std::vector<double> v;
        while(std::getline(stream,cell,',')) v.push_back(std::strtod(cell.c_str(),nullptr));
        CHECK(v.size()==42); ++rows;
        if(v[40]!=0 || v[41]!=0) ++oldNonzero;
        YokeRaw::Sample s; s.timeMs=uint32_t(v[0]);
        const int cols[22]={2,3,10,11,12,6,-1,18,19,20,21,22,23,24,25,26,27,32,36,37,38,39};
        for(int i=0;i<22;++i) s.value[i]=cols[i]<0 ? 0 : float(v[cols[i]]);
        s.value[0]/=47.88025898f;
        if(!batches.empty() && batches.back().timeMs==s.timeMs) batches.back()=s;
        else batches.push_back(s);
    }
    YokeGains::Settings settings; settings.set(YokeGains::Ground,4500);
    settings.set(YokeGains::Total,9000);
    YokeSC::Effects fx(settings); YokeSC::Receiver rx; uint8_t packet[113];
    uint16_t seq=0; uint32_t nextTick=1000,lastSend=0,origin=batches.front().timeMs;
    int nonzero=0,peakR=0,peakP=0,sent=0; float maxForce=0;
    for(const auto& s:batches) {
        uint32_t now=1000+s.timeMs-origin;
        if(lastSend && now-lastSend<20) continue;
        while(nextTick<now) { fx.update(rx,nextTick,true,YokeSC::Ground); nextTick+=2; }
        rawFrame(packet,s,++seq); rawFeed(rx,packet,now); lastSend=now; ++sent;
        fx.update(rx,now,true,YokeSC::Ground);
        const float r=fx.command(rx,0,.9f,YokeSC::Ground),p=fx.command(rx,1,.9f,YokeSC::Ground);
        int pr=int(std::round(std::fabs(r)*255)),pp=int(std::round(std::fabs(p)*255));
        if(pr || pp) ++nonzero;
        if(pr>peakR) peakR=pr; if(pp>peakP) peakP=pp;
        if(std::fabs(fx.calculated().ground[1])>maxForce) maxForce=std::fabs(fx.calculated().ground[1]);
        CHECK(pr<=40 && pp<=4);
        if(s.value[YokeRaw::GroundSpeed]<=.5f) CHECK(pr==0 && pp==0);
        nextTick=now+2;
    }
    CHECK(rows==5670 && oldNonzero==8 && nonzero>200 && peakR==40 && peakP==4);
    std::printf("SC8 CSV replay: %d rows, %zu batches, %d packets, %d nonzero ground PWM samples; peaks R=%d P=%d, model pitch=%.4f (gain .450, total .90).\n",
        rows,batches.size(),sent,nonzero,peakR,peakP,maxForce);
}
