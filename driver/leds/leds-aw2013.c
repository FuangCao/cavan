/*
 * Copyright (C) 2016 Jwaoo, Inc.
 * drivers/leds/leds-aw2013.c
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

#define AW2013_DEBUG					1

#define AW2013_I2C_RATE					(100 * 1000)
#define AW2013_CHIP_ID					0x33

#define aw2013_pr_info(fmt, args ...) \
	 pr_err("%s[%d]: " fmt "\n", __FUNCTION__, __LINE__, ##args)

#define aw2013_pr_pos_info() \
	 pr_err("%s => %s[%d]\n", __FILE__, __FUNCTION__, __LINE__)

enum aw2013_register {
	REG_RSTR = 0x00,
	REG_GCR = 0x01,
	REG_ISR = 0x02,
	REG_LCTR = 0x30,
	REG_LCFG0 = 0x31,
	REG_LCFG1 = 0x32,
	REG_LCFG2 = 0x33,
	REG_PWM0 = 0x34,
	REG_PWM1 = 0x35,
	REG_PWM2 = 0x36,
	REG_LED0T0 = 0x37,
	REG_LED0T1 = 0x38,
	REG_LED0T2 = 0x39,
	REG_LED1T0 = 0x3A,
	REG_LED1T1 = 0x3B,
	REG_LED1T2 = 0x3C,
	REG_LED2T0 = 0x3D,
	REG_LED2T1 = 0x3E,
	REG_LED2T2 = 0x3F,
	REG_IADR = 0x77,
};

struct aw2013_device;

struct aw2013_led {
	struct led_classdev cdev;
	int index;
	char name[16];
	struct aw2013_device *dev;
};

struct aw2013_device {
	struct i2c_client *client;
	struct aw2013_led leds[3];
};

static int aw2013_read_data(struct i2c_client *client, u8 addr, void *buff, size_t size)
{
	int ret;
	struct i2c_msg msgs[] = {
		{
			.addr = client->addr,
			.flags = (client->flags & I2C_M_TEN),
			.len = 1,
			.buf = (__u8 *) &addr,
#ifdef CONFIG_I2C_ROCKCHIP_COMPAT
			.scl_rate = AW2013_I2C_RATE,
#endif
		}, {
			.addr = client->addr,
			.flags = (client->flags & I2C_M_TEN) | I2C_M_RD,
			.len = size,
			.buf = (__u8 *) buff,
#ifdef CONFIG_I2C_ROCKCHIP_COMPAT
			.scl_rate = AW2013_I2C_RATE,
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

static int aw2013_write_data(struct i2c_client *client, const void *buff, size_t size)
{
	int ret;
	struct i2c_msg msg = {
		.addr = client->addr,
		.flags = (client->flags & I2C_M_TEN),
		.len = size,
		.buf = (__u8 *) buff,
#ifdef CONFIG_I2C_ROCKCHIP_COMPAT
		.scl_rate = AW2013_I2C_RATE,
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

static inline int aw2013_read_register(struct i2c_client *client, u8 addr, u8 *value)
{
#if AW2013_DEBUG
	dev_info(&client->dev, "read: addr = 0x%02x\n", addr);
#endif

	return aw2013_read_data(client, addr, value, 1);
}

static inline int aw2013_write_register(struct i2c_client *client, u8 addr, u8 value)
{
	u8 buff[] = { addr, value };

#if AW2013_DEBUG
	dev_info(&client->dev, "write: addr = 0x%02x, value = 0x%04x\n", addr, value);
#endif

	return aw2013_write_data(client, buff, sizeof(buff));
}

// ============================================================

static inline int aw2013_reset(struct i2c_client *client)
{
	return aw2013_write_register(client, REG_RSTR, 0x55);
}

static int aw2013_init_register(struct aw2013_device *aw2013)
{
	int i;
	int ret;
	u8 value;
	struct i2c_client *client = aw2013->client;

	aw2013_pr_pos_info();

	ret = aw2013_read_register(client, REG_RSTR, &value);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to aw2013_read_register: %d\n", ret);
		return ret;
	}

	dev_info(&client->dev, "Chip ID = 0x%02x\n", value);

	if (value != AW2013_CHIP_ID) {
		dev_err(&client->dev, "Invalid Chip ID, Need 0x%02x\n", AW2013_CHIP_ID);
		return -EINVAL;
	}

	ret = aw2013_reset(client);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to aw2013_reset: %d\n", ret);
		return ret;
	}

	for (i = 0; i < 3; i++) {
		ret |= aw2013_write_register(client, REG_LCFG0 + i, 0x01);
	}

	return ret;
}

static void aw2013_set_brightness(struct led_classdev *cdev, enum led_brightness brightness)
{
	int ret;
	u8 gcr, gcr_new;
	u8 lctr, lctr_new;
	struct aw2013_led *led = (struct aw2013_led *) cdev;
	struct aw2013_device *aw2013 = led->dev;
	struct i2c_client *client = aw2013->client;

	aw2013_pr_pos_info();

	ret = aw2013_read_register(client, REG_LCTR, &lctr);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to aw2013_read_register: %d\n", ret);
		return;
	}

	ret = aw2013_read_register(client, REG_GCR, &gcr);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to aw2013_read_register: %d\n", ret);
		return;
	}

	if (brightness == LED_OFF) {
		lctr_new = lctr & (~(1 << led->index));
		if ((lctr_new & 0x07) == 0) {
			gcr_new = gcr & (~0x01);
		} else {
			gcr_new = gcr | 0x01;
		}
	} else {
		lctr_new = lctr | (1 << led->index);
		gcr_new = gcr | 0x01;
	}

	if (gcr_new != gcr) {
		aw2013_write_register(client, REG_GCR, gcr_new);
	}

	if (lctr_new != lctr) {
		aw2013_write_register(client, REG_LCTR, lctr_new);
	}

	aw2013_write_register(client, REG_PWM0 + led->index, brightness);
}

static int aw2013_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int i;
	int ret;
	struct aw2013_device *aw2013;

	aw2013 = devm_kzalloc(&client->dev, sizeof(struct aw2013_device), GFP_KERNEL);
	if (aw2013 == NULL) {
		dev_err(&client->dev, "kzalloc failed\n");
		return -ENOMEM;
	}

	aw2013->client = client;
	i2c_set_clientdata(client, aw2013);

	ret = aw2013_init_register(aw2013);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to aw2013_init_register: %d\n", ret);
		goto out_devm_kfree;
	}

	for (i = 0; i < ARRAY_SIZE(aw2013->leds); i++) {
		struct aw2013_led *led = aw2013->leds + i;
		struct led_classdev *cdev = &led->cdev;

		led->index = i;
		led->dev = aw2013;
		snprintf(led->name, sizeof(led->name), "aw2013-led%02d", i);

		cdev->name = led->name;
		cdev->brightness = LED_OFF;
		cdev->brightness_set = aw2013_set_brightness;

		ret = led_classdev_register(&client->dev, cdev);
		if (ret < 0) {
			dev_err(&client->dev, "Failed to led_classdev_register %s: %d", cdev->name, ret);

			while (--i >= 0) {
				led_classdev_unregister(&aw2013->leds[i].cdev);
			}

			goto out_devm_kfree;
		}
	}

	return 0;

out_devm_kfree:
	devm_kfree(&client->dev, aw2013);
	return ret;
}

static int aw2013_i2c_remove(struct i2c_client *client)
{
	int i;
	struct aw2013_device *aw2013 = i2c_get_clientdata(client);

	for (i = 0; i < ARRAY_SIZE(aw2013->leds); i++) {
		led_classdev_unregister(&aw2013->leds[i].cdev);
	}

	devm_kfree(&client->dev, aw2013);

	return 0;
}

static void aw2013_i2c_shutdown(struct i2c_client *client)
{
	aw2013_pr_pos_info();
}

static const struct i2c_device_id aw2013_i2c_id[] = {
	{ "aw2013", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, aw2013_i2c_id);

static struct i2c_driver aw2013_i2c_driver = {
	.driver = {
		.name = "aw2013",
		.owner = THIS_MODULE,
	},
	.probe = aw2013_i2c_probe,
	.remove   = aw2013_i2c_remove,
	.shutdown = aw2013_i2c_shutdown,
	.id_table = aw2013_i2c_id,
};

static int __init aw2013_module_init(void)
{
	aw2013_pr_pos_info();

	return i2c_add_driver(&aw2013_i2c_driver);
}

static void __exit aw2013_module_exit(void)
{
	aw2013_pr_pos_info();

	i2c_del_driver(&aw2013_i2c_driver);
}

subsys_initcall(aw2013_module_init);
module_exit(aw2013_module_exit);

MODULE_DESCRIPTION("AW2013 LED Driver");
MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_LICENSE("GPL");
