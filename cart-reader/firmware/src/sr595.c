#include "sr595.h"

#include "hardware/gpio.h"
#include "hardware/sync.h"
#include "pins.h"

static uint16_t sr_state;

static void sr_clock_bit(bool bit) {
	gpio_put(PIN_SR_SER, bit ? 1 : 0);
	__asm volatile("nop\nnop\nnop\nnop");
	gpio_put(PIN_SR_SCK, 1);
	__asm volatile("nop\nnop\nnop\nnop");
	gpio_put(PIN_SR_SCK, 0);
}

static void sr_commit(void) {
	uint32_t irq = save_and_disable_interrupts();
	uint16_t value = sr_state;
	for (int bit = 15; bit >= 0; bit--) {
		sr_clock_bit((value >> bit) & 1);
	}
	gpio_put(PIN_SR_RCLK, 1);
	__asm volatile("nop\nnop\nnop\nnop");
	gpio_put(PIN_SR_RCLK, 0);
	restore_interrupts(irq);
}

void sr_init(void) {
	gpio_init(PIN_SR_SER);
	gpio_init(PIN_SR_SCK);
	gpio_init(PIN_SR_RCLK);
	gpio_set_dir(PIN_SR_SER, GPIO_OUT);
	gpio_set_dir(PIN_SR_SCK, GPIO_OUT);
	gpio_set_dir(PIN_SR_RCLK, GPIO_OUT);
	gpio_put(PIN_SR_SER, 0);
	gpio_put(PIN_SR_SCK, 0);
	gpio_put(PIN_SR_RCLK, 0);

	sr_state = SR_IDLE_CONTROLS;
	sr_commit();
	sr_commit();
}

void sr_set_masked(uint16_t mask, uint16_t value) {
	uint16_t next = (uint16_t)((sr_state & ~mask) | (value & mask));
	if (next == sr_state) return;
	sr_state = next;
	sr_commit();
}

uint16_t sr_get(void) {
	return sr_state;
}
