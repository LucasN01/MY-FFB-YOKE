#pragma once
#include <stdint.h>
namespace hal {
enum class ResetReason : int { Unknown=0, PowerOn=1, RunPin=2, Soft=3, Wdt=4, Debug=5, Glitch=6, Brownout=7 };
inline ResetReason decodeResetReason(bool wdCaused, bool wdEnableCaused,
                                     uint32_t wdReason, uint32_t chipReset) {
    if (wdCaused && wdEnableCaused) return ResetReason::Wdt;
    if (wdReason & 0x1u) return ResetReason::Soft;
    if (chipReset & 0x00010000u) return ResetReason::PowerOn;
    if (chipReset & 0x00040000u) return ResetReason::RunPin;
    if (chipReset & (0x00080000u | 0x00200000u | 0x08000000u)) return ResetReason::Debug;
    if (chipReset & 0x04000000u) return ResetReason::Glitch;
    if (chipReset & 0x00020000u) return ResetReason::Brownout;
    return ResetReason::Unknown;
}
}
