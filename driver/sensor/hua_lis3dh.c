#include <linux/input/hua_sensor.h>

#define	INTERRUPT_MANAGEMENT 1

#define	G_MAX		16000	/** Maximum polled-device-reported g value */

/*
#define	SHIFT_ADJ_2G		4
#define	SHIFT_ADJ_4G		3
#define	SHIFT_ADJ_8G		2
#define	SHIFT_ADJ_16G		1
*/

#define SENSITIVITY_2G		1	/**	mg/LSB	*/
#define SENSITIVITY_4G		2	/**	mg/LSB	*/
#define SENSITIVITY_8G		4	/**	mg/LSB	*/
#define SENSITIVITY_16G		12	/**	mg/LSB	*/


#define	HIGH_RESOLUTION		0x08

#define	AXISDATA_REG		0x28
#define WHOAMI_LIS3DH_ACC	0x33	/*	Expctd content for WAI	*/

/*	CONTROL REGISTERS	*/
#define WHO_AM_I		0x0F	/*	WhoAmI register		*/
#define	TEMP_CFG_REG		0x1F	/*	temper sens control reg	*/
/* ctrl 1: ODR3 ODR2 ODR ODR0 LPen Zenable Yenable Zenable */
#define	CTRL_REG1		0x20	/*	control reg 1		*/
#define	CTRL_REG2		0x21	/*	control reg 2		*/
#define	CTRL_REG3		0x22	/*	control reg 3		*/
#define	CTRL_REG4		0x23	/*	control reg 4		*/
#define	CTRL_REG5		0x24	/*	control reg 5		*/
#define	CTRL_REG6		0x25	/*	control reg 6		*/

#define	FIFO_CTRL_REG		0x2E	/*	FiFo control reg	*/

#define	INT_CFG1		0x30	/*	interrupt 1 config	*/
#define	INT_SRC1		0x31	/*	interrupt 1 source	*/
#define	INT_THS1		0x32	/*	interrupt 1 threshold	*/
#define	INT_DUR1		0x33	/*	interrupt 1 duration	*/

#define	INT_CFG2		0x34	/*	interrupt 2 config	*/
#define	INT_SRC2		0x35	/*	interrupt 2 source	*/
#define	INT_THS2		0x36	/*	interrupt 2 threshold	*/
#define	INT_DUR2		0x37	/*	interrupt 2 duration	*/

#define	TT_CFG			0x38	/*	tap config		*/
#define	TT_SRC			0x39	/*	tap source		*/
#define	TT_THS			0x3A	/*	tap threshold		*/
#define	TT_LIM			0x3B	/*	tap time limit		*/
#define	TT_TLAT			0x3C	/*	tap time latency	*/
#define	TT_TW			0x3D	/*	tap time window		*/
/*	end CONTROL REGISTRES	*/


#define ENABLE_HIGH_RESOLUTION	1

#define LIS3DH_ACC_PM_OFF		0x00
#define LIS3DH_ACC_ENABLE_ALL_AXES	0x07

#define PMODE_MASK			0x08
#define ODR_MASK			0XF0

#define ODR1		0x10  /* 1Hz output data rate */
#define ODR10		0x20  /* 10Hz output data rate */
#define ODR25		0x30  /* 25Hz output data rate */
#define ODR50		0x40  /* 50Hz output data rate */
#define ODR100		0x50  /* 100Hz output data rate */
#define ODR200		0x60  /* 200Hz output data rate */
#define ODR400		0x70  /* 400Hz output data rate */
#define ODR1250		0x90  /* 1250Hz output data rate */



#define	IA			0x40
#define	ZH			0x20
#define	ZL			0x10
#define	YH			0x08
#define	YL			0x04
#define	XH			0x02
#define	XL			0x01
/* */
/* CTRL REG BITS*/
#define	CTRL_REG3_I1_AOI1	0x40
#define	CTRL_REG6_I2_TAPEN	0x80
#define	CTRL_REG6_HLACTIVE	0x02
/* */

/* TAP_SOURCE_REG BIT */
#define	DTAP			0x20
#define	STAP			0x10
#define	SIGNTAP			0x08
#define	ZTAP			0x04
#define	YTAP			0x02
#define	XTAZ			0x01


#define	FUZZ			32
#define	FLAT			32
#define	I2C_RETRY_DELAY		5
#define	I2C_RETRIES		5
#define	I2C_AUTO_INCREMENT	0x80

/* Accelerometer Sensor Full Scale */
#define	LIS3DH_ACC_FS_MASK		0x30
#define LIS3DH_ACC_G_2G 		0x00
#define LIS3DH_ACC_G_4G 		0x10
#define LIS3DH_ACC_G_8G 		0x20
#define LIS3DH_ACC_G_16G		0x30

/* RESUME STATE INDICES */
#define	RES_CTRL_REG1		0
#define	RES_CTRL_REG2		1
#define	RES_CTRL_REG3		2
#define	RES_CTRL_REG4		3
#define	RES_CTRL_REG5		4
#define	RES_CTRL_REG6		5

#define	RES_INT_CFG1		6
#define	RES_INT_THS1		7
#define	RES_INT_DUR1		8
#define	RES_INT_CFG2		9
#define	RES_INT_THS2		10
#define	RES_INT_DUR2		11

#define	RES_TT_CFG		12
#define	RES_TT_THS		13
#define	RES_TT_LIM		14
#define	RES_TT_TLAT		15
#define	RES_TT_TW		16

#define	RES_TEMP_CFG_REG	17
#define	RES_REFERENCE_REG	18
#define	RES_FIFO_CTRL_REG	19

#define	RESUME_ENTRIES		20

#define DEVICE_INFO         "ST, LIS3DH"
#define DEVICE_INFO_LEN     32

/* end RESUME STATE INDICES */

#pragma pack(1)
struct lis3dh_data_package
{
	s16 x;
	s16 y;
	s16 z;
};

struct lis3de_data_package
{
	u8 not_x;
	s8 x;
	u8 not_y;
	s8 y;
	u8 not_z;
	s8 z;
};
#pragma pack()

struct lis3dh_delay_map_node
{
	u16 delay;
	u8 mask;
};

const struct lis3dh_delay_map_node lis3dh_delay_map[] =
{
	{1, ODR1250},
	{3, ODR400},
	{5, ODR200},
	{10, ODR100},
	{20, ODR50},
	{40, ODR25},
	{100, ODR10},
	{1000, ODR1}
};

static u8 lis3dh_delay2mask(const struct lis3dh_delay_map_node *map, size_t count, u32 delay)
{
	const struct lis3dh_delay_map_node *p;

	for (p = map + count - 1; p >= map; p--)
	{
		if (p->delay <= delay)
		{
			return p->mask;
		}
	}

	return map->mask;
}

static int lis3dh_sensor_chip_readid(struct hua_input_chip *chip)
{
	int ret;
	u8 id;

	ret = chip->read_register(chip, WHO_AM_I, &id);
	if (ret < 0)
	{
		pr_red_info("hua_sensor_i2c_read_register");
		return ret;
	}

	pr_bold_info("Device ID = 0x%02x", id);

	if (id != WHOAMI_LIS3DH_ACC)
	{
		pr_red_info("Invalid device id = 0x%02x", id);
		return -EINVAL;
	}

	chip->devid = id;

	return 0;
}

static int lis3dh_sensor_chip_set_power(struct hua_input_chip *chip, bool enable)
{
	int ret;
	u8 value;

	if (enable)
	{
		value = LIS3DH_ACC_ENABLE_ALL_AXES;
	}
	else
	{
		value = 0;
	}

	pr_func_info("value = 0x%02x", value);

	ret = chip->write_register(chip, CTRL_REG1, value);
	if (ret < 0)
	{
		pr_red_info("hua_sensor_i2c_write_register");
		return ret;
	}

	return 0;
}

static struct hua_input_init_data lis3dh_init_data[] =
{
	{CTRL_REG1, LIS3DH_ACC_ENABLE_ALL_AXES, 0},
	{CTRL_REG2, 0x00, 0},
	{CTRL_REG3, 0x00, 0},
	{CTRL_REG4, LIS3DH_ACC_G_2G | HIGH_RESOLUTION, 0},
	{CTRL_REG5, 0x00, 0},
	{CTRL_REG6, 0x00, 0},
	{TEMP_CFG_REG, 0x00, 0},
	{FIFO_CTRL_REG, 0x00, 0},
	{INT_CFG1, 0x00, 0},
	{INT_THS1, 0x00, 0},
	{INT_DUR1, 0x00, 0},
	{INT_CFG2, 0x00, 0},
	{INT_THS2, 0x00, 0},
	{INT_DUR2, 0x00, 0},
	{TT_CFG, 0x00, 0},
	{TT_THS, 0x00, 0},
	{TT_LIM, 0x00, 0},
	{TT_TLAT, 0x00, 0},
	{TT_TW, 0x00, 0}
};

static int lis3dh_acceleration_set_delay(struct hua_input_device *dev, unsigned int delay)
{
	struct hua_input_chip *chip = dev->chip;
	u8 mask = lis3dh_delay2mask(lis3dh_delay_map, ARRAY_SIZE(lis3dh_delay_map), delay);

	pr_func_info("mask = 0x%02x", mask);

	return chip->write_register(chip, CTRL_REG1, LIS3DH_ACC_ENABLE_ALL_AXES | mask);
}

static int lis3dh_acceleration_event_handler(struct hua_input_chip *chip, struct hua_input_device *dev)
{
	int ret;
	struct lis3dh_data_package package;
	struct hua_sensor_device *sensor = (struct hua_sensor_device *)dev;

	ret = chip->read_data(chip, I2C_AUTO_INCREMENT | AXISDATA_REG, &package, sizeof(package));
	if (ret < 0)
	{
		pr_red_info("hua_sensor_i2c_read_data");
		return ret;
	}

	sensor->report_vector(sensor, package.x >> 4, package.y >> 4, package.z >> 4);

	return 0;
}

static int lis3de_acceleration_event_handler(struct hua_input_chip *chip, struct hua_input_device *dev)
{
	int ret;
	struct lis3de_data_package package;
	struct hua_sensor_device *sensor = (struct hua_sensor_device *)dev;

	ret = chip->read_data(chip, I2C_AUTO_INCREMENT | AXISDATA_REG, &package, sizeof(package));
	if (ret < 0)
	{
		pr_red_info("hua_sensor_i2c_read_data");
		return ret;
	}

	pr_bold_info("[%d, %d, %d]", package.x, package.y, package.z);

	sensor->report_vector(sensor, package.x, package.y, package.z);

	return 0;
}

static int lis3dh_input_chip_probe(struct hua_input_chip *chip)
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
	dev->type = HUA_INPUT_DEVICE_TYPE_ACCELEROMETER;
	dev->poll_delay = 200;
	dev->set_delay = lis3dh_acceleration_set_delay;

	if (strcmp(chip->name, "LIS3DH") == 0)
	{
		dev->fuzz = 32;
		dev->flat = 32;
		sensor->max_range = 2;
		sensor->resolution = 2048;
		dev->event_handler = lis3dh_acceleration_event_handler;
	}
	else
	{
		dev->fuzz = 0;
		dev->flat = 0;
		sensor->max_range = 4;
		sensor->resolution = 256;
		dev->event_handler = lis3de_acceleration_event_handler;
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

static void lis3dh_input_chip_remove(struct hua_input_chip *chip)
{
	struct hua_sensor_device *sensor = hua_input_chip_get_dev_data(chip);

	pr_pos_info();

	hua_input_device_unregister(chip, &sensor->dev);
	kfree(sensor);
}

static int lis3dh_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
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

	if (strcmp(client->name, "lis3dh") == 0)
	{
		chip->name = "LIS3DH";
	}
	else
	{
		chip->name = "LIS3DE";
	}

	chip->devmask = 1 << HUA_INPUT_DEVICE_TYPE_ACCELEROMETER;
	chip->irq = -1;
	chip->irq_flags = 0;
	chip->init_data = lis3dh_init_data;
	chip->init_data_size = ARRAY_SIZE(lis3dh_init_data);
	chip->readid = lis3dh_sensor_chip_readid;
	chip->set_power = lis3dh_sensor_chip_set_power;
	chip->read_data = hua_input_read_data_i2c;
	chip->write_data = hua_input_write_data_i2c;
	chip->write_register = hua_input_write_register_i2c_smbus;

	chip->probe = lis3dh_input_chip_probe;
	chip->remove = lis3dh_input_chip_remove;

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

static int lis3dh_i2c_remove(struct i2c_client *client)
{
	struct hua_input_chip *chip = i2c_get_clientdata(client);

	pr_pos_info();

	hua_input_chip_unregister(chip);
	kfree(chip);

	return 0;
}

static const struct i2c_device_id lis3dh_id[] =
{
	{"lis3dh", 0}, {"lis3de", 0}, {}
};

MODULE_DEVICE_TABLE(i2c, lis3dh_id);

static struct i2c_driver lis3dh_driver =
{
	.driver =
	{
		.name = "lis3dh",
		.owner = THIS_MODULE,
	},

	.probe = lis3dh_i2c_probe,
	.remove = lis3dh_i2c_remove,
	.id_table = lis3dh_id,
};

static int __init hua_lis3dh_init(void)
{
	pr_pos_info();

	return i2c_add_driver(&lis3dh_driver);
}

static void __exit hua_lis3dh_exit(void)
{
	pr_pos_info();

	i2c_del_driver(&lis3dh_driver);
}

module_init(hua_lis3dh_init);
module_exit(hua_lis3dh_exit);

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Huamobile LIS3DH Three-Axis Digital Accelerometer Driver");
MODULE_LICENSE("GPL");
