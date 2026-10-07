#pragma once

#include <stdint.h>
#include "proto.h"

void bus_init(void);
uint8_t cart_mode(void);
uint8_t cart_voltage(void);

int cart_set_mode(uint8_t mode);
int cart_set_voltage(uint8_t voltage);

int cart_read(uint32_t addr, uint8_t *dst, uint16_t len, uint8_t space);
int cart_write(uint32_t addr, const uint8_t *src, uint16_t len, uint8_t space);

int bus_read_dmg_linear(uint32_t addr, uint8_t *value);
int bus_write_dmg_linear(uint32_t addr, uint8_t value);
int bus_read_agb_rom(uint32_t byte_addr, uint16_t *value);
int bus_write_agb_rom(uint32_t byte_addr, uint16_t value);
