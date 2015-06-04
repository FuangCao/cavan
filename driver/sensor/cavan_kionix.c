#include <cavan/cavan_sensor.h>
#include <cavan/cavan_i2c.h>

#define REG_DATA_START		0x06
#define REG_CHIP_ID			0x0F
#define REG_GRP2_CTRL		0x1B
#define REG_GRP2_INT_CTRL	0x1E
#define REG_GRP2_DATA_CTRL	0x21

#ifdef CONFIG_ARCH_SC8810
#include <mach/eic.h>
#endif

#pragma pack(1)
struct kionix_data_package
{
	short x;
	short y;
	short z;
};
#pragma pack()

static int kionix_sensor_chip_readid(struct cavan_input_chip *chip)
{
	int ret;
	u8 id;

	ret = chip->read_register(chip, REG_CHIP_ID, &id);
	if (ret < 0)
	{
		pr_red_info("cavan_sensor_i2c_read_register");
		return ret;
	}

	pr_bold_info("Device ID = 0x%02x", id);

	return 0;
}

static int kionix_acceleration_set_enable(struct cavan_input_device *dev, bool enable)
{
	int ret;
	struct cavan_input_chip *chip = dev->chip;

	ret = chip->write_register(chip, REG_GRP2_CTRL, 0);
	if (ret < 0)
	{
		pr_red_info("write_register");
		return ret;
	}

	if (enable == false)
	{
		return 0;
	}

	return chip->write_register(chip, REG_GRP2_CTRL, 1 << 6 | 1 << 7);
}

static int kionix_acceleration_event_handler(struct cavan_input_chip *chip, struct cavan_input_device *dev)
{
	int ret;
	struct kionix_data_package package;

	ret = chip->read_data(chip, REG_DATA_START, &package, sizeof(package));
	if (ret < 0)
	{
		pr_red_info("read_data");
		return ret;
	}

	cavan_sensor_report_vector(dev->input, package.y >> 4, package.x >> 4, package.z >> 4);

	return 0;
}

static int kionix_input_chip_probe(struct cavan_input_chip *chip)
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
	sensor->resolution = 4096;
	sensor->power_consume = 145;

	dev = &sensor->dev;
	dev->name = "KIONIX Three-Axis Digital Accelerometer";
	dev->fuzz = 32;
	dev->flat = 32;
	dev->use_irq = false;
	dev->type = CAVAN_INPUT_DEVICE_TYPE_ACCELEROMETER;
	dev->min_delay = 20;
	dev->poll_delay = 200;
	dev->set_enable = kionix_acceleration_set_enable;
	dev->event_handler = kionix_acceleration_event_handler;

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

static void kionix_input_chip_remove(struct cavan_input_chip *chip)
{
	struct cavan_sensor_device *sensor = cavan_input_chip_get_dev_data(chip);

	pr_pos_info();

	cavan_input_device_unregister(chip, &sensor->dev);
	kfree(sensor);
}

static struct cavan_input_init_data kionix_init_data[] =
{
	{REG_GRP2_CTRL, 0},
	{REG_GRP2_INT_CTRL,	0},
	{REG_GRP2_DATA_CTRL, 6}
};

static int kionix_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	struct cavan_input_chip *chip;

	pr_pos_info();

	chip = kzalloc(sizeof(*chip), GFP_KERNEL);
	if (chip == NULL)
	{
		pr_red_info("sensor == NULL");
		return -ENOMEM;
	}

	i2c_set_clientdata(client, chip);
	cavan_input_chip_set_bus_data(chip, client);

	chip->irq = -1;

#ifdef CONFIG_OF
	if (strcmp(client->dev.of_node->name, "kionix,kxcjk") == 0)
#else
	if (strcmp(client->name, "kxcjk") == 0)
#endif
	{
		chip->name = "KXCJK";
	}
	else
	{
		chip->name = "KXTIK";
	}

	chip->devmask = 1 << CAVAN_INPUT_DEVICE_TYPE_ACCELEROMETER;
	chip->init_data = kionix_init_data;
	chip->init_data_size = ARRAY_SIZE(kionix_init_data);
	chip->read_data = cavan_input_read_data_i2c;
	chip->write_data = cavan_input_write_data_i2c;
	chip->write_register = cavan_input_write_register_i2c_smbus;
	chip->readid = kionix_sensor_chip_readid;

	chip->probe = kionix_input_chip_probe;
	chip->remove = kionix_input_chip_remove;

	ret = cavan_input_chip_register(chip, &client->dev);
	if (ret < 0)
	{
		pr_red_info("cavan_input_chip_register");
		goto out_kfree_sensor;
	}

	return 0;

out_kfree_sensor:
	kfree(chip);
	return ret;
}

static int kionix_i2c_remove(struct i2c_client *client)
{
	struct cavan_input_chip *chip = i2c_get_clientdata(client);

	pr_pos_info();

	cavan_input_chip_unregister(chip);

	kfree(chip);

	return 0;
}

static const struct i2c_device_id kionix_id[] =
{
	{"kxtik", 0}, {"kxcjk", 0}, {}
};

#ifdef CONFIG_OF
static struct of_device_id kionix_match_table[] =
{
	{
		.compatible = "kionix,kxtik"
	},
	{
		.compatible = "kionix,kxcjk"
	},
	{}
};
#endif

MODULE_DEVICE_TABLE(i2c, kionix_id);

static struct i2c_driver kionix_driver =
{
	.driver =
	{
		.name = "kionix",
		.owner = THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = kionix_match_table,
#endif
	},

	.probe = kionix_i2c_probe,
	.remove = kionix_i2c_remove,
	.id_table = kionix_id,
};

static int __init cavan_kionix_init(void)
{
	pr_pos_info();

	return i2c_add_driver(&kionix_driver);
}

static void __exit cavan_kionix_exit(void)
{
	pr_pos_info();

	i2c_del_driver(&kionix_driver);
}

module_init(cavan_kionix_init);
module_exit(cavan_kionix_exit);

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Cavan KIONIX Three-Axis Digital Accelerometer Driver");
MODULE_LICENSE("GPL");
