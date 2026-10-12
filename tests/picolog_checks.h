#include "../PicoLog.h"
#include "../SimConnectBridge/PicoCsv.h"
static void picoLogChecks() {
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
    YokeSC::Receiver rx; YokeSC::Effects effects; YokeRaw::Sample sample; uint8_t packet[113];
    sample.timeMs=100; rawFrame(packet,sample,1); rawFeed(rx,packet,100);
    effects.update(rx,100,true); CHECK(effects.active() && effects.modeledRevision()==0);
    effects.update(rx,102,true); CHECK(effects.modeledRevision()==rx.revision);
    sample.timeMs=120; rawFrame(packet,sample,2); rawFeed(rx,packet,120);
    sample.timeMs=140; rawFrame(packet,sample,3); rawFeed(rx,packet,120); effects.update(rx,120,true);
    CHECK(effects.skipped()==1 && effects.modeledRevision()==rx.revision);
    // Existing rawChecks verifies this same smooth-runway zero-speed command.
    CHECK(effects.command(rx,0,.9f,YokeSC::Ground)==0);
    rx.revision+=70000; effects.update(rx,122,true); CHECK(effects.skipped()==65535);
    effects.reset(); CHECK(effects.skipped()==65535 && effects.modeledRevision()==0);
    rawFrame(packet,sample,4,2); rawFeed(rx,packet,124); CHECK(rx.isPaused());
    sample.timeMs=160; rawFrame(packet,sample,5,1); rawFeed(rx,packet,126); CHECK(!rx.isPaused());
}
