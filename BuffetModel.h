#ifndef YOKE_BUFFET_MODEL_H
#define YOKE_BUFFET_MODEL_H
#include <cmath>

// Envelopes only. Arduino synthesizes the carriers at its control-loop rate.
// Sources and deviations from TelemFFB: tesis/explicacion_efectos_proyecto_ffb_yoke.tex (seccion de actualizacion).
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
// Separated-flow/wake load ~ q*S*Cp' ~ IAS^2 (IAS ~ EAS at low altitude); full at `full` kt.
inline double quadratic(double v, double full) {
    const double x=v/full;
    return v<=0 ? 0 : (x>=1 ? 1 : x*x);
}
inline Output calculate(const Input& t, bool airborne) {
    Output result;
    if (!airborne || !range(t.ias,0,1000)) return result;
    // Invalid optional data disables only its effect; never invent stall alpha.
    if (range(t.aoa,-90,90) && range(t.stallAlpha,1,45) &&
        range(t.flapLeft,0,1) && range(t.flapRight,0,1) &&
        range(t.vs0,10,400) && range(t.vs1,10,400)) {
        const double flap=(t.flapLeft+t.flapRight)*0.5;
        const double reference=t.vs1+(t.vs0-t.vs1)*flap;
        // 14 CFR 23.207 (pre-amendment 23-64): stall warning (may be inherent buffet) starts
        // 5..max(10 kt,15% Vs) above Vs; buffet onset at the upper margin is a design hypothesis.
        // With C_L linear in alpha: onset/stall = (Vs/(Vs+margin))^2.
        const double margin=reference*0.15>10 ? reference*0.15 : 10;
        const double onset=t.stallAlpha*(reference/(reference+margin))*(reference/(reference+margin));
        const double airflow=(t.ias/reference)*(t.ias/reference);
        const double amplitude=ramp(t.aoa,onset,t.stallAlpha)*
                               (airflow>1.5 ? 1.5 : airflow);
        result.stall=amplitude>1 ? 1 : amplitude;
    }
    // Full-intensity IAS 150/140 kt: yoke calibration, not aircraft Vle; physics fixes the quadratic shape.
    if (t.retractable==1 && range(t.gear,0,1))
        result.gear=t.gear*quadratic(t.ias,150);
    if (range(t.spoilerLeft,0,1) && range(t.spoilerRight,0,1))
        result.spoiler=(t.spoilerLeft+t.spoilerRight)*0.5*quadratic(t.ias,140);
    return result;
}
}
#endif
