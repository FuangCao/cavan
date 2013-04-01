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

#define STK8313_BUILD_WORD(h, l) \
	((short)((h) << 8 | (l)))

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
#if 0
	int ret;
	struct i2c_client *client = chip->bus_data;

	for (client->addr = 1; client->addr < 0x7F; client->addr++)
	{
		ret = hua_input_test_i2c(client);
		if (ret < 0)
		{
			pr_red_info("address = 0x%02x", client->addr);
		}
		else
		{
			pr_green_info("address = 0x%02x", client->addr);
			return 0;
		}
	}

	return -EFAULT;
#else
	return 0;
#endif
}

static int stk8313_sensor_chip_set_power(struct hua_input_chip *chip, bool enable)
{
	int ret;
	u8 value;

	ret = chip->read_register(chip, REG_MODE, &value);
	if (ret < 0)
	{
		pr_red_info("read_register");
		return ret;
	}

	if (enable)
	{
		value |= 1;
	}
	else
	{
		value &= ~1;
	}

	pr_bold_info("value = 0x%02x", value);

	return chip->write_register(chip, REG_MODE, value);
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

	chip->set_power(chip, false);
	ret = chip->write_register(chip, REG_SR, value);
	chip->set_power(chip, true);

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

	x = STK8313_BUILD_WORD(package.xh, package.xl) >> 4;
	y = STK8313_BUILD_WORD(package.yh, package.yl) >> 4;
	z = STK8313_BUILD_WORD(package.zh, package.zl) >> 4;

	pr_bold_info("x = %d, y = %d, z = %d", x, y, z);

	hua_sensor_report_vector(dev->input, x, -y, z);

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
	sensor->max_range = 4;
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
	{REG_MODE, 0x00},
	{REG_SWRST, 0x00},
	{REG_PDET, 0x07 << 5},
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
	chip->set_power = stk8313_sensor_chip_set_power;

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
	{"stk8313", 0}, {"adxl34x", 0}, {}
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
