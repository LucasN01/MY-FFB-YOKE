#pragma once
#include <stddef.h>
#include <stdint.h>
namespace hal {
inline constexpr uint8_t usbDeviceDescriptor[] = {
    18, 1, 0x00, 0x02, 0xEF, 0x02, 0x01, 64,
    0x8A, 0x2E, 0x0F, 0x00, 0x00, 0x01, 1, 2, 3, 1
};
inline constexpr uint8_t usbConfigurationDescriptor[] = {
    9, 2, 75, 0, 2, 1, 0, 0xA0, 50,
    8, 11, 0, 2, 2, 2, 0, 0,
    9, 4, 0, 0, 1, 2, 2, 0, 4,
    5, 0x24, 0, 0x20, 0x01,
    5, 0x24, 1, 0, 1,
    4, 0x24, 2, 6,
    5, 0x24, 6, 0, 1,
    7, 5, 0x81, 3, 8, 0, 1,
    9, 4, 1, 0, 2, 0x0A, 0, 0, 0,
    7, 5, 0x01, 2, 64, 0, 0,
    7, 5, 0x82, 2, 64, 0, 0
};
inline constexpr uint16_t usbLanguageId = 0x0409;
inline constexpr const char* usbManufacturer = "Raspberry Pi";
inline constexpr const char* usbProduct = "Pico 2";
inline constexpr const char* usbCdcName = "TinyUSB Serial";
inline void usbSerialString(const uint8_t (&id)[8], char (&serial)[17]) {
    constexpr char hex[] = "0123456789ABCDEF";
    for (size_t i = 0; i < 8; ++i) {
        serial[2*i] = hex[id[i] >> 4];
        serial[2*i+1] = hex[id[i] & 15];
    }
    serial[16] = '\0';
}
}
