#pragma once
#include "eeprom_image.h"
struct FlashBackend {
    static void read(uint8_t* dst, size_t n);
    static bool eraseProgram(const uint8_t* src, size_t n);
};
namespace hal { extern EepromImage<FlashBackend> eeprom; }
