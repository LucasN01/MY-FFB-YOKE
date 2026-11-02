#pragma once
#include "reset_reason.h"
namespace hal {
void watchdogBegin(uint32_t ms);
void watchdogFeed();
ResetReason resetReason();
}
