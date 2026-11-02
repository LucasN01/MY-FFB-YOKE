#include "../include/hal/hal.h"
#include "../include/hal/pwm_plan.h"
#include <algorithm>
#include <array>
#include <deque>
#include <string>
#include <vector>
#include <cstring>

// Host-only controls and observations. No hardware or SDK dependencies.
namespace hal_fake {
uint64_t timeUs = 0;
std::array<bool, 30> gpio = {};
std::array<hal::PinMode, 30> modes = {};
std::array<uint32_t, 30> pwm = {};
std::array<std::deque<uint16_t>, 30> adc;
std::vector<std::string> events;
std::array<std::deque<uint8_t>, 2> i2cRx;
std::array<std::deque<uint8_t>, 2> i2cResults;
std::array<std::vector<uint8_t>, 2> i2cTx;
std::deque<uint8_t> usbRx, usbTx;
bool usbDtr = false, usbInitialized = false;
size_t usbCapacity = 256;
unsigned usbLocks = 0, usbBackgroundCalls = 0;
uint32_t watchdogMs = 0, watchdogFeeds = 0, irqState = 1;
hal::ResetReason reset = hal::ResetReason::Unknown;
std::array<uint8_t, 4096> flash = [] {
    std::array<uint8_t, 4096> bytes;
    bytes.fill(0xFF);
    return bytes;
}();
size_t readBytes = 0, erasedBytes = 0, programmedBytes = 0;
unsigned erases = 0, programs = 0;
bool flashFail = false;
std::array<hal::IrqHandler, 30> handlers = {};
std::array<void*, 30> contexts = {};
PwmPlan pwmPlan = planPwm(150000000, 1000, 255);
void setInput(uint8_t pin, bool high) {
    if (pin >= gpio.size()) return;
    const bool changed = gpio[pin] != high;
    gpio[pin] = high;
    if (changed && irqState && handlers[pin]) handlers[pin](contexts[pin]);
}
void event(const char* name, uint32_t a, uint32_t b) {
    events.push_back(std::string(name) + ":" + std::to_string(a) + ":" + std::to_string(b));
}
}

void FlashBackend::read(uint8_t* dst, size_t n) {
    hal_fake::readBytes = n;
    std::memcpy(dst, hal_fake::flash.data(), n);
}
bool FlashBackend::eraseProgram(const uint8_t* src, size_t n) {
    using namespace hal_fake;
    if (flashFail) return false;
    flash.fill(0xFF);
    ++erases; erasedBytes = flash.size();
    std::memcpy(flash.data(), src, n);
    ++programs; programmedBytes = n;
    return true;
}

namespace hal {
EepromImage<FlashBackend> eeprom;
CdcSerial usbSerial;
I2cBus wire0(0, 4, 5), wire1(1, 26, 27);
uint32_t micros() { return uint32_t(hal_fake::timeUs); }
uint32_t millis() { return uint32_t(hal_fake::timeUs / 1000); }
void pinMode(uint8_t pin, PinMode mode) {
    hal_fake::event("pinMode", pin, uint32_t(mode));
    if (pin >= 30) return;
    hal_fake::modes[pin] = mode;
    if (mode == PinMode::InputPullup) hal_fake::gpio[pin] = true;
}
bool digitalRead(uint8_t pin) { return pin < 30 && hal_fake::gpio[pin]; }
void digitalWrite(uint8_t pin, bool high) {
    hal_fake::event("digitalWrite", pin, high);
    if (pin < 30) hal_fake::gpio[pin] = high;
}
void attachChangeInterrupt(uint8_t pin, IrqHandler handler, void* ctx) {
    if (pin >= 30) return;
    hal_fake::handlers[pin] = handler; hal_fake::contexts[pin] = ctx;
}
uint32_t irqSave() { const auto old = hal_fake::irqState; hal_fake::irqState = 0; return old; }
void irqRestore(uint32_t state) { hal_fake::irqState = state; }
void pwmSetup(uint32_t freqHz, uint32_t range) {
    hal_fake::event("pwmSetup", freqHz, range);
    freqHz = constrain(freqHz, 100u, 10000000u);
    if (range < 3 || range > 65535) range = hal_fake::pwmPlan.scale;
    hal_fake::pwmPlan = planPwm(150000000, freqHz, range);
}
void pwmWrite(uint8_t pin, int value) {
    hal_fake::event("pwmWrite", pin, uint32_t(value));
    if (pin < 30) hal_fake::pwm[pin] = pwmLevel(hal_fake::pwmPlan, value);
}
uint16_t analogRead12(uint8_t pin) {
    if (pin < 26 || pin > 29 || hal_fake::adc[pin].empty()) return 0;
    auto& q = hal_fake::adc[pin]; const auto value = q.front(); q.pop_front(); return value;
}
void watchdogBegin(uint32_t ms) { hal_fake::watchdogMs = ms; }
void watchdogFeed() { ++hal_fake::watchdogFeeds; }
ResetReason resetReason() { return hal_fake::reset; }
namespace usb {
void init() { hal_fake::usbInitialized = true; }
void background() { ++hal_fake::usbBackgroundCalls; }
void lock() { ++hal_fake::usbLocks; }
void unlock() { if (hal_fake::usbLocks) --hal_fake::usbLocks; }
}
int CdcSerial::available() { return int(hal_fake::usbRx.size()); }
int CdcSerial::read() {
    auto& q = hal_fake::usbRx;
    if (q.empty()) return -1;
    const auto b = q.front(); q.pop_front(); return b;
}
int CdcSerial::availableForWrite() {
    return int(hal_fake::usbCapacity > hal_fake::usbTx.size()
        ? hal_fake::usbCapacity - hal_fake::usbTx.size() : 0);
}
bool CdcSerial::dtr() { return hal_fake::usbDtr; }
size_t CdcSerial::write(const uint8_t* data, size_t n) {
    if (!dtr()) return 0;
    n = (std::min)(n, size_t(availableForWrite()));
    hal_fake::usbTx.insert(hal_fake::usbTx.end(), data, data+n);
    return n;
}
size_t CdcSerial::write(const char* data, size_t n) {
    return write(reinterpret_cast<const uint8_t*>(data), n);
}
I2cBus::I2cBus(uint8_t index, uint8_t defaultSda, uint8_t defaultScl)
    : index_(index), sda_(defaultSda), scl_(defaultScl) {}
bool I2cBus::setSDA(uint8_t pin) {
    if (pin == sda_) return true;
    if (running_ || pin >= 30 || pin % 4 != index_*2) return false;
    sda_ = pin; return true;
}
bool I2cBus::setSCL(uint8_t pin) {
    if (pin == scl_) return true;
    if (running_ || pin >= 30 || pin % 4 != index_*2+1) return false;
    scl_ = pin; return true;
}
void I2cBus::begin() { running_ = true; }
void I2cBus::end() {
    running_ = false; transmitting_ = false; length_ = offset_ = 0;
    pinMode(sda_, PinMode::Input); pinMode(scl_, PinMode::Input);
}
void I2cBus::setClock(uint32_t hz) { clock_ = hz; }
void I2cBus::setTimeout(uint32_t ms, bool resetOnTimeout) {
    timeoutMs_ = ms; resetOnTimeout_ = resetOnTimeout; timedOut_ = false;
}
void I2cBus::beginTransmission(uint8_t addr) {
    address_ = addr; transmitting_ = true; length_ = offset_ = 0;
}
size_t I2cBus::write(uint8_t b) {
    if (!transmitting_ || length_ == sizeof(buffer_)) return 0;
    buffer_[length_++] = b; return 1;
}
uint8_t I2cBus::endTransmission(bool stop) {
    if (!running_ || !transmitting_) return 4;
    hal_fake::event("i2cWrite", index_, address_);
    hal_fake::event("i2cStop", index_, stop);
    hal_fake::i2cTx[index_].assign(buffer_, buffer_+length_);
    auto& results = hal_fake::i2cResults[index_];
    const uint8_t result = results.empty() ? 0 : results.front();
    if (!results.empty()) results.pop_front();
    transmitting_ = false; length_ = offset_ = 0;
    timedOut_ = result == 5;
    if (timedOut_ && resetOnTimeout_) { end(); begin(); }
    return result;
}
size_t I2cBus::requestFrom(uint8_t addr, size_t qty, bool stop) {
    if (!running_ || transmitting_ || !qty || qty > sizeof(buffer_)) return 0;
    hal_fake::event("i2cRead", index_, addr);
    hal_fake::event("i2cStop", index_, stop);
    auto& results = hal_fake::i2cResults[index_];
    const uint8_t result = results.empty() ? 0 : results.front();
    if (!results.empty()) results.pop_front();
    offset_ = length_ = 0;
    timedOut_ = result == 5;
    if (timedOut_ && resetOnTimeout_) { end(); begin(); }
    if (result) return 0;
    auto& q = hal_fake::i2cRx[index_];
    while (length_ < qty && !q.empty()) { buffer_[length_++] = q.front(); q.pop_front(); }
    return length_;
}
int I2cBus::available() { return int(length_ - offset_); }
int I2cBus::read() { return offset_ < length_ ? buffer_[offset_++] : -1; }
}
