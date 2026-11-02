#if defined(YOKE_TARGET_RP2350)
#include "hal/hal_watchdog.h"
#include "hal/reset_reason.h"
#include "hardware/watchdog.h"
#include "hardware/structs/powman.h"
#include "hardware/regs/powman.h"

static_assert(WATCHDOG_REASON_TIMER_BITS == 0x1);
static_assert(POWMAN_CHIP_RESET_HAD_POR_BITS == 0x00010000);
static_assert(POWMAN_CHIP_RESET_HAD_RUN_LOW_BITS == 0x00040000);
static_assert(POWMAN_CHIP_RESET_HAD_DP_RESET_REQ_BITS == 0x00080000);
static_assert(POWMAN_CHIP_RESET_HAD_RESCUE_BITS == 0x00200000);
static_assert(POWMAN_CHIP_RESET_HAD_HZD_SYS_RESET_REQ_BITS == 0x08000000);
static_assert(POWMAN_CHIP_RESET_HAD_GLITCH_DETECT_BITS == 0x04000000);
static_assert(POWMAN_CHIP_RESET_HAD_BOR_BITS == 0x00020000);
namespace hal {
void watchdogBegin(uint32_t ms) { watchdog_enable(ms, true); }
void watchdogFeed() { watchdog_update(); }
ResetReason resetReason() {
    return decodeResetReason(watchdog_caused_reboot(), watchdog_enable_caused_reboot(),
                             watchdog_hw->reason, powman_hw->chip_reset);
}
}
#endif
