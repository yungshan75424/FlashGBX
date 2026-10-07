#include "flash_prog.h"

#include "bus.h"
#include "pico/stdlib.h"

#define POLL_US 20000u
#define BUFFER_WORDS 32u

static int poll_dmg(uint32_t addr, uint8_t expect) {
	uint64_t deadline = time_us_64() + POLL_US;
	for (;;) {
		uint8_t value = 0;
		int status = bus_read_dmg_linear(addr, &value);
		if (status != ST_OK) return status;
		if ((value & 0x80) == (expect & 0x80)) return ST_OK;
		if (time_us_64() >= deadline) return ST_TIMEOUT;
	}
}

static int poll_agb(uint32_t addr, uint16_t expect) {
	uint64_t deadline = time_us_64() + POLL_US;
	for (;;) {
		uint16_t value = 0;
		int status = bus_read_agb_rom(addr, &value);
		if (status != ST_OK) return status;
		if ((value & 0x0080) == (expect & 0x0080)) return ST_OK;
		if (time_us_64() >= deadline) return ST_TIMEOUT;
	}
}

static int flash_dmg(uint32_t addr, const uint8_t *src, uint16_t len) {
	for (uint16_t i = 0; i < len; i++) {
		uint8_t ignored = 0;
		int status = bus_read_dmg_linear(addr + i, &ignored);
		if (status != ST_OK) return status;

		status = bus_write_dmg_linear(0x555, 0xAA);
		if (status != ST_OK) return status;
		status = bus_write_dmg_linear(0x2AA, 0x55);
		if (status != ST_OK) return status;
		status = bus_write_dmg_linear(0x555, 0xA0);
		if (status != ST_OK) return status;
		status = bus_write_dmg_linear(addr + i, src[i]);
		if (status != ST_OK) return status;
		status = poll_dmg(addr + i, src[i]);
		if (status != ST_OK) return status;
	}
	return ST_OK;
}

static int agb_cmd(uint32_t addr, uint16_t data) {
	return bus_write_agb_rom(addr, data);
}

static int flash_agb_word(uint32_t addr, uint16_t data) {
	int status = agb_cmd(0xAAA, 0x00AA);
	if (status != ST_OK) return status;
	status = agb_cmd(0x555, 0x0055);
	if (status != ST_OK) return status;
	status = agb_cmd(0xAAA, 0x00A0);
	if (status != ST_OK) return status;
	status = agb_cmd(addr, data);
	if (status != ST_OK) return status;
	return poll_agb(addr, data);
}

static int flash_agb_buffer(uint32_t addr, const uint8_t *src, uint16_t len) {
	uint16_t done = 0;
	while (done < len) {
		uint16_t chunk = (uint16_t)(len - done);
		if (chunk > BUFFER_WORDS * 2u) chunk = (uint16_t)(BUFFER_WORDS * 2u);
		uint16_t words = (uint16_t)(chunk / 2u);
		uint32_t sa = addr + done;
		int status = agb_cmd(0xAAA, 0x00AA);
		if (status != ST_OK) return status;
		status = agb_cmd(0x555, 0x0055);
		if (status != ST_OK) return status;
		status = agb_cmd(sa, 0x0025);
		if (status != ST_OK) return status;
		status = agb_cmd(sa, (uint16_t)(words - 1u));
		if (status != ST_OK) return status;

		uint16_t last = 0;
		for (uint16_t w = 0; w < words; w++) {
			uint16_t word = (uint16_t)src[done + w * 2u] | ((uint16_t)src[done + w * 2u + 1u] << 8);
			last = word;
			status = agb_cmd(sa + (uint32_t)w * 2u, word);
			if (status != ST_OK) return status;
		}
		status = agb_cmd(sa, 0x0029);
		if (status != ST_OK) return status;
		status = poll_agb(sa + (uint32_t)(words - 1u) * 2u, last);
		if (status != ST_OK) return status;
		done = (uint16_t)(done + chunk);
	}
	return ST_OK;
}

int cart_flash(uint32_t addr, const uint8_t *src, uint16_t len, uint8_t flags) {
	if (len == 0 || len > FRAME_MAX_PAYLOAD) return ST_BAD_LEN;

	if (flags & FLASH_AGB) {
		if (cart_mode() != MODE_AGB) return ST_BAD_MODE;
		if ((addr | len) & 1u) return ST_ALIGN;
		if (flags & FLASH_BUFFER) return flash_agb_buffer(addr, src, len);
		for (uint16_t i = 0; i < len; i += 2) {
			uint16_t word = (uint16_t)src[i] | ((uint16_t)src[i + 1] << 8);
			int status = flash_agb_word(addr + i, word);
			if (status != ST_OK) return status;
		}
		return ST_OK;
	}

	if (flags & FLASH_BUFFER) return ST_BAD_MODE;
	if (cart_mode() != MODE_DMG) return ST_BAD_MODE;
	return flash_dmg(addr, src, len);
}
