#pragma once
#include <stdint.h>
namespace hal {
void pwmSetup(uint32_t freqHz, uint32_t range);
void pwmWrite(uint8_t pin, int value);
}
