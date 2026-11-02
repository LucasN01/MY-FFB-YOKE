#if defined(YOKE_TARGET_RP2350)
#include "hal/hal_time.h"
#include "hal/hal_gpio.h"
#include "pico/time.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/sync.h"

namespace hal {
// Shared only with the ADC backend; pinMode invalidates the pad configuration.
extern bool adcPinsInitialized[4];
namespace {
PinMode modes[30] = {};
struct Callback { IrqHandler handler; void* context; } callbacks[30] = {};
bool callbackInstalled = false;
void dispatch(uint pin, uint32_t) {
    if (pin < 30 && callbacks[pin].handler)
        callbacks[pin].handler(callbacks[pin].context);
}
}
uint32_t micros() { return time_us_32(); }
uint32_t millis() { return to_ms_since_boot(get_absolute_time()); }
void pinMode(uint8_t pin, PinMode mode) {
    if (pin >= 30) return;
    modes[pin] = mode;
    if (pin >= 26) adcPinsInitialized[pin - 26] = false;
    if (mode == PinMode::Output) {
        gpio_set_function(pin, GPIO_FUNC_SIO);
        gpio_set_drive_strength(pin, GPIO_DRIVE_STRENGTH_4MA);
        gpio_set_dir(pin, true);
    } else {
        gpio_set_dir(pin, false);
        gpio_set_function(pin, GPIO_FUNC_SIO);
        if (mode == PinMode::InputPullup) {
            gpio_pull_up(pin);
            gpio_put(pin, false);
        } else {
            gpio_disable_pulls(pin);
        }
    }
}
bool digitalRead(uint8_t pin) { return gpio_get(pin); }
void digitalWrite(uint8_t pin, bool high) {
    if (pin >= 30) return;
    gpio_set_function(pin, GPIO_FUNC_SIO);
    if (modes[pin] == PinMode::InputPullup) gpio_set_dir(pin, !high);
    else gpio_put(pin, high);
}
void attachChangeInterrupt(uint8_t pin, IrqHandler handler, void* ctx) {
    if (pin >= 30) return;
    callbacks[pin] = {handler, ctx};
    if (!callbackInstalled) {
        gpio_set_irq_callback(dispatch);
        irq_set_enabled(IO_IRQ_BANK0, true);
        callbackInstalled = true;
    }
    gpio_set_irq_enabled(pin, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
}
uint32_t irqSave() { return save_and_disable_interrupts(); }
void irqRestore(uint32_t state) { restore_interrupts(state); }
}
#endif
