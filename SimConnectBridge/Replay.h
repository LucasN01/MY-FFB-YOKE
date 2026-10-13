#ifndef YOKE_REPLAY_H
#define YOKE_REPLAY_H
#include <string>
#include <vector>
#include <cstdlib>
#include <cerrno>
#include <cctype>
#include <cmath>
#include <limits>
#include <cstdint>
#include "../RawTelemetry.h"
namespace YokeReplay {
static const char header[]="host_time_ms,sample_time_ms,sample_index,pressure_psf,ias_knots,wind_x_knots,wind_y_knots,wind_z_knots,on_ground,slew,aoa_deg,stall_alpha_deg,flap_left,flap_right,vs0_knots,vs1_knots,gear_fraction,gear_retractable,spoiler_left,spoiler_right,ground_speed_knots,yoke_roll,yoke_pitch,ground_accel_x_ft_s2,ground_accel_y_ft_s2,paused,running,armed,sent,reference_knots,gust_gain,gust_longitudinal,gusts_enabled,buffets_enabled";
struct ReplayRow {
    uint64_t host=0; uint32_t sample=0; float v[22]={};
    bool paused=false,running=false,armed=false,sent=false,gusts=false,buffets=false;
    double ref=120,gain=1,longi=.25;
};
inline std::string clean(std::string s) { if(!s.empty() && s.back()=='\r') s.pop_back(); return s; }
inline bool parseHeader(const std::string& s) { return clean(s)==header; }
inline std::vector<std::string> split(const std::string& line) {
    const std::string s=clean(line); std::vector<std::string> fields;
    size_t start=0;
    for(;;) {
        const size_t end=s.find(',',start); fields.push_back(s.substr(start,end-start));
        if(end==std::string::npos) break;
        start=end+1;
    }
    return fields;
}
inline bool number(const std::string& s,double& value) {
    if(s.empty()) return false;
    std::string lower=s;
    for(char& c:lower) c=char(std::tolower(static_cast<unsigned char>(c)));
    const std::string nan=lower[0]=='-' ? lower.substr(1) : lower;
    bool nanToken=nan=="nan";
    if(nan.size()>5 && nan.compare(0,4,"nan(")==0 && nan.back()==')') {
        nanToken=true;
        for(size_t j=4;j+1<nan.size();++j) if(!std::isalnum(static_cast<unsigned char>(nan[j])) && nan[j]!='_') nanToken=false;
    }
    if(nanToken) { value=std::numeric_limits<double>::quiet_NaN(); return true; }
    if(lower=="inf" || lower=="-inf") { value=lower[0]=='-' ? -std::numeric_limits<double>::infinity() : std::numeric_limits<double>::infinity(); return true; }
    size_t i=(s[0]=='+' || s[0]=='-') ? 1 : 0; bool digits=false;
    while(i<s.size() && s[i]>='0' && s[i]<='9') { digits=true; ++i; }
    if(i<s.size() && s[i]=='.') { ++i; while(i<s.size() && s[i]>='0' && s[i]<='9') { digits=true; ++i; } }
    if(!digits) return false;
    if(i<s.size() && (s[i]=='e' || s[i]=='E')) {
        ++i; if(i<s.size() && (s[i]=='+' || s[i]=='-')) ++i;
        const size_t begin=i; while(i<s.size() && s[i]>='0' && s[i]<='9') ++i;
        if(i==begin) return false;
    }
    if(i!=s.size()) return false;
    errno=0; char* end=nullptr; value=std::strtod(s.c_str(),&end);
    return *end==0 && errno!=ERANGE && std::isfinite(value);
}
inline bool integer(const std::string& s,uint64_t max,uint64_t& value) {
    if(s.empty()) return false;
    for(char c:s) if(c<'0' || c>'9') return false;
    errno=0; char* end=nullptr; value=std::strtoull(s.c_str(),&end,10);
    return *end==0 && errno!=ERANGE && value<=max;
}
inline bool parseRow(const std::string& s,ReplayRow& row) {
    const auto fields=split(s); if(fields.size()!=34) return false;
    ReplayRow r; uint64_t value=0;
    if(!integer(fields[0],UINT64_MAX,r.host) || !integer(fields[1],UINT32_MAX,value)) return false;
    r.sample=uint32_t(value);
    if(!integer(fields[2],UINT32_MAX,value)) return false;
    for(size_t i=0;i<22;++i) { double v; if(!number(fields[i+3],v)) return false; r.v[i]=float(v); }
    bool* bits[]={&r.paused,&r.running,&r.armed,&r.sent,&r.gusts,&r.buffets};
    const size_t columns[]={25,26,27,28,32,33};
    for(size_t i=0;i<6;++i) { if(!integer(fields[columns[i]],1,value)) return false; *bits[i]=value!=0; }
    if(!number(fields[29],r.ref) || !number(fields[30],r.gain) || !number(fields[31],r.longi)) return false;
    row=r; return true;
}
inline YokeRaw::Sample payload(const ReplayRow& r) {
    YokeRaw::Sample s; s.timeMs=r.sample;
    for(int i=0;i<22;++i) s.value[i]=r.v[i];
    s.referenceKnots=float(r.ref); s.gustGain=float(r.gain); s.gustLongitudinal=float(r.longi);
    s.options=uint8_t((r.gusts ? YokeRaw::Gusts : 0) | (r.buffets ? YokeRaw::Buffets : 0));
    return s;
}
}
#endif
