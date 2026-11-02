#if defined(YOKE_TARGET_RP2350)
#include "hal/hal_i2c.h"
#include "hal/hal_gpio.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "pico/time.h"

namespace hal {
namespace {
i2c_inst_t* controller(uint8_t index) { return index == 0 ? i2c0 : i2c1; }
bool waitClock(uint8_t scl) {
    const uint64_t deadline = time_us_64() + 500;
    while (time_us_64() < deadline && !digitalRead(scl)) {}
    return digitalRead(scl);
}
bool probe(uint8_t address, uint8_t sda, uint8_t scl, uint32_t clock) {
    const uint32_t halfPeriod = (1000000 / clock) / 2;
    pinMode(sda, PinMode::InputPullup);
    pinMode(scl, PinMode::InputPullup);
    digitalWrite(sda, true);
    sleep_us(halfPeriod);
    digitalWrite(scl, true);
    bool clockOk = waitClock(scl);
    bool ack = false;
    if (clockOk) {
        digitalWrite(sda, false);
        sleep_us(halfPeriod);
        digitalWrite(scl, false);
        sleep_us(halfPeriod);
        const uint8_t writeAddress = uint8_t(address << 1);
        for (int bit = 7; bit >= 0; --bit) {
            digitalWrite(sda, (writeAddress & (1u << bit)) != 0);
            sleep_us(halfPeriod);
            digitalWrite(scl, true);
            sleep_us(halfPeriod);
            clockOk = waitClock(scl);
            if (!clockOk) break;
            digitalWrite(scl, false);
            sleep_us(5);
        }
        if (clockOk) {
            digitalWrite(sda, true);
            sleep_us(halfPeriod);
            digitalWrite(scl, true);
            clockOk = waitClock(scl);
            if (clockOk) {
                ack = !digitalRead(sda);
                sleep_us(halfPeriod);
                digitalWrite(scl, false);
            }
        }
    }
    // STOP is also required when clock stretching timed out.
    sleep_us(halfPeriod);
    digitalWrite(sda, false);
    sleep_us(halfPeriod);
    digitalWrite(scl, true);
    sleep_us(halfPeriod);
    digitalWrite(sda, true);
    sleep_us(halfPeriod);
    gpio_set_function(scl, GPIO_FUNC_I2C);
    gpio_set_function(sda, GPIO_FUNC_I2C);
    return ack;
}
void recover(I2cBus& bus, uint8_t sda, uint8_t scl, uint32_t clock) {
    bus.end();
    pinMode(sda, PinMode::InputPullup);
    pinMode(scl, PinMode::InputPullup);
    if (!digitalRead(sda)) {
        for (int pulse = 0; pulse < 9 && !digitalRead(sda); ++pulse) {
            digitalWrite(scl, false);
            sleep_us(5);
            digitalWrite(scl, true);
            sleep_us(5);
        }
        if (digitalRead(sda)) {
            digitalWrite(sda, false);
            sleep_us(5);
            digitalWrite(sda, true);
        }
    }
    bus.setClock(clock);
    bus.begin();
}
}
I2cBus wire0(0, 4, 5);
I2cBus wire1(1, 26, 27);
I2cBus::I2cBus(uint8_t index, uint8_t defaultSda, uint8_t defaultScl)
    : index_(index), sda_(defaultSda), scl_(defaultScl) {}
bool I2cBus::setSDA(uint8_t pin) {
    if (!running_ && pin < 30 && pin % 4 == index_ * 2) { sda_ = pin; return true; }
    return pin == sda_;
}
bool I2cBus::setSCL(uint8_t pin) {
    if (!running_ && pin < 30 && pin % 4 == index_ * 2 + 1) { scl_ = pin; return true; }
    return pin == scl_;
}
void I2cBus::begin() {
    if (running_) return;
    i2c_init(controller(index_), clock_);
    i2c_set_slave_mode(controller(index_), false, 0);
    gpio_set_function(sda_, GPIO_FUNC_I2C);
    gpio_pull_up(sda_);
    gpio_set_function(scl_, GPIO_FUNC_I2C);
    gpio_pull_up(scl_);
    running_ = true;
}
void I2cBus::end() {
    if (!running_) return;
    i2c_deinit(controller(index_));
    pinMode(sda_, PinMode::Input);
    pinMode(scl_, PinMode::Input);
    running_ = false;
    transmitting_ = false;
}
void I2cBus::setClock(uint32_t hz) {
    clock_ = hz;
    if (running_) i2c_set_baudrate(controller(index_), clock_);
}
void I2cBus::setTimeout(uint32_t ms, bool resetOnTimeout) {
    timeoutMs_ = ms;
    resetOnTimeout_ = resetOnTimeout;
    timedOut_ = false;
}
void I2cBus::beginTransmission(uint8_t addr) {
    if (!running_ || transmitting_) return;
    address_ = addr;
    length_ = offset_ = 0;
    transmitting_ = true;
}
size_t I2cBus::write(uint8_t b) {
    if (!transmitting_ || length_ == sizeof(buffer_)) return 0;
    buffer_[length_++] = b;
    return 1;
}
uint8_t I2cBus::endTransmission(bool stop) {
    if (!running_ || !transmitting_) return 4;
    transmitting_ = false;
    if (length_ == 0) return probe(address_, sda_, scl_, clock_) ? 0 : 2;
    const int result = i2c_write_blocking_until(controller(index_), address_, buffer_, length_,
                                               !stop, make_timeout_time_ms(timeoutMs_));
    if (result == PICO_ERROR_TIMEOUT) {
        timedOut_ = true;
        if (resetOnTimeout_) recover(*this, sda_, scl_, clock_);
        return 5;
    }
    const size_t requested = length_;
    length_ = 0;
    return result == int(requested) ? 0 : 4;
}
size_t I2cBus::requestFrom(uint8_t addr, size_t qty, bool stop) {
    if (!running_ || transmitting_ || qty == 0 || qty > sizeof(buffer_)) return 0;
    const int result = i2c_read_blocking_until(controller(index_), addr, buffer_, qty,
                                              !stop, make_timeout_time_ms(timeoutMs_));
    if (result == PICO_ERROR_TIMEOUT) {
        timedOut_ = true;
        if (resetOnTimeout_) recover(*this, sda_, scl_, clock_);
    }
    length_ = result < 0 ? 0 : size_t(result);
    offset_ = 0;
    return length_;
}
int I2cBus::read() { return offset_ < length_ ? buffer_[offset_++] : -1; }
int I2cBus::available() { return int(length_ - offset_); }
}
#endif
