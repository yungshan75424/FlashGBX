#include "bus.h"

#include "hardware/gpio.h"
#include "hardware/structs/sio.h"
#include "pico/stdlib.h"
#include "pins.h"
#include "sr595.h"

/* First-board waits. Tighten GBA /RD toward 150 ns and GB toward 200 ns after a cart reads clean. */
#define SETUP_NS 100u
#define AGB_ACCESS_NS 300u
#define DMG_ACCESS_NS 500u

static uint8_t g_mode = MODE_DMG;
static uint8_t g_volt = VOLT_OFF;
static uint32_t dmg_bank = 0xFFFFFFFFu;

static void wait_ns(uint32_t ns) {
	busy_wait_at_least_cycles((ns * 125u + 999u) / 1000u);
}

static void init_bus_pin(uint pin) {
	gpio_init(pin);
	gpio_set_function(pin, GPIO_FUNC_SIO);
	gpio_set_dir(pin, GPIO_OUT);
	gpio_disable_pulls(pin);
	gpio_set_drive_strength(pin, GPIO_DRIVE_STRENGTH_8MA);
	gpio_put(pin, 0);
}

static void pins_output(uint32_t mask, bool output) {
	if (output) sio_hw->gpio_oe_set = mask;
	else sio_hw->gpio_oe_clr = mask;
}

static void ad_put(uint16_t value) {
	gpio_put_masked(MASK_AD, value);
}

static uint16_t ad_get(void) {
	return (uint16_t)(gpio_get_all() & MASK_AD);
}

static void h_put(uint8_t value) {
	uint32_t bits = ((uint32_t)(value & 0x7F)) << 16;
	if (value & 0x80) bits |= (1u << PIN_H7);
	gpio_put_masked(MASK_H, bits);
}

static uint8_t h_get(void) {
	uint32_t in = gpio_get_all();
	uint8_t value = (uint8_t)((in >> 16) & 0x7F);
	if (in & (1u << PIN_H7)) value |= 0x80;
	return value;
}

static int require_power(void) {
	if (g_volt == VOLT_OFF) return ST_NO_POWER;
	return ST_OK;
}

static void control_idle_dirs_out(void) {
	sr_set_masked(SR_nRD | SR_nWR | SR_nCS | SR_nCS2 | SR_CLK | SR_DIR_AD | SR_DIR_H,
		SR_nRD | SR_nWR | SR_nCS | SR_nCS2 | SR_DIR_AD | SR_DIR_H);
}

/* Present address, optional chip-select, pulse /RD, sample. */
static void read_cycle(uint16_t ad, uint8_t high, bool data_on_ad, bool cs, bool cs2, uint32_t access_ns, uint16_t *ad_out, uint8_t *h_out) {
	ad_put(ad);
	h_put(high);
	pins_output(MASK_AD | MASK_H, true);

	uint16_t level = SR_nRD | SR_nWR | SR_DIR_AD | SR_DIR_H;
	if (!cs) level |= SR_nCS;
	if (!cs2) level |= SR_nCS2;
	sr_set_masked(SR_nRD | SR_nWR | SR_nCS | SR_nCS2 | SR_CLK | SR_DIR_AD | SR_DIR_H, level);
	wait_ns(SETUP_NS);

	if (data_on_ad) {
		pins_output(MASK_AD, false);
		sr_set_masked(SR_DIR_AD, 0);
	} else {
		pins_output(MASK_H, false);
		sr_set_masked(SR_DIR_H, 0);
	}
	sr_set_masked(SR_nRD, 0);
	wait_ns(access_ns);

	if (ad_out) *ad_out = ad_get();
	if (h_out) *h_out = h_get();

	sr_set_masked(SR_nRD | SR_nCS | SR_nCS2 | SR_DIR_AD | SR_DIR_H,
		SR_nRD | SR_nCS | SR_nCS2 | SR_DIR_AD | SR_DIR_H);
	pins_output(MASK_AD | MASK_H, true);
}

static void write_cycle(uint16_t ad_addr, uint8_t high_addr, uint16_t ad_data, uint8_t high_data, bool data_on_ad, bool cs, bool cs2, uint32_t access_ns) {
	ad_put(ad_addr);
	h_put(high_addr);
	pins_output(MASK_AD | MASK_H, true);

	uint16_t level = SR_nRD | SR_nWR | SR_DIR_AD | SR_DIR_H;
	if (!cs) level |= SR_nCS;
	if (!cs2) level |= SR_nCS2;
	sr_set_masked(SR_nRD | SR_nWR | SR_nCS | SR_nCS2 | SR_CLK | SR_DIR_AD | SR_DIR_H, level);
	wait_ns(SETUP_NS);

	if (data_on_ad) ad_put(ad_data);
	else h_put(high_data);

	sr_set_masked(SR_nWR, 0);
	wait_ns(access_ns);
	sr_set_masked(SR_nWR | SR_nCS | SR_nCS2, SR_nWR | SR_nCS | SR_nCS2);
}

static void dmg_write_bus(uint16_t addr, uint8_t value, bool sram) {
	write_cycle(addr, value, addr, value, false, sram, false, DMG_ACCESS_NS);
}

static uint8_t dmg_read_bus(uint16_t addr, bool sram) {
	uint8_t value = 0;
	read_cycle(addr, 0, false, sram, false, DMG_ACCESS_NS, 0, &value);
	return value;
}

static void dmg_map(uint32_t linear, uint16_t *bus_addr) {
	if (linear < 0x4000u) {
		*bus_addr = (uint16_t)linear;
		return;
	}
	uint32_t bank = linear >> 14;
	if (bank != dmg_bank) {
		dmg_write_bus(0x2000, (uint8_t)bank, false);
		dmg_write_bus(0x3000, (uint8_t)(bank >> 8), false);
		dmg_bank = bank;
	}
	*bus_addr = (uint16_t)(0x4000u + (linear & 0x3FFFu));
}

static uint16_t agb_read_rom_word(uint32_t byte_addr) {
	uint32_t word = byte_addr >> 1;
	uint16_t data = 0;
	read_cycle((uint16_t)word, (uint8_t)(word >> 16), true, true, false, AGB_ACCESS_NS, &data, 0);
	return data;
}

static void agb_write_rom_word(uint32_t byte_addr, uint16_t data) {
	uint32_t word = byte_addr >> 1;
	write_cycle((uint16_t)word, (uint8_t)(word >> 16), data, (uint8_t)(word >> 16), true, true, false, AGB_ACCESS_NS);
}

static uint8_t agb_read_sram_byte(uint32_t addr) {
	uint8_t value = 0;
	read_cycle((uint16_t)addr, 0, false, false, true, AGB_ACCESS_NS, 0, &value);
	return value;
}

static void agb_write_sram_byte(uint32_t addr, uint8_t value) {
	write_cycle((uint16_t)addr, value, (uint16_t)addr, value, false, false, true, AGB_ACCESS_NS);
}

void bus_init(void) {
	for (uint pin = 0; pin < 16; pin++) init_bus_pin(pin);
	for (uint pin = 16; pin <= 22; pin++) init_bus_pin(pin);
	init_bus_pin(PIN_H7);
	sr_init();
	control_idle_dirs_out();
	g_mode = MODE_DMG;
	g_volt = VOLT_OFF;
	dmg_bank = 0xFFFFFFFFu;
}

uint8_t cart_mode(void) { return g_mode; }
uint8_t cart_voltage(void) { return g_volt; }

int cart_set_voltage(uint8_t voltage) {
	if (voltage > VOLT_5V) return ST_BAD_VOLT;
	if (voltage == VOLT_5V && g_mode == MODE_AGB) return ST_BAD_VOLT;

	sr_set_masked(SR_BUFEN, 0);
	sr_set_masked(SR_EN5 | SR_EN3, 0);
	sleep_ms(10);

	if (voltage == VOLT_5V) sr_set_masked(SR_EN5 | SR_EN3, SR_EN5);
	else if (voltage == VOLT_3V3) sr_set_masked(SR_EN5 | SR_EN3, SR_EN3);

	if (voltage != VOLT_OFF) {
		sleep_ms(5);
		control_idle_dirs_out();
		sr_set_masked(SR_BUFEN, SR_BUFEN);
	}

	g_volt = voltage;
	return ST_OK;
}

int cart_set_mode(uint8_t mode) {
	if (mode != MODE_DMG && mode != MODE_AGB) return ST_BAD_MODE;
	int status = cart_set_voltage(VOLT_OFF);
	if (status != ST_OK) return status;
	g_mode = mode;
	dmg_bank = 0xFFFFFFFFu;
	return ST_OK;
}

int bus_read_dmg_linear(uint32_t addr, uint8_t *value) {
	int status = require_power();
	if (status != ST_OK) return status;
	if (g_mode != MODE_DMG) return ST_BAD_MODE;
	uint16_t bus_addr;
	dmg_map(addr, &bus_addr);
	*value = dmg_read_bus(bus_addr, false);
	return ST_OK;
}

int bus_write_dmg_linear(uint32_t addr, uint8_t value) {
	int status = require_power();
	if (status != ST_OK) return status;
	if (g_mode != MODE_DMG) return ST_BAD_MODE;
	uint16_t bus_addr;
	dmg_map(addr, &bus_addr);
	dmg_write_bus(bus_addr, value, false);
	return ST_OK;
}

int bus_read_agb_rom(uint32_t byte_addr, uint16_t *value) {
	int status = require_power();
	if (status != ST_OK) return status;
	if (g_mode != MODE_AGB) return ST_BAD_MODE;
	if (byte_addr & 1u) return ST_ALIGN;
	*value = agb_read_rom_word(byte_addr);
	return ST_OK;
}

int bus_write_agb_rom(uint32_t byte_addr, uint16_t value) {
	int status = require_power();
	if (status != ST_OK) return status;
	if (g_mode != MODE_AGB) return ST_BAD_MODE;
	if (byte_addr & 1u) return ST_ALIGN;
	agb_write_rom_word(byte_addr, value);
	return ST_OK;
}

int cart_read(uint32_t addr, uint8_t *dst, uint16_t len, uint8_t space) {
	int status = require_power();
	if (status != ST_OK) return status;
	if (len == 0 || len > FRAME_MAX_PAYLOAD) return ST_BAD_LEN;
	if (space > SPACE_SRAM) return ST_BAD_SPACE;

	if (g_mode == MODE_DMG) {
		for (uint16_t i = 0; i < len; i++) {
			uint32_t cur = addr + i;
			uint16_t bus_addr;
			if (space == SPACE_ROM) dmg_map(cur, &bus_addr);
			else bus_addr = (uint16_t)cur;
			dst[i] = dmg_read_bus(bus_addr, space == SPACE_SRAM);
		}
		return ST_OK;
	}

	if (space == SPACE_ROM) {
		if ((addr | len) & 1u) return ST_ALIGN;
		for (uint16_t i = 0; i < len; i += 2) {
			uint16_t word = agb_read_rom_word(addr + i);
			dst[i] = (uint8_t)word;
			dst[i + 1] = (uint8_t)(word >> 8);
		}
		return ST_OK;
	}

	for (uint16_t i = 0; i < len; i++) dst[i] = agb_read_sram_byte(addr + i);
	return ST_OK;
}

int cart_write(uint32_t addr, const uint8_t *src, uint16_t len, uint8_t space) {
	int status = require_power();
	if (status != ST_OK) return status;
	if (len == 0 || len > FRAME_MAX_PAYLOAD) return ST_BAD_LEN;
	if (space > SPACE_SRAM) return ST_BAD_SPACE;

	if (g_mode == MODE_DMG) {
		for (uint16_t i = 0; i < len; i++) {
			/* Raw bus address. MBC registers are writes below 0x4000, not linear ROM. */
			dmg_write_bus((uint16_t)(addr + i), src[i], space == SPACE_SRAM);
		}
		dmg_bank = 0xFFFFFFFFu;
		return ST_OK;
	}

	if (space == SPACE_ROM) {
		if ((addr | len) & 1u) return ST_ALIGN;
		for (uint16_t i = 0; i < len; i += 2) {
			uint16_t word = (uint16_t)src[i] | ((uint16_t)src[i + 1] << 8);
			agb_write_rom_word(addr + i, word);
		}
		return ST_OK;
	}

	for (uint16_t i = 0; i < len; i++) agb_write_sram_byte(addr + i, src[i]);
	return ST_OK;
}
