#pragma once
#include "hal/hal.h"
#include "mis_defines.h"

namespace YokeI2C {
inline void configureBus0() {
        hal::wire0.setSDA(LCD_SDA_PIN);
        hal::wire0.setSCL(LCD_SCL_PIN);

        hal::wire0.begin();
        hal::wire0.setTimeout(2, true);

        hal::wire0.setClock(YOKE_I2C0_HZ);
}
}
