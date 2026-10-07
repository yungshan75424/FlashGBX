#include "tusb.h"

#define USB_VID 0x2E8A
#define USB_PID 0xC401

enum {
	ITF_NUM_CDC = 0,
	ITF_NUM_CDC_DATA,
	ITF_NUM_TOTAL
};

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN)

static tusb_desc_device_t const desc_device = {
	.bLength            = sizeof(tusb_desc_device_t),
	.bDescriptorType    = TUSB_DESC_DEVICE,
	.bcdUSB             = 0x0200,
	.bDeviceClass       = TUSB_CLASS_MISC,
	.bDeviceSubClass    = MISC_SUBCLASS_COMMON,
	.bDeviceProtocol    = MISC_PROTOCOL_IAD,
	.bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
	.idVendor           = USB_VID,
	.idProduct          = USB_PID,
	.bcdDevice          = 0x0100,
	.iManufacturer      = 0x01,
	.iProduct           = 0x02,
	.iSerialNumber      = 0x03,
	.bNumConfigurations = 0x01
};

static uint8_t const desc_configuration[] = {
	TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 500),
	TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 0, 0x81, 8, 0x02, 64, 0x82, 64),
};

static char const *string_desc_arr[] = {
	"Cart Reader",
	"GB GBC GBA cart reader",
	"CR0100",
};

uint8_t const *tud_descriptor_device_cb(void) {
	return (uint8_t const *)&desc_device;
}

uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
	(void)index;
	return desc_configuration;
}

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
	static uint16_t desc[32];
	uint8_t chr_count;

	(void)langid;

	if (index == 0) {
		desc[1] = 0x0409;
		desc[0] = (TUSB_DESC_STRING << 8) | (2 + 2);
		return desc;
	}

	if (index > 3) return NULL;

	const char *str = string_desc_arr[index - 1];
	chr_count = 0;
	while (str[chr_count] && chr_count < 31) {
		desc[1 + chr_count] = (uint16_t)str[chr_count];
		chr_count++;
	}

	desc[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));
	return desc;
}
