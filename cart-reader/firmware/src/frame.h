#pragma once

#include <stdint.h>
#include "proto.h"

uint8_t crc8_update(uint8_t crc, const uint8_t *data, uint32_t len);

static inline uint32_t rd_le32(const uint8_t *p) {
	return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static inline uint16_t rd_le16(const uint8_t *p) {
	return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}
