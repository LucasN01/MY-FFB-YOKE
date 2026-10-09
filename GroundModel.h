#ifndef YOKE_GROUND_MODEL_H
#define YOKE_GROUND_MODEL_H
#include <cmath>
namespace YokeGround {
// Ground velocity in knots, not IAS: wind at rest must not cause taxi vibration.
inline double envelope(double onGround, double speedKnots, bool active) {
    if (!active || onGround != 1 || !std::isfinite(speedKnots) || speedKnots <= 0.5)
        return 0;
    const double level=(speedKnots-0.5)/39.5;
    return level>1 ? 1 : level;
}
struct Force { double roll=0, pitch=0; };
// P3D/FSX: body accelerations in ft/s^2. No synthetic carrier or random bumps.
// Pico: HP removes static load/slow turns; LP softens steps before force mixing.
class Model {
public:
    void reset() { ready=false; output=Force{}; for(int i=0;i<2;++i) previous[i]=hp[i]=lp[i]=0; }
    Force update(double lateral, double vertical, double dt, double onGround,
                 double speedKnots, bool active) {
        const double level=envelope(onGround,speedKnots,active);
        if(!level || !std::isfinite(lateral) || !std::isfinite(vertical) ||
           std::fabs(lateral)>640 || std::fabs(vertical)>640 ||
           !std::isfinite(dt) || dt<0 || dt>=.25) { reset(); return {}; }
        if(dt==0) return output; // no elapsed time: preserve filters, never invent a step
        const double input[2]={lateral,vertical};
        if(!ready) {
            for(int i=0;i<2;++i) previous[i]=input[i];
            ready=true; return {}; // seed without a startup/pause impulse
        }
        const double tau=1.0/(2.0*3.141592653589793*3.0);
        const double a=tau/(tau+dt), b=dt/(.02+dt);
        for(int i=0;i<2;++i) {
            hp[i]=a*(hp[i]+input[i]-previous[i]); previous[i]=input[i];
            lp[i]+=b*(hp[i]-lp[i]);
        }
        // Haptic mapping: vertical bumps also shake roll (existing roll-dominant yoke).
        // 0.25 g filtered acceleration reaches full signal, before speed scaling.
        output={limit((.3*lp[0]+.7*lp[1])/8.043512)*level,
                limit(lp[1]/8.043512)*level};
        return output;
    }
private:
    static double limit(double x) { return x < -1 ? -1 : (x > 1 ? 1 : x); }
    bool ready=false;
    Force output;
    double previous[2]={}, hp[2]={}, lp[2]={};
};
}
#endif
