#pragma once
#include "mis_defines.h"

constexpr bool validPotCalibration(int negative, int center, int positive) {
    return negative >= 0 && negative <= 4095 && center >= 0 && center <= 4095 &&
        positive >= 0 && positive <= 4095 &&
        ((negative < center && center < positive) ||
         (positive < center && center < negative));
}
static_assert(validPotCalibration(ROLL_POT_ADC_NEG, ROLL_POT_ADC_CENTER, ROLL_POT_ADC_POS),
              "Calibracion ADC Roll invalida");
static_assert(validPotCalibration(PITCH_POT_ADC_NEG, PITCH_POT_ADC_CENTER, PITCH_POT_ADC_POS),
              "Calibracion ADC Pitch invalida");

class PotentiometerPosition {
public:
    PotentiometerPosition(int negative, int center, int positive)
        : neg(negative), mid(center), pos(positive) {}
    void configure(int negative,int center,int positive) {
        if(neg==negative && mid==center && pos==positive) return;
        neg=negative; mid=center; pos=positive; initialized=false;
    }
    float update(int adc) {
        if (!initialized) { held = adc; initialized = true; }
        // Banda movil: conserva desplazamientos lentos acumulados, sin saltar
        // toda la banda al superar el umbral.
        if (adc > held + YOKE_POT_HYSTERESIS_ADC) held = adc - YOKE_POT_HYSTERESIS_ADC;
        if (adc < held - YOKE_POT_HYSTERESIS_ADC) held = adc + YOKE_POT_HYSTERESIS_ADC;
        const int delta = held - mid;
        const bool positiveSide = pos > mid ? delta >= 0 : delta <= 0;
        float value = positiveSide ? float(delta) / (pos - mid) : -float(delta) / (neg - mid);
        return value < -1 ? -1 : (value > 1 ? 1 : value);
    }
private:
    int neg, mid, pos, held = 0;
    bool initialized = false;
};
