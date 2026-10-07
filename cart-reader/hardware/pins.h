#pragma once
/*
 * Pin assignment for the GB / GBC / GBA cart reader.
 * Raspberry Pi Pico (RP2040), not Pico W. GP23 and GP24 stay unused.
 *
 * Cart fingers are the AGB slot numbering from GBATEK (label up, left to right).
 * In 8-bit mode (GB/GBC) the same fingers carry A0-A15 on pins 6-21 and D0-D7
 * on pins 22-29. Pin 30 is /CS2 for GBA and /RESET for GB.
 */

#define PIN_AD_BASE 0
#define PIN_AD_COUNT 16
#define MASK_AD 0x0000FFFFu

/* H0-H6 = GP16-GP22, H7 = GP26. Cart pins 22-29. */
#define PIN_H0 16
#define PIN_H7 26
#define MASK_H ((0x7Fu << 16) | (1u << 26))

#define PIN_SR_RCLK 25
#define PIN_SR_SER 27
#define PIN_SR_SCK 28

/*
 * Two chained 74LVC595s. Firmware shifts bit 15 first and bit 0 last,
 * so bit 0 lands on the first chip's QA.
 *
 * U5 (first, SER from GP27)          U6 (second, SER from U5 QH')
 * QA bit0  /RD                         QA bit8  EN_5V
 * QB bit1  /WR                         QB bit9  EN_3V3
 * QC bit2  /CS                         QC-QH    unused, stay 0
 * QD bit3  /CS2
 * QE bit4  CLK
 * QF bit5  DIR_AD   1 = MCU drives AD toward the cart
 * QG bit6  DIR_H    1 = MCU drives pins 22-29 toward the cart
 * QH bit7  BUF_EN   1 = transceiver /OE asserted
 */
#define SR_nRD    (1u << 0)
#define SR_nWR    (1u << 1)
#define SR_nCS    (1u << 2)
#define SR_nCS2   (1u << 3)
#define SR_CLK    (1u << 4)
#define SR_DIR_AD (1u << 5)
#define SR_DIR_H  (1u << 6)
#define SR_BUFEN  (1u << 7)
#define SR_EN5    (1u << 8)
#define SR_EN3    (1u << 9)

#define SR_IDLE_CONTROLS (SR_nRD | SR_nWR | SR_nCS | SR_nCS2 | SR_DIR_AD | SR_DIR_H)
