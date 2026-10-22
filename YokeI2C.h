#pragma once
#include <Arduino.h>
#include <Wire.h>
#include "mis_defines.h"

namespace YokeI2C {
inline void configureBus0() {
#if defined(ARDUINO_ARCH_RP2040)
        Wire.setSDA(LCD_SDA_PIN);
        Wire.setSCL(LCD_SCL_PIN);
#endif
        Wire.begin();
#if defined(ARDUINO_ARCH_RP2040)
        Wire.setTimeout(2, true);
#endif
        Wire.setClock(YOKE_I2C0_HZ);
#if defined(WIRE_HAS_TIMEOUT)
        Wire.setWireTimeout(1500, true);
        Wire.clearWireTimeoutFlag();
#endif
}
}
