#ifndef YOKE_MOTION_H
#define YOKE_MOTION_H
#include <stdint.h>
#include <math.h>
#include "mis_defines.h"

// All sources use normalized position and center-to-end travels/second.
// Call with the timestamp of a NEW sample, not the control-loop timestamp
// when holding a SimConnect sample between packets.
class YokeMotion {
public:
    float position=0, velocity=0;
    void reset() { position=velocity=0; stationaryUs=0; initialized=false; }
    void update(float value, uint32_t sampleUs) {
        if(!isfinite(value)) { reset(); return; }
        value=value < -1 ? -1 : (value > 1 ? 1 : value);
        const uint32_t elapsed=uint32_t(sampleUs-last);
        if(!initialized || elapsed>250000) {
            position=value; velocity=0; stationaryUs=0; last=sampleUs; initialized=true; return;
        }
        if(!elapsed) return;
        const float dt=elapsed*.000001f;
        float raw=(value-position)/dt;
        raw=raw < -YOKE_MOTION_MAX_SPEED ? -YOKE_MOTION_MAX_SPEED :
            (raw > YOKE_MOTION_MAX_SPEED ? YOKE_MOTION_MAX_SPEED : raw);
        // Allow encoder quantization between edges; clear residual motion after
        // 40 ms stationary. A reversal must never reinforce the old direction.
        stationaryUs=fabsf(raw)<=YOKE_MOTION_DEAD_SPEED ? stationaryUs+elapsed : 0;
        if(raw*velocity<0) velocity=0;
        velocity+=dt/(YOKE_MOTION_FILTER_TAU_S+dt)*(raw-velocity);
        if(stationaryUs>=40000) { velocity=0; stationaryUs=40000; }
        position=value; last=sampleUs;
    }
private:
    uint32_t last=0;
    uint32_t stationaryUs=0;
    bool initialized=false;
};
#endif
