#include <linux/input/hua_ts.h>

#define FT5216_XAXIS_MIN			0
#define FT5216_XAXIS_MAX			320
#define FT5216_YAXIS_MIN			0
#define FT5216_YAXIS_MAX			480
#define FT5216_POINT_COUNT			5
#define FT5216_DEVICE_NAME			"ft5216_ts"

#define FT5216_REG_DEVICE_MODE		0x00
#define FT5216_REG_GEST_ID			0x01
#define FT5216_REG_TD_STATUS		0x02
#define FT5216_REG_POINTS			0x03
#define FT5216_REG_POWER_MODE		0xA5

#define FT5216_MODE_ACTIVE			0x00
#define FT5216_MODE_MONITOR			0x01
#define FT5216_MODE_STANDBY			0x02
#define FT5216_MODE_HIBERNATE		0x03

#define FT5216_FIRMWARE_MAX_SIZE	(50 << 10)
#define FT5216_FIRMWARE_BLOCK_SIZE	128

#define FT5216_BUILD_AXIS(h, l) \
	(((u16)((h) & 0x0F)) << 8 | (l))

#define FT5216_EVENT_FLAG(xh) \
	((xh) >> 4)

#define FT5216_TOUCH_ID(yh) \
	((yh) >> 4)

#pragma pack(1)
struct ft5216_touch_point
{
	u8 xh;
	u8 xl;
	u8 yh;
	u8 yl;
	u16 reserved;
};

struct ft5216_data_package
{
	u8 device_mode;
	u8 gest_id;
	u8 td_status;
	struct ft5216_touch_point points[FT5216_POINT_COUNT];
};
#pragma pack()

static inline ssize_t ft5216_data_package(struct hua_input_chip *chip, struct ft5216_data_package *package)
{
	return chip->read_data(chip, 0, package, sizeof(*package));
}

static const char *ft5216_power_mode_tostring(int mode)
{
	switch (mode)
	{
	case FT5216_MODE_ACTIVE:
		return "Active";
	case FT5216_MODE_MONITOR:
		return "Monitor";
	case FT5216_MODE_STANDBY:
		return "Standby";
	case FT5216_MODE_HIBERNATE:
		return "Hibernate";
	default:
		return "Unknown";
	}
}

static int ft5216_change_power_mode(struct hua_input_chip *chip, u8 mode, int retry)
{
	pr_bold_info("Ft5216 change power mode => %s", ft5216_power_mode_tostring(mode));

	while (chip->write_register(chip, FT5216_REG_POWER_MODE, mode) < 0 && retry--)
	{
		msleep(10);
	}

	if (retry < 0)
	{
		pr_red_info("Failed");
	}
	else
	{
		pr_green_info("OK");
	}

	return retry;
}

static int ft5216_ts_event_handler(struct hua_input_chip *chip, struct hua_input_device *dev)
{
	int ret;
	int count;
	struct input_dev *input = dev->input;
	struct ft5216_data_package package;
	struct ft5216_touch_point *p, *p_end;

	ret = ft5216_data_package(chip, &package);
	if (ret < 0)
	{
		pr_red_info("ft5216_read_data_package");
		return ret;
	}

	count = package.td_status & 0x0F;
	if (count == 0)
	{
		hua_ts_mt_touch_release(input);
		return 0;
	}

	if (unlikely(count > FT5216_POINT_COUNT))
	{
		pr_red_info("Too much points = %d", count);
		count = FT5216_POINT_COUNT;
	}

	for (p = package.points, p_end = p + count; p < p_end; p++)
	{
		hua_ts_report_mt_data2(input, FT5216_TOUCH_ID(p->yh), \
			FT5216_BUILD_AXIS(p->xh, p->xl), FT5216_BUILD_AXIS(p->yh, p->yl));
	}

	input_sync(input);

	return 0;
}

static int ft5216_set_power(struct hua_input_chip *chip, bool enable)
{
	if (enable)
	{
		sprd_ts_reset_enable(false);
		sprd_ts_power_enable(true);
		msleep(50);
		sprd_ts_reset_enable(true);
		msleep(200);
	}
	else
	{
		sprd_ts_power_enable(false);
	}

	return 0;
}

static int ft5216_set_active(struct hua_input_chip *chip, bool enable)
{
	u8 mode;
	int retry;

	if (enable)
	{
		mode = FT5216_MODE_ACTIVE;
		retry = 10;
	}
	else
	{
		mode = FT5216_MODE_HIBERNATE;
		retry = 2;
	}

	return ft5216_change_power_mode(chip, mode, retry);
}

static int ft5216_readid(struct hua_input_chip *chip)
{
	int ret;
	char buff[94];
	const u8 *p, *p_end;

	pr_pos_info();

	ret = chip->read_data(chip, 0x22, buff, sizeof(buff));
	if (ret < 0)
	{
		pr_red_info("huamobile_i2c_read_data");
		return ret;
	}

	hua_input_print_memory(buff, ret);

	for (p = buff, p_end = p + 29; p < p_end; p++)
	{
		if (*p != 0xFF)
		{
			return -EINVAL;
		}
	}

	for (p_end = buff + sizeof(buff); p < p_end; p++)
	{
		if (*p != 0)
		{
			return -EINVAL;
		}
	}

	return 0;
}

static struct hua_ts_touch_key ft5216_touch_keys[] =
{
	{
		.code = KEY_MENU,
		.x = 50,
		.y = 513,
		.width = 120,
		.height = 60,
	},
	{
		.code = KEY_HOME,
		.x = 170,
		.y = 513,
		.width = 120,
		.height = 60,
	},
	{
		.code = KEY_BACK,
		.x = 290,
		.y = 513,
		.width = 120,
		.height = 60,
	}
};

static int ft5216_input_chip_probe(struct hua_input_chip *chip)
{
	int ret;
	struct hua_ts_device *ts;
	struct hua_input_device *dev;

	ts = kzalloc(sizeof(struct hua_ts_device), GFP_KERNEL);
	if (ts == NULL)
	{
		pr_red_info("kzalloc");
		return -ENOMEM;
	}

	hua_input_chip_set_dev_data(chip, ts);

	ts->xmin = FT5216_XAXIS_MIN;
	ts->xmax = FT5216_XAXIS_MAX;
	ts->ymin = FT5216_YAXIS_MIN;
	ts->ymax = FT5216_YAXIS_MAX;
	ts->point_count = FT5216_POINT_COUNT;
	ts->key_count = ARRAY_SIZE(ft5216_touch_keys);
	ts->keys = ft5216_touch_keys;

	dev = &ts->dev;
	dev->type = HUA_INPUT_DEVICE_TYPE_TOUCHSCREEN;
	dev->use_irq = true;
	dev->event_handler = ft5216_ts_event_handler;

	ret = hua_input_device_register(chip, dev);
	if (ret < 0)
	{
		pr_red_info("hua_input_device_register");
		goto out_kfree_ts;
	}

	return 0;

out_kfree_ts:
	kfree(ts);
	return 0;
}

static void ft5216_input_chip_remove(struct hua_input_chip *chip)
{
	struct hua_ts_device *ts = hua_input_chip_get_dev_data(chip);

	hua_input_device_unregister(chip, &ts->dev);
	kfree(ts);
}

static int ft5216_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	struct hua_input_chip *chip;

	pr_pos_info();

	chip = kzalloc(sizeof(struct hua_input_chip), GFP_KERNEL);
	if (chip == NULL)
	{
		pr_red_info("kzalloc");
		return -ENOMEM;
	}

	i2c_set_clientdata(client, chip);
	hua_input_chip_set_bus_data(chip, client);

	chip->irq = client->irq;
	chip->irq_flags = IRQF_TRIGGER_FALLING;
	chip->name = "FT5216";
	chip->vendor = "FocalTech";
	chip->devmask = 1 << HUA_INPUT_DEVICE_TYPE_TOUCHSCREEN;

	chip->probe = ft5216_input_chip_probe;
	chip->remove = ft5216_input_chip_remove;
	chip->set_power = ft5216_set_power;
	chip->set_active = ft5216_set_active;
	chip->readid = ft5216_readid;
	chip->read_data = hua_input_read_data_i2c;
	chip->write_data = hua_input_write_data_i2c;
	chip->read_register = hua_input_read_register_i2c_smbus;
	chip->write_register = hua_input_write_register_i2c_smbus;

	ret = hua_input_chip_register(chip);
	if (ret < 0)
	{
		pr_red_info("hua_input_chip_register");
		goto out_kfree_chip;
	}

	pr_green_info("FT5216 Probe Complete");
	return 0;

out_kfree_chip:
	kfree(chip);
	return ret;
}

static int ft5216_i2c_remove(struct i2c_client *client)
{
	struct hua_input_chip *chip= i2c_get_clientdata(client);

	hua_input_chip_unregister(chip);
	kfree(chip);

	return 0;
}

static const struct i2c_device_id ft5216_ts_id_table[] =
{
	{FT5216_DEVICE_NAME, 0},
	{}
};

static struct i2c_driver ft5216_ts_driver =
{
	.probe = ft5216_i2c_probe,
	.remove = ft5216_i2c_remove,

	.id_table = ft5216_ts_id_table,
	.driver =
	{
		.name = FT5216_DEVICE_NAME,
		.owner = THIS_MODULE,
	}
};

static int __init ft5216_ts_init(void)
{
	return i2c_add_driver(&ft5216_ts_driver);
}

static void __exit ft5216_ts_exit(void)
{
	i2c_del_driver(&ft5216_ts_driver);
}

module_init(ft5216_ts_init);
module_exit(ft5216_ts_exit);

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Huamobile FocalTech FT5216 TouchScreen Driver");
MODULE_LICENSE("GPL");
