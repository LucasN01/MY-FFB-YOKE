#pragma once
#if defined(ARDUINO_ARCH_RP2040) && defined(USE_TINYUSB) && YOKE_USB_GUARD
#include <pico/mutex.h>
extern mutex_t __usb_mutex;   // Adafruit_TinyUSB_rp2040.cpp:79 (enlace C++)
// Mismo criterio que cores/rp2040/SerialUSB.cpp (CoreMutex sobre el mutex USB).
// La IRQ usb_task_irq usa try_enter: si el hilo tiene el mutex, la IRQ saltea y
// tud_task() se ejecuta luego desde yield(). Mismo nucleo: nunca espera.
struct UsbLock {
    UsbLock() { mutex_enter_blocking(&__usb_mutex); }
    ~UsbLock() { mutex_exit(&__usb_mutex); }
    UsbLock(const UsbLock&) = delete; UsbLock& operator=(const UsbLock&) = delete;
};
#else
struct UsbLock { UsbLock() {} };
#endif
