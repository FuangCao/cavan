#include <huamobile/hua_sensor.h>
#include <huamobile/hua_i2c.h>

#define REG_DATA_START	0x00
#define REG_DETECTION	0x04
#define REG_CHIP_ID		0x08

#define CHIP_ID_MXC6225XC	0x05
#define CHIP_ID_MXC6225XU	0x25

#pragma pack(1)
struct mxc6225xc_data_package
{
	s8 x;
	s8 y;
};

struct mxc6225xu_data_package
{
	s8 y;
	s8 x;
};
#pragma pack()

static int mxc6225_sensor_chip_readid(struct hua_input_chip *chip)
{
	int ret;
	u8 id;

	ret = chip->read_register(chip, REG_CHIP_ID, &id);
	if (ret < 0)
	{
		pr_red_info("hua_sensor_i2c_read_register");
		return ret;
	}

	chip->devid = id & 0x3F;

	pr_bold_info("Device ID = 0x%02x = 0x%02x", id, chip->devid);

	switch (chip->devid)
	{
	case CHIP_ID_MXC6225XC:
		chip->name = "MXC6225XC";
		break;

	case CHIP_ID_MXC6225XU:
		chip->name = "MXC6225XU";
		break;

	default:
		pr_red_info("Invalid Chip ID 0x%02x", chip->devid);
	}

	pr_bold_info("This chip is %s", chip->name);

	return 0;
}

static int mxc6225_sensor_chip_set_active(struct hua_input_chip *chip, bool enable)
{
	int ret;

	ret = chip->write_register(chip, REG_DETECTION, enable ? 0x00 : 0x80);
	if (ret < 0)
	{
		pr_red_info("hua_sensor_i2c_read_register");
		return ret;
	}

	return 0;
}

static int mxc6225xu_acceleration_event_handler(struct hua_input_chip *chip, struct hua_input_device *dev)
{
	int ret;
	struct mxc6225xu_data_package package;

	ret = chip->read_data(chip, 0, &package, sizeof(package));
	if (ret < 0)
	{
		pr_red_info("hua_sensor_i2c_read_data");
		return ret;
	}

	hua_sensor_report_vector(dev->input, package.y, package.x, 32);
	return 0;
}

static int mxc6225xc_acceleration_event_handler(struct hua_input_chip *chip, struct hua_input_device *dev)
{
	int ret;
	struct mxc6225xc_data_package package;

	ret = chip->read_data(chip, 0, &package, sizeof(package));
	if (ret < 0)
	{
		pr_red_info("hua_sensor_i2c_read_data");
		return ret;
	}

	hua_sensor_report_vector(dev->input, -package.y, -package.x, 32);
	return 0;
}

static int mxc6225_input_chip_probe(struct hua_input_chip *chip)
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

	sensor->power_consume = 145;

	dev = &sensor->dev;
	dev->name = "MXC6225 Two-Axis Digital Accelerometer";
	dev->type = HUA_INPUT_DEVICE_TYPE_ACCELEROMETER;
	dev->min_delay = 20;
	dev->poll_delay = 200;

	dev->fuzz = 4;
	dev->flat = 4;
	sensor->axis_count = 2;
	sensor->max_range = 4;
	sensor->resolution = 256;

	if (chip->devid == CHIP_ID_MXC6225XC)
	{
		dev->event_handler = mxc6225xc_acceleration_event_handler;
	}
	else
	{
		dev->event_handler = mxc6225xu_acceleration_event_handler;
	}

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

static void mxc6225_input_chip_remove(struct hua_input_chip *chip)
{
	struct hua_sensor_device *sensor = hua_input_chip_get_dev_data(chip);

	pr_pos_info();

	hua_input_device_unregister(chip, &sensor->dev);
	kfree(sensor);
}

static int mxc6225_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	struct hua_input_chip *chip;

	chip = kzalloc(sizeof(*chip), GFP_KERNEL);
	if (chip == NULL)
	{
		pr_red_info("sensor == NULL");
		return -ENOMEM;
	}

	i2c_set_clientdata(client, chip);
	hua_input_chip_set_bus_data(chip, client);

	chip->name = "MXC6225";
	chip->devmask = 1 << HUA_INPUT_DEVICE_TYPE_ACCELEROMETER;
	chip->irq = -1;
	chip->irq_flags = 0;
	chip->readid = mxc6225_sensor_chip_readid;
	chip->set_active = mxc6225_sensor_chip_set_active;
	chip->read_data = hua_input_read_data_i2c;
	chip->write_data = hua_input_write_data_i2c;
	chip->write_register = hua_input_write_register_i2c_smbus;

	chip->probe = mxc6225_input_chip_probe;
	chip->remove = mxc6225_input_chip_remove;

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

static int mxc6225_i2c_remove(struct i2c_client *client)
{
	struct hua_input_chip *chip = i2c_get_clientdata(client);

	pr_pos_info();

	hua_input_chip_unregister(chip);
	kfree(chip);

	return 0;
}

static const struct i2c_device_id mxc6225_id[] =
{
	{"mxc6225", 0}, {}
};

#ifdef CONFIG_OF
static struct of_device_id mxc6225_match_table[] =
{
	{
		.compatible = "freescale,mxc6225"
	},
	{}
};
#endif

MODULE_DEVICE_TABLE(i2c, mxc6225_id);

static struct i2c_driver mxc6225_driver =
{
	.driver =
	{
		.name = "mxc6225",
		.owner = THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = mxc6225_match_table,
#endif
	},

	.probe = mxc6225_i2c_probe,
	.remove = mxc6225_i2c_remove,
	.id_table = mxc6225_id,
};

static int __init hua_mxc6225_init(void)
{
	pr_pos_info();

	return i2c_add_driver(&mxc6225_driver);
}

static void __exit hua_mxc6225_exit(void)
{
	pr_pos_info();

	i2c_del_driver(&mxc6225_driver);
}

module_init(hua_mxc6225_init);
module_exit(hua_mxc6225_exit);

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Huamobile max6225 Three-Axis Digital Accelerometer Driver");
MODULE_LICENSE("GPL");
