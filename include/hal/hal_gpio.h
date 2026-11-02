#pragma once
#include <stdint.h>
namespace hal {
enum class PinMode : uint8_t { Input, InputPullup, Output };
void pinMode(uint8_t pin, PinMode mode);
bool digitalRead(uint8_t pin);
void digitalWrite(uint8_t pin, bool high);
using IrqHandler = void (*)(void* ctx);
void attachChangeInterrupt(uint8_t pin, IrqHandler handler, void* ctx);
uint32_t irqSave();
void irqRestore(uint32_t state);
}
