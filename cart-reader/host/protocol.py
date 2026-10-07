"""Binary framing shared with cart-reader/firmware/src/frame.c."""

import struct

CMD_PING = 0x01
CMD_SET_MODE = 0x02
CMD_SET_VOLTAGE = 0x03
CMD_READ = 0x04
CMD_WRITE = 0x05
CMD_FLASH = 0x06

MODE_DMG = 0
MODE_AGB = 1
VOLT_OFF = 0
VOLT_3V3 = 1
VOLT_5V = 2
SPACE_ROM = 0
SPACE_SRAM = 1
FLASH_AGB = 0x01
FLASH_BUFFER = 0x02

ST_OK = 0
ST_BAD_CRC = 1
ST_BAD_LEN = 2
ST_BAD_VOLT = 3
ST_TIMEOUT = 4
ST_BAD_MODE = 5
ST_NO_POWER = 6
ST_ALIGN = 7
ST_BAD_CMD = 8
ST_BAD_SPACE = 9

FRAME_MAX_PAYLOAD = 4096


def crc8(data: bytes) -> int:
	crc = 0
	for byte in data:
		crc ^= byte
		for _ in range(8):
			if crc & 0x80:
				crc = ((crc << 1) ^ 0x07) & 0xFF
			else:
				crc = (crc << 1) & 0xFF
	return crc


def pack_request(cmd: int, payload: bytes = b"") -> bytes:
	if len(payload) > FRAME_MAX_PAYLOAD:
		raise ValueError("payload longer than 4096")
	body = bytes((cmd, len(payload) & 0xFF, (len(payload) >> 8) & 0xFF)) + payload
	return b"CR" + body + bytes((crc8(body),))


def parse_response(buf: bytes):
	if len(buf) < 7 or buf[:2] != b"CR":
		raise ValueError("short response")
	cmd = buf[2]
	status = buf[3]
	length = buf[4] | (buf[5] << 8)
	end = 6 + length
	if len(buf) < end + 1:
		raise ValueError("truncated response")
	if buf[end] != crc8(buf[2:end]):
		raise ValueError("bad crc")
	return cmd, status, buf[6:end]


def pack_read(addr: int, length: int, space: int) -> bytes:
	return pack_request(CMD_READ, struct.pack("<IHB", addr, length, space))


def pack_write(addr: int, data: bytes, space: int) -> bytes:
	payload = struct.pack("<IHB", addr, len(data), space) + data
	return pack_request(CMD_WRITE, payload)


def pack_flash(addr: int, data: bytes, flags: int) -> bytes:
	payload = struct.pack("<BIH", flags, addr, len(data)) + data
	return pack_request(CMD_FLASH, payload)
