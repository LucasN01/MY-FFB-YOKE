#ifndef YOKE_BUFFET_MODEL_H
#define YOKE_BUFFET_MODEL_H
#include <cmath>

// Envelopes only. Arduino synthesizes the carriers at its control-loop rate.
// Sources and deviations from TelemFFB: EFECTOS_SIMCONNECT.md.
namespace YokeBuffet {
struct Input {
    double ias, aoa, stallAlpha, flapLeft, flapRight, vs0, vs1;
    double gear, retractable, spoilerLeft, spoilerRight;
};
struct Output { double stall=0, gear=0, spoiler=0; };
inline bool range(double v, double low, double high) {
    return std::isfinite(v) && v>=low && v<=high;
}
inline double ramp(double v, double low, double high) {
    const double x=(v-low)/(high-low);
    return x<0 ? 0 : (x>1 ? 1 : x);
}
inline Output calculate(const Input& t, bool airborne) {
    Output result;
    if (!airborne || !range(t.ias,0,1000)) return result;
    // Invalid optional data disables only its effect; never invent stall alpha.
    if (range(t.aoa,-90,90) && range(t.stallAlpha,1,45) &&
        range(t.flapLeft,0,1) && range(t.flapRight,0,1) &&
        range(t.vs0,10,400) && range(t.vs1,10,400)) {
        const double flap=(t.flapLeft+t.flapRight)*0.5;
        const double onset=t.stallAlpha*(0.5+0.2*flap);
        const double reference=t.vs1+(t.vs0-t.vs1)*flap;
        const double airflow=(t.ias/reference)*(t.ias/reference);
        const double amplitude=ramp(t.aoa,onset,t.stallAlpha)*
                               (airflow>1.5 ? 1.5 : airflow);
        result.stall=amplitude>1 ? 1 : amplitude;
    }
    // IAS thresholds in knots, an explicit yoke calibration (not aircraft Vle).
    if (t.retractable==1 && range(t.gear,0,1))
        result.gear=t.gear*ramp(t.ias,100,150);
    if (range(t.spoilerLeft,0,1) && range(t.spoilerRight,0,1))
        result.spoiler=(t.spoilerLeft+t.spoilerRight)*0.5*ramp(t.ias,80,140);
    return result;
}
}
#endif
