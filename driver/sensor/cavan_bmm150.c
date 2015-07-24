#include <cavan/cavan_sensor.h>
#include <cavan/cavan_i2c.h>
#include <cavan/cavan_io.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>

#define BMM150_DEVICE_NAME			"bmm150"

#define BMM150_RESOLUTION			0xFFFF
#define BMM150_RANGE				0xFFFF
#define BMM150_FUZZ					30
#define BMM150_CHIP_ID				0x32

enum bmm150_register_map
{
	REG_CHIP_ID = 0x40,
	REG_DATA = 0x42,
	REG_DATA_X_LSB = 0x42,
	REG_DATA_X_MSB = 0x43,
	REG_DATA_Y_LSB = 0x44,
	REG_DATA_Y_MSB = 0x45,
	REG_DATA_Z_LSB = 0x46,
	REG_DATA_Z_MSB = 0x47,
	REG_DATA_RHALL_LSB = 0x48,
	REG_DATA_RHALL_MSB = 0x49,
	REG_INT_STATUS = 0x4A,
	REG_POWER_CTRL = 0x4B,
	REG_OPMODE = 0x4C,
	REG_INT_ENABLE = 0x4D,
	REG_INT_SETTING = 0x4E,
	REG_LOW_THRESHOLD = 0x4F,
	REG_HIGH_THRESHOLD = 0x50,
	REG_REP_XY = 0x51,
	REG_REP_Z = 0x52,
};

#pragma pack(1)
struct bmm150_data_package
{
	s16 x;
	s16 y;
	s16 z;
	s16 rhall;
};
#pragma pack()

struct bmm150_chip
{
	struct cavan_sensor_device geomagnetic;
};

const struct cavan_input_rate_map bmm150_rate_map[] =
{
	{ 100, 0x00 },
	{ 500, 0x01 },
	{ 166, 0x02 },
	{ 125, 0x03 },
	{ 66, 0x04 },
	{ 50, 0x05 },
	{ 40, 0x06 },
	{ 33, 0x07 },
};

static int bmm150_sensor_chip_readid(struct cavan_input_chip *chip)
{
	int ret;
	u8 chip_id;

	ret = chip->read_register(chip, REG_CHIP_ID, &chip_id);
	if (ret < 0)
	{
		pr_red_info("read_register REG_CHIP_ID");
		return ret;
	}

	pr_bold_info("REG_CHIP_ID = 0x%02x", chip_id);

	if (chip_id != BMM150_CHIP_ID) {
		pr_red_info("Chip ID Invalid!");
		return -EINVAL;
	}

	return 0;
}

static int bmm150_sensor_chip_set_active(struct cavan_input_chip *chip, bool enable)
{
	pr_pos_info();

	return cavan_io_update_bits8(chip, REG_POWER_CTRL, enable, 0x01);
}

// ================================================================================

static int bmm150_geomagnetic_set_delay(struct cavan_input_device *dev, unsigned int delay)
{
	const struct cavan_input_rate_map *map;

	pr_pos_info();

	map = cavan_input_find_rate_map(bmm150_rate_map, ARRAY_SIZE(bmm150_rate_map), delay);
	if (map == NULL)
	{
		pr_red_info("cavan_input_find_rate_map");
		return -EINVAL;
	}

	return cavan_io_update_bits8(dev->chip, REG_OPMODE, map->value << 3, 7 << 3);
}

static int bmm150_geomagnetic_set_enable(struct cavan_input_device *dev, bool enable)
{
	pr_pos_info();

	return cavan_io_update_bits8(dev->chip, REG_OPMODE, enable ? 0 : (3 << 1), 3 << 1);
}

static int bmm150_geomagnetic_event_handler(struct cavan_input_chip *chip, struct cavan_input_device *dev)
{
	int ret;
	struct bmm150_data_package pkg;

	ret = chip->read_data(chip, REG_DATA, &pkg, sizeof(pkg));
	if (ret < 0) {
		pr_red_info("chip->read_data REG_DATA");
		return ret;
	}

	cavan_sensor_report_vector(dev->input, pkg.x >> 3, pkg.y >> 3, pkg.z >> 1);

	return 0;
}

// ================================================================================

static int bmm150_input_chip_probe(struct cavan_input_chip *chip)
{
	int ret;
	struct bmm150_chip *bmm150;
	struct cavan_sensor_device *sensor;
	struct cavan_input_device *dev;

	pr_pos_info();

	bmm150 = kzalloc(sizeof(struct bmm150_chip), GFP_KERNEL);
	if (bmm150 == NULL)
	{
		pr_red_info("kzalloc");
		return -ENOMEM;
	}

	cavan_input_chip_set_dev_data(chip, bmm150);

	sensor = &bmm150->geomagnetic;
	sensor->max_range = BMM150_RANGE;
	sensor->resolution = BMM150_RESOLUTION;
	sensor->power_consume = 145;

	dev = &sensor->dev;
	dev->name = "BMM150 Three-Axis Digital Geomagnetic";
	dev->fuzz = BMM150_FUZZ;
	dev->flat = BMM150_FUZZ;
	dev->use_irq = false;
	dev->type = CAVAN_INPUT_DEVICE_TYPE_MAGNETIC_FIELD;
	dev->min_delay = 33;
	dev->poll_delay = 200;
	dev->set_enable = bmm150_geomagnetic_set_enable;
	dev->set_delay = bmm150_geomagnetic_set_delay;
	dev->event_handler = bmm150_geomagnetic_event_handler;

	ret = cavan_input_device_register(chip, dev);
	if (ret < 0)
	{
		pr_red_info("cavan_input_device_register");
		goto out_kfree_bmm150;
	}

	return 0;

out_kfree_bmm150:
	kfree(bmm150);
	return ret;
}

static void bmm150_input_chip_remove(struct cavan_input_chip *chip)
{
	struct bmm150_chip *bmm150 = cavan_input_chip_get_dev_data(chip);

	pr_pos_info();

	cavan_input_device_unregister(chip, &bmm150->geomagnetic.dev);
	kfree(bmm150);
}

static struct cavan_input_init_data bmm150_init_data[] =
{
};

static int bmm150_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
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
	chip->name = "BMM150";
	chip->devmask = 1 << CAVAN_INPUT_DEVICE_TYPE_MAGNETIC_FIELD;
	chip->init_data = bmm150_init_data;
	chip->init_data_size = ARRAY_SIZE(bmm150_init_data);
	chip->read_data = cavan_input_read_data_i2c;
	chip->write_data = cavan_input_write_data_i2c;
	chip->write_register = cavan_input_write_register_i2c_smbus;
	chip->readid = bmm150_sensor_chip_readid;
	chip->set_active = bmm150_sensor_chip_set_active;

	chip->probe = bmm150_input_chip_probe;
	chip->remove = bmm150_input_chip_remove;

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

static int bmm150_i2c_remove(struct i2c_client *client)
{
	struct cavan_input_chip *chip = i2c_get_clientdata(client);

	pr_pos_info();

	cavan_input_chip_unregister(chip);
	kfree(chip);

	return 0;
}

static const struct i2c_device_id bmm150_id[] =
{
	{ BMM150_DEVICE_NAME, 0 }, {}
};

MODULE_DEVICE_TABLE(i2c, bmm150_id);

static struct of_device_id bmm150_match_table[] =
{
	{
		.compatible = "bosch," BMM150_DEVICE_NAME
	},
	{}
};

static struct i2c_driver bmm150_driver =
{
	.driver =
	{
		.name = BMM150_DEVICE_NAME,
		.owner = THIS_MODULE,
		.of_match_table = bmm150_match_table,
	},

	.probe = bmm150_i2c_probe,
	.remove = bmm150_i2c_remove,
	.id_table = bmm150_id,
};

static int __init cavan_bmm150_init(void)
{
	pr_pos_info();

	return i2c_add_driver(&bmm150_driver);
}

static void __exit cavan_bmm150_exit(void)
{
	pr_pos_info();

	i2c_del_driver(&bmm150_driver);
}

module_init(cavan_bmm150_init);
module_exit(cavan_bmm150_exit);

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Cavan bmm150 Three-Axis Digital Geomagnetic Driver");
MODULE_LICENSE("GPL");
