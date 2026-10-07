#pragma once

#include <stdint.h>
#include <stdbool.h>

void sr_init(void);
void sr_set_masked(uint16_t mask, uint16_t value);
uint16_t sr_get(void);
