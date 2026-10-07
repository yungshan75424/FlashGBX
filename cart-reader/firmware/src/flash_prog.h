#pragma once

#include <stdint.h>

int cart_flash(uint32_t addr, const uint8_t *src, uint16_t len, uint8_t flags);
