#include <cavan/cavan_sensor.h>
#include <cavan/cavan_i2c.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>

#define BMA2XX_DEVICE_NAME			"bma2xx"

enum bma2xx_register_map
{
	REG_CHIP_ID = 0x00,
	REG_VERSION,
	REG_X_AXIS_LSB,
	REG_X_AXIS_MSB,
	REG_Y_AXIS_LSB,
	REG_Y_AXIS_MSB,
	REG_Z_AXIS_LSB,
	REG_Z_AXIS_MSB,
	REG_TEMP_RD,
	REG_STATUS1,
	REG_STATUS2,
	REG_STATUS_TAP_SLOPE,
	REG_STATUS_ORIENT_HIGH,
	REG_RANGE_SEL = 0x0F,
	REG_BW_SEL,
	REG_MODE_CTRL,
	REG_LOW_NOISE_CTRL,
	REG_DATA_CTRL,
	REG_RESET,
	REG_INT_ENABLE1 = 0x16,
	REG_INT_ENABLE2,
	REG_INT1_PAD_SEL = 0x19,
	REG_INT_DATA_SEL,
	REG_INT2_PAD_SEL,
	REG_INT_SRC = 0x1E,
	REG_INT_SET = 0x20,
	REG_INT_CTRL,
	REG_LOW_DURN,
	REG_LOW_THRES,
	REG_LOW_HIGH_HYST,
	REG_HIGH_DURN,
	REG_HIGH_THRES,
	REG_SLOPE_DURN,
	REG_SLOPE_THRES,
	REG_TAP_PARAM = 0x2A,
	REG_TAP_THRES,
	REG_ORIENT_PARAM,
	REG_THETA_BLOCK,
	REG_THETA_FLAT,
	REG_FLAT_HOLD_TIME,
	REG_STATUS_LOW_POWER = 0x31,
	REG_SELF_TEST,
	REG_EEPROM_CTRL,
	REG_SERIAL_CTRL,
	REG_CTRL_UNLOCK,
	REG_OFFSET_CTRL,
	REG_OFFSET_PARAMS,
	REG_OFFSET_FILT_X,
	REG_OFFSET_FILT_Y,
	REG_OFFSET_FILT_Z,
	REG_OFFSET_UNFILT_X,
	REG_OFFSET_UNFILT_Y,
	REG_OFFSET_UNFILT_Z,
	REG_SPARE_0,
	REG_SPARE_1
};

#pragma pack(1)
struct bma2xx_data_package
{
	s16 x;
	s16 y;
	s16 z;
};
#pragma pack()

struct bma2xx_rate_map_node
{
	u8 value;
	u32 delay;
};

static struct bma2xx_rate_map_node bma2xx_rate_map[] =
{
	{0x0F, 1},
	{0x0E, 2},
	{0x0D, 4},
	{0x0C, 8},
	{0x0B, 16},
	{0x0A, 32},
	{0x09, 64},
	{0x08, 128},
};

static int bma2xx_sensor_chip_readid(struct cavan_input_chip *chip)
{
	int ret;
	u8 value;

	ret = chip->read_register(chip, REG_CHIP_ID, &value);
	if (ret < 0)
	{
		pr_red_info("read_register REG_MODE");
		return ret;
	}

	pr_bold_info("REG_CHIP_ID = 0x%02x", value);

	return 0;
}

static int bma2xx_sensor_chip_set_active(struct cavan_input_chip *chip, bool enable)
{
	int ret;
	u8 value;

	pr_pos_info();

	ret = chip->read_register(chip, REG_MODE_CTRL, &value);
	if (ret < 0)
	{
		pr_red_info("read_register");
		return ret;
	}

	if (enable)
	{
		value &= ~(3 << 6);
	}
	else
	{
		value |= 1 << 7;
	}

	pr_bold_info("value = 0x%02x", value);

	ret = chip->write_register(chip, REG_MODE_CTRL, value);
	if (ret < 0)
	{
		pr_red_info("write_register");
		return ret;
	}

	return 0;
}

static int bma2xx_acceleration_set_delay(struct cavan_input_device *dev, unsigned int delay)
{
	int ret;
	u8 value;
	struct bma2xx_rate_map_node *p;
	struct cavan_input_chip *chip = dev->chip;

	ret = chip->read_register(chip, REG_BW_SEL, &value);
	if (ret < 0)
	{
		pr_red_info("read_register");
		return ret;
	}

	for (p = bma2xx_rate_map + ARRAY_SIZE(bma2xx_rate_map) - 1; p > bma2xx_rate_map && p->delay > delay; p--);

	value = (value & (~(0x1F))) | p->value;

	pr_bold_info("value = 0x%02x, rate = 0x%02x", value, p->value);

	ret = chip->write_register(chip, REG_BW_SEL, value);

	return ret;
}

static int bma2xx_acceleration_event_handler(struct cavan_input_chip *chip, struct cavan_input_device *dev)
{
	int ret;
	struct bma2xx_data_package package;

	ret = chip->read_data(chip, REG_X_AXIS_LSB, &package, sizeof(package));
	if (ret < 0)
	{
		pr_red_info("read_data");
		return ret;
	}

	cavan_sensor_report_vector(dev->input, package.x >> 6, package.y >> 6, package.z >> 6);

	return 0;
}

static int bma2xx_input_chip_probe(struct cavan_input_chip *chip)
{
	int ret;
	struct cavan_sensor_device *sensor;
	struct cavan_input_device *dev;

	pr_pos_info();

	sensor = kzalloc(sizeof(*sensor), GFP_KERNEL);
	if (sensor == NULL)
	{
		pr_red_info("kzalloc");
		return -ENOMEM;
	}

	cavan_input_chip_set_dev_data(chip, sensor);

	sensor->max_range = 4;
	sensor->resolution = 1024;
	sensor->power_consume = 145;

	dev = &sensor->dev;
	dev->name = "BMA2XX Three-Axis Digital Accelerometer";
	dev->fuzz = 4;
	dev->flat = 4;
	dev->use_irq = false;
	dev->type = CAVAN_INPUT_DEVICE_TYPE_ACCELEROMETER;
	dev->min_delay = 20;
	dev->poll_delay = 200;
	dev->set_delay = bma2xx_acceleration_set_delay;
	dev->event_handler = bma2xx_acceleration_event_handler;

	ret = cavan_input_device_register(chip, dev);
	if (ret < 0)
	{
		pr_red_info("cavan_input_device_register");
		goto out_kfree_sensor;
	}

	return 0;

out_kfree_sensor:
	kfree(sensor);
	return ret;
}

static void bma2xx_input_chip_remove(struct cavan_input_chip *chip)
{
	struct cavan_sensor_device *sensor = cavan_input_chip_get_dev_data(chip);

	pr_pos_info();

	cavan_input_device_unregister(chip, &sensor->dev);
	kfree(sensor);
}

static struct cavan_input_init_data bma2xx_init_data[] =
{
	{REG_BW_SEL, 0x0C},
	{REG_RANGE_SEL, 0x00},
#if 0
	{REG_INT_CTRL, 0x01},
	{REG_INT_ENABLE1, 1 << 5 | 1 << 6 | 1 << 7},
	{REG_INT1_PAD_SEL, 0xFF},
#else
	{REG_INT_CTRL, 0x00},
	{REG_INT_ENABLE1, 0x00},
	{REG_INT1_PAD_SEL, 0x00},
#endif
};

static int bma2xx_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
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

	chip->irq = -1;
	chip->name = "BMA2XX";
	chip->devmask = 1 << CAVAN_INPUT_DEVICE_TYPE_ACCELEROMETER;
	chip->init_data = bma2xx_init_data;
	chip->init_data_size = ARRAY_SIZE(bma2xx_init_data);
	chip->read_data = cavan_input_read_data_i2c;
	chip->write_data = cavan_input_write_data_i2c;
	chip->write_register = cavan_input_write_register_i2c_smbus;
	chip->readid = bma2xx_sensor_chip_readid;
	chip->set_active = bma2xx_sensor_chip_set_active;

	chip->probe = bma2xx_input_chip_probe;
	chip->remove = bma2xx_input_chip_remove;

	ret = cavan_input_chip_register(chip, &client->dev);
	if (ret < 0)
	{
		pr_red_info("cavan_input_chip_register");
		goto out_kfree_chip;
	}

	return 0;

out_kfree_chip:
	kfree(chip);
	return ret;
}

static int bma2xx_i2c_remove(struct i2c_client *client)
{
	struct cavan_input_chip *chip = i2c_get_clientdata(client);

	pr_pos_info();

	cavan_input_chip_unregister(chip);
	kfree(chip);

	return 0;
}

static const struct i2c_device_id bma2xx_id[] =
{
	{BMA2XX_DEVICE_NAME, 0}, {}
};

MODULE_DEVICE_TABLE(i2c, bma2xx_id);

static struct of_device_id bma2xx_match_table[] =
{
	{
		.compatible = "bcm," BMA2XX_DEVICE_NAME
	},
	{}
};

static struct i2c_driver bma2xx_driver =
{
	.driver =
	{
		.name = BMA2XX_DEVICE_NAME,
		.owner = THIS_MODULE,
		.of_match_table = bma2xx_match_table,
	},

	.probe = bma2xx_i2c_probe,
	.remove = bma2xx_i2c_remove,
	.id_table = bma2xx_id,
};

static int __init cavan_bma2xx_init(void)
{
	pr_pos_info();

	return i2c_add_driver(&bma2xx_driver);
}

static void __exit cavan_bma2xx_exit(void)
{
	pr_pos_info();

	i2c_del_driver(&bma2xx_driver);
}

module_init(cavan_bma2xx_init);
module_exit(cavan_bma2xx_exit);

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Cavan bma2xx Three-Axis Digital Accelerometer Driver");
MODULE_LICENSE("GPL");
