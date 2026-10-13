#ifndef YOKE_PICO_CSV_H
#define YOKE_PICO_CSV_H
#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <cerrno>
#include <cstdlib>
#include <string>
#include <vector>
namespace YokeCsv {
struct CsvGate {
    enum class State : uint8_t { Pending, On, Off };
    State state=State::Pending; uint64_t pendingSince=0; std::vector<std::string> held;
    void start(bool pico) { state=pico ? State::Pending : State::On; pendingSince=0; held.clear(); }
    template<class W> void row(const std::string& r,uint64_t now,W write) {
        if(state==State::On) write(r);
        else if(state==State::Pending) { if(held.empty()) pendingSince=now; held.push_back(r); }
    }
    template<class W> void recordState(bool on,W write) {
        state=on ? State::On : State::Off;
        if(on) for(const auto& r:held) write(r);
        held.clear();
    }
    template<class W> void tick(uint64_t now,W write) {
        if(state==State::Pending && !held.empty() && (now-pendingSince>=2000 || held.size()>=2000))
            recordState(true,write);
    }
    template<class W> void finish(W write) { if(state==State::Pending) recordState(true,write); }
    void reopen() { start(true); }
    bool writesPico() const { return state!=State::Off; }
};
inline bool recordLine(const char* line,bool& on) {
    if(!line || line[0]!='L' || line[1]!=',' || (line[2]!='0' && line[2]!='1') || line[3]!=0) return false;
    on=line[2]=='1'; return true;
}
inline bool slowLine(const char* line,uint32_t v[9]) {
    if(!line || line[0]!='W' || line[1]!=',') return false;
    const char* p=line+2;
    for(size_t i=0;i<9;++i) {
        if(*p<'0' || *p>'9') return false;
        errno=0; char* end=nullptr;
        const unsigned long long value=std::strtoull(p,&end,10);
        if(errno==ERANGE || value>UINT32_MAX || (i<8 ? *end!=',' : *end!=0)) return false;
        v[i]=uint32_t(value); p=end+(i<8 ? 1 : 0);
    }
    return true;
}
static const char modelHeader[]="host_time_ms,seq,sample_time_ms,revision,flags,pressure,gust_roll,gust_pitch,buffet_stall,buffet_gear,buffet_spoiler,ground_roll,ground_pitch,ground_envelope,center_stiffness,pos_roll,pos_pitch,vel_roll,vel_pitch,cmd_roll,cmd_pitch,pwm_roll,pwm_pitch,peak_pwm_roll,peak_pwm_pitch,dropped,skipped";
static const char gainsHeader[]="host_time_ms,seq,enabled_bits,total,center_pitch,center_rest,gust_roll,gust_pitch,ground,stall,gear,spoiler,damper,friction,center_roll";
struct LineSplitter {
    char line[256]={0}; size_t n=0; bool overflow=false;
    template<class F> void feed(const char* data,size_t size,F onLine) {
        for(size_t i=0;i<size;++i) {
            const char c=data[i];
            if(c=='\n' || c=='\r') {
                line[n]=0; if(!overflow && n) onLine(line);
                n=0; overflow=false;
            } else if(n+1<sizeof(line)) line[n++]=c;
            else overflow=true;
        }
    }
};
inline bool integers(const char* line,char prefix,int64_t* values,size_t count) {
    if(!line || line[0]!=prefix || line[1]!=',') return false;
    const char* p=line+2;
    for(size_t i=0;i<count;++i) {
        const char* digits=p;
        if(*digits=='+' || *digits=='-') ++digits;
        if(*digits<'0' || *digits>'9') return false;
        errno=0; char* end=nullptr;
        const long long v=std::strtoll(p,&end,10);
        if(errno==ERANGE || (i+1<count ? *end!=',' : *end!=0)) return false;
        values[i]=v; p=end+(i+1<count ? 1 : 0);
    }
    return true;
}
inline bool row(const char* line,char* out,size_t size,bool model) {
    int64_t v[26]; const size_t count=model ? 26 : 14;
    if(!integers(line,model ? 'M' : 'G',v,count)) return false;
    size_t n=0;
    for(size_t i=0;i<count;++i) {
        const bool physical=model ? i>=4 && i<20 : i>=2;
        const bool velocity=model && (i==16 || i==17);
        const int added=physical ? std::snprintf(out+n,size-n,velocity ? "%s%.3f" : "%s%.4f",
            i ? "," : "",double(v[i])/(velocity ? 1000 : 10000)) :
            std::snprintf(out+n,size-n,"%s%lld",i ? "," : "",(long long)v[i]);
        if(added<0 || size_t(added)>=size-n) return false;
        n+=size_t(added);
    }
    return true;
}
inline bool modelRow(const char* line,char* out,size_t size) { return row(line,out,size,true); }
inline bool gainsRow(const char* line,char* out,size_t size) { return row(line,out,size,false); }
}
#endif
