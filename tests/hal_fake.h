#pragma once
#include "../include/hal/hal.h"
#include <array>
#include <deque>
#include <vector>
#include <string>
namespace hal_fake {
extern uint64_t timeUs;
extern std::array<hal::PinMode, 30> modes;
extern std::array<uint32_t, 30> pwm;
extern std::array<std::deque<uint16_t>, 30> adc;
extern std::vector<std::string> events;
extern std::array<std::deque<uint8_t>, 2> i2cRx, i2cResults;
extern std::array<std::vector<uint8_t>, 2> i2cTx;
extern std::deque<uint8_t> usbRx, usbTx;
extern bool usbDtr, usbInitialized;
extern size_t usbCapacity;
extern unsigned usbLocks, usbBackgroundCalls;
extern uint32_t watchdogMs, watchdogFeeds;
extern hal::ResetReason reset;
extern std::array<uint8_t, 4096> flash;
extern size_t readBytes, erasedBytes, programmedBytes;
extern unsigned erases, programs;
extern bool flashFail;
void setInput(uint8_t pin, bool high);
}
