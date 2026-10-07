"""Talk to the cart reader firmware over USB CDC. One command at a time."""

import argparse
import sys

from protocol import (
	CMD_PING,
	CMD_SET_MODE,
	CMD_SET_VOLTAGE,
	FLASH_AGB,
	FLASH_BUFFER,
	FRAME_MAX_PAYLOAD,
	MODE_AGB,
	MODE_DMG,
	SPACE_ROM,
	SPACE_SRAM,
	ST_OK,
	VOLT_3V3,
	VOLT_5V,
	VOLT_OFF,
	pack_flash,
	pack_read,
	pack_request,
	pack_write,
	parse_response,
)

STATUS = {
	0: "ok",
	1: "bad crc",
	2: "bad length",
	3: "refusing 5V in GBA mode",
	4: "flash poll timeout",
	5: "bad mode",
	6: "cart power is off",
	7: "address or length is not 16-bit aligned",
	8: "bad command",
	9: "bad space",
}


def transact(port, request: bytes, timeout: float):
	port.timeout = timeout
	port.write_timeout = timeout
	port.reset_input_buffer()
	port.write(request)
	port.flush()
	header = port.read(6)
	if len(header) != 6:
		raise TimeoutError("no response header")
	length = header[4] | (header[5] << 8)
	rest = port.read(length + 1)
	if len(rest) != length + 1:
		raise TimeoutError("truncated response")
	return parse_response(header + rest)


def need_ok(cmd, status, payload):
	if status != ST_OK:
		raise SystemExit(STATUS.get(status, "status %d" % status))
	if cmd is not None and False:
		return payload
	return payload


def main(argv=None):
	parser = argparse.ArgumentParser(description="GB / GBC / GBA cart reader host")
	parser.add_argument("--port", required=True, help="USB CDC serial port")
	sub = parser.add_subparsers(dest="cmd", required=True)

	sub.add_parser("ping")

	mode = sub.add_parser("mode")
	mode.add_argument("which", choices=("dmg", "agb"))

	volt = sub.add_parser("voltage")
	volt.add_argument("which", choices=("off", "3.3", "5"))

	read = sub.add_parser("read")
	read.add_argument("--addr", type=lambda s: int(s, 0), required=True)
	read.add_argument("--len", dest="length", type=lambda s: int(s, 0), required=True)
	read.add_argument("--space", choices=("rom", "sram"), default="rom")
	read.add_argument("--out", required=True)

	write = sub.add_parser("write")
	write.add_argument("--addr", type=lambda s: int(s, 0), required=True)
	write.add_argument("--space", choices=("rom", "sram"), default="rom")
	write.add_argument("--file", required=True)

	flash = sub.add_parser("flash")
	flash.add_argument("--addr", type=lambda s: int(s, 0), required=True)
	flash.add_argument("--file", required=True)
	flash.add_argument("--agb", action="store_true")
	flash.add_argument("--buffer", action="store_true", help="AGB 32-word buffer program")

	args = parser.parse_args(argv)
	try:
		import serial
	except ImportError:
		raise SystemExit("pip install pyserial")
	port = serial.Serial(args.port, timeout=5)
	try:
		if args.cmd == "ping":
			_, status, payload = transact(port, pack_request(CMD_PING), 2)
			need_ok(None, status, payload)
			print(payload.decode("ascii", "replace"))
			return

		if args.cmd == "mode":
			value = MODE_DMG if args.which == "dmg" else MODE_AGB
			_, status, _ = transact(port, pack_request(CMD_SET_MODE, bytes((value,))), 2)
			need_ok(None, status, b"")
			print(args.which)
			return

		if args.cmd == "voltage":
			value = {"off": VOLT_OFF, "3.3": VOLT_3V3, "5": VOLT_5V}[args.which]
			_, status, _ = transact(port, pack_request(CMD_SET_VOLTAGE, bytes((value,))), 2)
			need_ok(None, status, b"")
			print(args.which)
			return

		space = SPACE_ROM
		if args.cmd in ("read", "write"):
			space = SPACE_ROM if args.space == "rom" else SPACE_SRAM

		if args.cmd == "read":
			with open(args.out, "wb") as out:
				left = args.length
				addr = args.addr
				while left:
					chunk = min(left, FRAME_MAX_PAYLOAD)
					_, status, payload = transact(port, pack_read(addr, chunk, space), 30)
					need_ok(None, status, payload)
					if len(payload) != chunk:
						raise SystemExit("short read: got %d wanted %d" % (len(payload), chunk))
					out.write(payload)
					addr += chunk
					left -= chunk
			print("wrote %s" % args.out)
			return

		data = open(args.file, "rb").read() if args.cmd in ("write", "flash") else b""

		if args.cmd == "write":
			off = 0
			while off < len(data):
				chunk = data[off:off + FRAME_MAX_PAYLOAD]
				_, status, _ = transact(port, pack_write(args.addr + off, chunk, space), 30)
				need_ok(None, status, b"")
				off += len(chunk)
			print("wrote %d bytes" % len(data))
			return

		if args.cmd == "flash":
			flags = FLASH_AGB if args.agb else 0
			if args.buffer:
				flags |= FLASH_BUFFER
			off = 0
			while off < len(data):
				chunk = data[off:off + FRAME_MAX_PAYLOAD]
				_, status, _ = transact(port, pack_flash(args.addr + off, chunk, flags), 120)
				need_ok(None, status, b"")
				off += len(chunk)
			print("programmed %d bytes" % len(data))
	finally:
		port.close()


if __name__ == "__main__":
	sys.exit(main())
