#include "frame.h"

/* CRC-8 poly 0x07 init 0. crc8(01 00 00) == 0x6B, same as host/protocol.py. */
uint8_t crc8_update(uint8_t crc, const uint8_t *data, uint32_t len) {
	for (uint32_t i = 0; i < len; i++) {
		crc ^= data[i];
		for (int bit = 0; bit < 8; bit++) {
			if (crc & 0x80) crc = (uint8_t)((crc << 1) ^ 0x07);
			else crc = (uint8_t)(crc << 1);
		}
	}
	return crc;
}
