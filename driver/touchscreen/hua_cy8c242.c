#include <linux/input/hua_ts.h>
#include <linux/input/hua_sensor.h>

#define HOME_AXIS_CENTER_130		0
#define HUA_SUPPORT_PROXIMITY		1

#define CY8C242_XAXIS_MIN			1
#define CY8C242_XAXIS_MAX			322
#define CY8C242_YAXIS_MIN			1
#define CY8C242_YAXIS_MAX			480
#define CY8C242_POINT_COUNT			2
#define CY8C242_DEVICE_NAME			"cy8c242_ts"

#define CY8C242_REG_DEVICE_MODE		0x00
#define CY8C242_REG_GEST_ID			0x01
#define CY8C242_REG_TD_STATUS		0x02
#define CY8C242_REG_POINTS			0x03
#define CY8C242_REG_POWER_MODE		0xA5

#define CY8C242_MODE_ACTIVE			0x00
#define CY8C242_MODE_MONITOR		0x01
#define CY8C242_MODE_STANDBY		0x02
#define CY8C242_MODE_HIBERNATE		0x03

#define CY8C242_FIRMWARE_MAX_SIZE	(50 << 10)
#define CY8C242_FIRMWARE_BLOCK_SIZE	128

#define CY8C242_BUILD_AXIS(h, l) \
	(((u16)((h) & 0x0F)) << 8 | (l))

#define CY8C242_EVENT_FLAG(xh) \
	((xh) >> 4)

#define CY8C242_TOUCH_ID(yh) \
	((yh) >> 4)

#pragma pack(1)
struct cy8c242_touch_point
{
	u8 xh;
	u8 xl;
	u8 yh;
	u8 yl;
	u16 reserved;
};

struct cy8c242_data_package
{
	u8 device_mode;
	u8 gest_id;
	u8 td_status;
	struct cy8c242_touch_point points[CY8C242_POINT_COUNT];
};
#pragma pack()

struct hua_cy8c242_device
{
	struct hua_ts_device ts;
	struct hua_sensor_device prox;
};

static inline ssize_t cy8c242_data_package(struct hua_input_chip *chip, struct cy8c242_data_package *package)
{
	return chip->read_data(chip, 0, package, sizeof(*package));
}

static const char *cy8c242_power_mode_tostring(int mode)
{
	switch (mode)
	{
	case CY8C242_MODE_ACTIVE:
		return "Active";
	case CY8C242_MODE_MONITOR:
		return "Monitor";
	case CY8C242_MODE_STANDBY:
		return "Standby";
	case CY8C242_MODE_HIBERNATE:
		return "Hibernate";
	default:
		return "Unknown";
	}
}

static int cy8c242_change_power_mode(struct hua_input_chip *chip, u8 mode, int retry)
{
	pr_bold_info("CY8C242 change power mode => %s", cy8c242_power_mode_tostring(mode));

	while (chip->write_register(chip, CY8C242_REG_POWER_MODE, mode) < 0 && retry--)
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

static int cy8c242_ts_event_handler(struct hua_input_chip *chip, struct hua_input_device *dev)
{
	int ret;
	int count;
	struct input_dev *input = dev->input;
	struct cy8c242_data_package package;
	struct cy8c242_touch_point *p, *p_end;

	ret = cy8c242_data_package(chip, &package);
	if (ret < 0)
	{
		pr_red_info("cy8c242_read_data_package");
		return ret;
	}

	count = package.td_status & 0x0F;
	if (count == 0)
	{
		hua_ts_mt_touch_release(input);
		return 0;
	}

	if (unlikely(count > CY8C242_POINT_COUNT))
	{
		// pr_red_info("Too much points = %d", count);
		count = CY8C242_POINT_COUNT;
	}

	for (p = package.points, p_end = p + count; p < p_end; p++)
	{
		hua_ts_report_mt_data(input, CY8C242_BUILD_AXIS(p->xh, p->xl), \
			CY8C242_BUILD_AXIS(p->yh, p->yl));
	}

	input_sync(input);

	return 0;
}

static int cy8c242_set_power(struct hua_input_chip *chip, bool enable)
{
	if (enable)
	{
		sprd_ts_reset_enable(false);
		sprd_ts_power_enable(true);

		msleep(50);

		sprd_ts_reset_enable(true);
		msleep(50);
		sprd_ts_reset_enable(false);

		msleep(200);

		return cy8c242_change_power_mode(chip, CY8C242_MODE_ACTIVE, 10);
	}
	else
	{
		int ret = cy8c242_change_power_mode(chip, CY8C242_MODE_HIBERNATE, 2);
		if (ret < 0)
		{
			return ret;
		}

		sprd_ts_power_enable(false);
	}

	return 0;
}

static struct hua_ts_touch_key cy8c242_touch_keys[] =
{
#if HOME_AXIS_CENTER_130
	{
		.code = KEY_MENU,
		.x = 20,
		.y = 530,
		.width = 40,
		.height = 60,
	},
	{
		.code = KEY_HOME,
		.x = 130,
		.y = 530,
		.width = 100,
		.height = 60,
	},
	{
		.code = KEY_BACK,
		.x = 260,
		.y = 530,
		.width = 120,
		.height = 60,
	}
#else
	{
		.code = KEY_MENU,
		.x = 30,
		.y = 530,
		.width = 60,
		.height = 60,
	},
	{
		.code = KEY_HOME,
		.x = 150,
		.y = 530,
		.width = 90,
		.height = 60,
	},
	{
		.code = KEY_BACK,
		.x = 270,
		.y = 530,
		.width = 100,
		.height = 60,
	}
#endif
};

static int cy8c242_readid(struct hua_input_chip *chip)
{
	int ret;
	char buff[18];

	pr_pos_info();

	ret = chip->read_data(chip, 0x00, buff, sizeof(buff));
	if (ret < 0)
	{
		pr_red_info("huamobile_i2c_read_data");
		return ret;
	}

	hua_input_print_memory(buff, ret);

	return 0;
}

static int cy8c242_enter_upgrade_mode(struct hua_input_chip *chip, int retry)
{
	while (retry--)
	{
		int ret;
		u8 value;

		sprd_ts_power_enable(false);
		sprd_ts_irq_output(0);
		msleep(10);
		sprd_ts_power_enable(true);
		msleep(50);
		sprd_ts_irq_output(-1);

		ret = chip->read_register(chip, 0x01, &value);
		if (ret < 0)
		{
			pr_red_info("huamobile_i2c_read_register");
			return ret;
		}

		pr_bold_info("value = 0x%02x", value);

		if (value == 0x11 || value == 0x10)
		{
			return 0;
		}
	}

	return -ETIMEDOUT;
}

static void cy8c242_ts_reset(void)
{
	sprd_ts_reset_enable(true);
	msleep(100);
	sprd_ts_reset_enable(false);
}

static int cy8c242_exit_upgrade_mode(struct hua_input_chip *chip, int retry)
{
	int ret;
	u8 value;

	msleep(200);

	while (retry--)
	{
		cy8c242_ts_reset();

		msleep(100);

		ret = chip->read_register(chip, 0x01, &value);
		if (ret < 0)
		{
			pr_red_info("huamobile_i2c_read_register");
			return ret;
		}

		pr_bold_info("value = 0x%02x", value);

		if (value != 0x11 && value != 0x10)
		{
			return 0;
		}
	}

	return -ETIMEDOUT;
}

static int cy8c242_byte2value(const char byte)
{
	switch (byte)
	{
	case '0' ... '9':
		return byte - '0';

	case 'a' ... 'f':
		return byte - 'a' + 10;

	case 'A' ... 'F':
		return byte - 'A' + 10;

	case 'p':
		return -2;

	default:
		return -1;
	}
}

static int cy8c242_firmware2data(const char *p, const char *end, u8 *datas)
{
	int ret;
	int count;

	for (count = 0; p < end; p++)
	{
		ret = cy8c242_byte2value(*p);
		if (ret < 0)
		{
			if (ret == -2)
			{
				break;
			}

			continue;
		}

		p++;
		datas[count++] = ret << 4 | cy8c242_byte2value(*p);
	}

	return count;
}

static int cy8c242_firmware_write_data(struct hua_input_chip *chip, const char *buff, size_t size)
{
	int ret;
	u32 delay;
	const char *p, *end_line, *end_file;
	u8 datas[32];

	pr_pos_info();

	if (size < 100)
	{
		pr_red_info("file size too small");
		return 0;
	}

	for (p = buff, end_file = p + size; p < end_file; p = end_line + 1)
	{
		if (((p - buff) & 0x0F) == 0)
		{
			pr_bold_info("Remain %d byte", end_file - p);
		}

		while (BYTE_IS_SPACE(*p))
		{
			if (p < end_file)
			{
				p++;
			}
			else
			{
				return 0;
			}
		}

		for (end_line = p; end_line < end_file; end_line++)
		{
			if (BYTE_IS_LF(*end_line))
			{
				break;
			}
		}

		if (p == end_line)
		{
			continue;
		}

		switch (*p)
		{
		case 'w':
		case 'W':
			ret = cy8c242_firmware2data(p + 2, end_line, datas);
			ret = i2c_master_send(chip->bus_data, datas + 1, ret - 1);
			if (ret < 0)
			{
				pr_red_info("i2c_master_send");
				return ret;
			}
			break;

		case 'r':
		case 'R':
			ret = i2c_master_recv(chip->bus_data, datas, 3);
			if (ret < 0)
			{
				pr_red_info("i2c_master_recv");
				return ret;
			}

			if (datas[2] != 0x20)
			{
				pr_red_info("datas[2] = 0x%02x", datas[2]);
				return -EFAULT;
			}
			break;

		case '[':
			ret = sscanf(p, "[delay=%d]", &delay);
			if (ret == 1)
			{
				msleep(delay);
			}
			else
			{
				pr_bold_info("unknown delay");
			}
			break;

		case '#':
			pr_green_info("Comment Line");
			break;

		default:
			pr_red_info("unknown char 0x%02x at %d", *p, p - buff);
		}
	}

	return 0;
}

static int cy8c242_firmware_upgrade(struct hua_input_chip *chip, const void *buff, size_t size)
{
	int ret;

	pr_pos_info();

	pr_bold_info("buff size = %d", size);

	ret = cy8c242_enter_upgrade_mode(chip, 10);
	if (ret < 0)
	{
		pr_red_info("cy8c242_enter_upgrade_mode");
		return ret;
	}

	ret = cy8c242_firmware_write_data(chip, buff, size);
	if (ret < 0)
	{
		pr_red_info("cy8c242_firmware_write_data");
		return ret;
	}

	ret = cy8c242_exit_upgrade_mode(chip, 10);
	if (ret < 0)
	{
		pr_red_info("cy8c242_exit_upgrade_mode");
		return ret;
	}

	cy8c242_readid(chip);

	return 0;
}

static int cy8c242_calibration(struct hua_input_device *dev, const void *buff, size_t size)
{
	int ret;
	u8 value;
	int i;
	struct hua_input_chip *chip = dev->chip;
	struct i2c_client *client = hua_input_chip_get_bus_data(chip);
	char data[2] = {0x1C, 0x01};

	pr_pos_info();

	for (i = 0; i < 5; i++)
	{
		cy8c242_ts_reset();

		msleep(100);

		ret = i2c_master_send(client, data, sizeof(data));
		if (ret < 0)
		{
			pr_red_info("i2c_master_send");
			return ret;
		}

		msleep(1000);

		ret = i2c_master_send(client, data, 1);
		if (ret < 0)
		{
			pr_red_info("i2c_master_send");
			return ret;
		}

		ret = i2c_master_recv(client, &value, 1);
		if (ret < 0)
		{
			pr_red_info("i2c_master_recv");
			return ret;
		}

		pr_bold_info("value[%d] = %d", i, value);

		if (value == 0)
		{
			return 0;
		}
	}

	return -EFAULT;
}

static int cy8c242_proximity_event_handler(struct hua_input_chip *chip, struct hua_input_device *dev)
{
	int ret;
	u8 value;

	ret = chip->read_register(chip, 0x13, &value);
	if (ret < 0)
	{
		pr_red_info("dev->read_register");
		return ret;
	}

	hua_sensor_report_value(dev->input, value == 0);

	return 0;
}

static int cy8c242_proximity_set_enable(struct hua_input_device *dev, bool enable)
{
	u8 value = enable ? 1 : 0;
	struct hua_input_chip *chip = dev->chip;

	pr_func_info("value = %d", value);

	return chip->write_register(chip, 0x12, value);
}

static int cy8c242_input_chip_probe(struct hua_input_chip *chip)
{
	int ret;

	struct hua_cy8c242_device *dev;
	struct hua_input_device *base_dev;
	struct hua_ts_device *ts;
	struct hua_sensor_device *prox;

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (chip == NULL)
	{
		pr_red_info("kzalloc");
		return -ENOMEM;
	}

	hua_input_chip_set_dev_data(chip, dev);

	// ============================================================

	ts = &dev->ts;
	ts->xmin = CY8C242_XAXIS_MIN;
	ts->xmax = CY8C242_XAXIS_MAX;
	ts->ymin = CY8C242_YAXIS_MIN;
	ts->ymax = CY8C242_YAXIS_MAX;
	ts->point_count = CY8C242_POINT_COUNT;
	ts->key_count = ARRAY_SIZE(cy8c242_touch_keys);
	ts->keys = cy8c242_touch_keys;

	base_dev = &ts->dev;
	base_dev->type = HUA_INPUT_DEVICE_TYPE_TOUCHSCREEN;
	base_dev->use_irq = true;
	base_dev->event_handler = cy8c242_ts_event_handler;
	base_dev->calibration = cy8c242_calibration;

	ret = hua_input_device_register(chip, base_dev);
	if (ret < 0)
	{
		pr_red_info("hua_input_device_register");
		goto out_free_dev;
	}

	// ============================================================

	prox = &dev->prox;
	prox->min_delay = 20;
	prox->max_range = 1;
	prox->resolution = 1;
	prox->power_consume = 0;

	base_dev = &prox->dev;
	base_dev->type = HUA_INPUT_DEVICE_TYPE_PROXIMITY;
	base_dev->poll_delay = 200;
	base_dev->set_enable = cy8c242_proximity_set_enable;
	base_dev->event_handler = cy8c242_proximity_event_handler;

	ret = hua_input_device_register(chip, base_dev);
	if (ret < 0)
	{
		pr_red_info("hua_input_device_register");
		goto out_hua_input_device_unregister_ts;
	}

	return 0;

out_hua_input_device_unregister_ts:
	hua_input_device_unregister(chip, &ts->dev);
out_free_dev:
	kfree(dev);
	return ret;
}

static void cy8c242_input_chip_remove(struct hua_input_chip *chip)
{
	struct hua_cy8c242_device *dev = hua_input_chip_get_dev_data(chip);

	pr_pos_info();

	hua_input_device_unregister(chip, &dev->prox.dev);
	pr_pos_info();
	hua_input_device_unregister(chip, &dev->ts.dev);
	pr_pos_info();
	kfree(dev);
	pr_pos_info();
}

static int cy8c242_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	struct hua_input_chip *chip;

	pr_pos_info();

	chip = kzalloc(sizeof(*chip), GFP_KERNEL);
	if (chip == NULL)
	{
		pr_red_info("kzalloc");
		return -ENOMEM;
	}

	i2c_set_clientdata(client, chip);
	hua_input_chip_set_bus_data(chip, client);

	chip->irq = client->irq;
	chip->irq_flags = IRQF_TRIGGER_FALLING;
	chip->name = "CY8C242";
	chip->devmask = 1 << HUA_INPUT_DEVICE_TYPE_TOUCHSCREEN | 1 << HUA_INPUT_DEVICE_TYPE_PROXIMITY;

	chip->set_power = cy8c242_set_power;
	chip->readid = cy8c242_readid;
	chip->probe = cy8c242_input_chip_probe;
	chip->remove = cy8c242_input_chip_remove;

	chip->read_data = hua_input_read_data_i2c;
	chip->write_data = hua_input_write_data_i2c;
	chip->read_register = hua_input_read_register_i2c_smbus;
	chip->write_register = hua_input_write_register_i2c_smbus;
	chip->firmware_size = KB(180);
	chip->firmware_upgrade = cy8c242_firmware_upgrade;

	ret = hua_input_chip_register(chip);
	if (ret < 0)
	{
		pr_red_info("hua_input_chip_register");
		goto out_kfree_chip;
	}

	pr_green_info("CY8C242 Probe Complete");

	return 0;

out_kfree_chip:
	kfree(chip);
	return ret;
}

static int cy8c242_i2c_remove(struct i2c_client *client)
{
	struct hua_input_chip *chip = i2c_get_clientdata(client);

	pr_pos_info();

	hua_input_chip_unregister(chip);
	kfree(chip);

	return 0;
}

static const struct i2c_device_id cy8c242_ts_id_table[] =
{
	{CY8C242_DEVICE_NAME, 0},
	{}
};

static struct i2c_driver cy8c242_ts_driver =
{
	.probe = cy8c242_i2c_probe,
	.remove = cy8c242_i2c_remove,

	.id_table = cy8c242_ts_id_table,
	.driver =
	{
		.name = CY8C242_DEVICE_NAME,
		.owner = THIS_MODULE,
	}
};

static int __init cy8c242_ts_init(void)
{
	return i2c_add_driver(&cy8c242_ts_driver);
}

static void __exit cy8c242_ts_exit(void)
{
	i2c_del_driver(&cy8c242_ts_driver);
}

module_init(cy8c242_ts_init);
module_exit(cy8c242_ts_exit);

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Huamobile CY8C242 TouchScreen Driver");
MODULE_LICENSE("GPL");
