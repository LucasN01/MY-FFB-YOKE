#if defined(YOKE_TARGET_RP2350)
#include "hal/usb_descriptor_data.h"
#include "pico/unique_id.h"
#include "tusb.h"
#include <cstring>

extern "C" const uint8_t* tud_descriptor_device_cb() { return hal::usbDeviceDescriptor; }
extern "C" const uint8_t* tud_descriptor_configuration_cb(uint8_t) {
    return hal::usbConfigurationDescriptor;
}
extern "C" const uint16_t* tud_descriptor_string_cb(uint8_t index, uint16_t) {
    static uint16_t descriptor[32];
    if (index > 4) return nullptr;
    size_t length;
    if (index == 0) {
        descriptor[1] = hal::usbLanguageId;
        length = 1;
    } else {
        char serial[17];
        const char* value;
        switch (index) {
        case 1: value = hal::usbManufacturer; break;
        case 2: value = hal::usbProduct; break;
        case 3: {
            pico_unique_board_id_t id;
            pico_get_unique_board_id(&id);
            hal::usbSerialString(id.id, serial);
            value = serial;
            break;
        }
        default: value = hal::usbCdcName; break;
        }
        length = std::strlen(value);
        for (size_t i = 0; i < length; ++i) descriptor[i + 1] = uint8_t(value[i]);
    }
    descriptor[0] = uint16_t((TUSB_DESC_STRING << 8) | (2 * length + 2));
    return descriptor;
}
#endif
