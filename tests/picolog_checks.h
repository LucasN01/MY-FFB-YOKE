#include "../PicoLog.h"
#include "../SimConnectBridge/PicoCsv.h"
#include "comparar_ensayo.h"
static std::string replayLine(const YokeReplay::ReplayRow& r,unsigned index) {
    std::ostringstream line; line<<std::setprecision(9)<<r.host<<","<<r.sample<<","<<index;
    for(float v:r.v) line<<","<<v;
    line<<","<<r.paused<<","<<r.running<<","<<r.armed<<","<<r.sent<<","<<r.ref<<","<<r.gain<<","<<r.longi<<","<<r.gusts<<","<<r.buffets;
    return line.str();
}
static void replayAndComparatorChecks() {
    CHECK(YokeReplay::parseHeader(YokeReplay::header));
    std::string header=YokeReplay::header; header[0]='X'; CHECK(!YokeReplay::parseHeader(header));
    YokeReplay::ReplayRow r; r.host=1000; r.sample=100; r.armed=r.running=r.sent=r.buffets=true;
    r.v[YokeRaw::OnGround]=1; r.v[YokeRaw::PressurePSF]=100; r.v[YokeRaw::GroundSpeed]=40;
    YokeReplay::ReplayRow decoded; const std::string valid=replayLine(r,1);
    CHECK(YokeReplay::parseRow(valid,decoded) && decoded.sample==100 && decoded.armed);
    CHECK(!YokeReplay::parseRow(valid.substr(0,valid.rfind(',')),decoded));
    auto fields=YokeReplay::split(valid);
    auto joined=[&]() { std::string s; for(const auto& f:fields) { if(!s.empty()) s+=","; s+=f; } return s; };
    fields[3+YokeRaw::SpoilerLeft]="-nan(ind)";
    CHECK(YokeReplay::parseRow(joined(),decoded) && std::isnan(decoded.v[YokeRaw::SpoilerLeft]));
    fields[3+YokeRaw::SpoilerLeft]="NaN";
    CHECK(YokeReplay::parseRow(joined(),decoded) && std::isnan(decoded.v[YokeRaw::SpoilerLeft]));
    for(const char* token:{"-nan","nan(snan)","-nan(snan)"}) {
        fields[3+YokeRaw::SpoilerLeft]=token;
        CHECK(YokeReplay::parseRow(joined(),decoded) && std::isnan(decoded.v[YokeRaw::SpoilerLeft]));
    }
    fields[3+YokeRaw::SpoilerLeft]="INF";
    CHECK(YokeReplay::parseRow(joined(),decoded) && std::isinf(decoded.v[YokeRaw::SpoilerLeft]) && decoded.v[YokeRaw::SpoilerLeft]>0);
    fields[3+YokeRaw::SpoilerLeft]="-inf";
    CHECK(YokeReplay::parseRow(joined(),decoded) && std::isinf(decoded.v[YokeRaw::SpoilerLeft]) && decoded.v[YokeRaw::SpoilerLeft]<0);
    fields[3+YokeRaw::SpoilerLeft]="abc"; CHECK(!YokeReplay::parseRow(joined(),decoded));
    YokeEnsayo::Input in; in.bridgeCounts=true; in.lines=30; in.gains=1;
    YokeSC::PicoForceModels models;
    for(unsigned i=0;i<30;++i) {
        r.host=1000+i*20; r.sample=100+i*20; r.v[YokeRaw::AccelX]=float(std::sin(i*.4)*4);
        r.v[YokeRaw::AccelY]=float(std::cos(i*.3)*6);
        CHECK(YokeReplay::parseRow(replayLine(r,i+1),decoded)); in.tel.push_back(decoded);
        models.update(YokeReplay::payload(decoded));
        YokeLog::Frame f{}; f.seq=uint16_t(i+1); f.t=r.sample; f.rev=i+1;
        f.flags=YokeLog::Active|YokeLog::Armed|YokeLog::Dtr|YokeLog::Raw|YokeLog::Modeled;
        f.q=models.pressure; f.env=models.groundEnvelope;
        for(int axis=0;axis<2;++axis) { f.gust[axis]=models.gust[axis]; f.ground[axis]=models.ground[axis]; }
        for(int j=0;j<3;++j) f.buffet[j]=models.buffet[j];
        char line[YOKE_LOG_M_MAX],row[512]; const int n=YokeLog::formatModel(line,sizeof(line),f);
        CHECK(n>0); line[n-1]=0; CHECK(YokeCsv::modelRow(line+1,row,sizeof(row)));
        YokeEnsayo::PicoRow p; CHECK(YokeEnsayo::picoRow(std::to_string(r.host)+","+row,p)); in.pico.push_back(p);
    }
    auto result=YokeEnsayo::compare(in,"sintetico");
    CHECK(result.aOk && result.bOk && result.code()==0 && result.matched==30);
    auto changed=in; changed.pico[10].v[10]+=.01;
    result=YokeEnsayo::compare(changed,"alterado");
    CHECK(!result.aOk && result.report.find("groundR@300:")!=std::string::npos);
    changed=in; for(size_t i=5;i<changed.pico.size();++i) changed.pico[i].v[25]=1;
    result=YokeEnsayo::compare(changed,"skip"); CHECK(result.excluded>0 && result.aOk);
    changed=in; for(auto& row:changed.pico) row.v[25]=5;
    result=YokeEnsayo::compare(changed,"skip previo"); CHECK(result.excluded==0 && result.aOk);
    changed=in; changed.tel.resize(4); changed.pico.resize(4);
    const uint16_t sequences[]={65534,65535,0,1};
    for(size_t i=0;i<4;++i) changed.pico[i].v[0]=sequences[i];
    result=YokeEnsayo::compare(changed,"wrap");
    CHECK(result.violations[7]==0 && result.report.find("puente=0.000%")!=std::string::npos);
    changed=in; changed.tel.resize(4); changed.pico.resize(2);
    changed.tel[1]=changed.tel[0]; changed.tel[1].host=1100;
    changed.tel[2]=changed.tel[0]; changed.tel[2].host=1200;
    changed.tel[3]=changed.tel[0]; changed.tel[3].host=1301; changed.tel[3].sample=200;
    changed.tel[3].v[YokeRaw::AccelX]=changed.tel[3].v[YokeRaw::AccelY]=20;
    changed.pico[1]=changed.pico[0]; changed.pico[1].host=1301;
    changed.pico[1].v[0]=2; changed.pico[1].v[1]=200; changed.pico[1].v[2]=2;
    result=YokeEnsayo::compare(changed,"watchdog con duplicados"); CHECK(result.aOk);
    YokeEnsayo::Input pause=in; pause.tel.resize(3); pause.pico.resize(3);
    for(size_t i=0;i<3;++i) {
        pause.tel[i].host=900+i*100; pause.tel[i].armed=i==1;
        pause.pico[i].host=1000+i*80; pause.pico[i].v[18]=i==2 ? 0 : .1;
        pause.pico[i].v[19]=0;
    }
    result=YokeEnsayo::compare(pause,"pausa 60ms");
    CHECK(result.violations[6]==0 && result.report.find("transiciones=1 evaluadas=1 latencia_max=60ms")!=std::string::npos);
    pause.pico[2].host=1300;
    result=YokeEnsayo::compare(pause,"pausa 200ms"); CHECK(result.violations[6]==1);
    pause.tel.push_back(pause.tel.back()); pause.tel.back().host=1200; pause.tel.back().armed=true;
    result=YokeEnsayo::compare(pause,"rearmado 100ms");
    CHECK(result.violations[6]==0 && result.report.find("B6: SIN DATOS")!=std::string::npos);
    pause.tel.resize(2); pause.tel[0].paused=true; pause.tel[0].armed=false;
    result=YokeEnsayo::compare(pause,"pausa inicial");
    CHECK(result.violations[6]==0 && result.report.find("transiciones=0 evaluadas=0")!=std::string::npos);
    changed=in; changed.tel.resize(4);
    const uint64_t phaseHosts[]={1000,4000,4500,8000};
    for(size_t i=0;i<4;++i) { changed.tel[i].host=phaseHosts[i]; changed.tel[i].v[YokeRaw::GroundSpeed]=i==1 ? 40 : 0; }
    result=YokeEnsayo::compare(changed,"fusion de fases");
    CHECK(result.phases.find("01 PARADO")!=std::string::npos && result.phases.find("02 ")==std::string::npos);
    changed=in; changed.picoHost=false; for(auto& row:changed.pico) row.host=0;
    result=YokeEnsayo::compare(changed,"fallback");
    CHECK(result.report.find("Pico por fase: SIN DATOS")!=std::string::npos);
    CHECK(result.phases.find(",SIN DATOS,SIN DATOS")!=std::string::npos);
    changed=in; changed.pico.erase(changed.pico.begin()+10);
    result=YokeEnsayo::compare(changed,"drop");
    CHECK(result.report.find("no_emparejadas_drop=1")!=std::string::npos);
    changed=in; for(auto& row:changed.tel) row.armed=false;
    result=YokeEnsayo::compare(changed,"monitor");
    CHECK(result.noData && result.code()==2 && result.report.find("RESULTADO: SIN DATOS")!=std::string::npos);
}
static void picoLogChecks() {
    replayAndComparatorChecks();
    uint16_t drop=0; uint32_t lastLoggedRev=0;
    const uint32_t revisions[]={5,0,0,9};
    for(uint32_t r:revisions) {
        if(r==0) lastLoggedRev=0;
        drop=YokeLog::nextDrop(drop,r,lastLoggedRev,0,0);
        if(r!=0) lastLoggedRev=r;
        CHECK(drop==0);
    }
    CHECK(YokeLog::nextDrop(0,8,5,0,0)==2);
    CHECK(YokeLog::nextDrop(0,8,5,4,3)==1);
    CHECK(YokeLog::nextDrop(0,8,5,6,3)==0);
    CHECK(YokeLog::nextDrop(65534,8,5,0,0)==65535);
    CHECK(YokeLog::nextDrop(65535,8,5,0,0)==65535);
    CHECK(YokeLog::nextDrop(7,5,5,0,0)==7);
    YokeLog::Frame f{};
    f.seq=f.dropped=f.skip=65535; f.t=f.rev=0xFFFFFFFFu; f.flags=255;
    f.q=f.env=f.kc=1e9f;
    for(int i=0;i<2;++i) {
        f.gust[i]=f.ground[i]=f.pos[i]=f.cmd[i]=1e9f;
        f.vel[i]=-1e9f; f.pwm[i]=i ? -255 : 255; f.peak[i]=255;
    }
    for(int i=0;i<3;++i) f.buffet[i]=1e9f;
    char line[YOKE_LOG_M_MAX], row[512]; int64_t values[26];
    int n=YokeLog::formatModel(line,sizeof(line),f);
    CHECK(n>0 && n<YOKE_LOG_M_MAX);
    line[n-1]=0;
    CHECK(YokeCsv::integers(line+1,'M',values,26));
    CHECK(values[0]==65535 && values[1]==4294967295LL && values[2]==4294967295LL);
    CHECK(values[20]==255 && values[21]==-255);
    CHECK(values[24]==65535 && values[25]==65535);
    for(int i=4;i<20;++i) CHECK(values[i]==(i==16 || i==17 ? -9999999 : 99999));
    CHECK(YokeLog::formatModel(row,4,f)==-1);
    f=YokeLog::Frame{}; f.q=std::numeric_limits<float>::quiet_NaN();
    f.vel[1]=std::numeric_limits<float>::infinity();
    n=YokeLog::formatModel(line,sizeof(line),f); CHECK(n>0); line[n-1]=0;
    CHECK(YokeCsv::integers(line+1,'M',values,26));
    CHECK(values[4]==0 && values[17]==0 && (values[3]&YokeLog::NonFinite));
    f=YokeLog::Frame{}; f.seq=12; f.t=1234; f.rev=7; f.flags=YokeLog::Modeled;
    f.q=.12345f; f.gust[0]=-.34567f; f.gust[1]=.34567f;
    f.buffet[0]=.1f; f.buffet[1]=.2f; f.buffet[2]=.3f;
    f.ground[0]=-.4f; f.ground[1]=.5f; f.env=.6f; f.kc=.7f;
    f.pos[0]=-.8f; f.pos[1]=.9f; f.vel[0]=-123.456f; f.vel[1]=234.567f;
    f.cmd[0]=-.12f; f.cmd[1]=.34f; f.pwm[0]=-23; f.pwm[1]=34;
    f.peak[0]=45; f.peak[1]=56; f.dropped=8; f.skip=9;
    n=YokeLog::formatModel(line,sizeof(line),f);
    YokeCsv::LineSplitter splitter; int calls=0;
    auto onLine=[&](const char* l) { ++calls; CHECK(YokeCsv::modelRow(l,row,sizeof(row))); };
    splitter.feed(line,5,onLine); splitter.feed(line+5,17,onLine); splitter.feed(line+22,n-22,onLine);
    CHECK(calls==1);
    std::stringstream fields(row); std::string cell; std::vector<double> decoded;
    while(std::getline(fields,cell,',')) decoded.push_back(std::stod(cell));
    CHECK(decoded.size()==26);
    const float expected[]={f.q,f.gust[0],f.gust[1],f.buffet[0],f.buffet[1],f.buffet[2],
        f.ground[0],f.ground[1],f.env,f.kc,f.pos[0],f.pos[1],f.vel[0],f.vel[1],f.cmd[0],f.cmd[1]};
    for(int i=0;i<16;++i) CHECK(std::fabs(decoded[i+4]-expected[i])<.0001);
    CHECK(decoded[0]==12 && decoded[1]==1234 && decoded[2]==7 && decoded[3]==128);
    CHECK(decoded[20]==-23 && decoded[21]==34 && decoded[22]==45 && decoded[23]==56 && decoded[24]==8 && decoded[25]==9);
    line[n-1]=0; const std::string valid(line+1);
    CHECK(!YokeCsv::modelRow(valid.substr(0,valid.rfind(',')).c_str(),row,sizeof(row)));
    CHECK(!YokeCsv::modelRow((valid+",0").c_str(),row,sizeof(row)));
    CHECK(!YokeCsv::modelRow(("M,a"+valid.substr(valid.find(',',2))).c_str(),row,sizeof(row)));
    CHECK(!YokeCsv::modelRow("",row,sizeof(row)));
    CHECK(!YokeCsv::modelRow(("X"+valid.substr(1)).c_str(),row,sizeof(row)));
    CHECK(!YokeCsv::modelRow("M, 1",row,sizeof(row)));
    uint16_t raw[12]; for(int i=0;i<12;++i) raw[i]=uint16_t(i*909);
    raw[11]=10000;
    n=YokeLog::formatGains(line,sizeof(line),23,4094,raw,12); CHECK(n>0);
    line[n-1]=0; CHECK(YokeCsv::gainsRow(line+1,row,sizeof(row)));
    fields.clear(); fields.str(row); decoded.clear();
    while(std::getline(fields,cell,',')) decoded.push_back(std::stod(cell));
    CHECK(decoded.size()==14 && decoded[0]==23 && decoded[1]==4094);
    for(int i=0;i<12;++i) CHECK(std::fabs(decoded[i+2]-raw[i]/10000.0)<.000001);
    CHECK(YokeLog::formatGains(line,4,0,0,raw,12)==-1);
    CHECK(YokeLog::formatGains(line,sizeof(line),0,0,raw,11)==-1);
    calls=0; const std::string oversized(256,'a');
    splitter.feed(oversized.c_str(),oversized.size(),[&](const char*) { ++calls; });
    splitter.feed("\nnext\n",6,[&](const char* l) { ++calls; CHECK(std::strcmp(l,"next")==0); });
    CHECK(calls==1);
    YokeSC::Receiver rx, sequentialRx; YokeSC::Effects effects, sequentialEffects;
    YokeRaw::Sample sample; uint8_t packet[113];
    sample.value[YokeRaw::PressurePSF]=100;
    sample.timeMs=100; rawFrame(packet,sample,1); rawFeed(rx,packet,100); rawFeed(sequentialRx,packet,100);
    effects.update(rx,100,true); CHECK(effects.active() && effects.modeledRevision()==0);
    sequentialEffects.update(sequentialRx,100,true);
    effects.update(rx,102,true); CHECK(effects.modeledRevision()==rx.revision);
    sequentialEffects.update(sequentialRx,102,true);
    sample.timeMs=120; rawFrame(packet,sample,2);
    rawFeed(sequentialRx,packet,118);
    effects.update(rx,118,true); // Keep control ticks equal, before the batched arrival.
    sequentialEffects.update(sequentialRx,118,true);
    rawFeed(rx,packet,120); // Both new frames arrive before the next update.
    sample.timeMs=140; sample.value[YokeRaw::PressurePSF]=150;
    rawFrame(packet,sample,3); rawFeed(rx,packet,120); rawFeed(sequentialRx,packet,120);
    effects.update(rx,120,true); sequentialEffects.update(sequentialRx,120,true);
    CHECK(effects.skipped()==1 && effects.modeledRevision()==rx.revision);
    CHECK(sequentialEffects.skipped()==0);
    const float command=effects.command(rx,0,.9f,YokeSC::Centering,.5f);
    CHECK(command!=0);
    CHECK(near(command,sequentialEffects.command(sequentialRx,0,.9f,YokeSC::Centering,.5f)));
    rx.revision+=70000; effects.update(rx,122,true); CHECK(effects.skipped()==65535);
    effects.reset(); CHECK(effects.skipped()==65535 && effects.modeledRevision()==0);
    rawFrame(packet,sample,4,2); rawFeed(rx,packet,124); CHECK(rx.isPaused());
    sample.timeMs=160; rawFrame(packet,sample,5,1); rawFeed(rx,packet,126); CHECK(!rx.isPaused());
}
