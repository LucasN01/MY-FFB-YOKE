#pragma once
#include "hal/hal.h"

// Cuadratura x4 por IRQ en ambos canales, inicializada desde setup.
// El constructor no toca GPIO antes de que arranque el core.
class PicoEncoder {
public:
    PicoEncoder(uint8_t a, uint8_t b) : pinA(a), pinB(b) {}
    void begin() {
        hal::pinMode(pinA, hal::PinMode::InputPullup);
        hal::pinMode(pinB, hal::PinMode::InputPullup);
        previous = hal::digitalRead(pinA) | (hal::digitalRead(pinB) << 1);
        hal::attachChangeInterrupt(pinA, changed, this);
        hal::attachChangeInterrupt(pinB, changed, this);
    }
    long read() const {
        const uint32_t irq = hal::irqSave();
        const int32_t result = count;
        hal::irqRestore(irq);
        return result;
    }
    void write(int32_t value) {
        const uint32_t irq = hal::irqSave();
        count = value;
        hal::irqRestore(irq);
    }
private:
    uint8_t pinA, pinB, previous = 0;
    volatile int32_t count = 0;
    static void changed(void* context) {
        auto& encoder = *static_cast<PicoEncoder*>(context);
        // Mismo signo que PJRC Encoder con A en el bit menos significativo.
        static const int8_t delta[16] = {0,1,-1,0, -1,0,0,1, 1,0,0,-1, 0,-1,1,0};
        const uint8_t current = hal::digitalRead(encoder.pinA) | (hal::digitalRead(encoder.pinB) << 1);
        encoder.count += delta[encoder.previous | (current << 2)];
        encoder.previous = current;
    }
};
