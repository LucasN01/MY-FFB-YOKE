#ifndef YOKE_COMPARAR_ENSAYO_H
#define YOKE_COMPARAR_ENSAYO_H
#include "../mis_defines.h"
#include "../SimConnectBridge/Replay.h"
#include "../SimConnectBridge/PicoCsv.h"
#include "../SimTelemetry.h"
#include "../PicoForceModels.h"
#include "../PicoLog.h"
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <array>
#include <algorithm>
#include <iomanip>
namespace YokeEnsayo {
struct PicoRow { uint64_t host=0; std::array<double,26> v{}; };
struct Mark { uint64_t host=0; std::string phase; };
struct Input {
    std::vector<YokeReplay::ReplayRow> tel;
    std::vector<PicoRow> pico;
    std::vector<Mark> marks;
    size_t gains=0; bool picoHost=true,bridgeCounts=false;
    uint64_t lines=0,badLines=0; unsigned long us=0,lp=0,gap=0;
};
struct Result {
    bool aOk=false,noData=false,bOk=true;
    size_t matched=0,excluded=0,expected=0;
    std::array<size_t,10> violations{};
    std::string report,phases;
    int code() const { return !bOk || (!aOk && !noData) ? 1 : (noData ? 2 : 0); }
};
inline bool picoRow(const std::string& line,PicoRow& out,bool host=true) {
    const auto fields=YokeReplay::split(line); if(fields.size()!=(host ? 27 : 26)) return false;
    PicoRow r;
    if(host && !YokeReplay::integer(fields[0],UINT64_MAX,r.host)) return false;
    for(size_t i=0;i<26;++i) {
        if(!YokeReplay::number(fields[i+(host ? 1 : 0)],r.v[i]) || !std::isfinite(r.v[i])) return false;
        if(i<4 || i>=20) {
            const double max=i==1 || i==2 ? double(UINT32_MAX) : (i==3 ? 255 : (i>=20 && i<=23 ? 32767 : 65535));
            const double min=i>=20 && i<=23 ? -32768 : 0;
            if(r.v[i]<min || r.v[i]>max || std::floor(r.v[i])!=r.v[i]) return false;
        }
    }
    out=r; return true;
}
inline std::array<double,10> signals(const YokeSC::PicoForceModels& m) {
    return {{m.pressure,m.gust[0],m.gust[1],m.buffet[0],m.buffet[1],m.buffet[2],m.ground[0],m.ground[1],m.groundEnvelope,0}};
}
inline const char* phase(const YokeReplay::ReplayRow& r) {
    using namespace YokeRaw;
    if(r.paused) return "PAUSA";
    if(r.v[Slew]!=0) return "SLEW";
    if(r.v[OnGround]==1) return r.v[GroundSpeed]<=.5f ? "PARADO" : "RODAJE";
    if(r.v[AoA]>=r.v[StallAlpha]-3) return "PERDIDA";
    if(r.v[Gear]>.1f) return "TREN";
    return "VUELO";
}
inline double percent(size_t part,size_t total) { return total ? 100.0*double(part)/double(total) : 0; }
inline Result compare(const Input& in,const std::string& name) {
    Result result; YokeSC::Receiver rx; YokeSC::PicoForceModels models;
    std::map<uint32_t,std::array<double,10>> expected;
    std::map<uint32_t,const YokeReplay::ReplayRow*> firstSent;
    std::vector<const YokeReplay::ReplayRow*> sent;
    uint16_t sequence=0; uint64_t previousHost=0; bool havePrevious=false,armedData=false;
    for(const auto& r:in.tel) if(r.sent) {
        sent.push_back(&r); firstSent.emplace(r.sample,&r); armedData|=r.armed;
        if(havePrevious && r.host-previousHost>=300) models.reset();
        previousHost=r.host; havePrevious=true;
        auto s=YokeReplay::payload(r); bool armed=r.armed;
        if(!YokeRaw::valid(s)) { s=YokeRaw::Sample{}; armed=false; }
        uint8_t packet[YokeRaw::packetSize]={0xa5,0x5a,8,uint8_t(r.paused && !armed ? 2 : (armed ? 1 : 0))};
        YokeSC::put16(packet+4,++sequence); YokeRaw::encodePayload(packet,s);
        YokeSC::put16(packet+111,YokeSC::crc(packet,111));
        const uint32_t revision=rx.revision;
        for(uint8_t byte:packet) rx.feed(byte,uint32_t(r.host));
        if(!rx.armed || rx.raw.value[YokeRaw::Slew]!=0) models.reset();
        if(rx.revision!=revision && rx.armed && rx.rawAvailable) {
            models.update(rx.raw); expected[rx.raw.timeMs]=signals(models);
        }
    }
    result.expected=expected.size(); result.noData=!armedData;
    std::set<uint32_t> revisions,matchedTimes,excludedTimes,allRevisions;
    std::array<size_t,9> good{}; std::array<double,9> worst{}; std::array<uint32_t,9> worstTime{};
    uint16_t previousSkip=0; bool excluding=false; uint32_t exclusionStart=0;
    uint64_t sequenceLoss=0; uint16_t previousSeq=0; bool haveSeq=false;
    size_t signGood[2]={},signTotal[2]={};
    double sx=0,sy=0,sxx=0,syy=0,sxy=0; size_t correlationN=0;
    for(const auto& p:in.pico) {
        const auto& v=p.v; const unsigned flags=unsigned(v[3]);
        const uint32_t revision=uint32_t(v[2]),time=uint32_t(v[1]);
        allRevisions.insert(revision);
        const uint16_t seq=uint16_t(v[0]);
        if(haveSeq) { const uint16_t d=uint16_t(seq-previousSeq); if(d>1) sequenceLoss+=d-1; }
        previousSeq=seq; haveSeq=true;
        if(!(flags&YokeLog::Active) && (v[18]!=0 || v[19]!=0)) ++result.violations[1];
        if((flags&YokeLog::Menu) && (v[18]!=0 || v[19]!=0 || v[20]!=0 || v[21]!=0)) ++result.violations[2];
        if(std::fabs(v[20])>PWM_MAX || std::fabs(v[21])>PWM_MAX || std::fabs(v[22])>PWM_MAX ||
           std::fabs(v[23])>PWM_MAX || std::fabs(v[18])>SIMCONNECT_EXTRA_LIMIT+.00005 ||
           std::fabs(v[19])>SIMCONNECT_EXTRA_LIMIT+.00005) ++result.violations[3];
        if(flags&YokeLog::NonFinite) ++result.violations[8];
        const auto tel=firstSent.find(time);
        if((flags&YokeLog::Modeled) && tel!=firstSent.end()) {
            if(tel->second->v[YokeRaw::OnGround]==0 && (v[10]!=0 || v[11]!=0 || v[12]!=0)) ++result.violations[4];
            if(tel->second->v[YokeRaw::GroundSpeed]<=.5 && v[12]!=0) ++result.violations[5];
        }
        const uint16_t skip=uint16_t(v[25]);
        if(skip>previousSkip) { excluding=true; exclusionStart=time; }
        previousSkip=skip;
        const auto e=expected.find(time);
        if((flags&YokeLog::Modeled) && revisions.insert(revision).second && e!=expected.end()) {
            const auto& target=e->second;
            const bool zero=v[5]==0 && v[6]==0 && v[10]==0 && v[11]==0 &&
                target[1]==0 && target[2]==0 && target[6]==0 && target[7]==0;
            if(excluding && (zero || uint32_t(time-exclusionStart)>=2000)) excluding=false;
            if(excluding) { ++result.excluded; excludedTimes.insert(time); }
            else {
                ++result.matched; matchedTimes.insert(time);
                for(size_t i=0;i<9;++i) {
                    const double difference=std::fabs(v[i+4]-target[i]);
                    const double tolerance=i==1 || i==2 || i==6 || i==7 ? .002 : .0002;
                    if(difference<=tolerance) ++good[i];
                    if(difference>worst[i]) { worst[i]=difference; worstTime[i]=time; }
                }
            }
        }
        for(size_t axis=0;axis<2;++axis) {
            if((flags&YokeLog::Active) && std::fabs(v[14+axis])>.2 && std::fabs(v[16+axis])<.05 &&
               v[7]==0 && v[8]==0 && v[9]==0 && v[10]==0 && v[11]==0) {
                ++signTotal[axis]; if(v[18+axis]*v[14+axis]<0) ++signGood[axis];
            }
        }
        if(tel!=firstSent.end()) {
            const double x=std::pow(tel->second->v[YokeRaw::IAS]/tel->second->ref,2),y=v[4];
            if(std::isfinite(x) && std::isfinite(y)) { ++correlationN; sx+=x; sy+=y; sxx+=x*x; syy+=y*y; sxy+=x*y; }
        }
    }
    size_t pauseChecks=0; bool wasArmed=false,wasPaused=false;
    if(in.picoHost) for(const auto* r:sent) {
        if((r->paused && !wasPaused) || (!r->armed && wasArmed)) {
            const auto p=std::lower_bound(in.pico.begin(),in.pico.end(),r->host+150,
                [](const PicoRow& row,uint64_t host) { return row.host<host; });
            if(p!=in.pico.end()) { ++pauseChecks; if(p->v[18]!=0 || p->v[19]!=0) ++result.violations[6]; }
        }
        wasArmed=r->armed; wasPaused=r->paused;
    }
    const double dropped=in.pico.empty() ? 0 : in.pico.back().v[24];
    const double picoLoss=dropped+allRevisions.size()>0 ? dropped/(dropped+allRevisions.size()) : 0;
    const double bridgeLoss=haveSeq && previousSeq ? double(sequenceLoss)/previousSeq : (sequenceLoss ? 1 : 0);
    if(picoLoss>.02 || bridgeLoss>.02) ++result.violations[7];
    if(in.bridgeCounts && (in.lines ? double(in.badLines)/in.lines>.001 : in.badLines!=0)) ++result.violations[9];
    const size_t eligibleExpected=expected.size()-excludedTimes.size();
    result.aOk=result.matched>0 && percent(matchedTimes.size(),eligibleExpected)>=95;
    for(size_t n:good) if(percent(n,result.matched)<99) result.aOk=false;
    for(size_t i=1;i<=9;++i) if(result.violations[i]) result.bOk=false;
    const char* names[]={"q","gustR","gustP","stall","gear","spoiler","groundR","groundP","env"};
    const size_t worstIndex=size_t(std::max_element(worst.begin(),worst.end())-worst.begin());
    std::ostringstream report; report<<std::fixed<<std::setprecision(3);
    report<<"ENSAYO "<<name<<" | filas tel="<<in.tel.size()<<" sent="<<sent.size()<<" pico="<<in.pico.size()<<" gan="<<in.gains<<"\n";
    report<<"A fidelidad: "<<(result.noData ? "SIN DATOS" : (result.aOk ? "OK" : "FALLO"))
        <<" emparejadas="<<percent(matchedTimes.size(),eligibleExpected)<<"% q="<<percent(good[0],result.matched)
        <<"% gust="<<std::min(percent(good[1],result.matched),percent(good[2],result.matched))
        <<"% buffet="<<std::min(percent(good[3],result.matched),std::min(percent(good[4],result.matched),percent(good[5],result.matched)))
        <<"% ground="<<std::min(percent(good[6],result.matched),percent(good[7],result.matched))<<"% env="<<percent(good[8],result.matched)
        <<"% peor="<<names[worstIndex]<<"@"<<worstTime[worstIndex]<<":"<<std::setprecision(6)<<worst[worstIndex]
        <<" excluidas="<<result.excluded<<" esperadas="<<result.expected<<"\n"<<std::setprecision(3);
    for(size_t i=1;i<=9;++i) {
        const bool missing=i==6 ? !in.picoHost || pauseChecks==0 : (i==9 && !in.bridgeCounts);
        report<<"B"<<i<<": "<<(result.violations[i] ? "FALLO" : (missing ? "SIN DATOS" : "OK"))<<" violaciones="<<result.violations[i];
        if(i==6) report<<" comprobaciones="<<pauseChecks;
        if(i==7) report<<" pico="<<picoLoss*100<<"% puente="<<bridgeLoss*100<<"% dropped="<<dropped;
        if(i==9) report<<" lineas="<<in.lines<<" malas="<<in.badLines;
        report<<"\n";
    }
    const double denom=std::sqrt(std::max(0.0,(correlationN*sxx-sx*sx)*(correlationN*syy-sy*sy)));
    report<<"C us/lp/gap max="<<in.us<<"/"<<in.lp<<"/"<<in.gap<<" | lp>=2000="<<(in.lp>=2000)
        <<" | centrado signo R="<<percent(signGood[0],signTotal[0])<<"% P="<<percent(signGood[1],signTotal[1])
        <<"% | corr q-ias="<<(denom>0 ? (correlationN*sxy-sx*sy)/denom : 0)
        <<" | skip="<<(in.pico.empty() ? 0 : in.pico.back().v[25])<<" excluidas="<<result.excluded<<"\n";
    struct Segment { size_t first,last; std::string title; };
    std::vector<Segment> segments;
    for(size_t i=0;i<sent.size();++i) {
        const std::string title=phase(*sent[i]);
        if(segments.empty() || segments.back().title!=title) segments.push_back({i,i,title});
        else segments.back().last=i;
    }
    for(size_t i=1;i<segments.size();) {
        const uint64_t end=segments[i].last+1<sent.size() ? sent[segments[i].last+1]->host : sent.back()->host;
        if(end-sent[segments[i].first]->host<2000) { segments[i-1].last=segments[i].last; segments.erase(segments.begin()+i); }
        else ++i;
    }
    const char* metrics[]={"ias","ground_speed","on_ground","pressure","gust_roll","gust_pitch","buffet_stall","buffet_gear","buffet_spoiler","ground_roll","ground_pitch","center_stiffness","cmd_roll","cmd_pitch","pwm_roll","pwm_pitch","peak_roll","peak_pitch"};
    std::ostringstream phases; phases<<"order,name,routine_phase,duration_s,active_percent";
    for(const char* metric:metrics) phases<<","<<metric<<"_mean,"<<metric<<"_max";
    phases<<"\n"<<std::fixed<<std::setprecision(6);
    for(size_t n=0;n<segments.size();++n) {
        const auto& segment=segments[n]; const uint64_t start=sent[segment.first]->host;
        const uint64_t end=segment.last+1<sent.size() ? sent[segment.last+1]->host : sent.back()->host;
        double sums[18]={},maxima[18]={}; size_t counts[18]={},active=0,rows=0;
        auto add=[&](size_t i,double value) { if(std::isfinite(value)) { sums[i]+=value; maxima[i]=std::max(maxima[i],value); ++counts[i]; } };
        for(size_t i=segment.first;i<=segment.last;++i) {
            add(0,sent[i]->v[YokeRaw::IAS]); add(1,sent[i]->v[YokeRaw::GroundSpeed]); add(2,sent[i]->v[YokeRaw::OnGround]);
        }
        for(const auto& p:in.pico) if(p.host>=start && (p.host<end || (n+1==segments.size() && p.host==end))) {
            ++rows; if(unsigned(p.v[3])&YokeLog::Active) ++active;
            const size_t columns[]={4,5,6,7,8,9,10,11,13,18,19,20,21,22,23};
            for(size_t i=0;i<15;++i) add(i+3,i==0 || i==8 ? p.v[columns[i]] : std::fabs(p.v[columns[i]]));
        }
        std::string mark;
        for(const auto& m:in.marks) if(m.host<=start) mark=m.phase;
        const double duration=double(end-start)/1000;
        phases<<n+1<<","<<segment.title<<","<<mark<<","<<duration<<","<<percent(active,rows);
        for(size_t i=0;i<18;++i) phases<<","<<(counts[i] ? sums[i]/counts[i] : 0)<<","<<maxima[i];
        phases<<"\n";
        report<<"F"<<n+1<<" "<<segment.title<<": "<<duration<<"s ias="<<maxima[0]<<" gs="<<maxima[1]<<" q="<<maxima[3]
            <<" gust="<<std::max(maxima[4],maxima[5])<<" buf="<<std::max(maxima[6],std::max(maxima[7],maxima[8]))
            <<" gnd="<<std::max(maxima[9],maxima[10])<<" cmdR/P="<<maxima[12]<<"/"<<maxima[13]
            <<" pwmR/P="<<maxima[14]<<"/"<<maxima[15]<<" act="<<percent(active,rows)<<"% marca="<<mark<<"\n";
    }
    report<<"RESULTADO: "<<(result.code()==1 ? "FALLO" : (result.noData ? "SIN DATOS" : "OK"))<<"\n";
    result.report=report.str(); result.phases=phases.str(); return result;
}
inline bool counter(const std::string& line,const char* key,uint64_t& value) {
    const size_t at=line.find(key); if(at==std::string::npos) return false;
    const size_t start=at+std::string(key).size(),end=line.find_first_not_of("0123456789",start);
    return YokeReplay::integer(line.substr(start,end-start),UINT64_MAX,value);
}
inline bool load(const std::string& folder,Input& in,std::string& error) {
    auto path=[&](const char* file) { return folder+"/"+file; };
    std::ifstream tel(path("telemetry.csv")); std::string line;
    if(!std::getline(tel,line) || !YokeReplay::parseHeader(line)) { error="telemetry.csv: cabecera invalida o archivo ausente"; return false; }
    while(std::getline(tel,line)) {
        YokeReplay::ReplayRow r;
        if(!YokeReplay::parseRow(line,r) || (!in.tel.empty() && r.host<in.tel.back().host)) { error="telemetry.csv: fila invalida"; return false; }
        in.tel.push_back(r);
    }
    std::ifstream pico(path("pico.csv"));
    if(pico) {
        if(!std::getline(pico,line) || YokeReplay::clean(line)!=YokeCsv::modelHeader) { error="pico.csv: cabecera invalida"; return false; }
        while(std::getline(pico,line)) {
            PicoRow r;
            if(!picoRow(line,r) || (!in.pico.empty() && r.host<in.pico.back().host)) { error="pico.csv: fila invalida"; return false; }
            in.pico.push_back(r);
        }
    } else in.picoHost=false;
    std::ifstream log(path("pico.log"));
    while(std::getline(log,line)) {
        line=YokeReplay::clean(line);
        if(!in.picoHost && line.compare(0,2,"M,")==0) {
            char row[512]; PicoRow r;
            if(!YokeCsv::modelRow(line.c_str(),row,sizeof(row)) || !picoRow(row,r,false)) { error="pico.log: M invalida"; return false; }
            in.pico.push_back(r);
        }
        if(line.compare(0,4,"SC8 ")==0) {
            uint64_t n;
            if(counter(line,"us=",n)) in.us=std::max(in.us,static_cast<unsigned long>(n));
            if(counter(line,"lp=",n)) in.lp=std::max(in.lp,static_cast<unsigned long>(n));
            if(counter(line,"gap=",n)) in.gap=std::max(in.gap,static_cast<unsigned long>(n));
        }
    }
    if(in.pico.empty()) { error="sin filas pico.csv ni M en pico.log"; return false; }
    std::ifstream gains(path("ganancias.csv"));
    if(gains) {
        if(!std::getline(gains,line) || YokeReplay::clean(line)!=YokeCsv::gainsHeader) { error="ganancias.csv: cabecera invalida"; return false; }
        while(std::getline(gains,line)) {
            const auto fields=YokeReplay::split(line); uint64_t host; double value;
            if(fields.size()!=15 || !YokeReplay::integer(fields[0],UINT64_MAX,host)) { error="ganancias.csv: fila invalida"; return false; }
            for(size_t i=1;i<fields.size();++i) if(!YokeReplay::number(fields[i],value) || !std::isfinite(value)) { error="ganancias.csv: valor invalido"; return false; }
            ++in.gains;
        }
    }
    std::ifstream bridge(path("bridge.log"));
    while(std::getline(bridge,line)) if(line.compare(0,4,"Fin:")==0) in.bridgeCounts=counter(line,"lineas_pico=",in.lines) && counter(line,"lineas_pico_malas=",in.badLines);
    std::ifstream marks(path("marcas.csv"));
    if(std::getline(marks,line)) while(std::getline(marks,line)) {
        const auto fields=YokeReplay::split(line); Mark mark;
        if(fields.size()>=2 && YokeReplay::integer(fields[0],UINT64_MAX,mark.host)) { mark.phase=fields[1]; in.marks.push_back(mark); }
    }
    return true;
}
inline int run(const std::string& folder) {
    Input in; std::string error; Result result;
    if(!load(folder,in,error)) { result.report="A fidelidad: FALLO "+error+"\nRESULTADO: FALLO\n"; result.bOk=false; }
    else result=compare(in,folder);
    std::ofstream report(folder+"/informe.txt"),phases(folder+"/resumen_fases.csv");
    if(!report || !phases) { std::fprintf(stderr,"No se pudieron escribir los informes.\n"); return 1; }
    report<<result.report; phases<<result.phases;
    std::printf("%s",result.report.c_str()); return result.code();
}
}
#endif
