#pragma once
#include <stddef.h>
#include <stdint.h>
namespace hal {
class I2cBus {
public:
    I2cBus(uint8_t index, uint8_t defaultSda, uint8_t defaultScl);
    bool setSDA(uint8_t pin); bool setSCL(uint8_t pin);
    void begin(); void end();
    void setClock(uint32_t hz);
    void setTimeout(uint32_t ms, bool resetOnTimeout);
    void beginTransmission(uint8_t addr);
    size_t write(uint8_t b);
    uint8_t endTransmission(bool stop = true);
    size_t requestFrom(uint8_t addr, size_t qty, bool stop = true);
    int read(); int available();
private:
    uint8_t index_, sda_, scl_, address_ = 0;
    uint32_t clock_ = 100000, timeoutMs_ = 1000;
    bool running_ = false, transmitting_ = false, resetOnTimeout_ = false, timedOut_ = false;
    uint8_t buffer_[32] = {};
    size_t length_ = 0, offset_ = 0;
};
extern I2cBus wire0;
extern I2cBus wire1;
}
