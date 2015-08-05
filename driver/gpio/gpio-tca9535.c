/*
 * Copyright (C) 2015 JWAOO, Inc.
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

#define TCA9535_DEBUG			0

#define TCA9535_I2C_RATE		(100 * 1000)
#define TCA9535_GPIO_COUNT		16

#define tca9535_pr_info(fmt, args ...) \
	 pr_err("%s[%d]: " fmt "\n", __FUNCTION__, __LINE__, ##args)

#define tca9535_pr_pos_info() \
	 pr_err("%s => %s[%d]\n", __FILE__, __FUNCTION__, __LINE__)

enum tca9535_register
{
	REG_INPUT_PORT = 0x00,
	REG_OUTPUT_PORT = 0x02,
	REG_POLARITY_INVERSION = 0x04,
	REG_CONFIGURATION = 0x06,
};

struct tca9535_register_cache
{
	u16 input_port;
	u16 output_port;
	u16 polarity_inversion;
	u16 configuration;
};

struct tca9535_device
{
	struct gpio_chip gpio_chip;
	struct i2c_client *client;
	struct mutex lock;
	struct tca9535_register_cache cache;

	int gpio_irq;
	struct pinctrl *pinctrl;
	struct pinctrl_state *gpio_state;

	u16 key_mask;
	struct input_dev *input;
	int keys[TCA9535_GPIO_COUNT];

	struct mutex irq_lock;
	struct irq_domain *domain;
	u16 irq_mask;
	u16 irq_trig_raise;
	u16 irq_trig_fall;
};

static int tca9535_read_data(struct tca9535_device *tca9535, u8 addr, void *buff, size_t size, bool cache)
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

	mutex_lock(&tca9535->lock);

	ret = i2c_transfer(client->adapter, msgs, ARRAY_SIZE(msgs));
	if (unlikely(ret != ARRAY_SIZE(msgs))) {
		dev_err(&client->dev, "Failed to i2c_transfer: %d\n", ret);
		if (ret >= 0) {
			ret = -EFAULT;
		}
	} else if (cache) {
		memcpy(((u8 *) &tca9535->cache) + addr, buff, size);
	}

	mutex_unlock(&tca9535->lock);

	return ret;
}

static int tca9535_write_data(struct tca9535_device *tca9535, u8 addr, const void *buff, size_t size, bool cache)
{
	int ret;
	u8 data_buff[size + 1];
	struct i2c_client *client = tca9535->client;
	struct i2c_msg msg = {
		.addr = client->addr,
		.flags = (client->flags & I2C_M_TEN),
		.len = sizeof(data_buff),
		.buf = (__u8 *) data_buff,
#ifdef CONFIG_I2C_ROCKCHIP_COMPAT
		.scl_rate = TCA9535_I2C_RATE,
#endif
	};

	data_buff[0] = addr;
	memcpy(data_buff + 1, buff, size);

	mutex_lock(&tca9535->lock);

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (unlikely(ret != 1)) {
		dev_err(&client->dev, "Failed to i2c_transfer: %d\n", ret);
		if (ret >= 0) {
			ret = -EFAULT;
		}
	} else if (cache) {
		memcpy(((u8 *) &tca9535->cache) + addr, buff, size);
	}

	mutex_unlock(&tca9535->lock);

	return ret;
}

static inline int tca9535_read_register(struct tca9535_device *tca9535, u8 addr, u16 *value, bool cache)
{
	return tca9535_read_data(tca9535, addr, value, 2, cache);
}

static inline int tca9535_write_register(struct tca9535_device *tca9535, u8 addr, u16 value, bool cache)
{
#if TCA9535_DEBUG
	dev_info(&tca9535->client->dev, "write: addr = 0x%02x, value = 0x%04x\n", addr, value);
#endif

	return tca9535_write_data(tca9535, addr, &value, sizeof(value), cache);
}

// ============================================================

static int tca9535_init_register(struct tca9535_device *tca9535)
{
	int ret = 0;

	ret |= tca9535_write_register(tca9535, REG_OUTPUT_PORT, 0x0000, true);
	ret |= tca9535_write_register(tca9535, REG_POLARITY_INVERSION, 0x0000, true);
	ret |= tca9535_write_register(tca9535, REG_CONFIGURATION, 0xFFFF, true);
	ret |= tca9535_read_register(tca9535, REG_INPUT_PORT, &tca9535->cache.input_port, false);
	if (ret < 0) {
		return ret;
	}

	dev_info(&tca9535->client->dev, "REG_INPUT_PORT = 0x%04x\n", tca9535->cache.input_port);
	dev_info(&tca9535->client->dev, "REG_OUTPUT_PORT = 0x%04x\n", tca9535->cache.output_port);
	dev_info(&tca9535->client->dev, "REG_POLARITY_INVERSION = 0x%04x\n", tca9535->cache.polarity_inversion);
	dev_info(&tca9535->client->dev, "REG_CONFIGURATION = 0x%04x\n", tca9535->cache.configuration);

	return 0;
}

// ============================================================

static int tca9535_gpio_direction_input(struct gpio_chip *chip, unsigned offset)
{
	int ret;
	struct tca9535_device *tca9535 = container_of(chip, struct tca9535_device, gpio_chip);
	struct tca9535_register_cache *cache = &tca9535->cache;

#if TCA9535_DEBUG
	dev_info(&tca9535->client->dev, "%s: offset = %d\n", __FUNCTION__, offset);
#endif

	cache->configuration |= 1 << offset;

	ret = tca9535_write_register(tca9535, REG_CONFIGURATION, cache->configuration, false);
	if (ret < 0) {
		dev_err(&tca9535->client->dev, "Failed to tca9535_write_register: %d\n", ret);
		return ret;
	}

	return 0;
}

static int tca9535_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	int ret;
	u16 value;
	struct tca9535_device *tca9535 = container_of(chip, struct tca9535_device, gpio_chip);

#if TCA9535_DEBUG
	dev_info(&tca9535->client->dev, "%s: offset = %d\n", __FUNCTION__, offset);
#endif

	ret = tca9535_read_register(tca9535, REG_INPUT_PORT, &value, true);
	if (ret < 0) {
		dev_err(&tca9535->client->dev, "Failed to tca9535_read_register: %d\n", ret);
		value = tca9535->cache.input_port;
	}

	return (value & (1 << offset)) != 0;
}

static int tca9535_gpio_direction_output(struct gpio_chip *chip, unsigned offset, int value)
{
	int ret = 0;
	struct tca9535_device *tca9535 = container_of(chip, struct tca9535_device, gpio_chip);
	struct tca9535_register_cache *cache = &tca9535->cache;

#if TCA9535_DEBUG
	dev_info(&tca9535->client->dev, "%s: offset = %d, value = %d\n", __FUNCTION__, offset, value);
#endif

	cache->output_port = (cache->output_port & (~(1 << offset))) | (!!value) << 1;
	ret |= tca9535_write_register(tca9535, REG_OUTPUT_PORT, cache->output_port, false);
	cache->configuration &= ~(1 << offset);
	ret |= tca9535_write_register(tca9535, REG_CONFIGURATION, cache->configuration, false);
	if (ret < 0) {
		dev_err(&tca9535->client->dev, "Failed to tca9535_write_register: %d\n", ret);
		return ret;
	}

	return 0;
}

static void tca9535_gpio_set(struct gpio_chip *chip, unsigned offset, int value)
{
	struct tca9535_device *tca9535 = container_of(chip, struct tca9535_device, gpio_chip);
	struct tca9535_register_cache *cache = &tca9535->cache;

#if TCA9535_DEBUG
	dev_info(&tca9535->client->dev, "%s: offset = %d, value = %d\n", __FUNCTION__, offset, value);
#endif

	cache->output_port = (cache->output_port & (~(1 << offset))) | (!!value) << 1;

	tca9535_write_register(tca9535, REG_OUTPUT_PORT, cache->output_port, false);
}

static int tca9535_gpio_to_irq(struct gpio_chip *chip, unsigned offset)
{
	struct tca9535_device *tca9535 = container_of(chip, struct tca9535_device, gpio_chip);

#if TCA9535_DEBUG
	dev_info(&tca9535->client->dev, "%s: offset = %d\n", __FUNCTION__, offset);
#endif

	return irq_create_mapping(tca9535->domain, offset);
}

// ============================================================

static int tca9535_keypad_init(struct tca9535_device *tca9535)
{
	int ret;
	struct input_dev *input;
#ifdef CONFIG_OF
	struct device_node *child_node;
	struct device_node *node = tca9535->client->dev.of_node;
#endif

	tca9535->input = input_allocate_device();
	if (tca9535->input == NULL) {
		dev_err(&tca9535->client->dev, "Failed to input_allocate_device");
		return -ENOMEM;
	}

	input = tca9535->input;
	input->name = "tca9535-keypad";

#ifdef CONFIG_OF
	for_each_child_of_node(node, child_node) {
		u16 mask;
		u32 index, code;

		if (of_property_read_u32(child_node, "index", &index) < 0 || of_property_read_u32(child_node, "code", &code) < 0) {
			continue;
		}

		if (index >= tca9535->gpio_chip.ngpio) {
			dev_err(&tca9535->client->dev, "Invalid index = %d\n", index);
			continue;
		}

		tca9535->keys[index] = code;
		input_set_capability(input, EV_KEY, code);

		mask = 1 << index;
		tca9535->cache.configuration |= mask;
		tca9535->key_mask |= mask;

		dev_info(&tca9535->client->dev, "mask = 0x%02x, index = %d, code = %d\n", mask, index, code);
	}
#endif

	ret = tca9535_write_register(tca9535, REG_CONFIGURATION, tca9535->cache.configuration, false);
	if (ret < 0) {
		dev_err(&tca9535->client->dev, "Failed to tca9535_write_register: %d\n", ret);
		goto out_input_free_device;
	}

	ret = input_register_device(input);
	if (ret < 0) {
		dev_err(&tca9535->client->dev, "Failed to input_register_device: %d\n", ret);
		goto out_input_free_device;
	}

	return 0;

out_input_free_device:
	input_free_device(tca9535->input);
	tca9535->input = NULL;
	return ret;
}

static void tca9535_keypad_deinit(struct tca9535_device *tca9535)
{
	input_unregister_device(tca9535->input);
	input_free_device(tca9535->input);
}

static void tca9535_report_keys(struct tca9535_device *tca9535, u16 mask)
{
	int i;
	u16 value;

	if (mask == 0) {
		return;
	}

	value = tca9535->cache.input_port;

	for (i = 0; mask; mask >>= 1, i++) {
		if (mask & 1) {
			input_report_key(tca9535->input, tca9535->keys[i], !(value & (1 << i)));
		}
	}

	input_sync(tca9535->input);
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
	struct tca9535_register_cache *cache = &tca9535->cache;

#if TCA9535_DEBUG
	tca9535_pr_pos_info();
#endif

	value = cache->configuration;
	cache->configuration |= tca9535->irq_trig_fall | tca9535->irq_trig_raise;
	if (value != cache->configuration) {
		tca9535_write_register(tca9535, REG_CONFIGURATION, cache->configuration, false);
	}

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
		dev_err(&tca9535->client->dev, "irq %d: unsupported type %d\n", d->irq, type);
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
	mutex_init(&tca9535->irq_lock);

	tca9535->domain = irq_domain_add_simple(tca9535->client->dev.of_node, tca9535->gpio_chip.ngpio, irq_base, &tca9535_irq_simple_ops, tca9535);
	if (tca9535->domain == NULL) {
		dev_err(&tca9535->client->dev, "Failed to irq_domain_add_simple\n");
		return -ENODEV;
	}

	return 0;
}

static void tca9535_irq_deinit(struct tca9535_device *tca9535)
{
	irq_domain_remove(tca9535->domain);
	mutex_destroy(&tca9535->irq_lock);
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

static irqreturn_t tca9535_irq_handler(int irq, void *dev_id)
{
	int i;
	u16 mask;
	u16 value_old, value;
	struct tca9535_device *tca9535 = dev_id;

#if TCA9535_DEBUG
	tca9535_pr_pos_info();
#endif

	value_old = tca9535->cache.input_port;

	for (i = 10; tca9535_read_register(tca9535, REG_INPUT_PORT, &value, true) < 0; i--) {
		if (i < 1) {
			dev_err(&tca9535->client->dev, "Failed to tca9535_read_register\n");
			return IRQ_HANDLED;
		}

		msleep(1);
	}

#if TCA9535_DEBUG
	dev_info(&tca9535->client->dev, "REG_INPUT_PORT = 0x%04x => 0x%04x\n", value_old, value);
#endif

	mask = (value_old ^ value) & tca9535->cache.configuration;

#if TCA9535_DEBUG
	dev_info(&tca9535->client->dev, "mask = 0x%04x\n", mask);
#endif

	if (mask) {
		tca9535_report_keys(tca9535, mask & tca9535->key_mask);
		tca9535_handle_nested_irq(tca9535, mask & tca9535->irq_mask & ((tca9535->irq_trig_raise & value) | (tca9535->irq_trig_fall & value_old)));
	}

	return IRQ_HANDLED;
}

// ============================================================

static int tca9535_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int ret;
	struct gpio_chip *gpio_chip;
	struct tca9535_device *tca9535;

	tca9535 = devm_kzalloc(&client->dev, sizeof(struct tca9535_device), GFP_KERNEL);
	if (tca9535 == NULL) {
		dev_err(&client->dev, "kzalloc failed\n");
		return -ENOMEM;
	}

	tca9535->client = client;
	i2c_set_clientdata(client, tca9535);

	mutex_init(&tca9535->lock);

	ret = tca9535_init_register(tca9535);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to tca9535_init_register: %d\n", ret);
		goto out_devm_kfree;
	}

	tca9535->gpio_irq = of_get_gpio(client->dev.of_node, 0);
	if (gpio_is_valid(tca9535->gpio_irq)) {
		ret = gpio_request(tca9535->gpio_irq, "TCA9535-IRQ");
		if (ret < 0) {
			dev_err(&client->dev, "Failed to gpio_request %d: %d", tca9535->gpio_irq, ret);
			goto out_devm_kfree;
		}

		gpio_direction_input(tca9535->gpio_irq);

		client->irq = gpio_to_irq(tca9535->gpio_irq);
	} else {
		client->irq = -1;
	}

	dev_info(&client->dev, "gpio_irq = %d\n", tca9535->gpio_irq);
	dev_info(&client->dev, "irq = %d\n", client->irq);

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
		goto out_gpio_free;
	}

	ret = tca9535_keypad_init(tca9535);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to tca9535_keypad_init: %d\n", ret);
		goto out_gpiochip_remove;
	}

	ret = tca9535_irq_init(tca9535, id, 0);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to tca9535_irq_init: %d\n", ret);
		goto out_tca9535_keypad_deinit;
	}

	if (client->irq >= 0) {
		ret = devm_request_threaded_irq(&client->dev, client->irq, NULL, tca9535_irq_handler, IRQF_TRIGGER_FALLING | IRQF_ONESHOT, dev_name(&client->dev), tca9535);
		if (ret < 0) {
			dev_err(&client->dev, "Failed to devm_request_threaded_irq: %d\n", ret);
			goto out_tca9535_irq_deinit;
		}
	}

    return 0;

out_tca9535_irq_deinit:
	tca9535_irq_deinit(tca9535);
out_tca9535_keypad_deinit:
	tca9535_keypad_deinit(tca9535);
out_gpiochip_remove:
	if (gpiochip_remove(gpio_chip) < 0)
	{
		dev_warn(&client->dev, "Failed to gpiochip_remove\n");
	}
out_gpio_free:
	if (gpio_is_valid(tca9535->gpio_irq)) {
		gpio_free(tca9535->gpio_irq);
	}
out_devm_kfree:
	mutex_destroy(&tca9535->lock);
	devm_kfree(&client->dev, tca9535);

	return ret;
}
static int tca9535_i2c_remove(struct i2c_client *client)
{
	int ret;
	struct tca9535_device *tca9535 = i2c_get_clientdata(client);

	if (client->irq >= 0) {
		devm_free_irq(&client->dev, client->irq, tca9535);
	}

	tca9535_irq_deinit(tca9535);
	tca9535_keypad_deinit(tca9535);

	ret = gpiochip_remove(&tca9535->gpio_chip);
	if (ret < 0) {
		dev_warn(&client->dev, "Failed to gpiochip_remove: %d\n", ret);
	}

	if (gpio_is_valid(tca9535->gpio_irq)) {
		gpio_free(tca9535->gpio_irq);
	}

	mutex_destroy(&tca9535->lock);
	devm_kfree(&client->dev, tca9535);

    return 0;
}

static void tca9535_i2c_shutdown(struct i2c_client *client)
{
	tca9535_pr_pos_info();
}

static const struct i2c_device_id tca9535_i2c_id[] = {
	{ "tca9535", 0 },
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

subsys_initcall_sync(tca9535_module_init);
module_exit(tca9535_module_exit);

MODULE_DESCRIPTION("TCA9535 GPIO Driver");
MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_LICENSE("GPL");
