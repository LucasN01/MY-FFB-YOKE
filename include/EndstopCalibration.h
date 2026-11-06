#ifndef YOKE_ENDSTOP_CALIBRATION_H
#define YOKE_ENDSTOP_CALIBRATION_H
#include <stdint.h>
#include <math.h>
#include "mis_defines.h"

// Stop positions learned at every power-on from the four limit switches.
// RAM only. See LEEME md/DISENO_calibracion_topes.md.
namespace YokeEndstop {
enum Stop : uint8_t { RollNeg, RollPos, PitchNeg, PitchPos, StopCount };

#if YOKE_ENDSTOP_CALIBRATION
constexpr uint8_t pins[StopCount]={ROLL_NEG_ENDSTOP_PIN,ROLL_POS_ENDSTOP_PIN,
    PITCH_NEG_ENDSTOP_PIN,PITCH_POS_ENDSTOP_PIN};
constexpr bool pinAvailable(uint8_t p) {
    return (p<=22 || (p>=26 && p<=28)) && p!=ROLL_PWM_LEFT && p!=ROLL_PWM_RIGHT &&
        p!=PITCH_PWM_LEFT && p!=PITCH_PWM_RIGHT && p!=ROLL_EN_PIN && p!=PITCH_EN_PIN &&
        p!=LCD_SDA_PIN && p!=LCD_SCL_PIN &&
        (YOKE_CALIBRATION_ENABLED==0 || (p!=YOKE_LED_NORMAL_PIN && p!=YOKE_LED_CALIBRATION_PIN &&
            p!=YOKE_BUTTON_MODE_PIN && p!=YOKE_BUTTON_OK_PIN && p!=YOKE_BUTTON_BACK_PIN &&
            p!=YOKE_BUTTON_PLUS_PIN && p!=YOKE_BUTTON_MINUS_PIN)) &&
        (YOKE_POSITION_SOURCE!=0 || (p!=ROLL_ENC_A && p!=ROLL_ENC_B && p!=PITCH_ENC_A && p!=PITCH_ENC_B)) &&
        (YOKE_POSITION_SOURCE!=2 || (p!=ROLL_POT_PIN && p!=PITCH_POT_PIN)) &&
        (YOKE_POSITION_SOURCE!=3 || (p!=ROLL_AS5600_SDA_PIN && p!=ROLL_AS5600_SCL_PIN));
}
constexpr bool pinsValid(uint8_t i=0,uint8_t j=1) {
    return i>=StopCount ? true : (!pinAvailable(pins[i]) ? false :
        (j>=StopCount ? pinsValid(i+1,i+2) : (pins[i]!=pins[j] && pinsValid(i,j+1))));
}
static_assert(pinsValid(),"Pines de finales de carrera duplicados o en conflicto con el modo de posicion");
static_assert(SIMCONNECT_ADDITIONS_ENABLED,"La calibracion de topes requiere SIMCONNECT_ADDITIONS_ENABLED");
#endif

class Calibration {
public:
    bool ready() const { return done; }
    bool captured(uint8_t stop) const { return stop<StopCount && taken[stop]; }
    // Call once per control cycle with the raw switch states and the unclipped position.
    void update(const bool pressed[StopCount], const float position[2], bool valid) {
        if(done) return;
        for(uint8_t i=0;i<StopCount;++i) {
            if(pressed[i]==stable[i]) bounce[i]=0;
            else if(++bounce[i]>=YOKE_ENDSTOP_SWITCH_DEBOUNCE_CYCLES) { stable[i]=pressed[i]; bounce[i]=0; }
        }
        // Both stops of one axis cannot be reached together: inverted or miswired sensor.
        if((stable[RollNeg] && stable[RollPos]) || (stable[PitchNeg] && stable[PitchPos])) { reject(); return; }
        hasPosition=valid && isfinite(position[0]) && isfinite(position[1]);
        if(!hasPosition) return;
        for(uint8_t axis=0;axis<2;++axis) {
            const float p=position[axis];
            if(!observed[axis] || p<lowest[axis]) lowest[axis]=p;
            if(!observed[axis] || p>highest[axis]) highest[axis]=p;
            observed[axis]=true;
        }
        for(uint8_t i=0;i<StopCount;++i) {
            if(!stable[i]) continue;
            const float p=position[i/2];
            // Keep the extreme: simulated position lags the switch.
            if(!taken[i] || ((i&1) ? p>value[i] : p<value[i])) value[i]=p;
            last[i]=p; taken[i]=true; rejected=false;
        }
        // pressed too: a switch still in its debounce must not hand the force back.
        for(uint8_t i=0;i<StopCount;++i) if(!taken[i] || stable[i] || pressed[i]) return;
        finish();
    }
    // Position in stop coordinates: +-1 at the learned stops, 0 at their midpoint.
    float normalized(uint8_t axis, float position) const {
        if(!done || axis>1) return position;
        return (position-center[axis])/halfSpan[axis];
    }
    void rows(char* top, char* bottom) const {
        copy(top,"CALIBRAR TOPES");
        if(rejected) { copy(bottom,"REPETIR TOPES"); return; }
        if(!hasPosition) { copy(bottom,"SIN POSICION"); return; }
        bool all=true;
        for(uint8_t i=0;i<StopCount;++i) all=all && taken[i];
        if(all) { copy(bottom,"SOLTAR TOPE"); return; }
        static const char* const labels[StopCount]={"R-","R+","P-","P+"};
        for(uint8_t i=0;i<StopCount;++i) {
            bottom[i*3]=taken[i] ? ' ' : labels[i][0];
            bottom[i*3+1]=taken[i] ? ' ' : labels[i][1];
            bottom[i*3+2]=' ';
        }
        bottom[11]=0;
    }
private:
    static void copy(char* out, const char* text) {
        uint8_t i=0;
        for(;i<16 && text[i];++i) out[i]=text[i];
        out[i]=0;
    }
    void reject() {
        for(uint8_t i=0;i<StopCount;++i) taken[i]=false;
        observed[0]=observed[1]=false;
        rejected=true;
    }
    void finish() {
        for(uint8_t axis=0;axis<2;++axis) {
            const uint8_t n=axis*2, p=n+1;
            const float tolerance=YOKE_ENDSTOP_CAPTURE_TOLERANCE;
            // value-last: a lone outward sample; lowest/highest: switches of the other axis.
            if(value[p]-value[n]<YOKE_ENDSTOP_MIN_SPAN ||
               value[n]-lowest[axis]>tolerance || highest[axis]-value[p]>tolerance ||
               last[n]-value[n]>tolerance || value[p]-last[p]>tolerance) { reject(); return; }
        }
        for(uint8_t axis=0;axis<2;++axis) {
            center[axis]=(value[axis*2]+value[axis*2+1])*.5f;
            halfSpan[axis]=(value[axis*2+1]-value[axis*2])*.5f;
        }
        done=true;
    }
    bool stable[StopCount]={false,false,false,false};
    uint8_t bounce[StopCount]={0,0,0,0};
    bool taken[StopCount]={false,false,false,false};
    float value[StopCount]={0,0,0,0}, last[StopCount]={0,0,0,0};
    bool observed[2]={false,false};
    float lowest[2]={0,0}, highest[2]={0,0};
    float center[2]={0,0}, halfSpan[2]={1,1};
    bool hasPosition=false, rejected=false, done=false;
};
}
#endif
