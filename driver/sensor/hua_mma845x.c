#include <huamobile/hua_sensor.h>
#include <huamobile/hua_i2c.h>

#define REG_DATA_START		0x01
#define REG_CHIP_ID			0x0D
#define REG_XYZ_DATA_CFG	0x0E
#define REG_CTRL1			0x2A
#define REG_CTRL2			0x2B
#define REG_CTRL3			0x2C
#define REG_CTRL4			0x2D
#define REG_CTRL5			0x2E

#define MMA845X_BUILD_WORD(h, l) \
	((short)((h) << 8 | (l)))

#ifdef CONFIG_ARCH_SC8810
#include <mach/eic.h>
#endif

#pragma pack(1)
struct mma845x_data_package
{
	u8 xh, xl;
	u8 yh, yl;
	u8 zh, zl;
};
#pragma pack()

struct mma845x_rate_map_node
{
	u8 value;
	u32 delay;
};

static struct mma845x_rate_map_node mma845x_rate_map[] =
{
	{0x00, 2},
	{0x01, 3},
	{0x02, 5},
	{0x03, 10},
	{0x04, 20},
	{0x05, 80},
	{0x06, 160},
	{0x07, 640},
};

static int mma845x_sensor_chip_readid(struct hua_input_chip *chip)
{
	int ret;
	u8 id;

	ret = chip->read_register(chip, REG_CHIP_ID, &id);
	if (ret < 0)
	{
		pr_red_info("hua_sensor_i2c_read_register");
		return ret;
	}

	pr_bold_info("Device ID = 0x%02x", id);

	return 0;
}

static int mma845x_sensor_chip_set_active(struct hua_input_chip *chip, bool enable)
{
	int ret;
	u8 value;

	if (enable == false)
	{
		return 0;
	}

	ret = chip->read_register(chip, REG_CTRL2, &value);
	if (ret < 0)
	{
		pr_red_info("read_register");
		return ret;
	}

	ret = chip->write_register(chip, REG_CTRL2, value | 1 << 6);
	if (ret < 0)
	{
		pr_red_info("write_register");
		return ret;
	}

	msleep(2);

	return 0;
}

static int mma845x_acceleration_set_enable(struct hua_input_device *dev, bool enable)
{
	int ret;
	u8 value;
	struct hua_input_chip *chip = dev->chip;

	ret = chip->read_register(chip, REG_CTRL1, &value);
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

	return chip->write_register(chip, REG_CTRL1, value);
}

static int mma845x_acceleration_set_delay(struct hua_input_device *dev, unsigned int delay)
{
	int ret;
	u8 value;
	struct mma845x_rate_map_node *p;
	struct hua_input_chip *chip = dev->chip;

	ret = chip->read_register(chip, REG_CTRL1, &value);
	if (ret < 0)
	{
		pr_red_info("read_register");
		return ret;
	}

	for (p = mma845x_rate_map + ARRAY_SIZE(mma845x_rate_map) - 1; p > mma845x_rate_map && p->delay > delay; p--);

	value = (value & (~(0x07 << 3))) | p->value << 3;

	pr_bold_info("value = 0x%02x, rate = 0x%02x", value, p->value);

	mma845x_acceleration_set_enable(dev, false);

	ret = chip->write_register(chip, REG_CTRL1, value);

	mma845x_acceleration_set_enable(dev, true);

	return ret;
}

static int mma845x_acceleration_event_handler(struct hua_input_chip *chip, struct hua_input_device *dev)
{
	int ret;
	short x, y, z;
	struct mma845x_data_package package;

	ret = chip->read_data(chip, REG_DATA_START, &package, sizeof(package));
	if (ret < 0)
	{
		pr_red_info("read_data");
		return ret;
	}

	x = MMA845X_BUILD_WORD(package.xh, package.xl) >> 4;
	y = MMA845X_BUILD_WORD(package.yh, package.yl) >> 4;
	z = MMA845X_BUILD_WORD(package.zh, package.zl) >> 4;

	hua_sensor_report_vector(dev->input, y, x, z);

	return 0;
}

static int mma845x_input_chip_probe(struct hua_input_chip *chip)
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
	dev->name = "MMA845X Three-Axis Digital Accelerometer";
	dev->fuzz = 0;
	dev->flat = 0;
	dev->use_irq = false;
	dev->type = HUA_INPUT_DEVICE_TYPE_ACCELEROMETER;
	dev->poll_delay = 200;
	dev->set_enable = mma845x_acceleration_set_enable;
	dev->set_delay = mma845x_acceleration_set_delay;
	dev->event_handler = mma845x_acceleration_event_handler;

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

static void mma845x_input_chip_remove(struct hua_input_chip *chip)
{
	struct hua_sensor_device *sensor = hua_input_chip_get_dev_data(chip);

	pr_pos_info();

	hua_input_device_unregister(chip, &sensor->dev);
	kfree(sensor);
}

static struct hua_input_init_data mma845x_init_data[] =
{
	{REG_CTRL1,	0},
	{REG_CTRL2, 1},
	{REG_CTRL3, 0},
	{REG_CTRL4, 0},
	{REG_CTRL5, 0},
	{REG_XYZ_DATA_CFG, 2},
};

static int mma845x_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
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
	chip->name = "MMA845X";
	chip->devmask = 1 << HUA_INPUT_DEVICE_TYPE_ACCELEROMETER;
	chip->init_data = mma845x_init_data;
	chip->init_data_size = ARRAY_SIZE(mma845x_init_data);
	chip->read_data = hua_input_read_data_i2c;
	chip->write_data = hua_input_write_data_i2c;
	chip->write_register = hua_input_write_register_i2c_smbus;
	chip->readid = mma845x_sensor_chip_readid;
	chip->set_active = mma845x_sensor_chip_set_active;

	chip->probe = mma845x_input_chip_probe;
	chip->remove = mma845x_input_chip_remove;

	ret = hua_input_chip_register(chip, &client->dev);
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

static int mma845x_i2c_remove(struct i2c_client *client)
{
	struct hua_input_chip *chip = i2c_get_clientdata(client);

	pr_pos_info();

	hua_input_chip_unregister(chip);

	kfree(chip);

	return 0;
}

static const struct i2c_device_id mma845x_id[] =
{
	{"mma845x", 0}, {}
};

#ifdef CONFIG_OF
static struct of_device_id mma845x_match_table[] =
{
	{
		.compatible = "freescale,mma845x"
	},
	{}
};
#endif

MODULE_DEVICE_TABLE(i2c, mma845x_id);

static struct i2c_driver mma845x_driver =
{
	.driver =
	{
		.name = "mma845x",
		.owner = THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = mma845x_match_table,
#endif
	},

	.probe = mma845x_i2c_probe,
	.remove = mma845x_i2c_remove,
	.id_table = mma845x_id,
};

static int __init hua_mma845x_init(void)
{
	pr_pos_info();

	return i2c_add_driver(&mma845x_driver);
}

static void __exit hua_mma845x_exit(void)
{
	pr_pos_info();

	i2c_del_driver(&mma845x_driver);
}

module_init(hua_mma845x_init);
module_exit(hua_mma845x_exit);

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Huamobile MMA845X Three-Axis Digital Accelerometer Driver");
MODULE_LICENSE("GPL");
