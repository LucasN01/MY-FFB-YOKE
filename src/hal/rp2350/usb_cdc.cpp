#if defined(YOKE_TARGET_RP2350)
#include "hal/hal_usb.h"
#include "mis_defines.h"
#include "tusb.h"
#include "pico/mutex.h"
#include "pico/bootrom.h"
#include "hardware/irq.h"

namespace {
mutex_t usbMutex;
uint usbIrq;
void usbTaskIrq() {
    if (mutex_try_enter(&usbMutex, nullptr)) {
        tud_task();
        mutex_exit(&usbMutex);
    }
}
void triggerIrq() { irq_set_pending(usbIrq); }
}
namespace hal {
namespace usb {
void init() {
    mutex_init(&usbMutex);
    const tusb_rhport_init_t config = {TUSB_ROLE_DEVICE, TUSB_SPEED_FULL};
    tusb_init(0, &config);
    usbIrq = user_irq_claim_unused(true);
    irq_set_exclusive_handler(usbIrq, usbTaskIrq);
    irq_set_enabled(usbIrq, true);
    irq_add_shared_handler(USBCTRL_IRQ, triggerIrq, PICO_SHARED_IRQ_HANDLER_LOWEST_ORDER_PRIORITY);
}
void background() {
    if (mutex_try_enter(&usbMutex, nullptr)) {
        tud_task();
#ifndef YOKE_USB_GUARD
#error "YOKE_USB_GUARD must be defined"
#endif
#if YOKE_USB_GUARD
        tud_cdc_write_flush();
#endif
        mutex_exit(&usbMutex);
    }
#if !YOKE_USB_GUARD
    tud_cdc_write_flush();
#endif
}
void lock() { mutex_enter_blocking(&usbMutex); }
void unlock() { mutex_exit(&usbMutex); }
}
CdcSerial usbSerial;
int CdcSerial::available() { return tud_cdc_available(); }
int CdcSerial::read() { return tud_cdc_read_char(); }
int CdcSerial::availableForWrite() { return tud_cdc_write_available(); }
bool CdcSerial::dtr() { return tud_cdc_connected(); }
size_t CdcSerial::write(const uint8_t* data, size_t n) {
    if (!tud_cdc_connected()) return 0;
    return tud_cdc_write(data, n);
}
size_t CdcSerial::write(const char* data, size_t n) {
    return write(reinterpret_cast<const uint8_t*>(data), n);
}
}
extern "C" void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool) {
    if (itf != 0 || dtr) return;
    cdc_line_coding_t coding;
    tud_cdc_get_line_coding(&coding);
    if (coding.bit_rate == 1200) {
        reset_usb_boot(0, 0);
        while (true) {}
    }
}
#endif
