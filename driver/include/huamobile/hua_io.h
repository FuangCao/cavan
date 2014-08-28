#pragma once

#include <linux/gpio.h>
#ifdef CONFIG_OF
#include <linux/of_gpio.h>
#endif
#include <linux/regulator/consumer.h>
#include <huamobile/hua_input.h>

int hua_io_write_register_masked(struct hua_input_chip *chip, u8 addr, u8 value, u8 mask);
int hua_io_write_register16_masked(struct hua_input_chip *chip, u8 addr, u16 value, u16 mask);
ssize_t hua_io_read_write_file(const char *pathname, const char *buff, size_t size, bool store);

int hua_io_gpio_set_value(int gpio, int value);
int hua_io_set_power_regulator(struct hua_input_chip *chip, bool enable);
int hua_input_chip_io_init(struct hua_input_chip *chip);
void hua_input_chip_io_deinit(struct hua_input_chip *chip);

static inline int hua_io_reset_gpio_set_value(struct hua_input_chip *chip, int value)
{
	return hua_io_gpio_set_value(chip->gpio_reset, value);
}

static inline int hua_io_power_gpio_set_value(struct hua_input_chip *chip, int value)
{
	return hua_io_gpio_set_value(chip->gpio_power, value);
}

static inline int hua_io_irq_gpio_set_value(struct hua_input_chip *chip, int value)
{
	return hua_io_gpio_set_value(chip->gpio_irq, value);
}
