#ifndef YOKE_SIM_EFFECTS_H
#define YOKE_SIM_EFFECTS_H
#include "mis_defines.h"
#include "SimTelemetry.h"
#include "YokeMotion.h"
#include "GainSettings.h"
#include "PicoForceModels.h"
#include "NarrowbandNoise.h"
#include "FrictionModel.h"
#define YOKE_FRICTION_POSITION_MODEL (YOKE_FRICTION_MODEL==1 && YOKE_USE_SIMULATION!=1)

namespace YokeSC {
enum EffectMask { Turbulence=1, Stall=2, Gear=4, Spoiler=8, Ground=16, Centering=32, Damper=64, Friction=128 };
static const uint8_t configuredEffects =
    (SIMCONNECT_TURBULENCE_ENABLED ? Turbulence : 0) |
    (SIMCONNECT_STALL_BUFFET_ENABLED ? Stall : 0) |
    (SIMCONNECT_GEAR_BUFFET_ENABLED ? Gear : 0) |
    (SIMCONNECT_SPOILER_BUFFET_ENABLED ? Spoiler : 0) |
    (SIMCONNECT_GROUND_VIBRATION_ENABLED ? Ground : 0) |
    (SIMCONNECT_DYNAMIC_CENTERING_ENABLED ? Centering : 0) |
    (YOKE_DAMPER_ENABLED ? Damper : 0) |
    (YOKE_FRICTION_ENABLED ? Friction : 0);

// Cheap triangle carriers, zero mean, phase in thousandths of one cycle.
inline float triangle(uint16_t phase) {
    return phase<500 ? phase*.004f-1.0f : 3.0f-phase*.004f;
}
// Passive loads: stiffness-scaled viscous damping and Hayward-Armstrong local friction.
// Simulated position uses velocity-regularized Coulomb friction. Commands are not calibrated N.
// See the realism update section in tesis/capitulos/02_realismo.tex.
inline float motionAdjustedForce(float base, float velocity, float gain,
                                bool damper=YOKE_DAMPER_ENABLED,
                                bool friction=YOKE_FRICTION_ENABLED,
                                const YokeGains::Settings& settings=YokeGains::defaults(),
                                float damperScale=1.0f) {
    if(!isfinite(base) || !isfinite(velocity) || !isfinite(gain) || gain<0) return 0;
    if(!isfinite(damperScale) || damperScale<0) return base;
    const float speed=fabsf(clip(velocity,-YOKE_MOTION_MAX_SPEED,YOKE_MOTION_MAX_SPEED));
    const float moving=speed>YOKE_MOTION_DEAD_SPEED ? speed-YOKE_MOTION_DEAD_SPEED : 0;
    float resistance=(damper ? settings.effective(YokeGains::Damper)*damperScale*moving : 0) +
        (friction ? settings.effective(YokeGains::Friction)*moving/(YOKE_FRICTION_SPEED+moving) : 0);
    resistance=clip(resistance,0,YOKE_MOTION_CORRECTION_LIMIT)*clip(gain,0,1);
    return base-(velocity<0 ? -resistance : resistance);
}
class Effects {
public:
    explicit Effects(const YokeGains::Settings& values=YokeGains::defaults()):settings(values) {}
    void reset() {
        models.reset(); modelReady=false;
        enabled=started=false; roadTime=0; centerPressure=0; groundSignal[0]=groundSignal[1]=0;
        for(uint8_t i=0;i<4;++i) { amplitude[i]=wave[i]=0; phase[i]=250; }
        for(uint8_t i=0;i<3;++i) { noise[i].reset(); carrier[i]=0; }
        for(uint8_t i=0;i<2;++i) { simMotion[i].reset(); texNoise[i].reset(); texCarrier[i]=0; friction[i].reset(); }
    }
    void update(const Receiver& rx, uint32_t now, bool connected,
                uint8_t mask=configuredEffects) {
        enabled=SIMCONNECT_ADDITIONS_ENABLED && rx.fresh(now) && connected;
        const uint32_t dt=uint32_t(now-last);
        last=now;
        if (!enabled || !started || dt>50) {
            models.reset(); modelReady=false;
            for(uint8_t i=0;i<4;++i) { amplitude[i]=0; phase[i]=250; wave[i]=0; }
            roadTime=0; centerPressure=0; groundSignal[0]=groundSignal[1]=0;
            for(uint8_t i=0;i<3;++i) { noise[i].reset(); carrier[i]=0; }
            for(uint8_t i=0;i<2;++i) { simMotion[i].reset(); texNoise[i].reset(); texCarrier[i]=0; friction[i].reset(); }
            started=enabled;
            return;
        }
        if(rx.rawAvailable && (!modelReady || modelRevision!=rx.revision)) {
            models.update(rx.raw); modelRevision=rx.revision; modelReady=true;
        } else if(!rx.rawAvailable) { models.reset(); modelReady=false; }
        // Low-level runway texture at motor-loop rate, independent of telemetry jitter.
        // The acceleration model remains responsible for actual bumps. Fixed frequencies
        // keep the speed law exclusively in the amplitude (0.5..40 kt).
        const float roadLevel=rx.rawAvailable ? models.groundEnvelope : 0;
        roadTime=roadLevel>0 ? uint32_t((roadTime+dt)%10000) : 0;
#if SIMCONNECT_GROUND_NARROWBAND_TEXTURE
        if(roadLevel>0 && dt) {
            texCarrier[0]=texNoise[0].next(); texCarrier[1]=texNoise[1].next();
        } else if(roadLevel==0) {
            for(uint8_t i=0;i<2;++i) { texNoise[i].reset(); texCarrier[i]=0; }
        }
        // Keep the former sine peak so the runway texture stays soft.
        const float texture=roadLevel>0 ? clip(roadLevel*.12f*(.65f*texCarrier[0]+.35f*texCarrier[1]),
                                               -.12f*roadLevel,.12f*roadLevel) : 0;
#else
        const float t=roadTime*.001f;
        const float texture=roadLevel*.12f*(.65f*sinf(6.2831853f*7*t)+
            .35f*sinf(6.2831853f*11.3f*t));
#endif
        for(uint8_t i=0;i<2;++i) {
            groundSignal[i]=(mask & Ground) ? (rx.rawAvailable ? models.ground[i] :
                (rx.groundForceAvailable ? rx.groundForce[i] : 0)) : 0;
            if(rx.rawAvailable && (mask & Ground))
                groundSignal[i]=clip(groundSignal[i]+texture,-roadLevel,roadLevel);
            if(rx.yokeAvailable) simMotion[i].update(rx.yoke[i],(rx.rawAvailable ? rx.raw.timeMs : rx.last)*1000UL);
            else simMotion[i].reset();
        }
        const uint8_t frequencies[4]={13,SIMCONNECT_GEAR_BUFFET_HZ,15,8};
        const float blend=float(dt)/(100.0f+dt); // 100 ms envelope and stiffness smoothing
        const float rest=settings.effective(YokeGains::CenterRest);
        const float stiffness=rest+SIMCONNECT_CENTERING_AERO_STIFFNESS*(rx.rawAvailable ? models.pressure : rx.pressure);
        centerPressure=(mask & Centering) && rx.centeringAvailable
            ? centerPressure+blend*(stiffness-centerPressure) : 0;
        for(uint8_t i=0;i<4;++i) {
            if (!(mask & (2<<i))) { amplitude[i]=0; wave[i]=0; continue; }
            // SC8 calculates ground locally; SC7 carries signed forces. Neither uses a carrier.
            if(i==3 && (rx.rawAvailable || rx.groundForceAvailable)) { amplitude[i]=wave[i]=0; continue; }
            const float target=i==3 ? rx.groundVibration : (rx.rawAvailable ? models.buffet[i] : rx.buffet[i]);
            // Stop taxi vibration immediately at rest or airborne (no filter tail).
            if(i==3 && target==0) { amplitude[i]=0; wave[i]=0; phase[i]=250; continue; }
            amplitude[i]+=blend*(target-amplitude[i]);
            phase[i]=uint16_t((phase[i]+dt*frequencies[i])%1000);
#if SIMCONNECT_BUFFET_NARROWBAND
            if(i<3) {
                if(dt) carrier[i]=clip(noise[i].next(),-1,1);
                wave[i]=amplitude[i]*carrier[i];
            } else
#endif
                wave[i]=amplitude[i]*triangle(phase[i]);
        }
    }
    // Firmware entry point: never fall back to unconnected sensors if sim position is missing.
    float commandWithPositionSource(const Receiver& rx, uint8_t axis, float gain,
                  float localPosition, uint8_t mask=configuredEffects,
                  bool useSimPosition=(YOKE_USE_SIMULATION == 1), float localVelocity=0) const {
        if(axis>1) return 0;
        if(useSimPosition && !rx.yokeAvailable) mask &= ~Centering;
        // No position filtering: a received zero cancels centering immediately.
        return command(rx,axis,gain,mask,useSimPosition ? rx.yoke[axis] : localPosition,
            useSimPosition ? (rx.yokeAvailable ? simMotion[axis].velocity : 0) : localVelocity);
    }
    float simulationVelocity(uint8_t axis) const { return axis<2 ? simMotion[axis].velocity : 0; }
    const PicoForceModels& calculated() const { return models; }
    float command(const Receiver& rx, uint8_t axis, float gain,
                  uint8_t mask=configuredEffects, float position=0, float velocity=0) const {
        if(!enabled || axis>1 || !isfinite(gain) || gain<0 || !isfinite(position)) return 0;
        float extra=0;
        if(mask & Turbulence) extra=gustCommand(rx.rawAvailable ? models.gust[axis] : rx.gust[axis],
            settings.effective(axis==0 ? YokeGains::GustRoll : YokeGains::GustPitch),gain,true,true);
        float vibration=0;
        if(mask & Stall) vibration+=wave[0]*settings.effective(YokeGains::Stall);
        if(mask & Gear) vibration+=wave[1]*settings.effective(YokeGains::Gear);
        if(mask & Spoiler) vibration+=wave[2]*settings.effective(YokeGains::Spoiler);
        extra+=vibration*(axis==0 ? 0.5f : 1.0f)*clip(gain,0,1);
        // TelemFFB aerodynamic spring principle: restoring force proportional to q and deflection.
        if(mask & Centering) extra-=clip(position,-1,1)*centerPressure*
            settings.effective(axis==0 ? YokeGains::CenterRoll : YokeGains::CenterPitch)*clip(gain,0,1);
        const float axisGain=gain*(axis==0 ? 0.5f : 1.0f);
        float damperScale=1;
#if YOKE_DAMPER_STIFFNESS_SCALING
        // Constant zeta=b/(2*sqrt(kJ)): b scales with sqrt(k) when the spring exists.
        if((mask & Centering) && rx.centeringAvailable) {
            const float reference=settings.effective(YokeGains::CenterRest)+SIMCONNECT_CENTERING_AERO_STIFFNESS;
            damperScale=clip(sqrtf(fmaxf(centerPressure,0)/reference),YOKE_DAMPER_SCALE_MIN,YOKE_DAMPER_SCALE_MAX);
        }
#endif
#if YOKE_FRICTION_POSITION_MODEL
        const float damping=motionAdjustedForce(0,velocity,axisGain,(mask & Damper)!=0,false,settings,damperScale);
        float stick=0;
        const float fc=settings.effective(YokeGains::Friction);
        // Idempotent for the same position: command may be called repeatedly per cycle.
        if((mask & Friction) && fc>0)
            stick=friction[axis].update(clip(position,-1,1),fc,YOKE_FRICTION_PRESLIDE)*clip(axisGain,0,1);
        else friction[axis].reset();
        const float ceiling=YOKE_MOTION_CORRECTION_LIMIT*clip(axisGain,0,1);
        extra+=clip(damping+stick,-ceiling,ceiling);
#else
        extra=motionAdjustedForce(extra,velocity,axisGain,(mask & Damper)!=0,(mask & Friction)!=0,settings,damperScale);
#endif
        // Taxi vibration has its own roll-dominant ratio and PWM ceiling.
        // Independent sum: velocity loads do not rescale the vibration carriers.
        if(mask & Ground) {
            const float limit=SIMCONNECT_GROUND_MAX_PWM/255.0f;
            const float signal=(rx.rawAvailable || rx.groundForceAvailable) ? groundSignal[axis] : wave[3];
            const float ground=clip(signal*settings.effective(YokeGains::Ground)*clip(gain,0,1),-limit,limit);
            extra+=axis==0 ? ground : ground/SIMCONNECT_GROUND_ROLL_PITCH_RATIO;
        }
        return clip(extra,-SIMCONNECT_EXTRA_LIMIT,SIMCONNECT_EXTRA_LIMIT);
    }
private:
// Coefficients precomputed at construction (global simEffects: before setup()).
YokeNoise::Narrowband noise[3]={
    YokeNoise::Narrowband(13,CONTROL_LOOP_MS*.001f,SIMCONNECT_BUFFET_DAMPING_RATIO,SIMCONNECT_BUFFET_RMS,0x9E3779B9u),
    YokeNoise::Narrowband(SIMCONNECT_GEAR_BUFFET_HZ,CONTROL_LOOP_MS*.001f,SIMCONNECT_BUFFET_DAMPING_RATIO,SIMCONNECT_BUFFET_RMS,0x7F4A7C15u),
    YokeNoise::Narrowband(15,CONTROL_LOOP_MS*.001f,SIMCONNECT_BUFFET_DAMPING_RATIO,SIMCONNECT_BUFFET_RMS,0x94D049BBu)};
YokeNoise::Narrowband texNoise[2]={
    YokeNoise::Narrowband(7,CONTROL_LOOP_MS*.001f,SIMCONNECT_BUFFET_DAMPING_RATIO,.7071f,0x2545F491u),
    YokeNoise::Narrowband(11.3f,CONTROL_LOOP_MS*.001f,SIMCONNECT_BUFFET_DAMPING_RATIO,.7071f,0x6C8E9CF5u)};
    float carrier[3]={0,0,0}, texCarrier[2]={0,0};
    mutable YokeFriction::Model friction[2];
    const YokeGains::Settings& settings;
    uint32_t last=0;
    bool started=false, enabled=false;
    uint16_t phase[4]={250,250,250,250};
    float amplitude[4]={0,0,0,0}, wave[4]={0,0,0,0};
    float centerPressure=0;
    float groundSignal[2]={0,0};
    uint32_t roadTime=0;
    PicoForceModels models;
    bool modelReady=false;
    uint32_t modelRevision=0;
    YokeMotion simMotion[2];
};
}
#endif
