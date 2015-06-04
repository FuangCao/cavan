#include <cavan/cavan_ts.h>
#include <cavan/cavan_sensor.h>
#include <cavan/cavan_i2c.h>
#include <cavan/cavan_io.h>

#define CAVAN_SUPPORT_PROXIMITY		1

#define BL86X8_XAXIS_MIN			1
#define BL86X8_XAXIS_MAX			(CONFIG_CAVAN_LCD_WIDTH + 2)
#define BL86X8_YAXIS_MIN			1
#define BL86X8_YAXIS_MAX			CONFIG_CAVAN_LCD_HEIGHT
#define BL86X8_POINT_COUNT			5
#define BL86X8_DEVICE_NAME			"bl86x8_ts"

#define BL86X8_REG_DEVICE_MODE		0x00
#define BL86X8_REG_GEST_ID			0x01
#define BL86X8_REG_TD_STATUS		0x02
#define BL86X8_REG_POINTS			0x03
#define BL86X8_REG_POWER_MODE		0xA5

#define BL86X8_MODE_ACTIVE			0x00
#define BL86X8_MODE_MONITOR			0x01
#define BL86X8_MODE_STANDBY			0x02
#define BL86X8_MODE_HIBERNATE		0x03

#define BL86X8_FIRMWARE_MAX_SIZE	(50 << 10)
#define BL86X8_FIRMWARE_BLOCK_SIZE	128

#define BL86X8_BUILD_AXIS(h, l) \
	(((u16)((h) & 0x0F)) << 8 | (l))

#define BL86X8_EVENT_FLAG(xh) \
	((xh) >> 4)

#define BL86X8_TOUCH_ID(yh) \
	((yh) >> 4)

#pragma pack(1)
struct bl86x8_touch_point
{
	u8 xh;
	u8 xl;
	u8 yh;
	u8 yl;
	u16 reserved;
};

struct bl86x8_data_package
{
	u8 device_mode;
	u8 gest_id;
	u8 td_status;
	struct bl86x8_touch_point points[BL86X8_POINT_COUNT];
};
#pragma pack()

struct cavan_bl86x8_device
{
	struct cavan_ts_device ts;
	struct cavan_sensor_device prox;
};

static inline ssize_t bl86x8_read_data_package(struct cavan_input_chip *chip, struct bl86x8_data_package *package)
{
	return chip->read_data(chip, 0, package, sizeof(*package));
}

static int bl86x8_ts_event_handler(struct cavan_input_chip *chip, struct cavan_input_device *dev)
{
	int ret;
	int count;
	struct input_dev *input = dev->input;
	struct bl86x8_data_package package;
	struct bl86x8_touch_point *p, *p_end;
	struct cavan_ts_device *ts = (struct cavan_ts_device *)dev;

	ret = bl86x8_read_data_package(chip, &package);
	if (ret < 0)
	{
		pr_red_info("bl86x8_read_data_package");
		cavan_input_chip_recovery(chip, false);
		return ret;
	}

	count = package.td_status & 0x07;
	if (count == 0)
	{
		if (ts->touch_count)
		{
			cavan_ts_mt_touch_release(input);
			ts->touch_count = 0;
		}

		return 0;
	}

	if (unlikely(count > BL86X8_POINT_COUNT))
	{
		// pr_red_info("Too much points = %d", count);
		count = BL86X8_POINT_COUNT;
	}

	for (p = package.points, p_end = p + count; p < p_end; p++)
	{
		cavan_ts_report_mt_data(input, BL86X8_BUILD_AXIS(p->xh, p->xl), \
			BL86X8_BUILD_AXIS(p->yh, p->yl));
	}

	input_sync(input);
	ts->touch_count = count;

	return 0;
}

static int bl86x8_set_power(struct cavan_input_chip *chip, bool enable)
{
	if (enable)
	{
		cavan_io_reset_gpio_set_value(chip, 0);
		cavan_io_set_power_regulator(chip, true);

		msleep(50);

		cavan_io_reset_gpio_set_value(chip, 0);
		msleep(50);
		cavan_io_reset_gpio_set_value(chip, 1);

		msleep(200);
	}
	else
	{
		cavan_io_set_power_regulator(chip, false);
	}

	return 0;
}

static struct cavan_ts_touch_key bl86x8_touch_keys[] =
{
	{
		.code = KEY_MENU,
		.x = 72,
		.y = 850,
		.width = 40,
		.height = 90,
	},
	{
		.code = KEY_HOME,
		.x = 264,
		.y = 850,
		.width = 40,
		.height = 90,
	},
	{
		.code = KEY_BACK,
		.x = 408,
		.y = 850,
		.width = 40,
		.height = 90,
	}
};

static int bl86x8_readid(struct cavan_input_chip *chip)
{
	int ret;
	struct bl86x8_data_package package;

	pr_pos_info();

	ret = bl86x8_read_data_package(chip, &package);
	if (ret < 0)
	{
		pr_red_info("cavan_i2c_read_data");
		return ret;
	}

	cavan_input_print_memory(&package, sizeof(package));

	return 0;
}

static int bl86x8_input_chip_probe(struct cavan_input_chip *chip)
{
	int ret;

	struct cavan_bl86x8_device *dev;
	struct cavan_input_device *base_dev;
	struct cavan_ts_device *ts;

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (chip == NULL)
	{
		pr_red_info("kzalloc");
		return -ENOMEM;
	}

	cavan_input_chip_set_dev_data(chip, dev);

	// ============================================================

	ts = &dev->ts;
	ts->xmin = BL86X8_XAXIS_MIN;
	ts->xmax = BL86X8_XAXIS_MAX;
	ts->ymin = BL86X8_YAXIS_MIN;
	ts->ymax = BL86X8_YAXIS_MAX;
	ts->point_count = BL86X8_POINT_COUNT;
	ts->key_count = ARRAY_SIZE(bl86x8_touch_keys);
	ts->keys = bl86x8_touch_keys;

	base_dev = &ts->dev;
	base_dev->type = CAVAN_INPUT_DEVICE_TYPE_TOUCHSCREEN;
	base_dev->use_irq = true;
	base_dev->event_handler = bl86x8_ts_event_handler;

	ret = cavan_input_device_register(chip, base_dev);
	if (ret < 0)
	{
		pr_red_info("cavan_input_device_register");
		goto out_free_dev;
	}

	return 0;

out_free_dev:
	kfree(dev);
	return ret;
}

static void bl86x8_input_chip_remove(struct cavan_input_chip *chip)
{
	struct cavan_bl86x8_device *dev = cavan_input_chip_get_dev_data(chip);

	pr_pos_info();

	cavan_input_device_unregister(chip, &dev->ts.dev);
	kfree(dev);
}

static int bl86x8_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	struct cavan_input_chip *chip;

	pr_pos_info();

	chip = kzalloc(sizeof(*chip), GFP_KERNEL);
	if (chip == NULL)
	{
		pr_red_info("kzalloc");
		return -ENOMEM;
	}

	i2c_set_clientdata(client, chip);
	cavan_input_chip_set_bus_data(chip, client);

	chip->irq = client->irq;
	chip->irq_flags = IRQF_TRIGGER_FALLING;
	chip->name = "BL86X8";
	chip->devmask = 1 << CAVAN_INPUT_DEVICE_TYPE_TOUCHSCREEN | 1 << CAVAN_INPUT_DEVICE_TYPE_PROXIMITY;

	chip->set_power = bl86x8_set_power;
	chip->readid = bl86x8_readid;
	chip->probe = bl86x8_input_chip_probe;
	chip->remove = bl86x8_input_chip_remove;

	chip->read_data = cavan_input_read_data_i2c;
	chip->write_data = cavan_input_write_data_i2c;
	chip->read_register = cavan_input_read_register_i2c_smbus;
	chip->write_register = cavan_input_write_register_i2c_smbus;

	ret = cavan_input_chip_register(chip, &client->dev);
	if (ret < 0)
	{
		pr_red_info("cavan_input_chip_register");
		goto out_kfree_chip;
	}

	pr_green_info("BL86X8 Probe Complete");

	return 0;

out_kfree_chip:
	kfree(chip);
	return ret;
}

static int bl86x8_i2c_remove(struct i2c_client *client)
{
	struct cavan_input_chip *chip = i2c_get_clientdata(client);

	pr_pos_info();

	cavan_input_chip_unregister(chip);
	kfree(chip);

	return 0;
}

static const struct i2c_device_id bl86x8_ts_id_table[] =
{
	{BL86X8_DEVICE_NAME, 0},
	{}
};

#ifdef CONFIG_OF
static struct of_device_id bl86x8_match_table[] =
{
	{
		.compatible = "batterlift,bl86x8"
	},
	{}
};
#endif

static struct i2c_driver bl86x8_ts_driver =
{
	.probe = bl86x8_i2c_probe,
	.remove = bl86x8_i2c_remove,

	.id_table = bl86x8_ts_id_table,
	.driver =
	{
		.name = BL86X8_DEVICE_NAME,
		.owner = THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = bl86x8_match_table,
#endif
	}
};

static int __init bl86x8_ts_init(void)
{
	return i2c_add_driver(&bl86x8_ts_driver);
}

static void __exit bl86x8_ts_exit(void)
{
	i2c_del_driver(&bl86x8_ts_driver);
}

module_init(bl86x8_ts_init);
module_exit(bl86x8_ts_exit);

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Cavan BL86X8 TouchScreen Driver");
MODULE_LICENSE("GPL");
