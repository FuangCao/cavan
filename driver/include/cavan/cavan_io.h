#pragma once

#include <linux/gpio.h>
#ifdef CONFIG_OF
#include <linux/of_gpio.h>
#endif
#include <linux/regulator/consumer.h>
#include <cavan/cavan_input.h>

int cavan_io_write_register_masked(struct cavan_input_chip *chip, u8 addr, u8 value, u8 mask);
int cavan_io_write_register16_masked(struct cavan_input_chip *chip, u8 addr, u16 value, u16 mask);
ssize_t cavan_io_read_write_file(const char *pathname, const char *buff, size_t size, bool store);

int cavan_io_gpio_set_value(int gpio, int value);
int cavan_io_set_power_regulator(struct cavan_input_chip *chip, bool enable);
int cavan_input_chip_io_init(struct cavan_input_chip *chip);
void cavan_input_chip_io_deinit(struct cavan_input_chip *chip);

static inline int cavan_io_reset_gpio_set_value(struct cavan_input_chip *chip, int value)
{
	return cavan_io_gpio_set_value(chip->gpio_reset, value);
}

static inline int cavan_io_power_gpio_set_value(struct cavan_input_chip *chip, int value)
{
	return cavan_io_gpio_set_value(chip->gpio_power, value);
}

static inline int cavan_io_irq_gpio_set_value(struct cavan_input_chip *chip, int value)
{
	return cavan_io_gpio_set_value(chip->gpio_irq, value);
}

static inline int cavan_io_update_bits8(struct cavan_input_chip *chip, u8 addr, u8 value, u8 mask)
{
	return cavan_io_write_register_masked(chip, addr, value, mask);
}

static inline int cavan_io_update_bits16(struct cavan_input_chip *chip, u8 addr, u16 value, u16 mask)
{
	return cavan_io_write_register16_masked(chip, addr, value, mask);
}
