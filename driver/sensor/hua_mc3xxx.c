#include <huamobile/hua_sensor.h>
#include <huamobile/hua_i2c.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>

#define MC3XXX_DEVICE_NAME			"mc3xxx"

#define MC3XXX_PCODE_3210     0x90
#define MC3XXX_PCODE_3230     0x19
#define MC3XXX_PCODE_3250     0x88
#define MC3XXX_PCODE_3410     0xA8
#define MC3XXX_PCODE_3410N    0xB8
#define MC3XXX_PCODE_3430     0x29
#define MC3XXX_PCODE_3430N    0x39
#define MC3XXX_PCODE_3510B    0x40
#define MC3XXX_PCODE_3530B    0x30
#define MC3XXX_PCODE_3510C    0x10
#define MC3XXX_PCODE_3530C    0x60

enum mc3xxx_register_map
{
	REG_XOUT = 0x00,
	REG_YOUT,
	REG_ZOUT,
	REG_Tilt_Status,
	REG_SAMPLING_RATE_STATUS,
	REG_SLEEP_COUNT,
	REG_INTERRUPT_ENABLE,
	REG_MODE_FEATURE,
	REG_SAMPLE_RATE,
	REG_TAP_DETECTION_ENABLE,
	REG_TAP_DWELL_REJECT,
	REG_DROP_CONTROL,
	REG_SHAKE_DEBOUNCE,
	REG_XOUT_EX_L,
	REG_XOUT_EX_H,
	REG_YOUT_EX_L,
	REG_YOUT_EX_H,
	REG_ZOUT_EX_L,
	REG_ZOUT_EX_H,
	REG_RANGE_CONTROL = 0x20,
	REG_SHAKE_THRESHOLD = 0x2B,
	REG_UD_Z_TH,
	REG_UD_X_TH,
	REG_RL_Z_TH,
	REG_RL_Y_TH,
	REG_FB_Z_TH,
	REG_DROP_THRESHOLD,
	REG_TAP_THRESHOLD,
	REG_CHIP_ID = 0x3B,
};

struct hua_mc3xxx_chip
{
	struct hua_input_chip chip;
	struct regulator *vdd;
	struct regulator *vio;
};

#ifdef CONFIG_ARCH_SC8810
#include <mach/eic.h>
#endif

#pragma pack(1)
struct mc3xxx_data_package_low
{
	s8 x;
	s8 y;
	s8 z;
};

struct mc3xxx_data_package_high
{
	s16 x;
	s16 y;
	s16 z;
};
#pragma pack()

struct mc3xxx_rate_map_node
{
	u8 value;
	u32 delay;
};

static const char *mc3xxx_get_chip_name_by_id(u8 id)
{
	switch (id)
	{
	case MC3XXX_PCODE_3210:
		return "MC3210";
	case MC3XXX_PCODE_3230:
		return "MC3230";
	case MC3XXX_PCODE_3250:
		return "MC3250";
	case MC3XXX_PCODE_3410:
		return "MC3410";
	case MC3XXX_PCODE_3410N:
		return "MC3410N";
	case MC3XXX_PCODE_3430:
		return "MC3430";
	case MC3XXX_PCODE_3430N:
		return "MC3430N";
	case MC3XXX_PCODE_3510B:
		return "MC3510B";
	case MC3XXX_PCODE_3530B:
		return "MC3530B";
	case MC3XXX_PCODE_3510C:
		return "MC3510C";
	case MC3XXX_PCODE_3530C:
		return "MC3530C";
	default:
		return NULL;
	}
}

static int mc3xxx_sensor_chip_readid(struct hua_input_chip *chip)
{
	int i;
	int ret;
	u8 value;
	const char *name;
	static u8 addrs[] = {0x4C, 0x6C, 0x4E, 0x6D, 0x6E, 0x6F};
	struct i2c_client *client = hua_input_chip_get_bus_data(chip);

	for (i = 0; i < ARRAY_SIZE(addrs); i++)
	{
		pr_bold_info("Try i2c addr 0x%02x", addrs[i]);

		value = REG_CHIP_ID;

		ret = hua_input_master_send_to_i2c(client, addrs[i], &value, 1);
		if (ret < 0)
		{
			continue;
		}

		ret = hua_input_master_recv_from_i2c(client, addrs[i], &value, 1);
		if (ret < 0)
		{
			continue;
		}

		name = mc3xxx_get_chip_name_by_id(value);
		if (name)
		{
			chip->name = name;
			chip->devid = value;
			client->addr = addrs[i];
			return 0;
		}
	}

	return -EFAULT;
}

static int mc3xxx_sensor_chip_set_power(struct hua_input_chip *chip, bool enable)
{
	struct hua_mc3xxx_chip *mc3xxx = (struct hua_mc3xxx_chip *) chip;

	if (enable)
	{
		if (mc3xxx->vio)
		{
			regulator_enable(mc3xxx->vio);
		}

		if (mc3xxx->vdd)
		{
			regulator_enable(mc3xxx->vdd);
		}

		msleep(20);
	}
	else
	{
		if (mc3xxx->vio)
		{
			regulator_disable(mc3xxx->vio);
		}

		if (mc3xxx->vdd)
		{
			regulator_disable(mc3xxx->vdd);
		}
	}

	return 0;
}

static int mc3xxx_sensor_chip_set_active_base(struct hua_input_chip *chip, bool enable)
{
	return chip->write_register(chip, REG_MODE_FEATURE, enable ? 0x41 : 0x43);
}

static int mc3xxx_sensor_chip_set_active(struct hua_input_chip *chip, bool enable)
{
	int ret;
	u8 value;

	ret = mc3xxx_sensor_chip_set_active_base(chip, false);
	if (ret < 0)
	{
		pr_red_info("mc3xxx_sensor_chip_set_active_base");
		return ret;
	}

	if (enable == false)
	{
		return 0;
	}

	switch (chip->devid)
	{
	case MC3XXX_PCODE_3230:
	case MC3XXX_PCODE_3430:
	case MC3XXX_PCODE_3430N:
		value = 0x32;
		break;

	case MC3XXX_PCODE_3510B:
	case MC3XXX_PCODE_3510C:
		value = 0xA5;
		break;

	case MC3XXX_PCODE_3530B:
	case MC3XXX_PCODE_3530C:
		value = 0x02;
		break;

	default:
		value = 0x3F;
	}

	chip->write_register(chip, REG_RANGE_CONTROL, value);
	chip->write_register(chip, REG_TAP_DETECTION_ENABLE, 0x00);
	chip->write_register(chip, REG_INTERRUPT_ENABLE, 0x00);

	return mc3xxx_sensor_chip_set_active_base(chip, true);
}

static int mc3xxx_acceleration_set_delay(struct hua_input_device *dev, unsigned int delay)
{
	int ret;
	u8 value;
	struct hua_input_chip *chip = dev->chip;

	switch (chip->devid)
	{
	case MC3XXX_PCODE_3510B:
	case MC3XXX_PCODE_3510C:
	case MC3XXX_PCODE_3530B:
	case MC3XXX_PCODE_3530C:
		value = 0x0A;
		break;

	default:
		value = 0x00;
	}

	pr_bold_info("value = 0x%02x", value);

	mc3xxx_sensor_chip_set_active_base(chip, false);
	ret = chip->write_register(chip, REG_SAMPLE_RATE, value);
	mc3xxx_sensor_chip_set_active_base(chip, true);

	return ret;
}

static int mc3xxx_acceleration_event_handler_high(struct hua_input_chip *chip, struct hua_input_device *dev)
{
	int ret;
	struct mc3xxx_data_package_high package;

	ret = chip->read_data(chip, REG_XOUT_EX_L, &package, sizeof(package));
	if (ret < 0)
	{
		pr_red_info("read_data");
		return ret;
	}

	hua_sensor_report_vector(dev->input, package.x, package.y, package.z);

	return 0;
}

static int mc3xxx_acceleration_event_handler_low(struct hua_input_chip *chip, struct hua_input_device *dev)
{
	int ret;
	struct mc3xxx_data_package_low package;

	ret = chip->read_data(chip, REG_XOUT, &package, sizeof(package));
	if (ret < 0)
	{
		pr_red_info("read_data");
		return ret;
	}

	hua_sensor_report_vector(dev->input, package.x, package.y, package.z);

	return 0;
}

static int mc3xxx_input_chip_probe(struct hua_input_chip *chip)
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
	sensor->power_consume = 145;

	dev = &sensor->dev;
	dev->name = "Three-Axis Digital Accelerometer";
	dev->fuzz = 0;
	dev->flat = 0;
	dev->use_irq = false;
	dev->type = HUA_INPUT_DEVICE_TYPE_ACCELEROMETER;
	dev->poll_delay = 200;
	dev->set_delay = mc3xxx_acceleration_set_delay;

	switch (chip->devid)
	{
    case MC3XXX_PCODE_3230:
    case MC3XXX_PCODE_3430:
    case MC3XXX_PCODE_3430N:
		sensor->max_range = 3;
		sensor->resolution = 256;
		dev->event_handler = mc3xxx_acceleration_event_handler_low;
		break;

	case MC3XXX_PCODE_3530B:
	case MC3XXX_PCODE_3530C:
		sensor->max_range = 4;
		sensor->resolution = 256;
		dev->event_handler = mc3xxx_acceleration_event_handler_low;
		break;

	default:
		sensor->resolution = 65536;
		dev->event_handler = mc3xxx_acceleration_event_handler_high;
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

static void mc3xxx_input_chip_remove(struct hua_input_chip *chip)
{
	struct hua_sensor_device *sensor = hua_input_chip_get_dev_data(chip);

	pr_pos_info();

	hua_input_device_unregister(chip, &sensor->dev);
	kfree(sensor);
}

static int mc3xxx_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	struct hua_input_chip *chip;
	struct hua_mc3xxx_chip *mc3xxx;

	pr_pos_info();

	mc3xxx = kzalloc(sizeof(*mc3xxx), GFP_KERNEL);
	if (mc3xxx == NULL)
	{
		pr_red_info("kzalloc");
		return -ENOMEM;
	}

	mc3xxx->vdd = regulator_get(&client->dev, "vdd");
	mc3xxx->vio = regulator_get(&client->dev, "vio");

	i2c_set_clientdata(client, mc3xxx);
	chip = &mc3xxx->chip;
	hua_input_chip_set_bus_data(chip, client);

	chip->irq = -1;
	chip->name = "MC3XXX";
	chip->devmask = 1 << HUA_INPUT_DEVICE_TYPE_ACCELEROMETER;
	chip->read_data = hua_input_read_data_i2c;
	chip->write_data = hua_input_write_data_i2c;
	chip->write_register = hua_input_write_register_i2c_smbus;
	chip->readid = mc3xxx_sensor_chip_readid;
	chip->set_active = mc3xxx_sensor_chip_set_active;
	chip->set_power = mc3xxx_sensor_chip_set_power;

	chip->probe = mc3xxx_input_chip_probe;
	chip->remove = mc3xxx_input_chip_remove;

	ret = hua_input_chip_register(chip);
	if (ret < 0)
	{
		pr_red_info("hua_input_chip_register");
		goto out_regulator_put;
	}

	return 0;

out_regulator_put:
	if (mc3xxx->vio)
	{
		regulator_put(mc3xxx->vio);
	}

	if (mc3xxx->vdd)
	{
		regulator_put(mc3xxx->vdd);
	}

	kfree(chip);
	return ret;
}

static int mc3xxx_i2c_remove(struct i2c_client *client)
{
	struct hua_mc3xxx_chip *mc3xxx = i2c_get_clientdata(client);
	struct hua_input_chip *chip = &mc3xxx->chip;

	pr_pos_info();

	hua_input_chip_unregister(chip);

	if (mc3xxx->vio)
	{
		regulator_put(mc3xxx->vio);
	}

	if (mc3xxx->vdd)
	{
		regulator_put(mc3xxx->vdd);
	}

	kfree(mc3xxx);

	return 0;
}

static const struct i2c_device_id mc3xxx_id[] =
{
	{MC3XXX_DEVICE_NAME, 0}, {}
};

MODULE_DEVICE_TABLE(i2c, mc3xxx_id);

static struct of_device_id mc3xxx_match_table[] =
{
	{
		.compatible = "mc," MC3XXX_DEVICE_NAME
	},
	{}
};

static struct i2c_driver mc3xxx_driver =
{
	.driver =
	{
		.name = MC3XXX_DEVICE_NAME,
		.owner = THIS_MODULE,
		.of_match_table = mc3xxx_match_table,
	},

	.probe = mc3xxx_i2c_probe,
	.remove = mc3xxx_i2c_remove,
	.id_table = mc3xxx_id,
};

static int __init hua_mc3xxx_init(void)
{
	pr_pos_info();

	return i2c_add_driver(&mc3xxx_driver);
}

static void __exit hua_mc3xxx_exit(void)
{
	pr_pos_info();

	i2c_del_driver(&mc3xxx_driver);
}

module_init(hua_mc3xxx_init);
module_exit(hua_mc3xxx_exit);

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Huamobile mc3xxx Three-Axis Digital Accelerometer Driver");
MODULE_LICENSE("GPL");
