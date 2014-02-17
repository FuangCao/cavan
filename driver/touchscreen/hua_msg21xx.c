#include <huamobile/hua_ts.h>
#include <huamobile/hua_sensor.h>
#include <huamobile/hua_i2c.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>
#include "msg21xx_vendor_api.c"

#define HUA_SUPPORT_PROXIMITY		1

#define FW_ADDR_MSG21XX				(0xC4 >> 1)
#define FW_ADDR_MSG21XX_TP			(0x4C >> 1)
#define FW_UPDATE_ADDR_MSG21XX		(0x92 >> 1)

#define MSG21XX_XAXIS_MAX_RAW		2048
#define MSG21XX_YAXIS_MAX_RAW		2048

#define MSG21XX_XAXIS_MIN			0
#define MSG21XX_XAXIS_MAX			(CONFIG_HUAMOBILE_LCD_WIDTH)
#define MSG21XX_YAXIS_MIN			0
#define MSG21XX_YAXIS_MAX			(CONFIG_HUAMOBILE_LCD_HEIGHT)

#define MSG21XX_KEY_WIDTH			(MSG21XX_XAXIS_MAX / 4)
#define MSG21XX_KEY_HEIGHT			(100)

#define MSG21XX_XAXIS_KEY1			(MSG21XX_KEY_WIDTH / 2)
#define MSG21XX_XAXIS_KEY2			(MSG21XX_XAXIS_KEY1 + MSG21XX_KEY_WIDTH)
#define MSG21XX_XAXIS_KEY3			(MSG21XX_XAXIS_KEY2 + MSG21XX_KEY_WIDTH)
#define MSG21XX_XAXIS_KEY4			(MSG21XX_XAXIS_KEY3 + MSG21XX_KEY_WIDTH)
#define MSG21XX_YAXIS_KEY			(MSG21XX_YAXIS_MAX + MSG21XX_KEY_HEIGHT / 2)

#define MSG21XX_POINT_COUNT			2
#define MSG21XX_DEVICE_NAME			"msg21xx_ts"

#define MSG21XX_BUILD_XAXIS(h, l) \
	(((u16)((h) & 0xF0)) << 4 | (l))

#define MSG21XX_BUILD_YAXIS(h, l) \
	(((u16)((h) & 0x0F)) << 8 | (l))

#define MSG21XX_CAL_XAXIS(x) \
	((x) * CONFIG_HUAMOBILE_LCD_WIDTH / 2048)

#define MSG21XX_CAL_YAXIS(y) \
	((y) * CONFIG_HUAMOBILE_LCD_HEIGHT / 2048)

#pragma pack(1)
struct msg21xx_touch_point_raw
{
	u8 hb;
	u8 xl;
	u8 yl;
};

struct msg21xx_touch_key
{
	u8 reserved;
	u8 code;
};

struct msg21xx_data_package
{
	u8 magic;
	struct msg21xx_touch_point_raw point;

	union
	{
		struct msg21xx_touch_point_raw delta;
		struct msg21xx_touch_key key;
	};

	u8 checksum;
};
#pragma pack()

struct hua_msg21xx_device
{
	struct hua_ts_device ts;
#if HUA_SUPPORT_PROXIMITY
	struct hua_sensor_device prox;
#endif
};

struct msg21xx_touch_point
{
	int x;
	int y;
};

struct hua_msg21xx_chip
{
	struct hua_input_chip chip;
	int gpio_reset;
	int gpio_irq;
	struct regulator *vdd;
	struct regulator *vcc_i2c;

	int distance;
	int touch_count;
	struct msg21xx_touch_point points[MSG21XX_POINT_COUNT];
};

static ssize_t msg21xx_read_data(struct hua_input_chip *chip, u8 addr, void *buff, size_t size)
{
	return hua_input_master_recv_from_i2c(hua_input_chip_get_bus_data(chip), addr, buff, size);
}

static ssize_t msg21xx_write_data(struct hua_input_chip *chip, u8 addr, const void *buff, size_t size)
{
	return hua_input_master_send_to_i2c(hua_input_chip_get_bus_data(chip), addr, buff, size);
}

static void msg21xx_chip_reset(struct hua_msg21xx_chip *msg21xx)
{
	gpio_direction_output(msg21xx->gpio_reset, 1);
	gpio_set_value(msg21xx->gpio_reset, 1);
	msleep(10);
	gpio_set_value(msg21xx->gpio_reset, 0);
	msleep(150);
	gpio_set_value(msg21xx->gpio_reset, 1);
	msleep(150);
}

static ssize_t msg21xx_read_data_package(struct hua_input_chip *chip, struct msg21xx_data_package *package)
{
	return chip->master_recv(chip, package, sizeof(*package));
}

static int msg21xx_ts_event_handler(struct hua_input_chip *chip, struct hua_input_device *dev)
{
	struct msg21xx_touch_point *p, *p_end;
	struct input_dev *input = dev->input;
	struct hua_ts_device *ts = (struct hua_ts_device *)dev;
	struct hua_msg21xx_chip *msg21xx = container_of(chip, struct hua_msg21xx_chip, chip);

	if (msg21xx->touch_count < 0)
	{
		return 0;
	}

	if (msg21xx->touch_count == 0)
	{
		if (ts->touch_count)
		{
			hua_ts_mt_touch_release(input);
		}
	}
	else
	{
		for (p = msg21xx->points, p_end = p + msg21xx->touch_count; p < p_end; p++)
		{
			hua_ts_report_mt_data(input, p->x, p->y);
		}

		input_sync(input);
	}

	ts->touch_count = msg21xx->touch_count;

	return 0;
}

#if HUA_SUPPORT_PROXIMITY
static int msg21xx_proximity_event_handler(struct hua_input_chip *chip, struct hua_input_device *dev)
{
	struct hua_msg21xx_chip *msg21xx = container_of(chip, struct hua_msg21xx_chip, chip);

	hua_sensor_report_value(dev->input, msg21xx->distance);

	return 0;
}

static int msg21xx_proximity_set_enable(struct hua_input_device *dev, bool enable)
{
	struct hua_input_chip *chip = dev->chip;
	u8 buff[] = {0x52, 0x00, 0x4a, enable ? 0xa0 : 0xa1};

	hua_input_print_memory(buff, sizeof(buff));

	return chip->master_send(chip, buff, sizeof(buff));
}
#endif

static int msg21xx_set_power(struct hua_input_chip *chip, bool enable)
{
	struct hua_msg21xx_chip *msg21xx = container_of(chip, struct hua_msg21xx_chip, chip);

	if (enable)
	{
		if (msg21xx->vcc_i2c)
		{
			regulator_enable(msg21xx->vcc_i2c);
		}

		if (msg21xx->vdd)
		{
			regulator_enable(msg21xx->vdd);
		}

		msg21xx_chip_reset(msg21xx);
	}
	else
	{
		if (msg21xx->vcc_i2c)
		{
			regulator_disable(msg21xx->vcc_i2c);
		}

		if (msg21xx->vdd)
		{
			regulator_disable(msg21xx->vdd);
		}

		gpio_direction_output(msg21xx->gpio_reset, 0);
		gpio_set_value(msg21xx->gpio_reset, 0);
	}

	return 0;
}

static struct hua_ts_touch_key msg21xx_touch_keys[] =
{
	{
		.code = KEY_BACK,
		.x = MSG21XX_XAXIS_KEY1,
		.y = MSG21XX_YAXIS_KEY,
		.width = 80,
		.height = 100,
	},
	{
		.code = KEY_HOMEPAGE,
		.x = MSG21XX_XAXIS_KEY2,
		.y = MSG21XX_YAXIS_KEY,
		.width = 80,
		.height = 100,
	},
	{
		.code = KEY_MENU,
		.x = MSG21XX_XAXIS_KEY3,
		.y = MSG21XX_YAXIS_KEY,
		.width = 80,
		.height = 100,
	}
};

static int msg21xx_readid(struct hua_input_chip *chip)
{
	int ret;
	char buff[8];

	pr_pos_info();

	ret = chip->master_recv(chip, buff, sizeof(buff));
	if (ret < 0)
	{
		pr_red_info("huamobile_i2c_read_data");
		return ret;
	}

	hua_input_print_memory(buff, ret);

	return 0;
}

static ssize_t msg21xx_firmware_id_show(struct device *dev, struct device_attribute *attr, char *buff)
{
	int ret;
	u16 version[2];
	struct i2c_client *client = container_of(dev, struct i2c_client, dev);
	struct hua_msg21xx_chip *msg21xx = i2c_get_clientdata(client);
	struct hua_input_chip *chip = &msg21xx->chip;
	char command[] = {0x53, 0x00, 0x2A};

	mutex_lock(&chip->lock);

	ret = hua_input_chip_set_power(chip, true);
	if (ret < 0)
	{
		pr_red_info("hua_input_chip_set_power_lock");
		goto out_mutex_unlock;
	}

	ret = chip->write_data(chip, FW_ADDR_MSG21XX_TP, command, sizeof(command));
	if (ret < 0)
	{
		pr_red_info("master_send");
		goto out_mutex_unlock;
	}

	ret = chip->read_data(chip, FW_ADDR_MSG21XX_TP, version, sizeof(version));
	if (ret < 0)
	{
		pr_red_info("master_recv");
		goto out_mutex_unlock;
	}

	ret = sprintf(buff, "%03d.%03d\n", version[0], version[1]);

out_mutex_unlock:
	mutex_unlock(&chip->lock);
	return ret;
}

static DEVICE_ATTR(firmware_id, S_IRUGO, msg21xx_firmware_id_show, NULL);

static int msg21xx_input_chip_probe(struct hua_input_chip *chip)
{
	int ret;

	struct hua_msg21xx_device *dev;
	struct hua_input_device *base_dev;
	struct hua_ts_device *ts;
#if HUA_SUPPORT_PROXIMITY
	struct hua_sensor_device *prox;
#endif

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (chip == NULL)
	{
		pr_red_info("kzalloc");
		return -ENOMEM;
	}

	hua_input_chip_set_dev_data(chip, dev);

	// ============================================================

	ts = &dev->ts;
	ts->xmin = MSG21XX_XAXIS_MIN;
	ts->xmax = MSG21XX_XAXIS_MAX;
	ts->ymin = MSG21XX_YAXIS_MIN;
	ts->ymax = MSG21XX_YAXIS_MAX;
	ts->point_count = MSG21XX_POINT_COUNT;
	ts->key_count = ARRAY_SIZE(msg21xx_touch_keys);
	ts->keys = msg21xx_touch_keys;

	base_dev = &ts->dev;
	base_dev->type = HUA_INPUT_DEVICE_TYPE_TOUCHSCREEN;
	base_dev->use_irq = true;
	base_dev->event_handler = msg21xx_ts_event_handler;

	ret = hua_input_device_register(chip, base_dev);
	if (ret < 0)
	{
		pr_red_info("hua_input_device_register");
		goto out_free_dev;
	}

	// ============================================================

#if HUA_SUPPORT_PROXIMITY
	prox = &dev->prox;
	prox->min_delay = 20;
	prox->max_range = 1;
	prox->resolution = 1;
	prox->power_consume = 0;

	base_dev = &prox->dev;
	base_dev->type = HUA_INPUT_DEVICE_TYPE_PROXIMITY;
	base_dev->use_irq = true;
	base_dev->poll_delay = 200;
	base_dev->set_enable = msg21xx_proximity_set_enable;
	base_dev->event_handler = msg21xx_proximity_event_handler;

	ret = hua_input_device_register(chip, base_dev);
	if (ret < 0)
	{
		pr_red_info("hua_input_device_register");
		goto out_hua_input_device_unregister_ts;
	}
#endif

	ret = device_create_file(&((struct i2c_client *)chip->bus_data)->dev, &dev_attr_firmware_id);
	if (ret < 0)
	{
		pr_red_info("device_create_file");
		goto out_hua_input_device_unregister_prox;
	}

	return 0;

out_hua_input_device_unregister_prox:
#if HUA_SUPPORT_PROXIMITY
	hua_input_device_unregister(chip, &prox->dev);
out_hua_input_device_unregister_ts:
#endif
	hua_input_device_unregister(chip, &ts->dev);
out_free_dev:
	kfree(dev);
	return ret;
}

static inline int msg21xx_is_touch_key(u8 *buff)
{
#if 0
	return buff[1] == 0xFF && buff[2] == 0xFF && buff[3] == 0xFF && buff[4] == 0xFF && buff[6] == 0xFF;
#else
	return *(u32 *)(buff + 1) == 0xFFFFFFFF;
#endif
}

static void msg21xx_chip_event_handler(struct hua_input_chip *chip)
{
	int ret;
	struct msg21xx_data_package package;
	struct hua_msg21xx_chip *msg21xx = container_of(chip, struct hua_msg21xx_chip, chip);

	ret = msg21xx_read_data_package(chip, &package);
	if (ret < 0)
	{
		pr_red_info("msg21xx_read_data_package");
		hua_input_chip_recovery(chip, false);
		return;
	}

	// hua_input_print_memory(&package, sizeof(package));

	if (package.magic != 0x52)
	{
		return;
	}

	if (msg21xx_is_touch_key((u8 *)&package))
	{
		u8 code = package.key.code;

		switch (code)
		{
		case 1:
			msg21xx->points[0].x = MSG21XX_XAXIS_KEY3;
			msg21xx->points[0].y = MSG21XX_YAXIS_KEY;
			msg21xx->touch_count = 1;
			break;

		case 2:
			msg21xx->points[0].x = MSG21XX_XAXIS_KEY1;
			msg21xx->points[0].y = MSG21XX_YAXIS_KEY;
			msg21xx->touch_count = 1;
			break;

		case 4:
			msg21xx->points[0].x = MSG21XX_XAXIS_KEY2;
			msg21xx->points[0].y = MSG21XX_YAXIS_KEY;
			msg21xx->touch_count = 1;
			break;

		case 0x40:
			msg21xx->distance = 1;
			msg21xx->touch_count = -1;
			break;

		case 0x80:
			msg21xx->distance = 0;
			msg21xx->touch_count = -1;
			break;

		case 0:
		case 0xFF:
			msg21xx->touch_count = 0;
			break;

		default:
			pr_red_info("invalid keycode = %d", code);
			return;
		}
	}
	else
	{
		int x, y;
		int dx, dy;
		struct msg21xx_touch_point_raw *p;

		p = &package.point;
		x = MSG21XX_BUILD_XAXIS(p->hb, p->xl);
		y = MSG21XX_BUILD_YAXIS(p->hb, p->yl);

		msg21xx->points[0].x = MSG21XX_CAL_XAXIS(x);
		msg21xx->points[0].y = MSG21XX_CAL_YAXIS(y);

#if MSG21XX_POINT_COUNT > 1
		p = &package.delta;

		dx = MSG21XX_BUILD_XAXIS(p->hb, p->xl);
		dy = MSG21XX_BUILD_YAXIS(p->hb, p->yl);

		if (dx || dy)
		{
			if (dx > 2048)
			{
				dx -= 4096;
			}

			if (dy > 2048)
			{
				dy -= 4096;
			}

			msg21xx->points[1].x = MSG21XX_CAL_XAXIS(x + dx);
			msg21xx->points[1].y = MSG21XX_CAL_YAXIS(y + dy);
			msg21xx->touch_count = 2;
		}
		else
#endif
		{
			msg21xx->touch_count = 1;
		}
	}

	hua_input_chip_report_events(chip, &chip->isr_list);
}

static void msg21xx_input_chip_remove(struct hua_input_chip *chip)
{
	struct hua_msg21xx_device *dev = hua_input_chip_get_dev_data(chip);

	pr_pos_info();

	device_remove_file(&((struct i2c_client *)chip->bus_data)->dev, &dev_attr_firmware_id);
#if HUA_SUPPORT_PROXIMITY
	hua_input_device_unregister(chip, &dev->prox.dev);
#endif
	hua_input_device_unregister(chip, &dev->ts.dev);
	kfree(dev);
}

int msg21xx_chip_firmware_upgrade(struct hua_input_chip *chip, struct hua_firmware *fw)
{
#ifdef MSG21XX_VENDOR_API
	int ret;

	ret = hua_firmware_fill(fw, (char *)temp, sizeof(temp), 0, 0);
	if (ret < 0)
	{
		pr_red_info("hua_firmware_fill");
		return ret;
	}

	if (firmware_auto_update())
	{
		return 0;
	}
#endif

	return -EFAULT;
}

static int msg21xx_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	struct device_node	*of_node;
	struct hua_input_chip *chip;
	struct hua_msg21xx_chip *msg21xx;

	pr_pos_info();

	msg21xx = kzalloc(sizeof(*msg21xx), GFP_KERNEL);
	if (msg21xx == NULL)
	{
		pr_red_info("kzalloc");
		return -ENOMEM;
	}

	client->flags = 0;
	i2c_set_clientdata(client, msg21xx);

	of_node = client->dev.of_node;
	if (of_node == NULL)
	{
		ret = -EINVAL;
		pr_red_info("of_node is null");
		goto out_kfree_msg21xx;
	}

	ret = of_get_named_gpio_flags(of_node, "reset-gpio-pin", 0, NULL);
	if (ret < 0)
	{
		pr_red_info("of_get_named_gpio_flags reset-gpio-pin");
		goto out_kfree_msg21xx;
	}

	msg21xx->gpio_reset = ret;

	ret = of_get_named_gpio_flags(of_node, "irq-gpio-pin", 0, NULL);
	if (ret < 0)
	{
		pr_red_info("of_get_named_gpio_flags irq-gpio-pin");
		goto out_kfree_msg21xx;
	}

	msg21xx->gpio_irq = ret;

	pr_bold_info("msg21xx->gpio_reset = %d", msg21xx->gpio_reset);
	pr_bold_info("msg21xx->gpio_irq = %d", msg21xx->gpio_irq);

	gpio_direction_input(msg21xx->gpio_irq);

	msg21xx->vdd = regulator_get(&client->dev, "vdd");
	msg21xx->vcc_i2c = regulator_get(&client->dev, "vcc_i2c");

	chip = &msg21xx->chip;
	hua_input_chip_set_bus_data(chip, client);

#ifdef MSG21XX_VENDOR_API
	msg21xx_i2c_client = client;
	msg21xx_irq = client->irq;
#endif

	chip->irq = client->irq;
	chip->irq_flags = IRQF_TRIGGER_RISING;
	chip->name = "MSG21XX";
	chip->devmask = 1 << HUA_INPUT_DEVICE_TYPE_TOUCHSCREEN;
#if HUA_SUPPORT_PROXIMITY
	chip->devmask |= 1 << HUA_INPUT_DEVICE_TYPE_PROXIMITY;
#endif

	chip->set_power = msg21xx_set_power;
	chip->readid = msg21xx_readid;
	chip->probe = msg21xx_input_chip_probe;
	chip->remove = msg21xx_input_chip_remove;

	chip->read_data = msg21xx_read_data;
	chip->write_data = msg21xx_write_data;
	chip->read_register = hua_input_read_register_i2c_smbus;
	chip->write_register = hua_input_write_register_i2c_smbus;
	chip->event_handler = msg21xx_chip_event_handler;
	chip->firmware_upgrade = msg21xx_chip_firmware_upgrade;

	ret = hua_input_chip_register(chip);
	if (ret < 0)
	{
		pr_red_info("hua_input_chip_register");
		goto out_regulator_put;
	}

	msg21xx->distance = 1;
	msg21xx->touch_count = 0;

	pr_green_info("MSG21XX Probe Complete");

	return 0;

out_regulator_put:
	if (msg21xx->vdd)
	{
		regulator_put(msg21xx->vdd);
	}

	if (msg21xx->vcc_i2c)
	{
		regulator_put(msg21xx->vcc_i2c);
	}
out_kfree_msg21xx:
	kfree(msg21xx);
	return ret;
}

static int msg21xx_i2c_remove(struct i2c_client *client)
{
	struct hua_msg21xx_chip *msg21xx = i2c_get_clientdata(client);
	struct hua_input_chip *chip = &msg21xx->chip;

	pr_pos_info();

	hua_input_chip_unregister(chip);

	if (msg21xx->vdd)
	{
		regulator_put(msg21xx->vdd);
	}

	if (msg21xx->vcc_i2c)
	{
		regulator_put(msg21xx->vcc_i2c);
	}

	kfree(msg21xx);

	return 0;
}

static const struct i2c_device_id msg21xx_ts_id_table[] =
{
	{MSG21XX_DEVICE_NAME, 0},
	{}
};

static struct of_device_id msg21xx_match_table[] =
{
	{
		.compatible = "mstar,msg21xx"
	},
	{}
};

static struct i2c_driver msg21xx_ts_driver =
{
	.probe = msg21xx_i2c_probe,
	.remove = msg21xx_i2c_remove,

	.id_table = msg21xx_ts_id_table,
	.driver =
	{
		.name = MSG21XX_DEVICE_NAME,
		.owner = THIS_MODULE,
		.of_match_table = msg21xx_match_table,
	}
};

static int __init msg21xx_ts_init(void)
{
	return i2c_add_driver(&msg21xx_ts_driver);
}

static void __exit msg21xx_ts_exit(void)
{
	i2c_del_driver(&msg21xx_ts_driver);
}

module_init(msg21xx_ts_init);
module_exit(msg21xx_ts_exit);

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Huamobile MSG21XX TouchScreen Driver");
MODULE_LICENSE("GPL");
