#pragma once
#include "mis_defines.h"
#include "hal/hal_usb.h"
#if YOKE_USB_GUARD
// La IRQ USB intenta tomar el mismo mutex; background atiende lo pendiente.
struct UsbLock {
    UsbLock() { hal::usb::lock(); }
    ~UsbLock() { hal::usb::unlock(); }
    UsbLock(const UsbLock&) = delete; UsbLock& operator=(const UsbLock&) = delete;
};
#else
struct UsbLock { UsbLock() {} };
#endif
