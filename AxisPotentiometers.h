#pragma once
#include <Arduino.h>
#include "PotCalibration.h"

class AxisPotentiometers {
public:
    explicit AxisPotentiometers(const YokePots::Settings& values):settings(values) {}
    void begin() {
        analogReadResolution(12);
        pinMode(ROLL_POT_PIN, INPUT);
        pinMode(PITCH_POT_PIN, INPUT);
    }
    void update() {
        const uint8_t pins[2] = {ROLL_POT_PIN, PITCH_POT_PIN};
        for (uint8_t axis = 0; axis < 2; ++axis) {
            analogRead(pins[axis]); // descartar primera conversion al cambiar canal
            uint32_t sum = 0;
            for (uint8_t n = 0; n < YOKE_POT_SAMPLES; ++n) sum += analogRead(pins[axis]);
            readings[axis] = (sum + YOKE_POT_SAMPLES / 2) / YOKE_POT_SAMPLES;
            const YokePots::Axis& calibration=settings.axes[axis];
            sensors[axis].configure(calibration.negative,calibration.center,calibration.positive);
            positions[axis] = sensors[axis].update(readings[axis]);
        }
    }
    uint16_t raw(uint8_t axis) const { return readings[axis]; }
    float position(uint8_t axis) const { return positions[axis]; }
private:
    const YokePots::Settings& settings;
    PotentiometerPosition sensors[2] = {
        {ROLL_POT_ADC_NEG, ROLL_POT_ADC_CENTER, ROLL_POT_ADC_POS},
        {PITCH_POT_ADC_NEG, PITCH_POT_ADC_CENTER, PITCH_POT_ADC_POS}
    };
    uint16_t readings[2] = {0, 0};
    float positions[2] = {0, 0};
};
