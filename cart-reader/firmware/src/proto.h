#pragma once

#include <stdint.h>

#define CMD_PING         0x01
#define CMD_SET_MODE     0x02
#define CMD_SET_VOLTAGE  0x03
#define CMD_READ         0x04
#define CMD_WRITE        0x05
#define CMD_FLASH        0x06

#define MODE_DMG 0
#define MODE_AGB 1

#define VOLT_OFF  0
#define VOLT_3V3  1
#define VOLT_5V   2

#define SPACE_ROM  0
#define SPACE_SRAM 1

#define FLASH_AGB    0x01
#define FLASH_BUFFER 0x02

#define ST_OK        0
#define ST_BAD_CRC   1
#define ST_BAD_LEN   2
#define ST_BAD_VOLT  3
#define ST_TIMEOUT   4
#define ST_BAD_MODE  5
#define ST_NO_POWER  6
#define ST_ALIGN     7
#define ST_BAD_CMD   8
#define ST_BAD_SPACE 9

#define FRAME_MAX_PAYLOAD 4096

/*
 * Request:  'C' 'R' cmd len_lo len_hi payload crc8
 * Response: 'C' 'R' cmd status len_lo len_hi payload crc8
 * CRC-8 poly 0x07, init 0, over everything after the two magic bytes.
 *
 * READ/WRITE payload starts with: addr uint32 le, data_len uint16 le, space uint8.
 * WRITE then has data_len bytes. READ's data_len is how many bytes to return.
 * FLASH payload: flags uint8, addr uint32 le, data_len uint16 le, data.
 *
 * WRITE ROM uses the raw bus address (so the host can poke an MBC register).
 * READ ROM and FLASH use a linear cartridge address. DMG linear reads at
 * 0x4000 and above switch MBC5 banks on the device.
 */
