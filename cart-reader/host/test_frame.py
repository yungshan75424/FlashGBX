import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(__file__))

from protocol import crc8, pack_flash, pack_read, pack_request, pack_write, parse_response, CMD_PING


class FrameTest(unittest.TestCase):
	def test_crc_matches_firmware_vectors(self):
		self.assertEqual(crc8(b""), 0x00)
		self.assertEqual(crc8(bytes((0x01, 0x00, 0x00))), 0x6B)

	def test_ping_round_trip(self):
		req = pack_request(CMD_PING)
		self.assertEqual(req[:2], b"CR")
		self.assertEqual(req[2], CMD_PING)
		body = req[2:-1]
		self.assertEqual(req[-1], crc8(body))
		response_body = bytes((CMD_PING, 0, 4, 0)) + b"CR\x01\x00"
		response = b"CR" + response_body + bytes((crc8(response_body),))
		cmd, status, payload = parse_response(response)
		self.assertEqual(cmd, CMD_PING)
		self.assertEqual(status, 0)
		self.assertEqual(payload, b"CR\x01\x00")

	def test_read_and_flash_layouts(self):
		req = pack_read(0x08000000, 4096, 0)
		self.assertEqual(req[5:12], bytes((0x00, 0x00, 0x00, 0x08, 0x00, 0x10, 0x00)))
		data = bytes((0xAA, 0x55))
		flash = pack_flash(0xAAA, data, 0x01)
		self.assertEqual(flash[5], 0x01)
		self.assertEqual(flash[6:10], bytes((0xAA, 0x0A, 0x00, 0x00)))
		self.assertEqual(flash[10:12], bytes((2, 0)))
		self.assertEqual(flash[12:14], data)
		write = pack_write(0x2000, b"\x01", 0)
		self.assertEqual(write[5:12], bytes((0x00, 0x20, 0x00, 0x00, 0x01, 0x00, 0x00)))
		self.assertEqual(write[12], 0x01)

	def test_bad_crc_rejected(self):
		req = bytearray(pack_request(CMD_PING))
		req[-1] ^= 0xFF
		with self.assertRaises(ValueError):
			parse_response(bytes(req) + b"\x00\x00")


if __name__ == "__main__":
	unittest.main()
