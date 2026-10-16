#pragma once

#include <stdint.h>

// The bank latched when this application first enabled its instruction cache.
// Unlike current_rom in flash, it does not change when scheduling the next boot.
uint8_t rbootMappedBank();
