#ifndef YOKE_PICO_FORCE_MODELS_H
#define YOKE_PICO_FORCE_MODELS_H
#include "RawTelemetry.h"
#include "GroundModel.h"
#include "GustModel.h"
#include "BuffetModel.h"
namespace YokeSC {
// Executed on Pico, once per new accepted SC8 sample; not on every motor tick.
class PicoForceModels {
public:
    float pressure=0, gust[2]={}, buffet[3]={}, ground[2]={}, groundEnvelope=0;
    void reset() {
        ready=false; pressure=groundEnvelope=0;
        for(int i=0;i<2;++i) gust[i]=ground[i]=0;
        for(int i=0;i<3;++i) buffet[i]=0;
        gustModel.reset(); groundModel.reset();
    }
    void update(const YokeRaw::Sample& s) {
        using namespace YokeRaw;
        if(!valid(s) || s.value[Slew]!=0) { reset(); return; }
        if(ready && int32_t(s.timeMs-lastSample)<=0) return;
        const double dt=ready ? uint32_t(s.timeMs-lastSample)*.001 : .02;
        if(ready && dt>=.25) { gustModel.reset(); groundModel.reset(); }
        // First dynamic sample only seeds the model; .02 does not create a force.
        ready=true; lastSample=s.timeMs;
        const double qPa=s.value[PressurePSF]*47.88025898;
        const double velocity=s.referenceKnots*.514444444;
        const double ratio=qPa/(.5*1.225*velocity*velocity);
        pressure=float(ratio>4 ? 4 : ratio);
        const bool air=s.value[OnGround]==0;
        // Gust hinge load ~ q*dalpha = q*w/V ~ rho*V*w: scale with V_e/V_ref = sqrt(Q) (Pratt-Walker).
        const double gustSpeed=pressure>0 ? std::sqrt(double(pressure)) : 0.0; // pressure <=4 -> 0..2
        // Plunge time constant tau=2(W/S)/(g*rho0*a*V_e): sustained-gust dalpha decays as the aircraft
        // accelerates vertically. C182RG: W/S=853 Pa, a=4.82/rad (Helmbold, AR 7.45) -> 29.5 m.
        const double ve=std::sqrt(2.0*(qPa>0 ? qPa : 0)/1.225);
        const double tauHigh=ve<=gustPlungeLength/1.5 ? 1.5 :
                             (ve>=gustPlungeLength/.25 ? .25 : gustPlungeLength/ve);
        const auto wind=gustModel.update(s.value[WindX],s.value[WindY],s.value[WindZ],
            dt,gustSpeed,s.gustGain,s.gustLongitudinal,air && (s.options&Gusts),tauHigh);
        gust[0]=float(wind.roll); gust[1]=float(wind.pitch);
        const YokeBuffet::Input b={s.value[IAS],s.value[AoA],s.value[StallAlpha],
            s.value[FlapLeft],s.value[FlapRight],s.value[VS0],s.value[VS1],
            s.value[Gear],s.value[Retractable],s.value[SpoilerLeft],s.value[SpoilerRight]};
        const auto amps=YokeBuffet::calculate(b,air && (s.options&Buffets));
        buffet[0]=float(amps.stall); buffet[1]=float(amps.gear); buffet[2]=float(amps.spoiler);
        // Runway texture amplitude: ISO 8608 (G_d ~ n^-2) gives white profile velocity with
        // level ~ V, so a resonant response grows ~ sqrt(V). Bumps from P3D keep the linear ramp.
        const double ramp=YokeGround::envelope(s.value[OnGround],s.value[GroundSpeed],true);
        groundEnvelope=float(std::sqrt(ramp));
        const auto force=groundModel.update(s.value[AccelX],s.value[AccelY],dt,
            s.value[OnGround],s.value[GroundSpeed],true);
        ground[0]=float(force.roll); ground[1]=float(force.pitch);
    }
private:
    static constexpr double gustPlungeLength=29.5; // metros; 2(W/S)/(g*rho0*a), perfil C182RG
    bool ready=false;
    uint32_t lastSample=0;
    YokeGust::Model gustModel;
    YokeGround::Model groundModel;
};
}
#endif
