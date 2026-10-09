#pragma once
#include <Arduino.h>
#include <hardware/sync.h>

// Cuadratura x4 por IRQ en ambos canales, inicializada desde setup.
// El constructor no toca GPIO antes de que arranque el core.
class PicoEncoder {
public:
    PicoEncoder(uint8_t a, uint8_t b) : pinA(a), pinB(b) {}
    void begin() {
        pinMode(pinA, INPUT_PULLUP);
        pinMode(pinB, INPUT_PULLUP);
        previous = digitalRead(pinA) | (digitalRead(pinB) << 1);
        attachInterruptParam(digitalPinToInterrupt(pinA), changed, CHANGE, this);
        attachInterruptParam(digitalPinToInterrupt(pinB), changed, CHANGE, this);
    }
    long read() const {
        const uint32_t irq = save_and_disable_interrupts();
        const int32_t result = count;
        restore_interrupts(irq);
        return result;
    }
    void write(int32_t value) {
        const uint32_t irq = save_and_disable_interrupts();
        count = value;
        restore_interrupts(irq);
    }
private:
    uint8_t pinA, pinB, previous = 0;
    volatile int32_t count = 0;
    static void changed(void* context) {
        auto& encoder = *static_cast<PicoEncoder*>(context);
        // Mismo signo que PJRC Encoder con A en el bit menos significativo.
        static const int8_t delta[16] = {0,1,-1,0, -1,0,0,1, 1,0,0,-1, 0,-1,1,0};
        const uint8_t current = digitalRead(encoder.pinA) | (digitalRead(encoder.pinB) << 1);
        encoder.count += delta[encoder.previous | (current << 2)];
        encoder.previous = current;
    }
};
