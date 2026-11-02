#ifndef YOKE_GUST_MODEL_H
#define YOKE_GUST_MODEL_H
#include <cmath>

// Telemetry-driven haptic approximation; see MODELO_TURBULENCIA.md.
// TelemFFB reference: TurbulenceModulator.py (filtering, mixing, vector limit).
// q is the speed factor V_e/V_ref (0..2), not normalized pressure.
// Keep the existing yoke's X/Y polarity, rather than importing Rhino signs.
namespace YokeGust {
struct Force { double roll=0, pitch=0; };
class Model {
public:
    void reset() { ready=false; output=Force{}; }
    Force update(double x, double y, double z, double dt, double q,
                 double gain, double longitudinal=0.25, bool active=true, double tauHigh=1.0) {
        if (!active || !std::isfinite(x) || !std::isfinite(y) ||
            !std::isfinite(z) || !std::isfinite(dt) || dt<0 || dt>=0.25 ||
            !std::isfinite(q) || q<0 || q>2 || !std::isfinite(gain) ||
            gain<0 || gain>10 || !std::isfinite(longitudinal) ||
            std::fabs(longitudinal)>1 || std::fabs(x)>1000 ||
            std::fabs(y)>1000 || std::fabs(z)>1000 ||
            !std::isfinite(tauHigh) || tauHigh<=0) {
            reset(); return output;
        }
        const double wind[3]={x,y,z}; // knots, AIRCRAFT WIND body axes
        if (!ready) {
            for (int i=0;i<3;++i) mean[i]=wind[i];
            ready=true; return output;
        }
        if (dt==0) return output; // no invented elapsed time
        const double a=std::exp(-dt/tauHigh), b=-std::expm1(-dt/0.08);
        double h[3];
        for (int i=0;i<3;++i) {
            mean[i]+=(1-a)*(wind[i]-mean[i]);
            h[i]=wind[i]-mean[i];
        }
        Force target;
        target.roll=0.70*h[0]/5.0;
        target.pitch=(h[1]+longitudinal*h[2])/5.0;
        target.roll*=q*gain; target.pitch*=q*gain;
        const double magnitude=std::hypot(target.roll,target.pitch);
        if (magnitude>1) { target.roll/=magnitude; target.pitch/=magnitude; }
        output.roll+=b*(target.roll-output.roll);
        output.pitch+=b*(target.pitch-output.pitch);
        return output;
    }
private:
    bool ready=false;
    double mean[3]={0,0,0};
    Force output;
};
}
#endif
