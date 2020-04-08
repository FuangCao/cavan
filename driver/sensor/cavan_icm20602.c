#include <cavan/cavan_sensor.h>
#include <cavan/cavan_i2c.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>

#define ICM20602_DEVICE_NAME			"icm20602"
#define ICM20602_CHIP_ID				0x12

#define ICM20602_RESOLUTION				0xFFFF
#define ICM20602_ACC_FUZZ				30
#define ICM20602_GYR_FUZZ				30
#define ICM20602_ACC_RANGE				4
#define ICM20602_GYR_RANGE				4000

enum icm20602_register_map {
	REG_XG_OFFS_TC_H = 0x04,
	REG_XG_OFFS_TC_L = 0x05,
	REG_YG_OFFS_TC_H = 0x07,
	REG_YG_OFFS_TC_L = 0x08,
	REG_ZG_OFFS_TC_H = 0x0A,
	REG_ZG_OFFS_TC_L = 0x0B,
	REG_SELF_TEST_X_ACCEL = 0x0D,
	REG_SELF_TEST_Y_ACCEL = 0x0E,
	REG_SELF_TEST_Z_ACCEL = 0x0F,
	REG_XG_OFFS_USRH = 0x13,
	REG_XG_OFFS_USRL = 0x14,
	REG_YG_OFFS_USRH = 0x15,
	REG_YG_OFFS_USRL = 0x16,
	REG_ZG_OFFS_USRH = 0x17,
	REG_ZG_OFFS_USRL = 0x18,
	REG_SMPLRT_DIV = 0x19,
	REG_CONFIG = 0x1A,
	REG_GYRO_CONFIG = 0x1B,
	REG_ACCEL_CONFIG = 0x1C,
	REG_ACCEL_CONFIG2 = 0x1D,
	REG_LP_MODE_CFG = 0x1E,
	REG_ACCEL_WOM_X_THR = 0x20,
	REG_ACCEL_WOM_Y_THR = 0x21,
	REG_ACCEL_WOM_Z_THR = 0x22,
	REG_FIFO_EN = 0x23,
	REG_FSYNC_INT = 0x36,
	REG_INT_PIN_CFG = 0x37,
	REG_INT_ENABLE = 0x38,
	REG_FIFO_WM_INT_STATUS = 0x39,
	REG_INT_STATUS = 0x3A,
	REG_ACCEL_XOUT_H = 0x3B,
	REG_ACCEL_XOUT_L = 0x3C,
	REG_ACCEL_YOUT_H = 0x3D,
	REG_ACCEL_YOUT_L = 0x3E,
	REG_ACCEL_ZOUT_H = 0x3F,
	REG_ACCEL_ZOUT_L = 0x40,
	REG_TEMP_OUT_H = 0x41,
	REG_TEMP_OUT_L = 0x42,
	REG_GYRO_XOUT_H = 0x43,
	REG_GYRO_XOUT_L = 0x44,
	REG_GYRO_YOUT_H = 0x45,
	REG_GYRO_YOUT_L = 0x46,
	REG_GYRO_ZOUT_H = 0x47,
	REG_GYRO_ZOUT_L = 0x48,
	REG_SELF_TEST_X_GYRO = 0x50,
	REG_SELF_TEST_Y_GYRO = 0x51,
	REG_SELF_TEST_Z_GYRO = 0x52,
	REG_FIFO_WM_TH1 = 0x60,
	REG_FIFO_WM_TH2 = 0x61,
	REG_SIGNAL_PATH_RESET = 0x68,
	REG_ACCEL_INTEL_CTRL = 0x69,
	REG_USER_CTRL = 0x6A,
	REG_PWR_MGMT_1 = 0x6B,
	REG_PWR_MGMT_2 = 0x6C,
	REG_I2C_IF = 0x70,
	REG_FIFO_COUNTH = 0x72,
	REG_FIFO_COUNTL = 0x73,
	REG_FIFO_R_W = 0x74,
	REG_WHO_AM_I = 0x75,
	REG_XA_OFFSET_H = 0x77,
	REG_XA_OFFSET_L = 0x78,
	REG_YA_OFFSET_H = 0x7A,
	REG_YA_OFFSET_L = 0x7B,
	REG_ZA_OFFSET_H = 0x7D,
	REG_ZA_OFFSET_L = 0x7E,
};

#pragma pack(1)
struct icm20602_data_package {
	s16 x;
	s16 y;
	s16 z;
};
#pragma pack()

struct icm20602_chip {
	struct cavan_sensor_device acceleration;
	struct cavan_sensor_device gyroscope;
};

static int icm20602_sensor_chip_readid(struct cavan_input_chip *chip)
{
	int ret;
	u8 chip_id;

	ret = chip->read_register(chip, REG_WHO_AM_I, &chip_id);
	if (ret < 0) {
		pr_red_info("read_register REG_CHIP_ID");
		return ret;
	}

	pr_bold_info("REG_CHIP_ID = 0x%02x", chip_id);

	switch (chip_id) {
	case ICM20602_CHIP_ID:
		chip->name = "ICM20602";
		break;

	default:
		pr_red_info("Invalid Chip ID\n");
		return -EINVAL;
	}

	pr_bold_info("chip name = %s", chip->name);

	return 0;
}

static int icm20602_sensor_chip_set_active(struct cavan_input_chip *chip, bool enable)
{
	int ret;

	ret = chip->write_register(chip, REG_PWR_MGMT_1, enable ? 0x01 : 0x51);
	if (ret < 0) {
		pr_red_info("chip->write_register REG_PWR_MGMT_1");
		return ret;
	}

	return 0;
}

// ================================================================================

static int icm20602_acceleration_set_delay(struct cavan_input_device *dev, unsigned int delay)
{
	pr_pos_info();
	return 0;
}

static int icm20602_acceleration_set_enable(struct cavan_input_device *dev, bool enable)
{
	pr_pos_info();
	return 0;
}

static int icm20602_acceleration_event_handler(struct cavan_input_chip *chip, struct cavan_input_device *dev)
{
	s16 x, y, z;
	u8 buff[6];
	int ret;

	ret = chip->read_data(chip, REG_ACCEL_XOUT_H, buff, sizeof(buff));
	if (ret < 0) {
		pr_red_info("chip->read_data REG_ACCEL_XOUT_H");
		return ret;
	}

	x = ((s16) buff[0]) << 8 | buff[1];
	y = ((s16) buff[2]) << 8 | buff[3];
	z = ((s16) buff[4]) << 8 | buff[5];

	cavan_gsensor_report_vector(dev->input, x, y, z);

	return 0;
}

// ================================================================================

static int icm20602_gyroscope_set_delay(struct cavan_input_device *dev, unsigned int delay)
{
	pr_pos_info();

	return 0;
}

static int icm20602_gyroscope_set_enable(struct cavan_input_device *dev, bool enable)
{
	pr_pos_info();

	return 0;
}

static int icm20602_gyroscope_event_handler(struct cavan_input_chip *chip, struct cavan_input_device *dev)
{
	s16 x, y, z;
	u8 buff[6];
	int ret;

	ret = chip->read_data(chip, REG_GYRO_XOUT_H, buff, sizeof(buff));
	if (ret < 0) {
		pr_red_info("chip->read_data REG_GYRO_XOUT_H");
		return ret;
	}

	x = ((s16) buff[0]) << 8 | buff[1];
	y = ((s16) buff[2]) << 8 | buff[3];
	z = ((s16) buff[4]) << 8 | buff[5];

	cavan_gyroscope_report_vector(dev->input, x, y, z);

	return 0;
}

// ================================================================================

static int icm20602_input_chip_probe(struct cavan_input_chip *chip)
{
	int ret;
	struct icm20602_chip *icm20602;
	struct cavan_sensor_device *sensor;
	struct cavan_input_device *dev;

	pr_pos_info();

	icm20602 = kzalloc(sizeof(struct icm20602_chip), GFP_KERNEL);
	if (icm20602 == NULL) {
		pr_red_info("kzalloc");
		return -ENOMEM;
	}

	cavan_input_chip_set_dev_data(chip, icm20602);

	sensor = &icm20602->acceleration;
	sensor->max_range = ICM20602_ACC_RANGE;
	sensor->resolution = ICM20602_RESOLUTION;
	sensor->power_consume = 145;

	dev = &sensor->dev;
	dev->name = "ICM20602 Three-Axis Digital Accelerometer";
	dev->fuzz = ICM20602_ACC_FUZZ;
	dev->flat = ICM20602_ACC_FUZZ;
	dev->use_irq = false;
	dev->type = CAVAN_INPUT_DEVICE_TYPE_ACCELEROMETER;
	dev->min_delay = 20;
	dev->poll_delay = 200;
	dev->set_enable = icm20602_acceleration_set_enable;
	dev->set_delay = icm20602_acceleration_set_delay;
	dev->event_handler = icm20602_acceleration_event_handler;

	ret = cavan_input_device_register(chip, dev);
	if (ret < 0) {
		pr_red_info("cavan_input_device_register");
		goto out_kfree_icm20602;
	}

	sensor = &icm20602->gyroscope;
	sensor->max_range = ICM20602_GYR_RANGE;
	sensor->resolution = ICM20602_RESOLUTION;
	sensor->power_consume = 145;

	dev = &sensor->dev;
	dev->name = "ICM20602 Three-Axis Digital Gyroscope";
	dev->fuzz = ICM20602_GYR_FUZZ;
	dev->flat = ICM20602_GYR_FUZZ;
	dev->use_irq = false;
	dev->type = CAVAN_INPUT_DEVICE_TYPE_GYROSCOPE;
	dev->min_delay = 20;
	dev->poll_delay = 200;
	dev->set_enable = icm20602_gyroscope_set_enable;
	dev->set_delay = icm20602_gyroscope_set_delay;
	dev->event_handler = icm20602_gyroscope_event_handler;

	ret = cavan_input_device_register(chip, dev);
	if (ret < 0) {
		pr_red_info("cavan_input_device_register");
		goto out_unregister_acceleration;
	}

	return 0;

out_unregister_acceleration:
	cavan_input_device_unregister(chip, &icm20602->acceleration.dev);
out_kfree_icm20602:
	kfree(icm20602);
	return ret;
}

static void icm20602_input_chip_remove(struct cavan_input_chip *chip)
{
	struct icm20602_chip *icm20602 = cavan_input_chip_get_dev_data(chip);

	pr_pos_info();

	cavan_input_device_unregister(chip, &icm20602->gyroscope.dev);
	cavan_input_device_unregister(chip, &icm20602->acceleration.dev);
	kfree(icm20602);
}

static struct cavan_input_init_data icm20602_init_data[] = {
	{ REG_GYRO_CONFIG, 3 << 3, 0 },
};

static int icm20602_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	struct cavan_input_chip *chip;

	pr_pos_info();

	chip = kzalloc(sizeof(*chip), GFP_KERNEL);
	if (chip == NULL) {
		pr_red_info("kzalloc");
		return -ENOMEM;
	}

	i2c_set_clientdata(client, chip);
	cavan_input_chip_set_bus_data(chip, client);

	chip->irq = -1;
	chip->name = "ICM20602";
	chip->devmask = 1 << CAVAN_INPUT_DEVICE_TYPE_ACCELEROMETER | 1 << CAVAN_INPUT_DEVICE_TYPE_GYROSCOPE;
	chip->init_data = icm20602_init_data;
	chip->init_data_size = ARRAY_SIZE(icm20602_init_data);
	chip->i2c_rate = 400 * 1000;
	chip->read_data = cavan_input_read_data_i2c;
	chip->write_data = cavan_input_write_data_i2c;
	chip->write_register = cavan_input_write_register_i2c_smbus;
	chip->readid = icm20602_sensor_chip_readid;
	chip->set_active = icm20602_sensor_chip_set_active;

	chip->probe = icm20602_input_chip_probe;
	chip->remove = icm20602_input_chip_remove;

	ret = cavan_input_chip_register(chip, &client->dev);
	if (ret < 0) {
		pr_red_info("cavan_input_chip_register");
		goto out_kfree_chip;
	}

	return 0;

out_kfree_chip:
	kfree(chip);
	return ret;
}

static int icm20602_i2c_remove(struct i2c_client *client)
{
	struct cavan_input_chip *chip = i2c_get_clientdata(client);

	pr_pos_info();

	cavan_input_chip_unregister(chip);
	kfree(chip);

	return 0;
}

static const struct i2c_device_id icm20602_id[] = {
	{ ICM20602_DEVICE_NAME, 0 }, {}
};

MODULE_DEVICE_TABLE(i2c, icm20602_id);

static struct of_device_id icm20602_match_table[] = {
	{ .compatible = "icm," ICM20602_DEVICE_NAME },
	{}
};

static struct i2c_driver icm20602_driver = {
	.driver = {
		.name = ICM20602_DEVICE_NAME,
		.owner = THIS_MODULE,
		.of_match_table = icm20602_match_table,
	},

	.probe = icm20602_i2c_probe,
	.remove = icm20602_i2c_remove,
	.id_table = icm20602_id,
};

static int __init cavan_icm20602_init(void)
{
	pr_pos_info();

	return i2c_add_driver(&icm20602_driver);
}

static void __exit cavan_icm20602_exit(void)
{
	pr_pos_info();

	i2c_del_driver(&icm20602_driver);
}

module_init(cavan_icm20602_init);
module_exit(cavan_icm20602_exit);

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Cavan icm20602 Three-Axis Digital Accelerometer Driver");
MODULE_LICENSE("GPL");
