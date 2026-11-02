#ifndef YOKE_NARROWBAND_NOISE_H
#define YOKE_NARROWBAND_NOISE_H
#include <stdint.h>
#include <math.h>
// Random narrow-band vibration: white noise through a 2nd-order resonator
// (Newland, Random Vibrations). Stationary RMS fixed by the AR(2) variance.
namespace YokeNoise {
class Narrowband {
public:
    // Coefficients are computed here, once, outside the control loop (static
    // construction before setup()). next() never calls expf/cosf/sqrtf.
    Narrowband(float hz, float stepSeconds, float zeta, float rms, uint32_t seed)
        :initialSeed(seed?seed:1u) {
        if(hz>0 && stepSeconds>0 && zeta>0 && zeta<1 && rms>0) {
            const float w=6.2831853f*hz;
            const float r=expf(-zeta*w*stepSeconds), theta=w*sqrtf(1-zeta*zeta)*stepSeconds;
            a1=2*r*cosf(theta); a2=-r*r;
            const float unit=(1-a2)/((1+a2)*((1-a2)*(1-a2)-a1*a1)); // var for unit-variance input
            gain=rms/sqrtf(unit/3.0f); // uniform [-1,1] has variance 1/3
        } // invalid parameters: a1=a2=gain=0 -> next() always returns 0
        reset();
    }
    void reset() { state=initialSeed; y1=y2=0; } // keeps coefficients
    // One step of the fixed period given to the constructor. Unclipped output.
    float next() {
        state^=state<<13; state^=state>>17; state^=state<<5; // xorshift32
        const float x=state*(2.0f/4294967296.0f)-1.0f;
        const float y=a1*y1+a2*y2+gain*x;
        y2=y1; y1=y; return y;
    }
private:
    uint32_t initialSeed, state=1;
    float a1=0, a2=0, gain=0, y1=0, y2=0;
};
}
#endif
