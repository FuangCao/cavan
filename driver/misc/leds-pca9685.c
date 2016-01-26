/*
 * Copyright (C) 2015 JWAOO, Inc.
 * drivers/leds/leds-pca9685.c
 * author: cavan.cfa@gmail.com
 * create date: 2016-01-25
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
#include <linux/leds.h>

#define PCA9685_DEBUG					0

#define PCA9685_I2C_RATE				(100 * 1000)
#define PCA9685_GPIO_COUNT				16
#define PCA9685_KEYPAD_DEBOUNCE_MS		50

#define pca9685_register_led_on(index) \
	(REG_LED0_ON_L + (index) * 4)

#define pca9685_register_led_off(index) \
	(REG_LED0_OFF_L + (index) * 4)

#define pca9685_pr_info(fmt, args ...) \
	 pr_err("%s[%d]: " fmt "\n", __FUNCTION__, __LINE__, ##args)

#define pca9685_pr_pos_info() \
	 pr_err("%s => %s[%d]\n", __FILE__, __FUNCTION__, __LINE__)

enum pca9685_register {
	REG_MODE1 = 0x00,
	REG_MODE2,
	REG_SUBADR1,
	REG_SUBADR2,
	REG_SUBADR3,
	REG_ALLCALLADR,
	REG_LED0_ON_L = 0x06,
	REG_LED0_ON_H,
	REG_LED0_OFF_L,
	REG_LED0_OFF_H,
	REG_LED1_ON_L,
	REG_LED1_ON_H,
	REG_LED1_OFF_L,
	REG_LED1_OFF_H,
	REG_LED2_ON_L,
	REG_LED2_ON_H,
	REG_LED2_OFF_L,
	REG_LED2_OFF_H,
	REG_LED3_ON_L,
	REG_LED3_ON_H,
	REG_LED3_OFF_L,
	REG_LED3_OFF_H,
	REG_LED4_ON_L,
	REG_LED4_ON_H,
	REG_LED4_OFF_L,
	REG_LED4_OFF_H,
	REG_LED5_ON_L,
	REG_LED5_ON_H,
	REG_LED5_OFF_L,
	REG_LED5_OFF_H,
	REG_LED6_ON_L,
	REG_LED6_ON_H,
	REG_LED6_OFF_L,
	REG_LED6_OFF_H,
	REG_LED7_ON_L,
	REG_LED7_ON_H,
	REG_LED7_OFF_L,
	REG_LED7_OFF_H,
	REG_LED8_ON_L,
	REG_LED8_ON_H,
	REG_LED8_OFF_L,
	REG_LED8_OFF_H,
	REG_LED9_ON_L,
	REG_LED9_ON_H,
	REG_LED9_OFF_L,
	REG_LED9_OFF_H,
	REG_LED10_ON_L,
	REG_LED10_ON_H,
	REG_LED10_OFF_L,
	REG_LED10_OFF_H,
	REG_LED11_ON_L,
	REG_LED11_ON_H,
	REG_LED11_OFF_L,
	REG_LED11_OFF_H,
	REG_LED12_ON_L,
	REG_LED12_ON_H,
	REG_LED12_OFF_L,
	REG_LED12_OFF_H,
	REG_LED13_ON_L,
	REG_LED13_ON_H,
	REG_LED13_OFF_L,
	REG_LED13_OFF_H,
	REG_LED14_ON_L,
	REG_LED14_ON_H,
	REG_LED14_OFF_L,
	REG_LED14_OFF_H,
	REG_LED15_ON_L,
	REG_LED15_ON_H,
	REG_LED15_OFF_L,
	REG_LED15_OFF_H,
	REG_ALL_LED_ON_L = 0xFA,
	REG_ALL_LED_ON_H,
	REG_ALL_LED_OFF_L,
	REG_ALL_LED_OFF_H,
	REG_PRE_SCALE,
	REG_TEST_MODE
};

struct pca9685_device;

struct pca9685_led {
	struct led_classdev cdev;
	int index;
	char name[16];
	struct pca9685_device *dev;
};

struct pca9685_device {
	struct i2c_client *client;

	int gpio_oe;
	struct pinctrl *pinctrl;
	struct pinctrl_state *gpio_state;

	u16 enable_mask;
	struct pca9685_led leds[16];
	struct pca9685_led led_all;
};

static int pca9685_read_data(struct i2c_client *client, u8 addr, void *buff, size_t size)
{
	int ret;
	struct i2c_msg msgs[] = {
		{
			.addr = client->addr,
			.flags = (client->flags & I2C_M_TEN),
			.len = 1,
			.buf = (__u8 *) &addr,
#ifdef CONFIG_I2C_ROCKCHIP_COMPAT
			.scl_rate = PCA9685_I2C_RATE,
#endif
		}, {
			.addr = client->addr,
			.flags = (client->flags & I2C_M_TEN) | I2C_M_RD,
			.len = size,
			.buf = (__u8 *) buff,
#ifdef CONFIG_I2C_ROCKCHIP_COMPAT
			.scl_rate = PCA9685_I2C_RATE,
#endif
		}
	};

	ret = i2c_transfer(client->adapter, msgs, ARRAY_SIZE(msgs));
	if (unlikely(ret != ARRAY_SIZE(msgs))) {
		dev_err(&client->dev, "Failed to i2c_transfer: %d\n", ret);
		if (ret >= 0) {
			ret = -EFAULT;
		}
	}

	return ret;
}

static int pca9685_write_data(struct i2c_client *client, const void *buff, size_t size)
{
	int ret;
	struct i2c_msg msg = {
		.addr = client->addr,
		.flags = (client->flags & I2C_M_TEN),
		.len = size,
		.buf = (__u8 *) buff,
#ifdef CONFIG_I2C_ROCKCHIP_COMPAT
		.scl_rate = PCA9685_I2C_RATE,
#endif
	};

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (unlikely(ret != 1)) {
		dev_err(&client->dev, "Failed to i2c_transfer: %d\n", ret);
		if (ret >= 0) {
			ret = -EFAULT;
		}
	}

	return ret;
}

static inline int pca9685_read_register8(struct i2c_client *client, u8 addr, u8 *value)
{
#if PCA9685_DEBUG
	dev_info(&client->dev, "read: addr = 0x%02x\n", addr);
#endif

	return pca9685_read_data(client, addr, value, 1);
}

static inline int pca9685_read_register16(struct i2c_client *client, u8 addr, u16 *value)
{
#if PCA9685_DEBUG
	dev_info(&client->dev, "read: addr = 0x%02x\n", addr);
#endif

	return pca9685_read_data(client, addr, value, 2);
}

static inline int pca9685_write_register8(struct i2c_client *client, u8 addr, u8 value)
{
	u8 buff[] = { addr, value };

#if PCA9685_DEBUG
	dev_info(&client->dev, "write: addr = 0x%02x, value = 0x%04x\n", addr, value);
#endif

	return pca9685_write_data(client, buff, sizeof(buff));
}

static inline int pca9685_write_register16(struct i2c_client *client, u8 addr, u16 value)
{
	u8 buff[] = { addr, value & 0xFF, (value >> 8) & 0xFF };

#if PCA9685_DEBUG
	dev_info(&client->dev, "write: addr = 0x%02x, value = 0x%04x\n", addr, value);
#endif

	return pca9685_write_data(client, buff, sizeof(buff));
}

// ============================================================

#if 0
static int pca9685_reset(struct pca9685_device *pca9685)
{
	int ret;
	u8 data = 0x06;
	struct i2c_client *client = pca9685->client;
	struct i2c_msg msg = {
		.addr = 0x00,
		.flags = (client->flags & I2C_M_TEN),
		.len = 1,
		.buf = (__u8 *) &data,
#ifdef CONFIG_I2C_ROCKCHIP_COMPAT
		.scl_rate = PCA9685_I2C_RATE,
#endif
	};

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (unlikely(ret != 1)) {
		dev_err(&client->dev, "Failed to i2c_transfer: %d\n", ret);
		if (ret >= 0) {
			ret = -EFAULT;
		}
	}

	return 0;
}
#endif

static int pca9685_init_register(struct pca9685_device *pca9685)
{
	int ret = 0;
	struct i2c_client *client = pca9685->client;

#if 0
	ret = pca9685_reset(pca9685);
	if (ret < 0) {
		dev_err(&pca9685->client->dev, "Failed to pca9685_reset: %d\n", ret);
		return ret;
	}
#endif

	ret |= pca9685_write_register8(client, REG_MODE1, 1 << 5);
	ret |= pca9685_write_register8(client, REG_MODE2, 1 << 4 | 1 << 1);

#if 0
	for (i = 0; i < 16; i++) {
		ret |= pca9685_write_register8(client, pca9685_register_led_on(i), 0x00);
		ret |= pca9685_write_register8(client, pca9685_register_led_off(i), 0x00);
	}

	ret |= pca9685_write_register8(client, REG_ALL_LED_ON_L, 0x00);
	ret |= pca9685_write_register8(client, REG_ALL_LED_OFF_L, 0x00);
#endif

	return ret;
}

static void pca9685_set_brightness(struct led_classdev *cdev, enum led_brightness brightness)
{
	u8 addr;
	u16 value;
	struct pca9685_led *led = (struct pca9685_led *) cdev;
	struct pca9685_device *pca9685 = led->dev;

	dev_info(cdev->dev, "name = %s, index = %d\n", cdev->name, led->index);

	if (brightness == LED_OFF) {
		value = 0;
		pca9685->enable_mask &= ~(1 << led->index);
	} else {
		value = brightness * 4095 / 255;
		pca9685->enable_mask |= (1 << led->index);
	}

	addr = pca9685_register_led_off(led->index);
	dev_info(cdev->dev, "addr = 0x%02x, value = 0x%04x\n", addr, value);

	pca9685_write_register16(pca9685->client, addr, value);

	gpio_set_value(pca9685->gpio_oe, pca9685->enable_mask == 0);
}

static int pca9685_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int i;
	int ret;
	struct pca9685_device *pca9685;

	pca9685 = devm_kzalloc(&client->dev, sizeof(struct pca9685_device), GFP_KERNEL);
	if (pca9685 == NULL) {
		dev_err(&client->dev, "kzalloc failed\n");
		return -ENOMEM;
	}

	pca9685->client = client;
	i2c_set_clientdata(client, pca9685);

	pca9685->pinctrl = devm_pinctrl_get(&client->dev);
	if (IS_ERR(pca9685->pinctrl)) {
		pca9685->pinctrl = NULL;
	} else {
		pca9685->gpio_state = pinctrl_lookup_state(pca9685->pinctrl, "gpio");
		if (IS_ERR(pca9685->gpio_state)) {
			pca9685->gpio_state = NULL;
		}
	}

	dev_info(&client->dev, "pinctrl = %p, gpio_state = %p\n", pca9685->pinctrl, pca9685->gpio_state);

	if (pca9685->pinctrl && pca9685->gpio_state) {
		pinctrl_select_state(pca9685->pinctrl, pca9685->gpio_state);
	}

	pca9685->gpio_oe = of_get_gpio(client->dev.of_node, 0);
	dev_info(&client->dev, "gpio_oe = %d\n", pca9685->gpio_oe);

	if (gpio_is_valid(pca9685->gpio_oe)) {
		gpio_request(pca9685->gpio_oe, "PCA9685-OE");
		gpio_direction_output(pca9685->gpio_oe, 1);
	}

	ret = pca9685_init_register(pca9685);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to pca9685_init_register: %d\n", ret);
		goto out_gpio_free;
	}

	for (i = 0; i < ARRAY_SIZE(pca9685->leds); i++) {
		struct pca9685_led *led = pca9685->leds + i;
		struct led_classdev *cdev = &led->cdev;

		led->index = i;
		led->dev = pca9685;
		snprintf(led->name, sizeof(led->name), "pca9685-led%02d", i);

		cdev->name = led->name;
		cdev->brightness = LED_OFF;
		cdev->brightness_set = pca9685_set_brightness;

		ret = led_classdev_register(&client->dev, cdev);
		if (ret < 0) {
			dev_err(&client->dev, "Failed to led_classdev_register %s: %d", cdev->name, ret);

			while (--i >= 0) {
				led_classdev_unregister(&pca9685->leds[i].cdev);
			}

			goto out_gpio_free;
		}
	}

	return 0;

out_gpio_free:
	if (gpio_is_valid(pca9685->gpio_oe)) {
		gpio_free(pca9685->gpio_oe);
	}
// out_devm_kfree:
	devm_kfree(&client->dev, pca9685);
	return ret;
}

static int pca9685_i2c_remove(struct i2c_client *client)
{
	int i;
	struct pca9685_device *pca9685 = i2c_get_clientdata(client);

	for (i = 0; i < ARRAY_SIZE(pca9685->leds); i++) {
		led_classdev_unregister(&pca9685->leds[i].cdev);
	}

	if (gpio_is_valid(pca9685->gpio_oe)) {
		gpio_free(pca9685->gpio_oe);
	}

	devm_kfree(&client->dev, pca9685);

	return 0;
}

static void pca9685_i2c_shutdown(struct i2c_client *client)
{
	pca9685_pr_pos_info();
}

static const struct i2c_device_id pca9685_i2c_id[] = {
	{ "pca9685", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, pca9685_i2c_id);

static struct i2c_driver pca9685_i2c_driver = {
	.driver = {
		.name = "pca9685",
		.owner = THIS_MODULE,
	},
	.probe = pca9685_i2c_probe,
	.remove   = pca9685_i2c_remove,
	.shutdown = pca9685_i2c_shutdown,
	.id_table = pca9685_i2c_id,
};

static int __init pca9685_module_init(void)
{
	pca9685_pr_pos_info();

	return i2c_add_driver(&pca9685_i2c_driver);
}

static void __exit pca9685_module_exit(void)
{
	pca9685_pr_pos_info();

	i2c_del_driver(&pca9685_i2c_driver);
}

subsys_initcall(pca9685_module_init);
module_exit(pca9685_module_exit);

MODULE_DESCRIPTION("PCA9685 LED Driver");
MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_LICENSE("GPL");
