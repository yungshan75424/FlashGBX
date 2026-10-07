#include <string.h>

#include "bsp/board_api.h"
#include "hardware/sync.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "tusb.h"

#include "bus.h"
#include "flash_prog.h"
#include "frame.h"
#include "proto.h"

#define RX_CAP (5u + FRAME_MAX_PAYLOAD + 1u)

static uint8_t rx[RX_CAP];
static uint16_t rx_got;
static uint16_t rx_need;

static volatile uint8_t job_phase;
static volatile uint8_t job_cmd;
static volatile uint8_t job_status;
static volatile uint8_t job_flags;
static volatile uint8_t job_space;
static volatile uint32_t job_addr;
static volatile uint16_t job_len;
static uint8_t job_buf[FRAME_MAX_PAYLOAD];

static void usb_write_all(const uint8_t *data, uint32_t len) {
	uint32_t off = 0;
	while (off < len) {
		tud_task();
		uint32_t space = tud_cdc_write_available();
		if (space == 0) continue;
		uint32_t n = len - off;
		if (n > space) n = space;
		off += tud_cdc_write(data + off, n);
		tud_cdc_write_flush();
	}
}

static void respond(uint8_t cmd, uint8_t status, const uint8_t *payload, uint16_t len) {
	uint8_t hdr[6];
	hdr[0] = 'C';
	hdr[1] = 'R';
	hdr[2] = cmd;
	hdr[3] = status;
	hdr[4] = (uint8_t)len;
	hdr[5] = (uint8_t)(len >> 8);
	uint8_t crc = crc8_update(0, hdr + 2, 4);
	if (len) crc = crc8_update(crc, payload, len);
	usb_write_all(hdr, 6);
	if (len) usb_write_all(payload, len);
	usb_write_all(&crc, 1);
}

static uint8_t run_job(uint8_t cmd) {
	job_cmd = cmd;
	job_status = 0xFF;
	__dmb();
	job_phase = 1;
	while (job_phase != 2) {
		tud_task();
		tight_loop_contents();
	}
	__dmb();
	return job_status;
}

static void core1_main(void) {
	while (1) {
		if (job_phase == 1) {
			uint8_t status = ST_BAD_CMD;
			switch (job_cmd) {
			case CMD_SET_MODE:
				status = (uint8_t)cart_set_mode(job_flags);
				break;
			case CMD_SET_VOLTAGE:
				status = (uint8_t)cart_set_voltage(job_flags);
				break;
			case CMD_READ:
				status = (uint8_t)cart_read(job_addr, job_buf, job_len, job_space);
				break;
			case CMD_WRITE:
				status = (uint8_t)cart_write(job_addr, job_buf, job_len, job_space);
				break;
			case CMD_FLASH:
				status = (uint8_t)cart_flash(job_addr, job_buf, job_len, job_flags);
				break;
			default:
				break;
			}
			job_status = status;
			__dmb();
			job_phase = 2;
		}
		tight_loop_contents();
	}
}

static void handle_frame(void) {
	uint16_t len = rd_le16(rx + 3);
	const uint8_t *payload = rx + 5;
	uint8_t cmd = rx[2];
	uint8_t expect = crc8_update(0, rx + 2, (uint32_t)3 + len);
	if (expect != rx[5 + len]) {
		respond(cmd, ST_BAD_CRC, 0, 0);
		return;
	}

	if (cmd == CMD_PING) {
		if (len != 0) {
			respond(cmd, ST_BAD_LEN, 0, 0);
			return;
		}
		uint8_t ver[4] = { 'C', 'R', 0x01, 0x00 };
		respond(cmd, ST_OK, ver, 4);
		return;
	}

	if (cmd == CMD_SET_MODE || cmd == CMD_SET_VOLTAGE) {
		if (len != 1) {
			respond(cmd, ST_BAD_LEN, 0, 0);
			return;
		}
		job_flags = payload[0];
		respond(cmd, run_job(cmd), 0, 0);
		return;
	}

	if (cmd == CMD_READ) {
		if (len != 7) {
			respond(cmd, ST_BAD_LEN, 0, 0);
			return;
		}
		job_addr = rd_le32(payload);
		job_len = rd_le16(payload + 4);
		job_space = payload[6];
		uint8_t status = run_job(cmd);
		if (status != ST_OK) respond(cmd, status, 0, 0);
		else respond(cmd, ST_OK, job_buf, job_len);
		return;
	}

	if (cmd == CMD_WRITE) {
		if (len < 7) {
			respond(cmd, ST_BAD_LEN, 0, 0);
			return;
		}
		job_addr = rd_le32(payload);
		job_len = rd_le16(payload + 4);
		job_space = payload[6];
		if ((uint32_t)job_len + 7u != len || job_len > FRAME_MAX_PAYLOAD) {
			respond(cmd, ST_BAD_LEN, 0, 0);
			return;
		}
		memcpy(job_buf, payload + 7, job_len);
		respond(cmd, run_job(cmd), 0, 0);
		return;
	}

	if (cmd == CMD_FLASH) {
		if (len < 7) {
			respond(cmd, ST_BAD_LEN, 0, 0);
			return;
		}
		job_flags = payload[0];
		job_addr = rd_le32(payload + 1);
		job_len = rd_le16(payload + 5);
		if ((uint32_t)job_len + 7u != len || job_len > FRAME_MAX_PAYLOAD) {
			respond(cmd, ST_BAD_LEN, 0, 0);
			return;
		}
		memcpy(job_buf, payload + 7, job_len);
		respond(cmd, run_job(cmd), 0, 0);
		return;
	}

	respond(cmd, ST_BAD_CMD, 0, 0);
}

static void feed(uint8_t byte) {
	if (rx_got == 0) {
		if (byte != 'C') return;
		rx[0] = byte;
		rx_got = 1;
		return;
	}
	if (rx_got == 1) {
		if (byte != 'R') {
			rx_got = (byte == 'C') ? 1 : 0;
			if (rx_got) rx[0] = 'C';
			return;
		}
		rx[1] = byte;
		rx_got = 2;
		return;
	}

	if (rx_got >= RX_CAP) {
		rx_got = 0;
		rx_need = 0;
		return;
	}

	rx[rx_got++] = byte;
	if (rx_got == 5) {
		uint16_t len = rd_le16(rx + 3);
		if (len > FRAME_MAX_PAYLOAD) {
			rx_got = 0;
			rx_need = 0;
			return;
		}
		rx_need = (uint16_t)(5u + len + 1u);
	}
	if (rx_need && rx_got >= rx_need) {
		handle_frame();
		rx_got = 0;
		rx_need = 0;
	}
}

int main(void) {
	board_init();
	tud_init(BOARD_TUD_RHPORT);
	bus_init();
	multicore_launch_core1(core1_main);

	while (1) {
		tud_task();
		while (tud_cdc_available()) {
			uint8_t buf[64];
			uint32_t n = tud_cdc_read(buf, sizeof buf);
			for (uint32_t i = 0; i < n; i++) feed(buf[i]);
		}
	}
}
