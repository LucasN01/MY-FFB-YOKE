#ifndef YOKE_PICO_LOG_H
#define YOKE_PICO_LOG_H
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>
#define YOKE_LOG_M_MAX 192
namespace YokeLog {
inline int formatRecord(char* out,size_t size,bool enabled) {
    const int n=snprintf(out,size,"\nL,%u\n",unsigned(enabled));
    return n>=0 && size_t(n)<size ? n : -1;
}
enum Flag : uint8_t { Active=1, Armed=2, Paused=4, Menu=8, Dtr=16, Raw=32, NonFinite=64, Modeled=128 };
struct Frame {
    uint16_t seq; uint32_t t, rev; uint8_t flags;
    float q, gust[2], buffet[3], ground[2], env, kc, pos[2], vel[2], cmd[2];
    int16_t pwm[2], peak[2]; uint16_t dropped, skip;
};
inline uint16_t nextDrop(uint16_t drop, uint32_t r, uint32_t lastLoggedRev,
                         uint16_t skipNow, uint16_t lastSkip) {
    if(r==0 || lastLoggedRev==0) return drop;
    const uint32_t d=uint32_t(r-lastLoggedRev);
    if(d<=1) return drop;
    const uint32_t skipped=uint32_t(skipNow)-lastSkip;
    if(skipped>=d-1) return drop;
    const uint32_t missed=d-1-skipped;
    return missed>=uint32_t(65535-drop) ? 65535 : uint16_t(drop+missed);
}
inline long scaled(float value, bool velocity, uint8_t& flags) {
    if(!isfinite(value)) { flags|=NonFinite; return 0; }
    const float limit=velocity ? 9999.999f : 9.9999f;
    value=fmaxf(-limit,fminf(limit,value));
    return lroundf(value*(velocity ? 1000 : 10000));
}
inline int formatModel(char* out, size_t size, const Frame& f) {
    uint8_t flags=f.flags;
    const float values[]={f.q,f.gust[0],f.gust[1],f.buffet[0],f.buffet[1],f.buffet[2],
        f.ground[0],f.ground[1],f.env,f.kc,f.pos[0],f.pos[1],f.vel[0],f.vel[1],f.cmd[0],f.cmd[1]};
    long v[16];
    for(uint8_t i=0;i<16;++i) v[i]=scaled(values[i],i==12 || i==13,flags);
    const int n=snprintf(out,size,"\nM,%u,%lu,%lu,%u,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%d,%d,%d,%d,%u,%u\n",
        unsigned(f.seq),(unsigned long)f.t,(unsigned long)f.rev,unsigned(flags),
        v[0],v[1],v[2],v[3],v[4],v[5],v[6],v[7],v[8],v[9],v[10],v[11],v[12],v[13],v[14],v[15],
        int(f.pwm[0]),int(f.pwm[1]),int(f.peak[0]),int(f.peak[1]),unsigned(f.dropped),unsigned(f.skip));
    return n>=0 && size_t(n)<size ? n : -1;
}
inline int formatGains(char* out, size_t size, uint16_t seq, uint16_t bits, const uint16_t* raw, uint8_t count) {
    if(count!=12 || !raw) return -1;
    int n=snprintf(out,size,"\nG,%u,%u",unsigned(seq),unsigned(bits));
    if(n<0 || size_t(n)>=size) return -1;
    for(uint8_t i=0;i<count;++i) {
        const int added=snprintf(out+n,size-size_t(n),",%u",unsigned(raw[i]));
        if(added<0 || size_t(added)>=size-size_t(n)) return -1;
        n+=added;
    }
    const int added=snprintf(out+n,size-size_t(n),"\n");
    return added==1 && size_t(n+1)<size ? n+1 : -1;
}
}
#endif
