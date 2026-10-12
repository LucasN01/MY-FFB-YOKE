#ifndef YOKE_PICO_CSV_H
#define YOKE_PICO_CSV_H
#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <cerrno>
#include <cstdlib>
namespace YokeCsv {
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
