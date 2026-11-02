#if defined(YOKE_TARGET_RP2350)
#include "hal/hal_eeprom.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "hardware/regs/addressmap.h"
#include <cstring>

namespace {
constexpr uint32_t kOffset = PICO_FLASH_SIZE_BYTES - 2 * FLASH_SECTOR_SIZE;
static_assert(PICO_FLASH_SIZE_BYTES == 4 * 1024 * 1024);
static_assert(kOffset == 0x3FE000);
static_assert(kOffset == kEepromFlashOffset);
}
void FlashBackend::read(uint8_t* dst, size_t n) {
    std::memcpy(dst, reinterpret_cast<const void*>(XIP_BASE + kOffset), n);
}
bool FlashBackend::eraseProgram(const uint8_t* src, size_t n) {
    const uint32_t state = save_and_disable_interrupts();
    flash_range_erase(kOffset, FLASH_SECTOR_SIZE);
    flash_range_program(kOffset, src, n);
    restore_interrupts(state);
    return true;
}
namespace hal { EepromImage<FlashBackend> eeprom; }
#endif
