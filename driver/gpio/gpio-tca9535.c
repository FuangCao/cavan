/*
 * Copyright (C) 2015 Jwaoo, Inc.
 * drivers/gpio/gpio-tca9535.c
 * author: cavan.cfa@gmail.com
 * create date: 2015-05-14
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/i2c.h>
#include <linux/i2c-mux.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/rk_fb.h>
#include <linux/rk_screen.h>
#include <linux/ktime.h>
#include <linux/module.h>
#include <linux/of_gpio.h>
#include <linux/irq.h>
#include <linux/irqdomain.h>
#include <linux/interrupt.h>
#include <linux/spi/spi.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/of_regulator.h>

#define TCA9535_DEBUG					0

#define TCA9535_I2C_RATE				(400 * 1000)
#define TCA9535_GPIO_COUNT				16
#define TCA9535_KEYPAD_DEBOUNCE_MS		50

#define tca9535_pr_info(fmt, args ...) \
	 pr_err("%s[%d]: " fmt "\n", __FUNCTION__, __LINE__, ##args)

#define tca9535_pr_pos_info() \
	 pr_err("%s => %s[%d]\n", __FILE__, __FUNCTION__, __LINE__)

#define tca9535_spi_transfer_data_func_declarer(bits) \
	static int tca9535_spi_transfer_data##bits(struct tca9535_spi_device *device, struct spi_transfer *xfer) { \
		int count = xfer->len / sizeof(u##bits); \
		u##bits *rx = xfer->rx_buf; \
		const u##bits *tx = xfer->tx_buf; \
		if (rx == NULL) { \
			u16 *buff; \
			size_t size; \
			const u##bits *tx_end = tx + count; \
			struct tca9535_device *tca9535 = device->master->tca9535; \
			if (tx == NULL) { \
				return -EINVAL; \
			} \
			tca9535_lock(tca9535); \
			size = (xfer->len * 8 * 2 + 1) * 2; \
			buff = kmalloc(size, GFP_KERNEL); \
			if (buff == NULL) { \
				while (likely(tx < tx_end)) { \
					device->write_word(device, *tx++, bits); \
				} \
			} else { \
				u16 *data = buff; \
				*data++ = REG_OUTPUT_PORT << 8 | REG_OUTPUT_PORT; \
				while (likely(tx < tx_end)) { \
					data = device->build_word(device, data, *tx++, bits); \
				} \
				tca9535_master_send(tca9535, ((u8 *) buff) + 1, size - 1); \
				kfree(buff); \
			} \
			tca9535_unlock(tca9535); \
		} else { \
			u##bits word = 0; \
			int flags = 0; \
			if (tx == NULL) { \
				flags |= SPI_MASTER_NO_TX; \
			} \
			while (likely(count > 0)) { \
				if (tx) { \
					word = *tx++; \
				} \
				word = device->transfer_word(device, word, bits, flags); \
				*rx++ = word; \
				count--; \
			} \
		} \
		return 0; \
	}

struct tca9535_device;

enum tca9535_register {
	REG_INPUT_PORT = 0x00,
	REG_OUTPUT_PORT = 0x02,
	REG_POLARITY_INVERSION = 0x04,
	REG_CONFIGURATION = 0x06,
};

struct tca9535_register_cache {
	u8 bytes[0];
	u16 words[0];
	u32 dwords[0];

	u16 input_port;
	u16 output_port;
	u16 polarity_inversion;
	u16 configuration;
};

struct tca9535_part_write_config {
	u8 raw_addr;
	u8 wr_addr;
	u8 wr_offset;
	u8 wr_size;
};

struct tca9535_spi_device {
	struct spi_device *device;
	struct tca9535_spi_master *master;

	u16 cs_mask;
	bool cs_inactive;
	bool sck_active;
	bool sck_inactive;

	struct tca9535_part_write_config wr_config;

	void (*write_word)(struct tca9535_spi_device *device, u32 word, u8 bits);
	u16 *(*build_word)(struct tca9535_spi_device *device, u16 *buff, u32 word, u8 bits);
	u32 (*transfer_word)(struct tca9535_spi_device *device, u32 word, u8 bits, int flags);
	int (*transfer_data)(struct tca9535_spi_device *device, struct spi_transfer *xfer);
};

struct tca9535_spi_master {
	struct spi_master *master;
	struct tca9535_device *tca9535;
	struct tca9535_spi_master *next;

	int index;
	u16 sck_mask;
	u16 miso_mask;
	u16 mosi_mask;

	u8 rd_addr;
	struct tca9535_part_write_config wr_config;

	struct tca9535_spi_device devices[0];
};

struct tca9535_regulator {
	u16 mask;
	bool enabled;
	bool active_low;
	struct regulator_dev *rdev;
	struct regulator_desc rdesc;
	struct tca9535_device *tca9535;
	struct tca9535_regulator *next;
};

struct tca9535_device {
	struct gpio_chip gpio_chip;
	struct i2c_client *client;
	struct device *dev;
	bool crashed;
	struct tca9535_register_cache cache;

	struct mutex lock;
	struct mutex irq_lock;
	struct mutex power_lock;

	int irq;
	int gpio_irq;
	int gpio_pwr;
	int gpio_pwr_one;
	bool enabled;
	bool suspend;
	bool powerdown;
	bool first_poweron;
	struct regulator *vcc;
	struct regulator *vbus;
	struct pinctrl *pinctrl;
	struct pinctrl_state *gpio_state;
	struct completion isr_completion;

	u16 key_mask;
	u16 key_changed;
	struct input_dev *input;
	struct timer_list timer;
	int keys[TCA9535_GPIO_COUNT];

	struct irq_domain *domain;
	u16 irq_mask;
	u16 irq_trig_raise;
	u16 irq_trig_fall;

	u32 mux_adap_count;
	u32 mux_gpio_offset;
	u16 mux_gpio_unmask;
	struct i2c_adapter **mux_adaps;

	struct tca9535_spi_master *spi_head;

	struct regulator_desc rdesc;
	struct regulator_dev *rdev;
	struct tca9535_regulator *regulator_head;
};

static inline void tca9535_lock_init(struct tca9535_device *tca9535)
{
	mutex_init(&tca9535->lock);
	mutex_init(&tca9535->irq_lock);
	mutex_init(&tca9535->power_lock);
}

static inline void tca9535_lock_deinit(struct tca9535_device *tca9535)
{
	mutex_destroy(&tca9535->lock);
	mutex_destroy(&tca9535->irq_lock);
	mutex_destroy(&tca9535->power_lock);
}

static inline void tca9535_lock(struct tca9535_device *tca9535)
{
	mutex_lock(&tca9535->lock);
}

static inline void tca9535_unlock(struct tca9535_device *tca9535)
{
	mutex_unlock(&tca9535->lock);
}

static inline void tca9535_power_lock(struct tca9535_device *tca9535)
{
	mutex_lock(&tca9535->power_lock);
}

static inline void tca9535_power_unlock(struct tca9535_device *tca9535)
{
	mutex_unlock(&tca9535->power_lock);
}

static u16 tca9535_set_bit(u16 value, u16 mask, bool enable)
{
	if (enable) {
		return value | mask;
	}

	return value & (~mask);
}

static inline bool tca9535_get_bit(u16 value, u16 mask)
{
	return (value & mask) != 0;
}

static void tca9535_part_write_init(struct tca9535_part_write_config *config, u8 addr, u16 mask)
{
	config->raw_addr = addr;

	if ((mask & 0xFF00) == 0) {
		config->wr_size = 1;
		config->wr_offset = 0;
	} else if ((mask & 0x00FF) == 0) {
		config->wr_size = 1;
		config->wr_offset = 1;
	} else {
		config->wr_size = 2;
		config->wr_offset = 0;
	}

	config->wr_addr = addr + config->wr_offset;

	tca9535_pr_info("mask = 0x%04x, wr_size = %d, wr_addr = %d, wr_offset = %d",
		mask, config->wr_size, config->wr_addr, config->wr_offset);
}

static int tca9535_i2c_transfer(struct tca9535_device *tca9535, struct i2c_msg *msgs, int num)
{
	int delay = 1;
	struct i2c_client *client = tca9535->client;

	while (unlikely(i2c_transfer(client->adapter, msgs, num) < num)) {
		if (tca9535->powerdown) {
			return -EFAULT;
		}

		dev_err(&client->dev, "Failed to i2c_transfer: crashed = %d, delay = %d(ms)\n", tca9535->crashed, delay);

		if (tca9535->crashed || delay > 400) {
			tca9535->crashed = true;
			return -EFAULT;
		}

		msleep(delay);
		delay += 20;
	}

	tca9535->crashed = false;

	return num;
}

static int tca9535_i2c_transfer_adapter_locked(struct tca9535_device *tca9535, struct i2c_msg *msgs, int num)
{
	int delay = 1;
	struct i2c_client *client = tca9535->client;

	while (unlikely(__i2c_transfer(client->adapter, msgs, num) < num)) {
		if (tca9535->powerdown) {
			return -EFAULT;
		}

		dev_err(&client->dev, "Failed to __i2c_transfer: crashed = %d, delay = %d(ms)\n", tca9535->crashed, delay);

		if (tca9535->crashed || delay > 400) {
			tca9535->crashed = true;
			return -EFAULT;
		}

		msleep(delay);
		delay += 20;
	}

	tca9535->crashed = false;

	return num;
}

static int tca9535_read_data(struct tca9535_device *tca9535, u8 addr, void *buff, size_t size)
{
	int ret;
	struct i2c_client *client = tca9535->client;
	struct i2c_msg msgs[] = {
		{
			.addr = client->addr,
			.flags = (client->flags & I2C_M_TEN),
			.len = 1,
			.buf = (__u8 *) &addr,
#ifdef CONFIG_I2C_ROCKCHIP_COMPAT
			.scl_rate = TCA9535_I2C_RATE,
#endif
		}, {
			.addr = client->addr,
			.flags = (client->flags & I2C_M_TEN) | I2C_M_RD,
			.len = size,
			.buf = (__u8 *) buff,
#ifdef CONFIG_I2C_ROCKCHIP_COMPAT
			.scl_rate = TCA9535_I2C_RATE,
#endif
		}
	};

	ret = tca9535_i2c_transfer(tca9535, msgs, 2);
	if (ret < 0) {
		return ret;
	}

	return size;
}

static int tca9535_master_send(struct tca9535_device *tca9535, const void *buff, size_t size)
{
	int ret;
	struct i2c_client *client = tca9535->client;
	struct i2c_msg msg = {
		.addr = client->addr,
		.flags = (client->flags & I2C_M_TEN),
		.len = size,
		.buf = (__u8 *) buff,
#ifdef CONFIG_I2C_ROCKCHIP_COMPAT
		.scl_rate = TCA9535_I2C_RATE,
#endif
	};

	ret = tca9535_i2c_transfer(tca9535, &msg, 1);
	if (ret < 0) {
		return ret;
	}

	return size;
}

static int tca9535_master_send_adapter_locked(struct tca9535_device *tca9535, const void *buff, size_t size)
{
	int ret;
	struct i2c_client *client = tca9535->client;
	struct i2c_msg msg = {
		.addr = client->addr,
		.flags = (client->flags & I2C_M_TEN),
		.len = size,
		.buf = (__u8 *) buff,
#ifdef CONFIG_I2C_ROCKCHIP_COMPAT
		.scl_rate = TCA9535_I2C_RATE,
#endif
	};

	ret = tca9535_i2c_transfer_adapter_locked(tca9535, &msg, 1);
	if (ret < 0) {
		return ret;
	}

	return size;
}

static int tca9535_read_u8(struct tca9535_device *tca9535, u8 addr, u8 *value)
{
	int ret;

	ret = tca9535_read_data(tca9535, addr, value, 1);
	if (ret < 0) {
		return ret;
	}

	tca9535->cache.bytes[addr] = *value;

	return 0;
}

static int tca9535_read_u16(struct tca9535_device *tca9535, u8 addr, u16 *value)
{
	int ret;

	ret = tca9535_read_data(tca9535, addr, value, 2);
	if (ret < 0) {
		return ret;
	}

	*(u16 *) (tca9535->cache.bytes + addr) = *value;

	return 0;
}

static int tca9535_write_u16(struct tca9535_device *tca9535, u8 addr, u16 value)
{
	int ret = 0;
	u16 *cache = (u16 *) (tca9535->cache.bytes + addr);

	tca9535_lock(tca9535);

	if (likely(value != *cache)) {
		u8 buff[] = { addr, value & 0xFF, value >> 8 };

		ret = tca9535_master_send(tca9535, buff, sizeof(buff));
		if (likely(ret > 0)) {
			*cache = value;
		}
	}

	tca9535_unlock(tca9535);

	return ret;
}

static int tca9535_write_u16_part(struct tca9535_device *tca9535, struct tca9535_part_write_config *config, u16 value)
{
	int ret = 0;
	u16 *cache = (u16 *) (tca9535->cache.bytes + config->raw_addr);

	tca9535_lock(tca9535);

	if (likely(value != *cache)) {
		u8 buff[config->wr_size + 1];

		buff[0] = config->wr_addr;
		memcpy(buff + 1, ((u8 *) &value) + config->wr_offset, config->wr_size);

		ret = tca9535_master_send(tca9535, buff, sizeof(buff));
		if (likely(ret > 0)) {
			*cache = value;
		}
	}

	tca9535_unlock(tca9535);

	return ret;
}

static int tca9535_write_u16_adapter_locked(struct tca9535_device *tca9535, u8 addr, u16 value)
{
	int ret = 0;
	u16 *cache = (u16 *) (tca9535->cache.bytes + addr);

	tca9535_lock(tca9535);

	if (value != *cache) {
		u8 buff[] = { addr, value & 0xFF, value >> 8 };

		ret = tca9535_master_send_adapter_locked(tca9535, buff, sizeof(buff));
		if (likely(ret > 0)) {
			*cache = value;
		}
	}

	tca9535_unlock(tca9535);

	return ret;
}

static int tca9535_register_sync_locked(struct tca9535_device *tca9535, u8 addr)
{
	u16 value = *(u16 *) (tca9535->cache.bytes + addr);
	u8 buff[] = { addr, value & 0xFF, value >> 8 };

	return tca9535_master_send(tca9535, buff, sizeof(buff));
}

static int tca9535_register_sync(struct tca9535_device *tca9535)
{
	int ret = 0;

	tca9535_lock(tca9535);

	ret |= tca9535_register_sync_locked(tca9535, REG_POLARITY_INVERSION);
	ret |= tca9535_register_sync_locked(tca9535, REG_OUTPUT_PORT);
	ret |= tca9535_register_sync_locked(tca9535, REG_CONFIGURATION);

	tca9535_unlock(tca9535);

	return ret;
}

// ============================================================

static int tca9535_register_init(struct tca9535_device *tca9535)
{
	int ret;
	u32 output_configs[2];
	u16 configuration, output_port;
	struct i2c_client *client = tca9535->client;

	ret = of_property_read_u32_array(client->dev.of_node, "output-config", output_configs, ARRAY_SIZE(output_configs));
	if (ret == 0) {
		configuration = ~(output_configs[0]);
		output_port = output_configs[1];
	} else {
		configuration = 0xFFFF;
		output_port = 0x0000;
	}

	ret = tca9535_write_u16(tca9535, REG_OUTPUT_PORT, output_port);
	if (ret < 0) {
		dev_err(tca9535->dev, "Failed to tca9535_write_u16 REG_OUTPUT_PORT: %d\n", ret);
		return ret;
	}

	ret = tca9535_write_u16(tca9535, REG_POLARITY_INVERSION, 0x0000);
	if (ret < 0) {
		dev_err(tca9535->dev, "Failed to tca9535_write_u16 REG_POLARITY_INVERSION: %d\n", ret);
		return ret;
	}

	ret = tca9535_write_u16(tca9535, REG_CONFIGURATION, configuration);
	if (ret < 0) {
		dev_err(tca9535->dev, "Failed to tca9535_write_u16 REG_CONFIGURATION: %d\n", ret);
		return ret;
	}

	dev_info(tca9535->dev, "REG_OUTPUT_PORT = 0x%04x\n", tca9535->cache.output_port);
	dev_info(tca9535->dev, "REG_POLARITY_INVERSION = 0x%04x\n", tca9535->cache.polarity_inversion);
	dev_info(tca9535->dev, "REG_CONFIGURATION = 0x%04x\n", tca9535->cache.configuration);

	return 0;
}

static int tca9535_set_enable_locked(struct tca9535_device *tca9535, bool enable)
{
	int ret = 0;

	if (tca9535->enabled == enable) {
		return 0;
	}

	dev_info(tca9535->dev, "%s: enable = %d\n", __FUNCTION__, enable);

	if (enable) {
		if (tca9535->vcc) {
			ret = regulator_enable(tca9535->vcc);
			if (ret < 0) {
				dev_err(tca9535->dev, "Failed to regulator_enable vcc: %d\n", ret);
				return ret;
			}
		}

		if (tca9535->vbus) {
			ret = regulator_enable(tca9535->vbus);
			if (ret < 0) {
				dev_err(tca9535->dev, "Failed to regulator_enable vbus: %d\n", ret);
				goto out_regulator_disable_vcc;
			}
		}

		if (gpio_is_valid(tca9535->gpio_pwr)) {
			gpio_direction_output(tca9535->gpio_pwr, tca9535->gpio_pwr_one);
			gpio_set_value_cansleep(tca9535->gpio_pwr, tca9535->gpio_pwr_one);
		}

		tca9535->powerdown = false;

		if (tca9535->first_poweron) {
			ret = tca9535_register_init(tca9535);
			if (ret < 0) {
				dev_err(tca9535->dev, "Failed to tca9535_register_init: %d\n", ret);
				goto out_power_down;
			}
		} else {
			ret = tca9535_register_sync(tca9535);
			if (ret < 0) {
				dev_err(tca9535->dev, "Failed to tca9535_register_sync: %d\n", ret);
				goto out_power_down;
			}
		}

		tca9535->enabled = true;
		tca9535->first_poweron = false;

		return 0;
	}

out_power_down:
	if (gpio_is_valid(tca9535->gpio_pwr)) {
		gpio_set_value_cansleep(tca9535->gpio_pwr, !tca9535->gpio_pwr_one);
	}

	if (tca9535->vbus) {
		regulator_disable(tca9535->vbus);
	}

out_regulator_disable_vcc:
	if (tca9535->vcc) {
		regulator_disable(tca9535->vcc);
	}

	tca9535->powerdown = true;
	tca9535->enabled = false;

	return ret;
}

static int tca9535_set_enable(struct tca9535_device *tca9535, bool enable)
{
	int ret;

	tca9535_power_lock(tca9535);
	ret = tca9535_set_enable_locked(tca9535, enable);
	tca9535_power_unlock(tca9535);

	return ret;
}

static int tca9535_set_suspend_locked(struct tca9535_device *tca9535, bool enable)
{
	int ret;
	bool power_enable;

	if (tca9535->suspend == enable) {
		return 0;
	}

	if (enable) {
		power_enable = (tca9535->rdev->use_count > 0);
	} else {
		power_enable = true;
	}

	ret = tca9535_set_enable_locked(tca9535, power_enable);
	if (ret < 0) {
		dev_err(tca9535->dev, "Failed to tca9535_set_enable_locked: %d\n", ret);
		return ret;
	}

	if (tca9535->irq >= 0) {
		if (enable) {
			disable_irq(tca9535->irq);
		} else {
			complete(&tca9535->isr_completion);
		}
	}

	tca9535->suspend = enable;

	return 0;
}

static int tca9535_set_suspend(struct tca9535_device *tca9535, bool enable)
{
	int ret;

	tca9535_power_lock(tca9535);
	ret = tca9535_set_suspend_locked(tca9535, enable);
	tca9535_power_unlock(tca9535);

	return ret;
}

// ============================================================

static int tca9535_gpio_direction_input(struct gpio_chip *chip, unsigned offset)
{
	u16 value;
	struct tca9535_device *tca9535 = container_of(chip, struct tca9535_device, gpio_chip);

	value = tca9535->cache.configuration | 1 << offset;

	return tca9535_write_u16(tca9535, REG_CONFIGURATION, value);
}

static int tca9535_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	u16 value;
	struct tca9535_device *tca9535 = container_of(chip, struct tca9535_device, gpio_chip);

	if (tca9535_read_u16(tca9535, REG_INPUT_PORT, &value) < 0) {
		value = tca9535->cache.input_port;
	}

	return (value & (1 << offset)) != 0;
}

static int tca9535_gpio_direction_output(struct gpio_chip *chip, unsigned offset, int enable)
{
	int ret;
	u16 value;
	struct tca9535_device *tca9535 = container_of(chip, struct tca9535_device, gpio_chip);

	value = (tca9535->cache.output_port & (~(1 << offset))) | (!!enable) << offset;

	ret = tca9535_write_u16(tca9535, REG_OUTPUT_PORT, value);
	if (ret < 0) {
		return ret;
	}

	value = tca9535->cache.configuration & (~(1 << offset));

	ret = tca9535_write_u16(tca9535, REG_CONFIGURATION, value);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

static void tca9535_gpio_set(struct gpio_chip *chip, unsigned offset, int enable)
{
	u16 value;
	struct tca9535_device *tca9535 = container_of(chip, struct tca9535_device, gpio_chip);

	value = (tca9535->cache.output_port & (~(1 << offset))) | (!!enable) << offset;

	tca9535_write_u16(tca9535, REG_OUTPUT_PORT, value);
}

static int tca9535_gpio_to_irq(struct gpio_chip *chip, unsigned offset)
{
	struct tca9535_device *tca9535 = container_of(chip, struct tca9535_device, gpio_chip);

#if TCA9535_DEBUG
	dev_info(tca9535->dev, "%s: offset = %d\n", __FUNCTION__, offset);
#endif

	return irq_create_mapping(tca9535->domain, offset);
}

// ============================================================

static void tca9535_keypad_timer(unsigned long data)
{
	struct tca9535_device *tca9535 = (struct tca9535_device *) data;
	struct input_dev *input = tca9535->input;
	const int *keys = tca9535->keys;
	u16 value;
	u16 mask;
	int i;

	mask = tca9535->key_changed;
	tca9535->key_changed = 0;

	value = tca9535->cache.input_port;

	for (i = 0; mask; mask >>= 1, i++) {
		if (mask & 1) {
			input_report_key(input, keys[i], !(value & (1 << i)));
		}
	}

	input_sync(input);
}

static int tca9535_keypad_init(struct tca9535_device *tca9535)
{
	int ret;
	int count;
	u16 configuration;
	struct input_dev *input;
#ifdef CONFIG_OF
	struct device_node *child_node;
	struct device_node *node = tca9535->client->dev.of_node;
#endif

	input = input_allocate_device();
	if (input == NULL) {
		dev_err(tca9535->dev, "Failed to input_allocate_device");
		return -ENOMEM;
	}

	count = 0;
	configuration = tca9535->cache.configuration;

#ifdef CONFIG_OF
	for_each_child_of_node(node, child_node) {
		u16 mask;
		u32 index, code;
		const char *label;

		if (strcmp(child_node->name, "key")) {
			continue;
		}

		if (of_property_read_u32(child_node, "index", &index) < 0 || of_property_read_u32(child_node, "code", &code) < 0) {
			continue;
		}

		if (index >= TCA9535_GPIO_COUNT) {
			dev_err(tca9535->dev, "Invalid index = %d\n", index);
			continue;
		}

		tca9535->keys[index] = code;
		input_set_capability(input, EV_KEY, code);

		mask = 1 << index;
		configuration |= mask;
		tca9535->key_mask |= mask;

		if (of_property_read_string(child_node, "label", &label) < 0) {
			label = "tca9535-key";
		}

		gpio_request(tca9535->gpio_chip.base + index, label);

		dev_info(tca9535->dev, "mask = 0x%02x, index = %d, code = %d\n", mask, index, code);

		count++;
	}
#endif

	if (count == 0) {
		ret = 0;
		dev_info(tca9535->dev, "No key found\n");
		goto out_input_free_device;
	}

	ret = tca9535_write_u16(tca9535, REG_CONFIGURATION, configuration);
	if (ret < 0) {
		dev_err(tca9535->dev, "Failed to tca9535_write_u16 REG_CONFIGURATION: %d\n", ret);
		goto out_input_free_device;
	}

	input->name = "tca9535-keypad";

	ret = input_register_device(input);
	if (ret < 0) {
		dev_err(tca9535->dev, "Failed to input_register_device: %d\n", ret);
		goto out_input_free_device;
	}

	tca9535->input = input;

	setup_timer(&tca9535->timer, tca9535_keypad_timer, (unsigned long) tca9535);

	return 0;

out_input_free_device:
	input_free_device(tca9535->input);
	tca9535->input = NULL;
	return ret;
}

static void tca9535_keypad_deinit(struct tca9535_device *tca9535)
{
	if (tca9535->input) {
		del_timer_sync(&tca9535->timer);

		input_unregister_device(tca9535->input);
		input_free_device(tca9535->input);
	}
}

static void tca9535_report_keys(struct tca9535_device *tca9535, u16 mask)
{
	if (tca9535->input && mask) {
		tca9535->key_changed |= mask;
		mod_timer(&tca9535->timer, jiffies + msecs_to_jiffies(TCA9535_KEYPAD_DEBOUNCE_MS));
	}
}

// ============================================================

static void tca9535_irq_mask(struct irq_data *d)
{
	struct tca9535_device *tca9535 = irq_data_get_irq_chip_data(d);

#if TCA9535_DEBUG
	tca9535_pr_pos_info();
#endif

	tca9535->irq_mask &= ~(1 << d->hwirq);
}

static void tca9535_irq_unmask(struct irq_data *d)
{
	struct tca9535_device *tca9535 = irq_data_get_irq_chip_data(d);

#if TCA9535_DEBUG
	tca9535_pr_pos_info();
#endif

	tca9535->irq_mask |= 1 << d->hwirq;
}

static void tca9535_irq_bus_lock(struct irq_data *d)
{
	struct tca9535_device *tca9535 = irq_data_get_irq_chip_data(d);

#if TCA9535_DEBUG
	tca9535_pr_pos_info();
#endif

	mutex_lock(&tca9535->irq_lock);
}

static void tca9535_irq_bus_sync_unlock(struct irq_data *d)
{
	u16 value;
	struct tca9535_device *tca9535 = irq_data_get_irq_chip_data(d);

#if TCA9535_DEBUG
	tca9535_pr_pos_info();
#endif

	value = tca9535->cache.configuration | tca9535->irq_trig_fall | tca9535->irq_trig_raise;

	tca9535_write_u16(tca9535, REG_CONFIGURATION, value);

	mutex_unlock(&tca9535->irq_lock);
}

static int tca9535_irq_set_type(struct irq_data *d, unsigned int type)
{
	struct tca9535_device *tca9535 = irq_data_get_irq_chip_data(d);
	u16 mask = 1 << d->hwirq;

#if TCA9535_DEBUG
	tca9535_pr_pos_info();
#endif

	if ((type & IRQ_TYPE_EDGE_BOTH) == 0) {
		dev_err(tca9535->dev, "irq %d: unsupported type %d\n", d->irq, type);
		return -EINVAL;
	}

	if (type & IRQ_TYPE_EDGE_FALLING) {
		tca9535->irq_trig_fall |= mask;
	} else {
		tca9535->irq_trig_fall &= ~mask;
	}

	if (type & IRQ_TYPE_EDGE_RISING) {
		tca9535->irq_trig_raise |= mask;
	} else {
		tca9535->irq_trig_raise &= ~mask;
	}

	return 0;
}

static struct irq_chip tca9535_irq_chip = {
	.name = "TCA9535",
	.irq_mask = tca9535_irq_mask,
	.irq_unmask = tca9535_irq_unmask,
	.irq_bus_lock = tca9535_irq_bus_lock,
	.irq_bus_sync_unlock = tca9535_irq_bus_sync_unlock,
	.irq_set_type = tca9535_irq_set_type,
};

static int tca9535_gpio_irq_map(struct irq_domain *d, unsigned int irq, irq_hw_number_t hwirq)
{
	irq_clear_status_flags(irq, IRQ_NOREQUEST);
	irq_set_chip_data(irq, d->host_data);
	irq_set_chip(irq, &tca9535_irq_chip);
	irq_set_nested_thread(irq, true);

#ifdef CONFIG_ARM
	set_irq_flags(irq, IRQF_VALID);
#else
	irq_set_noprobe(irq);
#endif

	return 0;
}

static const struct irq_domain_ops tca9535_irq_simple_ops = {
	.map = tca9535_gpio_irq_map,
	.xlate = irq_domain_xlate_twocell,
};

static int tca9535_irq_init(struct tca9535_device *tca9535, const struct i2c_device_id *id, int irq_base)
{
	tca9535->domain = irq_domain_add_simple(tca9535->client->dev.of_node, tca9535->gpio_chip.ngpio, irq_base, &tca9535_irq_simple_ops, tca9535);
	if (tca9535->domain == NULL) {
		dev_err(tca9535->dev, "Failed to irq_domain_add_simple\n");
		return -ENODEV;
	}

	return 0;
}

static void tca9535_irq_deinit(struct tca9535_device *tca9535)
{
	irq_domain_remove(tca9535->domain);
}

static void tca9535_handle_nested_irq(struct tca9535_device *tca9535, u16 mask)
{
	int i;

	if (mask == 0) {
		return;
	}

	for (i = 0; mask; mask >>= 1, i++) {
		if (mask & 1) {
			handle_nested_irq(irq_find_mapping(tca9535->domain, i));
		}
	}
}

static irqreturn_t tca9535_isr_handler(int irq, void *dev_id)
{
	struct tca9535_device *tca9535 = dev_id;

	disable_irq_nosync(irq);
	complete(&tca9535->isr_completion);

	return IRQ_HANDLED;
}

static int tca9535_isr_thread(void *data)
{
	u16 mask;
	int irq, irq_count;
	u16 configuration;
	u16 value_old, value;
	ktime_t timestamp = ktime_get();
	struct tca9535_device *tca9535 = data;
	struct completion *isr_completion = &tca9535->isr_completion;

	irq_count = 0;
	irq = tca9535->irq;

	while (1) {
		while (1) {
			if (++irq_count > 100) {
				ktime_t start = timestamp;

				timestamp = ktime_get();
				if (ktime_us_delta(timestamp, start) < 1000000LL) {
					dev_err(tca9535->dev, "Interrupt too frequently\n");
					msleep(500);
					timestamp = ktime_get();
				}

				irq_count = 0;
			}

			value_old = tca9535->cache.input_port;

			while (tca9535_read_u16(tca9535, REG_INPUT_PORT, &value) < 0) {
				dev_err(tca9535->dev, "Failed to tca9535_read_u16 REG_INPUT_PORT\n");
				msleep(2000);
			}

#if TCA9535_DEBUG
			dev_info(&client->dev, "REG_INPUT_PORT = 0x%04x => 0x%04x\n", value_old, value);
#endif

			configuration = tca9535->cache.configuration;
			mask = (value_old ^ value) & configuration;

#if TCA9535_DEBUG
			dev_info(&client->dev, "mask = 0x%04x\n", mask);
#endif

			if (mask == 0) {
				break;
			}

			tca9535_report_keys(tca9535, mask & tca9535->key_mask);
			tca9535_handle_nested_irq(tca9535, mask & tca9535->irq_mask & ((tca9535->irq_trig_raise & value) | (tca9535->irq_trig_fall & value_old)));
		}

		enable_irq(irq);
		wait_for_completion(isr_completion);
	}

	return 0;
}

// ============================================================

static int tca9535_i2c_mux_select(struct i2c_adapter *adap, void *data, u32 chan)
{
	u16 value;
	struct tca9535_device *tca9535 = data;

#if TCA9535_DEBUG
	dev_info(tca9535->dev, "%s: chan = %d\n", __FUNCTION__, chan);
#endif

	value = tca9535->cache.output_port;
	value = (value & tca9535->mux_gpio_unmask) | chan << tca9535->mux_gpio_offset;

	return tca9535_write_u16_adapter_locked(tca9535, REG_OUTPUT_PORT, value);
}

static int tca9535_i2c_mux_init(struct tca9535_device *tca9535)
{
	int ret;
	int i = 0;
	u16 configuration;
	u32 mux_gpio_count;
	struct device_node *child;
	struct i2c_client *client = tca9535->client;
	struct device_node *np = client->dev.of_node;
	struct i2c_adapter *parent = tca9535->client->adapter;

	ret = of_property_read_u32(np, "mux-gpio-offset", &tca9535->mux_gpio_offset);
	if (ret < 0) {
		dev_err(&client->dev, "No property mux-gpio-offset found!\n");
		return 0;
	}

	ret = of_property_read_u32(np, "mux-gpio-count", &mux_gpio_count);
	if (ret < 0) {
		dev_err(&client->dev, "No property mux-gpio-count found!\n");
		return 0;
	}

	dev_info(&client->dev, "mux_gpio_offset = %d, mux_gpio_count = %d\n", tca9535->mux_gpio_offset, mux_gpio_count);

	if (mux_gpio_count == 0) {
		return 0;
	}

	tca9535->mux_adap_count = 1 << mux_gpio_count;
	tca9535->mux_gpio_unmask = ~(((u16) (tca9535->mux_adap_count - 1)) << tca9535->mux_gpio_offset);
	dev_info(&client->dev, "mux_adap_count = %d, mux_gpio_unmask = 0x%04x\n", tca9535->mux_adap_count, tca9535->mux_gpio_unmask);

	tca9535->mux_adaps = kzalloc(sizeof(*tca9535->mux_adaps) * tca9535->mux_adap_count, GFP_KERNEL);
	if (tca9535->mux_adaps == NULL) {
		dev_err(&client->dev, "Failed to kzalloc\n");
		return -ENOMEM;
	}

	for_each_child_of_node(np, child) {
		u32 chan;

		if (strcmp(child->name, "i2c")) {
			continue;
		}

		ret = of_property_read_u32(child, "reg", &chan);
		if (ret < 0) {
			dev_err(&client->dev, "mux[%d], no property reg found!\n", i);
			goto out_i2c_del_adapter;
		}

		if (chan >= tca9535->mux_adap_count) {
			dev_err(&client->dev, "Invalid chan = %d!\n", chan);
			ret = -EINVAL;
			goto out_i2c_del_adapter;
		}

		tca9535->mux_adaps[i] = i2c_add_mux_adapter(parent, &client->dev, tca9535, 0, chan, 0, tca9535_i2c_mux_select, NULL);
		if (tca9535->mux_adaps[i] == NULL) {
			dev_err(&client->dev, "Failed to i2c_add_mux_adapter[%d]\n", i);
			goto out_i2c_del_adapter;
		}

		i++;
	}

	configuration = tca9535->cache.configuration & tca9535->mux_gpio_unmask;

	ret = tca9535_write_u16(tca9535, REG_CONFIGURATION, configuration);
	if (ret < 0) {
		dev_err(tca9535->dev, "Failed to tca9535_write_u16 REG_CONFIGURATION: %d\n", ret);
		goto out_i2c_del_adapter;
	}

	tca9535->mux_adap_count = i;

	dev_info(&client->dev, "%d port mux on %s adapter\n", tca9535->mux_adap_count, parent->name);

	return 0;

out_i2c_del_adapter:
	while (--i >= 0) {
		i2c_del_mux_adapter(tca9535->mux_adaps[i]);
	}

	kfree(tca9535->mux_adaps);
	return ret;
}

static void tca9535_i2c_mux_deinit(struct tca9535_device *tca9535)
{
	int i = tca9535->mux_adap_count;

	if (i == 0 || tca9535->mux_adaps == NULL) {
		return;
	}

	while (--i >= 0) {
		i2c_del_mux_adapter(tca9535->mux_adaps[i]);
	}

	kfree(tca9535->mux_adaps);
}

// ============================================================

tca9535_spi_transfer_data_func_declarer(8);
tca9535_spi_transfer_data_func_declarer(16);
tca9535_spi_transfer_data_func_declarer(32);

static inline u16 tca9535_spi_device_set_cs_pin(struct tca9535_spi_device *device, u16 value, bool enable)
{
	return tca9535_set_bit(value, device->cs_mask, enable);
}

static inline u16 tca9535_spi_master_set_sck_pin(struct tca9535_spi_master *master, u16 value, bool enable)
{
	return tca9535_set_bit(value, master->sck_mask, enable);
}

static inline u16 tca9535_spi_master_set_mosi_pin(struct tca9535_spi_master *master, u16 value, bool enable)
{
	return tca9535_set_bit(value, master->mosi_mask, enable);
}

static inline bool tca9535_spi_master_get_miso_pin(struct tca9535_spi_master *master, u16 value)
{
	return tca9535_get_bit(value, master->miso_mask);
}

static int tca9535_spi_set_cs(struct tca9535_spi_device *device, bool enable)
{
	struct tca9535_spi_master *master = device->master;
	struct tca9535_device *tca9535 = master->tca9535;
	u16 value = tca9535->cache.output_port;

	value = tca9535_spi_master_set_sck_pin(master, value, device->sck_inactive);
	value = tca9535_spi_device_set_cs_pin(device, value, enable ^ device->cs_inactive);

	return tca9535_write_u16_part(tca9535, &device->wr_config, value);
}

static u32 tca9535_spi_transfer_word_be(struct tca9535_spi_device *device, u32 word, u8 bits, int flags)
{
	struct tca9535_spi_master *master = device->master;
	struct tca9535_device *tca9535 = master->tca9535;
	struct tca9535_register_cache *cache = &tca9535->cache;

	for (word <<= (32 - bits); likely(bits); bits--) {
		u8 value_input;
		u16 value = tca9535_spi_master_set_sck_pin(master, cache->output_port, device->sck_active);

		if ((flags & SPI_MASTER_NO_TX) == 0) {
			value = tca9535_spi_master_set_mosi_pin(master, value, word & (1 << 31));
		}

		if (tca9535_write_u16_part(tca9535, &master->wr_config, value) < 0) {
			return 0;
		}

		value = tca9535_spi_master_set_sck_pin(master, cache->output_port, device->sck_inactive);

		if (tca9535_write_u16_part(tca9535, &master->wr_config, value) < 0) {
			return 0;
		}

		word <<= 1;

		if (tca9535_read_u8(tca9535, master->rd_addr, &value_input) < 0) {
			return 0;
		}

		word |= tca9535_spi_master_get_miso_pin(master, value_input);
	}

	return word;
}

static u32 tca9535_spi_transfer_word_le(struct tca9535_spi_device *device, u32 word, u8 bits, int flags)
{
	struct tca9535_spi_master *master = device->master;
	struct tca9535_device *tca9535 = master->tca9535;
	struct tca9535_register_cache *cache = &tca9535->cache;
	u8 bits_remain = 32 - bits;

	while (likely(bits--)) {
		u8 value_input;
		u16 value = tca9535_spi_master_set_sck_pin(master, cache->output_port, device->sck_active);

		if ((flags & SPI_MASTER_NO_TX) == 0) {
			value = tca9535_spi_master_set_mosi_pin(master, value, word & 1);
		}

		if (tca9535_write_u16_part(tca9535, &master->wr_config, value) < 0) {
			return 0;
		}

		value = tca9535_spi_master_set_sck_pin(master, cache->output_port, device->sck_inactive);

		if (tca9535_write_u16_part(tca9535, &master->wr_config, value) < 0) {
			return 0;
		}

		word >>= 1;

		if (tca9535_read_u8(tca9535, master->rd_addr, &value_input) < 0) {
			return 0;
		}

		word |= tca9535_spi_master_get_miso_pin(master, value_input) << 31;
	}

	return word >> bits_remain;
}

static void tca9535_spi_write_word_be(struct tca9535_spi_device *device, u32 word, u8 bits)
{
	struct tca9535_spi_master *master = device->master;
	struct tca9535_device *tca9535 = master->tca9535;
	struct tca9535_register_cache *cache = &tca9535->cache;
	u16 buff[bits * 2 + 1], *p, *p_end;
	u16 value = cache->output_port;

	p = buff;
	p_end = p + sizeof(buff) / 2;
	*p++ = REG_OUTPUT_PORT << 8 | REG_OUTPUT_PORT;

	word <<= (32 - bits);

	while (likely(p < p_end)) {
		value = tca9535_spi_master_set_sck_pin(master, value, device->sck_active);
		value = tca9535_spi_master_set_mosi_pin(master, value, word & (1 << 31));
		*p++ = value;

		value = tca9535_spi_master_set_sck_pin(master, value, device->sck_inactive);
		*p++ = value;

		word <<= 1;
	}

	if (tca9535_master_send(tca9535, ((u8 *) buff) + 1, sizeof(buff) - 1) > 0) {
		cache->output_port = value;
	}
}

static void tca9535_spi_write_word_le(struct tca9535_spi_device *device, u32 word, u8 bits)
{
	struct tca9535_spi_master *master = device->master;
	struct tca9535_device *tca9535 = master->tca9535;
	struct tca9535_register_cache *cache = &tca9535->cache;
	u16 buff[bits * 2 + 1], *p, *p_end;
	u16 value = cache->output_port;

	p = buff;
	p_end = p + sizeof(buff) / 2;
	*p++ = REG_OUTPUT_PORT << 8 | REG_OUTPUT_PORT;

	while (likely(p < p_end)) {
		value = tca9535_spi_master_set_sck_pin(master, value, device->sck_active);
		value = tca9535_spi_master_set_mosi_pin(master, value, word & 1);
		*p++ = value;

		value = tca9535_spi_master_set_sck_pin(master, value, device->sck_inactive);
		*p++ = value;

		word >>= 1;
	}

	if (tca9535_master_send(tca9535, ((u8 *) buff) + 1, sizeof(buff) - 1) > 0) {
		cache->output_port = value;
	}
}

static u16 *tca9535_spi_build_word_be(struct tca9535_spi_device *device, u16 *buff, u32 word, u8 bits)
{
	struct tca9535_spi_master *master = device->master;
	struct tca9535_device *tca9535 = master->tca9535;
	struct tca9535_register_cache *cache = &tca9535->cache;
	u16 value = cache->output_port;
	u16 *buff_end;

	word <<= (32 - bits);
	buff_end = buff + bits * 2;

	while (buff < buff_end) {
		value = tca9535_spi_master_set_sck_pin(master, value, device->sck_active);
		value = tca9535_spi_master_set_mosi_pin(master, value, word & (1 << 31));
		*buff++ = value;

		value = tca9535_spi_master_set_sck_pin(master, value, device->sck_inactive);
		*buff++ = value;

		word <<= 1;
	}

	cache->output_port = value;

	return buff;
}

static u16 *tca9535_spi_build_word_le(struct tca9535_spi_device *device, u16 *buff, u32 word, u8 bits)
{
	struct tca9535_spi_master *master = device->master;
	struct tca9535_device *tca9535 = master->tca9535;
	struct tca9535_register_cache *cache = &tca9535->cache;
	u16 value = cache->output_port;
	u16 *buff_end;

	buff_end = buff + bits * 2;

	while (buff < buff_end) {
		value = tca9535_spi_master_set_sck_pin(master, value, device->sck_active);
		value = tca9535_spi_master_set_mosi_pin(master, value, word & 1);
		*buff++ = value;

		value = tca9535_spi_master_set_sck_pin(master, value, device->sck_inactive);
		*buff++ = value;

		word >>= 1;
	}

	cache->output_port = value;

	return buff;
}

static int tca9535_spi_setup(struct spi_device *spi)
{
	int ret;
	struct tca9535_spi_master *master = spi_master_get_devdata(spi->master);
	struct tca9535_spi_device *device = master->devices + spi->chip_select;

	dev_info(&spi->dev, "chip_select = %d, mode = 0x%02x\n", spi->chip_select, spi->mode);
	dev_info(&spi->dev, "bits_per_word = %d\n", spi->bits_per_word);

	if (spi->bits_per_word <= 8) {
		device->transfer_data = tca9535_spi_transfer_data8;
	} else if (spi->bits_per_word <= 16) {
		device->transfer_data = tca9535_spi_transfer_data16;
	} else if (spi->bits_per_word <= 32) {
		device->transfer_data = tca9535_spi_transfer_data32;
	} else {
		return -EINVAL;
	}

	if (spi->mode & SPI_LSB_FIRST) {
		device->write_word = tca9535_spi_write_word_le;
		device->build_word = tca9535_spi_build_word_le;
		device->transfer_word = tca9535_spi_transfer_word_le;
	} else {
		device->write_word = tca9535_spi_write_word_be;
		device->build_word = tca9535_spi_build_word_be;
		device->transfer_word = tca9535_spi_transfer_word_be;
	}

	device->device = spi;
	device->sck_active = !(spi->mode & SPI_CPOL);
	device->sck_inactive = !device->sck_active;
	device->cs_inactive = !(spi->mode & SPI_CS_HIGH);

	dev_info(&spi->dev, "sck_inactive = %d, sck_active = %d\n", device->sck_inactive, device->sck_active);
	dev_info(&spi->dev, "cs_inactive = %d, cs_mask = 0x%04x\n", device->cs_inactive, device->cs_mask);

	ret = tca9535_spi_set_cs(device, false);
	if (ret < 0) {
		dev_err(&spi->dev, "Failed to tca9535_spi_set_cs: %d\n", ret);
		return ret;
	}

	return 0;
}

static void tca9535_spi_cleanup(struct spi_device *spi)
{
	tca9535_pr_pos_info();
}

static int tca9535_spi_transfer_one_message(struct spi_master *master, struct spi_message *msg)
{
	struct tca9535_spi_master *tca9535_spi = spi_master_get_devdata(master);
	struct tca9535_spi_device *device = tca9535_spi->devices + msg->spi->chip_select;
	struct spi_transfer *xfer;
	bool cs_change = true;

	list_for_each_entry(xfer, &msg->transfers, transfer_list) {
		if (cs_change) {
			tca9535_spi_set_cs(device, true);
		}

		cs_change = xfer->cs_change;

		device->transfer_data(device, xfer);

		if (cs_change && !list_is_last(&xfer->transfer_list, &msg->transfers)) {
			tca9535_spi_set_cs(device, false);
		}

		msg->actual_length += xfer->len;
	}

	if (!cs_change) {
		tca9535_spi_set_cs(device, false);
	}

	msg->status = 0;
	spi_finalize_current_message(master);

	return 0;
}

static int tca9535_spi_init(struct tca9535_device *tca9535)
{
	int ret;
	int index = 0;
	struct device_node *child;
	u32 sck_pin, mosi_pin, miso_pin;
	struct i2c_client *client = tca9535->client;
	struct device_node *np = client->dev.of_node;

	for_each_child_of_node(np, child) {
		int i;
		u32 reg;
		const __be32 *cs_values;
		struct property *cs_prop;
		u32 chipselects, cs_size;
		struct spi_master *master;
		struct tca9535_spi_master *tca9535_master;
		u16 configuration = tca9535->cache.configuration;

		if (strcmp(child->name, "spi")) {
			continue;
		}

		ret = of_property_read_u32(child, "reg", &reg);
		if (ret < 0) {
			dev_err(&client->dev, "spi[%d], no property reg found!\n", index);
			continue;
		}

		ret = of_property_read_u32(child, "sck-pin", &sck_pin);
		if (ret < 0) {
			dev_err(&client->dev, "No property sck-pin found!\n");
			continue;
		}

		ret = of_property_read_u32(child, "mosi-pin", &mosi_pin);
		if (ret < 0) {
			dev_err(&client->dev, "No property mosi-pin not found!\n");
			continue;
		}

		ret = of_property_read_u32(child, "miso-pin", &miso_pin);
		if (ret < 0) {
			dev_err(&client->dev, "No property miso-pin not found!\n");
			continue;
		}

		cs_prop = of_find_property(child, "cs-pins", NULL);
		if (cs_prop == NULL) {
			dev_err(&client->dev, "spi[%d], no property cs-pins found!\n", index);
			continue;
		}

		cs_size = cs_prop->length / sizeof(u32);

		ret = of_property_read_u32(child, "num-chipselects", &chipselects);
		if (ret < 0) {
			chipselects = cs_size;
		} else if (chipselects > cs_size) {
			dev_err(&client->dev, "Too much chipselects: chipselects = %d, cs_prop_size = %d\n", chipselects, cs_size);
			continue;
		}

		dev_info(&client->dev, "reg = %d, chipselects = %d", reg, chipselects);
		dev_info(&client->dev, "sck_pin = %d, mosi_pin = %d, miso_pin = %d\n", sck_pin, mosi_pin, miso_pin);

		if (chipselects == 0 || cs_prop->value == NULL) {
			continue;
		}

		master = spi_alloc_master(&client->dev, sizeof(struct tca9535_spi_master) + sizeof(struct tca9535_spi_device) * chipselects);
		if (master == NULL) {
			dev_err(&client->dev, "Failed to spi_alloc_master\n");
			continue;
		}

		tca9535_master = spi_master_get_devdata(master);
		tca9535_master->master = spi_master_get(master);
		tca9535_master->tca9535 = tca9535;
		tca9535_master->index = index;

		tca9535_master->sck_mask = 1 << sck_pin;
		tca9535_master->mosi_mask = 1 << mosi_pin;
		configuration &= ~(tca9535_master->sck_mask | tca9535_master->mosi_mask);
		tca9535_part_write_init(&tca9535_master->wr_config, REG_OUTPUT_PORT, tca9535_master->sck_mask | tca9535_master->mosi_mask);

		dev_info(&client->dev, "sck_mask = 0x%04x, mosi_mask = 0x%04x\n", tca9535_master->sck_mask, tca9535_master->mosi_mask);

		if (miso_pin < 8) {
			tca9535_master->rd_addr = REG_INPUT_PORT;
			tca9535_master->miso_mask = 1 << miso_pin;
		} else {
			tca9535_master->rd_addr = REG_INPUT_PORT + 1;
			tca9535_master->miso_mask = 1 << (miso_pin - 8);
		}

		configuration |= 1 << miso_pin;

		gpio_request(tca9535->gpio_chip.base + sck_pin, "tca9535-spi-sck");
		gpio_request(tca9535->gpio_chip.base + mosi_pin, "tca9535-spi-mosi");
		gpio_request(tca9535->gpio_chip.base + miso_pin, "tca9535-spi-miso");

		dev_info(&client->dev, "miso_mask = 0x%04x, rd_addr = 0x%02x\n", tca9535_master->miso_mask, tca9535_master->rd_addr);

		cs_values = cs_prop->value;

		for (i = 0; i < cs_size; i++) {
			struct tca9535_spi_device *device;
			int cs_pin = be32_to_cpup(cs_values + i);

			if (i < chipselects) {
				device = tca9535_master->devices + i;
			} else {
				device = tca9535_master->devices + chipselects - 1;
			}

			device->cs_mask |= 1 << cs_pin;
			configuration &= ~(device->cs_mask);
			tca9535_part_write_init(&device->wr_config, REG_OUTPUT_PORT, device->cs_mask | tca9535_master->sck_mask);

			device->master = tca9535_master;
			gpio_request(tca9535->gpio_chip.base + cs_pin, "tca9535-spi-cs");

			dev_info(&client->dev, "%d. cs = %d, mask = 0x%04x\n", i, cs_pin, device->cs_mask);
		}

		ret = tca9535_write_u16(tca9535, REG_CONFIGURATION, configuration);
		if (ret < 0) {
			dev_err(&client->dev, "Failed to tca9535_write_u16 REG_CONFIGURATION: %d\n", ret);
			goto label_kfree_master;
		}

		master->dev.of_node = child;
		master->flags = 0;
		master->mode_bits = SPI_CPOL | SPI_CPHA | SPI_LSB_FIRST;
		master->bus_num = -1;
		master->num_chipselect = chipselects;
		master->setup = tca9535_spi_setup;
		master->cleanup = tca9535_spi_cleanup;
		master->transfer_one_message = tca9535_spi_transfer_one_message;

		ret = spi_register_master(master);
		if (ret < 0) {
			dev_err(&client->dev, "Failed to spi_register_master: %d\n", ret);
			goto label_kfree_master;
		}

		tca9535_master->next = tca9535->spi_head;
		tca9535->spi_head = tca9535_master;
		index++;
		continue;

label_kfree_master:
		kfree(master);
	}

	dev_info(&client->dev, "spi_master_count = %d\n", index);

	return 0;
}

static void tca9535_spi_deinit(struct tca9535_device *tca9535)
{
	struct tca9535_spi_master *spi = tca9535->spi_head;

	while (spi) {
		struct spi_master *master = spi->master;
		struct tca9535_spi_master *next = spi->next;

		spi_unregister_master(master);
		kfree(master);
		spi = next;
	}
}

// ============================================================

static int tca9535_voltage_set_voltage(struct regulator_dev *rdev, int min_uV, int max_uV, unsigned *selector)
{
	return 0;
}

static int tca9535_voltage_get_voltage(struct regulator_dev *rdev)
{
	return 0;
}

static int tca9535_voltage_main_enable(struct regulator_dev *rdev)
{
	int ret;
	struct tca9535_device *tca9535 = rdev_get_drvdata(rdev);

	ret = tca9535_set_enable(tca9535, true);
	if (ret < 0) {
		dev_err(tca9535->dev, "Failed to tca9535_set_enable: %d\n", ret);
		return ret;
	}

	return 0;
}

static int tca9535_voltage_main_disable(struct regulator_dev *rdev)
{
	int ret = 0;
	struct tca9535_device *tca9535 = rdev_get_drvdata(rdev);

	tca9535_power_lock(tca9535);

	if (tca9535->suspend) {
		ret = tca9535_set_enable_locked(tca9535, false);
	}

	tca9535_power_unlock(tca9535);

	return ret;
}

static int tca9535_voltage_child_enable(struct regulator_dev *rdev)
{
	int ret;
	struct tca9535_regulator *regulator = rdev_get_drvdata(rdev);
	struct tca9535_device *tca9535 = regulator->tca9535;
	u16 value = tca9535_set_bit(tca9535->cache.output_port, regulator->mask, !regulator->active_low);

	ret = tca9535_set_enable(tca9535, true);
	if (ret < 0) {
		dev_err(&rdev->dev, "Failed to tca9535_set_enable: %d\n", ret);
		return ret;
	}

	ret = tca9535_write_u16(tca9535, REG_OUTPUT_PORT, value);
	if (ret < 0) {
		dev_err(&rdev->dev, "Failed to tca9535_write_u16 REG_OUTPUT_PORT\n");
		return ret;
	}

	regulator->enabled = true;

	return 0;
}

static int tca9535_voltage_child_disable(struct regulator_dev *rdev)
{
	int ret;
	struct tca9535_regulator *regulator = rdev_get_drvdata(rdev);
	struct tca9535_device *tca9535 = regulator->tca9535;
	u16 value = tca9535_set_bit(tca9535->cache.output_port, regulator->mask, regulator->active_low);

	ret = tca9535_write_u16(tca9535, REG_OUTPUT_PORT, value);
	if (ret < 0) {
		dev_err(&rdev->dev, "Failed to tca9535_write_u16 REG_OUTPUT_PORT\n");
		return ret;
	}

	regulator->enabled = false;

	return 0;
}

static int tca9535_voltage_child_is_enabled(struct regulator_dev *rdev)
{
	struct tca9535_regulator *regulator = rdev_get_drvdata(rdev);

	return regulator->enabled;
}

static int tca9535_voltage_main_is_enabled(struct regulator_dev *rdev)
{
	struct tca9535_device *tca9535 = rdev_get_drvdata(rdev);

	return tca9535->enabled;
}

static struct regulator_ops tca9535_voltage_main_ops = {
	.set_voltage = tca9535_voltage_set_voltage,
	.get_voltage = tca9535_voltage_get_voltage,
	.enable = tca9535_voltage_main_enable,
	.disable = tca9535_voltage_main_disable,
	.is_enabled = tca9535_voltage_main_is_enabled,
};

static struct regulator_ops tca9535_voltage_child_ops = {
	.set_voltage = tca9535_voltage_set_voltage,
	.get_voltage = tca9535_voltage_get_voltage,
	.enable = tca9535_voltage_child_enable,
	.disable = tca9535_voltage_child_disable,
	.is_enabled = tca9535_voltage_child_is_enabled,
};

static int tca9535_regulator_init(struct tca9535_device *tca9535)
{
	int ret;
	struct regulator_desc *desc;
	struct regulator_config config;
	struct device_node *child_node;
	const struct regulator_init_data *init_data;
	u16 output_port = tca9535->cache.output_port;
	struct device_node *node = tca9535->dev->of_node;
	u16 configuration = tca9535->cache.configuration;
	struct tca9535_regulator *regulator, *regulator_next;

	memset(&config, 0x00, sizeof(config));
	config.dev = tca9535->dev;

	init_data = of_get_regulator_init_data(tca9535->dev, node);
	if (init_data == NULL) {
		dev_err(tca9535->dev, "Failed to of_get_regulator_init_data\n");
		return -ENOMEM;
	}

	config.dev = tca9535->dev;
	config.init_data = init_data;
	config.driver_data = tca9535;
	config.of_node = node;

	desc = &tca9535->rdesc;
	desc->name = init_data->constraints.name;
	desc->type = REGULATOR_VOLTAGE;
	desc->owner = THIS_MODULE;
	desc->ops = &tca9535_voltage_main_ops;

	tca9535->rdev = regulator_register(desc, &config);
	if (tca9535->rdev == NULL) {
		dev_err(tca9535->dev, "Failed to regulator_register\n");
		return -EFAULT;
	}

	for_each_child_of_node(node, child_node) {
		int index;

		if (strcmp(child_node->name, "regulator")) {
			continue;
		}

		if (of_property_read_u32(child_node, "reg", &index) < 0) {
			continue;
		}

		if (index >= TCA9535_GPIO_COUNT) {
			dev_err(tca9535->dev, "Invalid index = %d\n", index);
			continue;
		}

		init_data = of_get_regulator_init_data(tca9535->dev, child_node);
		if (init_data == NULL) {
			dev_err(tca9535->dev, "Failed to of_get_regulator_init_data\n");
			goto out_regulator_unregister;
		}

		regulator = devm_kzalloc(tca9535->dev, sizeof(struct tca9535_regulator), GFP_KERNEL);
		if (regulator == NULL) {
			dev_err(tca9535->dev, "Failed to devm_kzalloc\n");
			goto out_regulator_unregister;
		}

		if (of_find_property(child_node, "enable-active-high", NULL) == NULL) {
			regulator->active_low = true;
		}

		regulator->tca9535 = tca9535;
		regulator->mask = (1 << index);

		config.init_data = init_data;
		config.driver_data = regulator;
		config.of_node = child_node;
		config.dev->of_node = child_node;

		desc = &regulator->rdesc;
		desc->name = init_data->constraints.name;
		desc->type = REGULATOR_VOLTAGE;
		desc->owner = THIS_MODULE;
		desc->ops = &tca9535_voltage_child_ops;

		if (of_find_property(child_node, "vin-supply", NULL)) {
			desc->supply_name = "vin";
		}

		regulator->rdev = regulator_register(&regulator->rdesc, &config);
		if (regulator->rdev == NULL) {
			dev_err(tca9535->dev, "Failed to regulator_register\n");
			devm_kfree(tca9535->dev, regulator);
			goto out_regulator_unregister;
		}

		if (init_data->constraints.boot_on) {
			output_port = tca9535_set_bit(output_port, regulator->mask, !regulator->active_low);
		}

		configuration &= ~regulator->mask;
		gpio_request(tca9535->gpio_chip.base + index, init_data->constraints.name);

		regulator->next = tca9535->regulator_head;
		tca9535->regulator_head = regulator;
	}

	tca9535->dev->of_node = node;

	ret = tca9535_write_u16(tca9535, REG_OUTPUT_PORT, output_port);
	if (ret < 0) {
		dev_err(tca9535->dev, "Failed to tca9535_write_u16 REG_OUTPUT_PORT: %d\n", ret);
		goto out_regulator_unregister;
	}

	ret = tca9535_write_u16(tca9535, REG_CONFIGURATION, configuration);
	if (ret < 0) {
		dev_err(tca9535->dev, "Failed to tca9535_write_u16 REG_CONFIGURATION: %d\n", ret);
		goto out_regulator_unregister;
	}

	return 0;

out_regulator_unregister:
	for (regulator = tca9535->regulator_head; regulator; regulator = regulator_next) {
		regulator_next = regulator->next;
		regulator_unregister(regulator->rdev);
		devm_kfree(tca9535->dev, regulator);
	}

	tca9535->regulator_head = NULL;

	regulator_unregister(tca9535->rdev);

	return ret;
}

static void tca9535_regulator_deinit(struct tca9535_device *tca9535)
{
	struct tca9535_regulator *regulator, *regulator_next;

	for (regulator = tca9535->regulator_head; regulator; regulator = regulator_next) {
		regulator_next = regulator->next;
		regulator_unregister(regulator->rdev);
		devm_kfree(tca9535->dev, regulator);
	}

	regulator_unregister(tca9535->rdev);
}

// ============================================================

static int tca9535_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	enum of_gpio_flags flags;
	struct gpio_chip *gpio_chip;
	struct tca9535_device *tca9535;

	tca9535 = devm_kzalloc(&client->dev, sizeof(struct tca9535_device), GFP_KERNEL);
	if (tca9535 == NULL) {
		dev_err(&client->dev, "kzalloc failed\n");
		return -ENOMEM;
	}

	tca9535->client = client;
	tca9535->dev = &client->dev;
	i2c_set_clientdata(client, tca9535);

	tca9535_lock_init(tca9535);
	tca9535->first_poweron = true;

	tca9535->gpio_irq = of_get_named_gpio(client->dev.of_node, "gpio-irq", 0);
	if (gpio_is_valid(tca9535->gpio_irq)) {
		ret = gpio_request(tca9535->gpio_irq, "TCA9535-IRQ");
		if (ret < 0) {
			dev_err(&client->dev, "Failed to gpio_request %d: %d", tca9535->gpio_irq, ret);
			goto out_devm_kfree;
		}

		gpio_direction_input(tca9535->gpio_irq);
		tca9535->irq = gpio_to_irq(tca9535->gpio_irq);
	} else {
		tca9535->irq = -1;
	}

	tca9535->gpio_pwr = of_get_named_gpio_flags(client->dev.of_node, "gpio-pwr", 0, &flags);
	if (gpio_is_valid(tca9535->gpio_pwr)) {
		tca9535->gpio_pwr_one = !(flags & OF_GPIO_ACTIVE_LOW);
		gpio_request(tca9535->gpio_pwr, "TCA9535-PWR");
	}

	dev_info(&client->dev, "gpio_irq = %d\n", tca9535->gpio_irq);
	dev_info(&client->dev, "irq = %d\n", tca9535->irq);

	tca9535->pinctrl = devm_pinctrl_get(&client->dev);
	if (IS_ERR(tca9535->pinctrl)) {
		tca9535->pinctrl = NULL;
	} else {
		tca9535->gpio_state = pinctrl_lookup_state(tca9535->pinctrl, "gpio");
		if (IS_ERR(tca9535->gpio_state)) {
			tca9535->gpio_state = NULL;
		}
	}

	dev_info(&client->dev, "pinctrl = %p, gpio_state = %p\n", tca9535->pinctrl, tca9535->gpio_state);

	if (tca9535->pinctrl && tca9535->gpio_state) {
		pinctrl_select_state(tca9535->pinctrl, tca9535->gpio_state);
	}

	dev_info(&client->dev, "gpio_pwr = %d\n", tca9535->gpio_pwr);

	tca9535->vcc = regulator_get(&client->dev, "vcc");
	if (IS_ERR(tca9535->vcc)) {
		dev_err(&client->dev, "Failed to regulator_get vcc\n");
		tca9535->vcc = NULL;
	}

	tca9535->vbus = regulator_get(&client->dev, "i2c");
	if (IS_ERR(tca9535->vbus)) {
		dev_err(&client->dev, "Failed to regulator_get i2c\n");
		tca9535->vbus = NULL;
	}

	ret = tca9535_set_enable(tca9535, true);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to tca9535_set_enable: %d\n", ret);
		goto out_regulator_put;
	}

	gpio_chip = &tca9535->gpio_chip;
	gpio_chip->dev = &client->dev;
	gpio_chip->label = client->name;
	gpio_chip->owner = THIS_MODULE;
	gpio_chip->can_sleep = 1;
	gpio_chip->base = -1;
	gpio_chip->ngpio = TCA9535_GPIO_COUNT;
	gpio_chip->direction_input  = tca9535_gpio_direction_input;
	gpio_chip->direction_output = tca9535_gpio_direction_output;
	gpio_chip->get = tca9535_gpio_get;
	gpio_chip->set = tca9535_gpio_set;
	gpio_chip->to_irq = tca9535_gpio_to_irq;

	ret = gpiochip_add(gpio_chip);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to gpiochip_add: %d\n", ret);
		goto out_tca9535_power_down;
	}

	ret = tca9535_regulator_init(tca9535);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to tca9535_regulator_init: %d\n", ret);
		goto out_gpiochip_remove;
	}

	ret = tca9535_keypad_init(tca9535);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to tca9535_keypad_init: %d\n", ret);
		goto out_tca9535_regulator_deinit;
	}

	ret = tca9535_irq_init(tca9535, id, 0);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to tca9535_irq_init: %d\n", ret);
		goto out_tca9535_keypad_deinit;
	}

	ret = tca9535_i2c_mux_init(tca9535);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to tca9535_i2c_mux_init: %d\n", ret);
		goto out_tca9535_irq_deinit;
	}

	ret = tca9535_spi_init(tca9535);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to tca9535_spi_init: %d\n", ret);
		goto out_tca9535_i2c_mux_deinit;
	}


	if (tca9535->irq >= 0) {
		unsigned long irqflags;

		init_completion(&tca9535->isr_completion);

		if (of_find_property(client->dev.of_node, "irq-trigger-edge", NULL)) {
			irqflags = IRQF_TRIGGER_FALLING;
		} else {
			irqflags = IRQF_TRIGGER_LOW;
		}

		ret = devm_request_irq(&client->dev, tca9535->irq, tca9535_isr_handler, irqflags, "tca9535", tca9535);
		if (ret < 0) {
			dev_err(&client->dev, "Failed to devm_request_threaded_irq: %d\n", ret);
			goto out_tca9535_spi_deinit;
		}

		disable_irq(tca9535->irq);
		kthread_run(tca9535_isr_thread, tca9535, "tca9535-isr");
	}

	return 0;

out_tca9535_spi_deinit:
	tca9535_spi_deinit(tca9535);
out_tca9535_i2c_mux_deinit:
	tca9535_i2c_mux_deinit(tca9535);
out_tca9535_irq_deinit:
	tca9535_irq_deinit(tca9535);
out_tca9535_keypad_deinit:
	tca9535_keypad_deinit(tca9535);
out_tca9535_regulator_deinit:
	tca9535_regulator_deinit(tca9535);
out_gpiochip_remove:
	if (gpiochip_remove(gpio_chip) < 0) {
		dev_warn(&client->dev, "Failed to gpiochip_remove\n");
	}
out_tca9535_power_down:
	tca9535_set_enable(tca9535, false);
out_regulator_put:
	if (tca9535->vbus) {
		regulator_put(tca9535->vbus);
	}

	if (tca9535->vcc) {
		regulator_put(tca9535->vcc);
	}

	if (gpio_is_valid(tca9535->gpio_pwr)) {
		gpio_free(tca9535->gpio_pwr);
	}

	if (gpio_is_valid(tca9535->gpio_irq)) {
		gpio_free(tca9535->gpio_irq);
	}
out_devm_kfree:
	tca9535_lock_deinit(tca9535);
	devm_kfree(&client->dev, tca9535);
	return ret;
}
static int tca9535_i2c_remove(struct i2c_client *client)
{
	int ret;
	struct tca9535_device *tca9535 = i2c_get_clientdata(client);

	if (tca9535->irq >= 0) {
		devm_free_irq(&client->dev, tca9535->irq, tca9535);
	}

	tca9535_spi_deinit(tca9535);
	tca9535_i2c_mux_deinit(tca9535);
	tca9535_irq_deinit(tca9535);
	tca9535_keypad_deinit(tca9535);
	tca9535_regulator_deinit(tca9535);

	ret = gpiochip_remove(&tca9535->gpio_chip);
	if (ret < 0) {
		dev_warn(&client->dev, "Failed to gpiochip_remove: %d\n", ret);
	}

	tca9535_set_enable(tca9535, false);

	if (tca9535->vbus) {
		regulator_put(tca9535->vbus);
	}

	if (tca9535->vcc) {
		regulator_put(tca9535->vcc);
	}

	if (gpio_is_valid(tca9535->gpio_pwr)) {
		gpio_free(tca9535->gpio_pwr);
	}

	if (gpio_is_valid(tca9535->gpio_irq)) {
		gpio_free(tca9535->gpio_irq);
	}

	tca9535_lock_deinit(tca9535);
	devm_kfree(&client->dev, tca9535);

	return 0;
}

static void tca9535_i2c_shutdown(struct i2c_client *client)
{
	struct tca9535_device *tca9535 = i2c_get_clientdata(client);

	tca9535_pr_pos_info();

	tca9535_set_enable(tca9535, false);
}

static int tca9535_i2c_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct tca9535_device *tca9535 = i2c_get_clientdata(client);

	return tca9535_set_suspend(tca9535, true);
}

static int tca9535_i2c_resume(struct i2c_client *client)
{
	struct tca9535_device *tca9535 = i2c_get_clientdata(client);

	return tca9535_set_suspend(tca9535, false);
}

static const struct i2c_device_id tca9535_i2c_id[] = {
	{ "tca9535", 0 },
	{ "tca9535-main", 0 },
	{ "tca9535-disp", 0 },
	{ "tca9535-left", 0 },
	{ "tca9535-right", 0 },
	{ "tca9535-handset", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, tca9535_i2c_id);

static struct i2c_driver tca9535_i2c_driver = {
	.driver = {
		.name = "tca9535",
		.owner = THIS_MODULE,
	},
	.probe = tca9535_i2c_probe,
	.remove   = tca9535_i2c_remove,
	.shutdown = tca9535_i2c_shutdown,
	.suspend = tca9535_i2c_suspend,
	.resume = tca9535_i2c_resume,
	.id_table = tca9535_i2c_id,
};

static int __init tca9535_module_init(void)
{
	tca9535_pr_pos_info();

	return i2c_add_driver(&tca9535_i2c_driver);
}

static void __exit tca9535_module_exit(void)
{
	tca9535_pr_pos_info();

	i2c_del_driver(&tca9535_i2c_driver);
}

subsys_initcall(tca9535_module_init);
module_exit(tca9535_module_exit);

MODULE_DESCRIPTION("TCA9535 GPIO Driver");
MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_LICENSE("GPL");
