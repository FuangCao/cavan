#include <linux/input/hua_sensor.h>

enum stk8313_register_map
{
	REG_XOUT1 = 0x00,
	REG_XOUT2,
	REG_YOUT1,
	REG_YOUT2,
	REG_ZOUT1,
	REG_ZOUT2,
	REG_TILT,
	REG_SRST,
	REG_INTSU = 0x09,
	REG_MODE,
	REG_SR,
	REG_PDET,
	REG_OFSX = 0x0F,
	REG_OFSY,
	REG_OFSZ,
	REG_PLAT,
	REG_PWIN,
	REG_FTH,
	REG_FTM,
	REG_STH,
	REG_ISTMP,
	REG_INTMAP,
	REG_SWRST = 0x20
};

#ifdef CONFIG_ARCH_SC8810
#include <mach/eic.h>
#endif

#pragma pack(1)
struct stk8313_data_package
{
	u8 xh, xl;
	u8 yh, yl;
	u8 zh, zl;
};
#pragma pack()

struct stk8313_rate_map_node
{
	u8 value;
	u32 delay;
};

static struct stk8313_rate_map_node stk8313_rate_map[] =
{
	{0x00, 3},
	{0x01, 5},
	{0x02, 10},
	{0x03, 20},
	{0x04, 40},
	{0x05, 80},
	{0x06, 160},
	{0x07, 320},
};

static int stk8313_sensor_chip_readid(struct hua_input_chip *chip)
{
	int ret;
	u8 value;

	ret = chip->read_register(chip, REG_MODE, &value);
	if (ret < 0)
	{
		pr_red_info("read_register REG_MODE");
		return ret;
	}

	pr_bold_info("REG_MODE = 0x%02x", value);

	return 0;
}

static int stk8313_sensor_chip_read_reg24(struct hua_input_chip *chip, u8 *value, int retry)
{
	int ret;
	u8 buff[2];

	while (retry--)
	{
		ret = chip->write_register(chip, 0x3D, 0x70);
		if (ret < 0)
		{
			pr_red_info("write_register");
			return ret;
		}

		ret = chip->write_register(chip, 0x3F, 0x02);
		if (ret < 0)
		{
			pr_red_info("write_register");
			return ret;
		}

		ret = chip->read_data(chip, 0x3E, buff, 2);
		if (ret < 0)
		{
			pr_red_info("read_register");
			return ret;
		}

		pr_bold_info("buff = 0x%02x 0x%02x", buff[0], buff[1]);

		if (buff[1] & (1 << 7))
		{
			*value = buff[0];
			break;
		}

		msleep(1);
	}

	return retry;
}

static int stk8313_sensor_chip_set_vd(struct hua_input_chip *chip, int retry)
{
	int ret;
	u8 reg24, value;

	pr_pos_info();

	ret = stk8313_sensor_chip_read_reg24(chip, &reg24, 10);
	if (ret < 0)
	{
		pr_red_info("stk8313_sensor_chip_read_reg24");
		return ret;
	}

	if (reg24 == 0)
	{
		return 0;
	}

	ret = chip->write_register(chip, 0x24, reg24);
	if (ret < 0)
	{
		pr_red_info("write_register");
		return ret;
	}

	ret = chip->read_register(chip, 0x24, &value);
	if (ret < 0)
	{
		pr_red_info("read_register");
		return ret;
	}

	pr_bold_info("value = 0x%02x, reg24 = 0x%02x", value, reg24);

	if (value != reg24)
	{
		pr_red_info("value != reg24");
		return -EINVAL;
	}

	return 0;
}

static int stk8313_sensor_chip_set_active(struct hua_input_chip *chip, bool enable)
{
	int ret;
	u8 value;

	pr_pos_info();

	ret = chip->read_register(chip, REG_MODE, &value);
	if (ret < 0)
	{
		pr_red_info("read_register");
		return ret;
	}

	value &= 0xF8;

	if (enable)
	{
		value |= 0x01;
	}

	pr_bold_info("value = 0x%02x", value);

	ret = chip->write_register(chip, REG_MODE, value);
	if (ret < 0)
	{
		pr_red_info("write_register");
		return ret;
	}

	if (enable == false)
	{
		return 0;
	}

	msleep(1);

	return stk8313_sensor_chip_set_vd(chip, 10);
}

static int stk8313_acceleration_set_delay(struct hua_input_device *dev, unsigned int delay)
{
	int ret;
	u8 value;
	struct stk8313_rate_map_node *p;
	struct hua_input_chip *chip = dev->chip;

	ret = chip->read_register(chip, REG_SR, &value);
	if (ret < 0)
	{
		pr_red_info("read_register");
		return ret;
	}

	for (p = stk8313_rate_map + ARRAY_SIZE(stk8313_rate_map) - 1; p > stk8313_rate_map && p->delay > delay; p--);

	value = (value & (~(0x07))) | p->value;

	pr_bold_info("value = 0x%02x, rate = 0x%02x", value, p->value);

	stk8313_sensor_chip_set_active(chip, false);
	ret = chip->write_register(chip, REG_SR, value);
	stk8313_sensor_chip_set_active(chip, true);

	return ret;
}

static int stk8313_acceleration_event_handler(struct hua_input_chip *chip, struct hua_input_device *dev)
{
	int ret;
	short x, y, z;
	struct stk8313_data_package package;

	ret = chip->read_data(chip, REG_XOUT1, &package, sizeof(package));
	if (ret < 0)
	{
		pr_red_info("read_data");
		return ret;
	}

	x = BUILD_WORD(package.xh, package.xl) >> 4;
	y = BUILD_WORD(package.yh, package.yl) >> 4;
	z = BUILD_WORD(package.zh, package.zl) >> 4;

	pr_bold_info("[%d, %d, %d]", x, y, z);

	hua_sensor_report_vector(dev->input, -x, -y, z);

	return 0;
}

static int stk8313_input_chip_probe(struct hua_input_chip *chip)
{
	int ret;
	struct hua_sensor_device *sensor;
	struct hua_input_device *dev;

	pr_pos_info();

	sensor = kzalloc(sizeof(*sensor), GFP_KERNEL);
	if (sensor == NULL)
	{
		pr_red_info("kzalloc");
		return -ENOMEM;
	}

	hua_input_chip_set_dev_data(chip, sensor);

	sensor->min_delay = 20;
	sensor->max_range = 16;
	sensor->resolution = 4096;
	sensor->power_consume = 145;

	dev = &sensor->dev;
	dev->name = "Three-Axis Digital Accelerometer";
	dev->fuzz = 0;
	dev->flat = 0;
	dev->use_irq = false;
	dev->type = HUA_INPUT_DEVICE_TYPE_ACCELEROMETER;
	dev->poll_delay = 200;
	dev->set_delay = stk8313_acceleration_set_delay;
	dev->event_handler = stk8313_acceleration_event_handler;

	ret = hua_input_device_register(chip, dev);
	if (ret < 0)
	{
		pr_red_info("hua_input_device_register");
		goto out_kfree_sensor;
	}

	return 0;

out_kfree_sensor:
	kfree(sensor);
	return ret;
}

static void stk8313_input_chip_remove(struct hua_input_chip *chip)
{
	struct hua_sensor_device *sensor = hua_input_chip_get_dev_data(chip);

	pr_pos_info();

	hua_input_device_unregister(chip, &sensor->dev);
	kfree(sensor);
}

static struct hua_input_init_data stk8313_init_data[] =
{
	{REG_SWRST, 0x00, 1},
	{REG_MODE, 0xC0},
	{REG_SR, 0x03},
	{REG_STH, 0x82},
	{REG_OFSX, 0x00},
	{REG_OFSY, 0x00},
	{REG_OFSZ, 0x00}
};

static int stk8313_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	struct hua_input_chip *chip;

	pr_pos_info();

	chip = kzalloc(sizeof(*chip), GFP_KERNEL);
	if (chip == NULL)
	{
		pr_red_info("sensor == NULL");
		return -ENOMEM;
	}

	i2c_set_clientdata(client, chip);
	hua_input_chip_set_bus_data(chip, client);

	chip->irq = -1;
	chip->name = "STK8313";
	chip->devmask = 1 << HUA_INPUT_DEVICE_TYPE_ACCELEROMETER;
	chip->init_data = stk8313_init_data;
	chip->init_data_size = ARRAY_SIZE(stk8313_init_data);
	chip->read_data = hua_input_read_data_i2c;
	chip->write_data = hua_input_write_data_i2c;
	chip->write_register = hua_input_write_register_i2c_smbus;
	chip->readid = stk8313_sensor_chip_readid;
	chip->set_active = stk8313_sensor_chip_set_active;

	chip->probe = stk8313_input_chip_probe;
	chip->remove = stk8313_input_chip_remove;

	ret = hua_input_chip_register(chip);
	if (ret < 0)
	{
		pr_red_info("hua_input_chip_register");
		goto out_kfree_sensor;
	}

	return 0;

out_kfree_sensor:
	kfree(chip);
	return ret;
}

static int stk8313_i2c_remove(struct i2c_client *client)
{
	struct hua_input_chip *chip = i2c_get_clientdata(client);

	pr_pos_info();

	hua_input_chip_unregister(chip);

	kfree(chip);

	return 0;
}

static const struct i2c_device_id stk8313_id[] =
{
	{"stk8313", 0}, {}
};

MODULE_DEVICE_TABLE(i2c, stk8313_id);

static struct i2c_driver stk8313_driver =
{
	.driver =
	{
		.name = "stk8313",
		.owner = THIS_MODULE,
	},

	.probe = stk8313_i2c_probe,
	.remove = stk8313_i2c_remove,
	.id_table = stk8313_id,
};

static int __init hua_stk8313_init(void)
{
	pr_pos_info();

	return i2c_add_driver(&stk8313_driver);
}

static void __exit hua_stk8313_exit(void)
{
	pr_pos_info();

	i2c_del_driver(&stk8313_driver);
}

module_init(hua_stk8313_init);
module_exit(hua_stk8313_exit);

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Huamobile stk8313 Three-Axis Digital Accelerometer Driver");
MODULE_LICENSE("GPL");
