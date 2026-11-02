#if defined(YOKE_TARGET_RP2350)
#include "hal/hal_pwm.h"
#include "hal/hal_adc.h"
#include "hal/pwm_plan.h"
#include "hardware/adc.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

namespace hal {
bool adcPinsInitialized[4] = {};
namespace {
PwmPlan plan = {};
uint32_t pwmRange = 255;
bool slicesInitialized[NUM_PWM_SLICES] = {};
bool adcInitialized = false;
int adcChannel = -1;
}
void pwmSetup(uint32_t freqHz, uint32_t range) {
    if (freqHz < 100) freqHz = 100;
    if (freqHz > 10000000) freqHz = 10000000;
    if (range >= 3 && range <= 65535) pwmRange = range;
    plan = planPwm(clock_get_hz(clk_sys), freqHz, pwmRange);
    for (bool& initialized : slicesInitialized) initialized = false;
}
void pwmWrite(uint8_t pin, int value) {
    const uint slice = pwm_gpio_to_slice_num(pin);
    if (!slicesInitialized[slice]) {
        pwm_config config = pwm_get_default_config();
        pwm_config_set_clkdiv_int_frac4(&config, plan.divInt, plan.divFrac);
        pwm_config_set_wrap(&config, plan.scale - 1);
        pwm_init(slice, &config, true);
        slicesInitialized[slice] = true;
    }
    gpio_set_function(pin, GPIO_FUNC_PWM);
    pwm_set_gpio_level(pin, pwmLevel(plan, value));
}
uint16_t analogRead12(uint8_t pin) {
    if (pin < 26 || pin > 29) return 0;
    if (!adcInitialized) { adc_init(); adcInitialized = true; }
    if (!adcPinsInitialized[pin - 26]) {
        adc_gpio_init(pin);
        adcPinsInitialized[pin - 26] = true;
    }
    if (adcChannel != pin - 26) {
        adcChannel = pin - 26;
        adc_select_input(adcChannel);
    }
    return adc_read();
}
}
#endif
