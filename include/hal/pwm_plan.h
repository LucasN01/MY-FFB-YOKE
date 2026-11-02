#pragma once
#include <stdint.h>
struct PwmPlan {
    uint32_t scale;
    uint8_t shiftUp, shiftDown;
    uint8_t divInt, divFrac;
};
// Inputs use the validated frequency/range supplied by pwmSetup.
inline PwmPlan planPwm(uint32_t clkHz, uint32_t freqHz, uint32_t range) {
    PwmPlan p = {range, 1, 1, 0, 0};
    while (clkHz / (float(p.scale) * freqHz) > 255.0f && p.scale < 32678) {
        p.scale *= 2;
        ++p.shiftUp;
    }
    while (clkHz / (float(p.scale) * freqHz) < 1.0f && p.scale >= 6) {
        p.scale /= 2;
        ++p.shiftDown;
    }
    const float div = clkHz / (float(p.scale) * freqHz);
    const uint32_t div16 = uint32_t(div * 16.0f + 0.5f);
    p.divInt = uint8_t(div16 >> 4);
    p.divFrac = uint8_t(div16 & 15);
    return p;
}
inline uint32_t pwmLevel(const PwmPlan& p, int value) {
    if (value <= 0) return 0;
    const uint64_t level = uint64_t(value) * (uint64_t(1) << p.shiftUp)
                           / (uint64_t(1) << p.shiftDown);
    return level > p.scale ? p.scale : uint32_t(level);
}
