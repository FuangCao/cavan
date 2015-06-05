#include <cavan/cavan_sensor.h>
#include <cavan/cavan_i2c.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>

#define BMI160_DEVICE_NAME			"bmi160"

#define BMI160_RESOLUTION			0xFFFF
#define BMI160_ACC_FUZZ				20
#define BMI160_GYR_FUZZ				20
#define BMI160_ACC_RANGE			4
#define BMI160_GYR_RANGE			4000

enum bmi160_register_map
{
	REG_CMD = 0x7E,
	REG_STEP_CONF_1 = 0x7B,
	REG_STEP_CONF_0 = 0x7A,
	REG_STEP_CNT_1 = 0x79,
	REG_STEP_CNT_0 = 0x78,
	REG_OFFSET_6 = 0x77,
	REG_OFFSET_5 = 0x76,
	REG_OFFSET_4 = 0x75,
	REG_OFFSET_3 = 0x74,
	REG_OFFSET_2 = 0x73,
	REG_OFFSET_1 = 0x72,
	REG_OFFSET_0 = 0x71,
	REG_NV_CONF = 0x70,
	REG_SELF_TEST = 0x6D,
	REG_PMU_TRIGGER = 0x6C,
	REG_IF_CONF = 0x6B,
	REG_CONF = 0x6A,
	REG_FOC_CONF = 0x69,
	REG_INT_FLAT_1 = 0x68,
	REG_INT_FLAT_0 = 0x67,
	REG_INT_ORIENT_1 = 0x66,
	REG_INT_ORIENT_0 = 0x65,
	REG_INT_TAP_1 = 0x64,
	REG_INT_TAP_0 = 0x63,
	REG_INT_MOTION_3 = 0x62,
	REG_INT_MOTION_2 = 0x61,
	REG_INT_MOTION_1 = 0x60,
	REG_INT_MOTION_0 = 0x5F,
	REG_INT_LOWHIGH_4 = 0x5E,
	REG_INT_LOWHIGH_3 = 0x5D,
	REG_INT_LOWHIGH_2 = 0x5C,
	REG_INT_LOWHIGH_1 = 0x5B,
	REG_INT_LOWHIGH_0 = 0x5A,
	REG_INT_DATA_1 = 0x59,
	REG_INT_DATA_0 = 0x58,
	REG_INT_MAP_2 = 0x57,
	REG_INT_MAP_1 = 0x56,
	REG_INT_MAP_0 = 0x55,
	REG_INT_LATCH = 0x54,
	REG_INT_OUT_CTRL = 0x53,
	REG_INT_EN_2 = 0x52,
	REG_INT_EN_1 = 0x51,
	REG_INT_EN_0 = 0x50,
	REG_MAG_IF_4 = 0x4F,
	REG_MAG_IF_3 = 0x4E,
	REG_MAG_IF_2 = 0x4D,
	REG_MAG_IF_1 = 0x4C,
	REG_MAG_IF_0 = 0x4B,
	REG_FIFO_CONFIG_1 = 0x47,
	REG_FIFO_CONFIG_0 = 0x46,
	REG_FIFO_DOWNS = 0x45,
	REG_MAG_CONF = 0x44,
	REG_GYR_RANGE = 0x43,
	REG_GYR_CONF = 0x42,
	REG_ACC_RANGE = 0x41,
	REG_ACC_CONF = 0x40,
	REG_FIFO_DATA = 0x24,
	REG_FIFO_LENGTH_1 = 0x23,
	REG_FIFO_LENGTH_0 = 0x22,
	REG_TEMPERATURE_1 = 0x21,
	REG_TEMPERATURE_0 = 0x20,
	REG_INT_STATUS_3 = 0x1F,
	REG_INT_STATUS_2 = 0x1E,
	REG_INT_STATUS_1 = 0x1D,
	REG_INT_STATUS_0 = 0x1C,
	REG_STATUS = 0x1B,
	REG_SENSORTIME_2 = 0x1A,
	REG_SENSORTIME_1 = 0x19,
	REG_SENSORTIME_0 = 0x18,
	REG_DATA_19 = 0x17,
	REG_DATA_18 = 0x16,
	REG_DATA_ACC_Z = 0x16,
	REG_DATA_17 = 0x15,
	REG_DATA_16 = 0x14,
	REG_DATA_ACC_Y = 0x14,
	REG_DATA_15 = 0x13,
	REG_DATA_14 = 0x12,
	REG_DATA_ACC_X = 0x12,
	REG_DATA_ACC = 0x12,
	REG_DATA_13 = 0x11,
	REG_DATA_12 = 0x10,
	REG_DATA_GYR_Z = 0x10,
	REG_DATA_11 = 0x0F,
	REG_DATA_10 = 0x0E,
	REG_DATA_GYR_Y = 0x0E,
	REG_DATA_9 = 0x0D,
	REG_DATA_8 = 0x0C,
	REG_DATA_GYR_X = 0x0C,
	REG_DATA_GYR = 0x0C,
	REG_DATA_7 = 0x0B,
	REG_DATA_6 = 0x0A,
	REG_DATA_RHALL = 0x0A,
	REG_DATA_5 = 0x09,
	REG_DATA_4 = 0x08,
	REG_DATA_MAG_Z = 0x08,
	REG_DATA_3 = 0x07,
	REG_DATA_2 = 0x06,
	REG_DATA_MAG_Y = 0x06,
	REG_DATA_1 = 0x05,
	REG_DATA_0 = 0x04,
	REG_DATA_MAG_X = 0x04,
	REG_DATA_MAG = 0x04,
	REG_PMU_STATUS = 0x03,
	REG_ERR_REG = 0x02,
	REG_CHIP_ID = 0x00,
};

#pragma pack(1)
struct bmi160_data_package
{
	s16 x;
	s16 y;
	s16 z;
};
#pragma pack()

struct bmi160_chip
{
	struct cavan_sensor_device acceleration;
	struct cavan_sensor_device gyroscope;
};

static int bmi160_sensor_chip_readid(struct cavan_input_chip *chip)
{
	int ret;
	u8 value;

	ret = chip->read_register(chip, REG_CHIP_ID, &value);
	if (ret < 0)
	{
		pr_red_info("read_register REG_CHIP_ID");
		return ret;
	}

	pr_bold_info("REG_CHIP_ID = 0x%02x", value);

	return 0;
}

static int bmi160_sensor_chip_set_active(struct cavan_input_chip *chip, bool enable)
{
	pr_pos_info();

	return 0;
}

// ================================================================================

static int bmi160_acceleration_set_delay(struct cavan_input_device *dev, unsigned int delay)
{
	pr_pos_info();

	return 0;
}

static int bmi160_acceleration_set_enable(struct cavan_input_device *dev, bool enable)
{
	int ret;

	pr_pos_info();

	ret = dev->chip->write_register(dev->chip, REG_CMD, enable ? 0x11 : 0x10);
	if (ret < 0) {
		pr_red_info("write_register");
		return ret;
	}

	msleep(100);

	return 0;
}

static int bmi160_acceleration_event_handler(struct cavan_input_chip *chip, struct cavan_input_device *dev)
{
	int ret;
	struct bmi160_data_package pkg;

	ret = chip->read_data(chip, REG_DATA_ACC, &pkg, sizeof(pkg));
	if (ret < 0) {
		pr_red_info("chip->read_data REG_DATA_ACC");
		return ret;
	}

	cavan_sensor_report_vector(dev->input, pkg.x, pkg.y, pkg.z);

	return 0;
}

// ================================================================================

static int bmi160_gyroscope_set_delay(struct cavan_input_device *dev, unsigned int delay)
{
	pr_pos_info();

	return 0;
}

static int bmi160_gyroscope_set_enable(struct cavan_input_device *dev, bool enable)
{
	int ret;

	pr_pos_info();

	ret = dev->chip->write_register(dev->chip, REG_CMD, enable ? 0x15 : 0x14);
	if (ret < 0) {
		pr_red_info("write_register");
		return ret;
	}

	msleep(100);

	return 0;
}

static int bmi160_gyroscope_event_handler(struct cavan_input_chip *chip, struct cavan_input_device *dev)
{
	int ret;
	struct bmi160_data_package pkg;

	ret = chip->read_data(chip, REG_DATA_GYR, &pkg, sizeof(pkg));
	if (ret < 0) {
		pr_red_info("chip->read_data REG_DATA_ACC");
		return ret;
	}

	cavan_sensor_report_vector(dev->input, pkg.x, pkg.y, pkg.z);

	return 0;
}

// ================================================================================

static int bmi160_input_chip_probe(struct cavan_input_chip *chip)
{
	int ret;
	struct bmi160_chip *bmi160;
	struct cavan_sensor_device *sensor;
	struct cavan_input_device *dev;

	pr_pos_info();

	bmi160 = kzalloc(sizeof(struct bmi160_chip), GFP_KERNEL);
	if (bmi160 == NULL)
	{
		pr_red_info("kzalloc");
		return -ENOMEM;
	}

	cavan_input_chip_set_dev_data(chip, bmi160);

	sensor = &bmi160->acceleration;
	sensor->max_range = BMI160_ACC_RANGE;
	sensor->resolution = BMI160_RESOLUTION;
	sensor->power_consume = 145;

	dev = &sensor->dev;
	dev->name = "BMI160 Three-Axis Digital Accelerometer";
	dev->fuzz = BMI160_ACC_FUZZ;
	dev->flat = BMI160_ACC_FUZZ;
	dev->use_irq = false;
	dev->type = CAVAN_INPUT_DEVICE_TYPE_ACCELEROMETER;
	dev->min_delay = 20;
	dev->poll_delay = 200;
	dev->set_enable = bmi160_acceleration_set_enable;
	dev->set_delay = bmi160_acceleration_set_delay;
	dev->event_handler = bmi160_acceleration_event_handler;

	ret = cavan_input_device_register(chip, dev);
	if (ret < 0)
	{
		pr_red_info("cavan_input_device_register");
		goto out_kfree_bmi160;
	}

	sensor = &bmi160->gyroscope;
	sensor->max_range = BMI160_GYR_RANGE;
	sensor->resolution = BMI160_RESOLUTION;
	sensor->power_consume = 145;

	dev = &sensor->dev;
	dev->name = "BMI160 Three-Axis Digital Gyroscope";
	dev->fuzz = BMI160_GYR_FUZZ;
	dev->flat = BMI160_GYR_FUZZ;
	dev->use_irq = false;
	dev->type = CAVAN_INPUT_DEVICE_TYPE_GYROSCOPE;
	dev->min_delay = 20;
	dev->poll_delay = 200;
	dev->set_enable = bmi160_gyroscope_set_enable;
	dev->set_delay = bmi160_gyroscope_set_delay;
	dev->event_handler = bmi160_gyroscope_event_handler;

	ret = cavan_input_device_register(chip, dev);
	if (ret < 0)
	{
		pr_red_info("cavan_input_device_register");
		goto out_unregister_acceleration;
	}

	return 0;

out_unregister_acceleration:
	cavan_input_device_unregister(chip, &bmi160->acceleration.dev);
out_kfree_bmi160:
	kfree(bmi160);
	return ret;
}

static void bmi160_input_chip_remove(struct cavan_input_chip *chip)
{
	struct bmi160_chip *bmi160 = cavan_input_chip_get_dev_data(chip);

	pr_pos_info();

	cavan_input_device_unregister(chip, &bmi160->gyroscope.dev);
	cavan_input_device_unregister(chip, &bmi160->acceleration.dev);
	kfree(bmi160);
}

static struct cavan_input_init_data bmi160_init_data[] =
{
};

static int bmi160_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
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
	chip->name = "BMI160";
	chip->devmask = 1 << CAVAN_INPUT_DEVICE_TYPE_ACCELEROMETER | 1 << CAVAN_INPUT_DEVICE_TYPE_GYROSCOPE;
	chip->init_data = bmi160_init_data;
	chip->init_data_size = ARRAY_SIZE(bmi160_init_data);
	chip->read_data = cavan_input_read_data_i2c;
	chip->write_data = cavan_input_write_data_i2c;
	chip->write_register = cavan_input_write_register_i2c_smbus;
	chip->readid = bmi160_sensor_chip_readid;
	chip->set_active = bmi160_sensor_chip_set_active;

	chip->probe = bmi160_input_chip_probe;
	chip->remove = bmi160_input_chip_remove;

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

static int bmi160_i2c_remove(struct i2c_client *client)
{
	struct cavan_input_chip *chip = i2c_get_clientdata(client);

	pr_pos_info();

	cavan_input_chip_unregister(chip);
	kfree(chip);

	return 0;
}

static const struct i2c_device_id bmi160_id[] =
{
	{ BMI160_DEVICE_NAME, 0 }, {}
};

MODULE_DEVICE_TABLE(i2c, bmi160_id);

static struct of_device_id bmi160_match_table[] =
{
	{
		.compatible = "bosch," BMI160_DEVICE_NAME
	},
	{}
};

static struct i2c_driver bmi160_driver =
{
	.driver =
	{
		.name = BMI160_DEVICE_NAME,
		.owner = THIS_MODULE,
		.of_match_table = bmi160_match_table,
	},

	.probe = bmi160_i2c_probe,
	.remove = bmi160_i2c_remove,
	.id_table = bmi160_id,
};

static int __init cavan_bmi160_init(void)
{
	pr_pos_info();

	return i2c_add_driver(&bmi160_driver);
}

static void __exit cavan_bmi160_exit(void)
{
	pr_pos_info();

	i2c_del_driver(&bmi160_driver);
}

module_init(cavan_bmi160_init);
module_exit(cavan_bmi160_exit);

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Cavan bmi160 Three-Axis Digital Accelerometer Driver");
MODULE_LICENSE("GPL");
