#include "hua_sensor_core.h"
#ifdef CONFIG_ARCH_SC8810
#include <mach/eic.h>
#endif

/* ADXL345/6 Register Map */
#define DEVID		0x00	/* R   Device ID */
#define THRESH_TAP	0x1D	/* R/W Tap threshold */
#define OFSX		0x1E	/* R/W X-axis offset */
#define OFSY		0x1F	/* R/W Y-axis offset */
#define OFSZ		0x20	/* R/W Z-axis offset */
#define DUR		0x21	/* R/W Tap duration */
#define LATENT		0x22	/* R/W Tap latency */
#define WINDOW		0x23	/* R/W Tap window */
#define THRESH_ACT	0x24	/* R/W Activity threshold */
#define THRESH_INACT	0x25	/* R/W Inactivity threshold */
#define TIME_INACT	0x26	/* R/W Inactivity time */
#define ACT_INACT_CTL	0x27	/* R/W Axis enable control for activity and */

/* inactivity detection */
#define THRESH_FF	0x28	/* R/W Free-fall threshold */
#define TIME_FF		0x29	/* R/W Free-fall time */
#define TAP_AXES	0x2A	/* R/W Axis control for tap/double tap */
#define ACT_TAP_STATUS	0x2B	/* R   Source of tap/double tap */
#define BW_RATE		0x2C	/* R/W Data rate and power mode control */
#define POWER_CTL	0x2D	/* R/W Power saving features control */
#define INT_ENABLE	0x2E	/* R/W Interrupt enable control */
#define INT_MAP		0x2F	/* R/W Interrupt mapping control */
#define INT_SOURCE	0x30	/* R   Source of interrupts */
#define DATA_FORMAT	0x31	/* R/W Data format control */
#define DATAX0		0x32	/* R   X-Axis Data 0 */
#define DATAX1		0x33	/* R   X-Axis Data 1 */
#define DATAY0		0x34	/* R   Y-Axis Data 0 */
#define DATAY1		0x35	/* R   Y-Axis Data 1 */
#define DATAZ0		0x36	/* R   Z-Axis Data 0 */
#define DATAZ1		0x37	/* R   Z-Axis Data 1 */
#define FIFO_CTL	0x38	/* R/W FIFO control */
#define FIFO_STATUS	0x39	/* R   FIFO status */
#define TAP_SIGN	0x3A	/* R   Sign and source for tap/double tap */

/* Orientation ADXL346 only */
#define ORIENT_CONF	0x3B	/* R/W Orientation configuration */
#define ORIENT		0x3C	/* R   Orientation status */

/* DEVIDs */
#define ID_ADXL345	0xE5
#define ID_ADXL346	0xE6

/* INT_ENABLE/INT_MAP/INT_SOURCE Bits */
#define DATA_READY	(1 << 7)
#define SINGLE_TAP	(1 << 6)
#define DOUBLE_TAP	(1 << 5)
#define ACTIVITY	(1 << 4)
#define INACTIVITY	(1 << 3)
#define FREE_FALL	(1 << 2)
#define WATERMARK	(1 << 1)
#define OVERRUN		(1 << 0)

/* ACT_INACT_CONTROL Bits */
#define ACT_ACDC	(1 << 7)
#define ACT_X_EN	(1 << 6)
#define ACT_Y_EN	(1 << 5)
#define ACT_Z_EN	(1 << 4)
#define INACT_ACDC	(1 << 3)
#define INACT_X_EN	(1 << 2)
#define INACT_Y_EN	(1 << 1)
#define INACT_Z_EN	(1 << 0)

/* TAP_AXES Bits */
#define SUPPRESS	(1 << 3)
#define TAP_X_EN	(1 << 2)
#define TAP_Y_EN	(1 << 1)
#define TAP_Z_EN	(1 << 0)

/* ACT_TAP_STATUS Bits */
#define ACT_X_SRC	(1 << 6)
#define ACT_Y_SRC	(1 << 5)
#define ACT_Z_SRC	(1 << 4)
#define ASLEEP		(1 << 3)
#define TAP_X_SRC	(1 << 2)
#define TAP_Y_SRC	(1 << 1)
#define TAP_Z_SRC	(1 << 0)

/* BW_RATE Bits */
#define LOW_POWER	(1 << 4)
#define RATE(x)		((x) & 0xF)

/* POWER_CTL Bits */
#define PCTL_LINK	(1 << 5)
#define PCTL_AUTO_SLEEP (1 << 4)
#define PCTL_MEASURE	(1 << 3)
#define PCTL_SLEEP	(1 << 2)
#define PCTL_WAKEUP(x)	((x) & 0x3)

/* DATA_FORMAT Bits */
#define SELF_TEST	(1 << 7)
#define SPI		(1 << 6)
#define INT_INVERT	(1 << 5)
#define FULL_RES	(1 << 3)
#define JUSTIFY		(1 << 2)
#define RANGE(x)	((x) & 0x3)
#define RANGE_PM_2g	0
#define RANGE_PM_4g	1
#define RANGE_PM_8g	2
#define RANGE_PM_16g	3

/*
 * Maximum value our axis may get in full res mode for the input device
 * (signed 13 bits)
 */
#define ADXL_FULLRES_MAX_VAL 4096

/*
 * Maximum value our axis may get in fixed res mode for the input device
 * (signed 10 bits)
 */
#define ADXL_FIXEDRES_MAX_VAL 512

/* FIFO_CTL Bits */
#define FIFO_MODE(x)	(((x) & 0x3) << 6)
#define FIFO_BYPASS	0
#define FIFO_FIFO	1
#define FIFO_STREAM	2
#define FIFO_TRIGGER	3
#define TRIGGER		(1 << 5)
#define SAMPLES(x)	((x) & 0x1F)

/* FIFO_STATUS Bits */
#define FIFO_TRIG	(1 << 7)
#define ENTRIES(x)	((x) & 0x3F)

/* TAP_SIGN Bits ADXL346 only */
#define XSIGN		(1 << 6)
#define YSIGN		(1 << 5)
#define ZSIGN		(1 << 4)
#define XTAP		(1 << 3)
#define YTAP		(1 << 2)
#define ZTAP		(1 << 1)

/* ORIENT_CONF ADXL346 only */
#define ORIENT_DEADZONE(x)	(((x) & 0x7) << 4)
#define ORIENT_DIVISOR(x)	((x) & 0x7)

/* ORIENT ADXL346 only */
#define ADXL346_2D_VALID		(1 << 6)
#define ADXL346_2D_ORIENT(x)		(((x) & 0x3) >> 4)
#define ADXL346_3D_VALID		(1 << 3)
#define ADXL346_3D_ORIENT(x)		((x) & 0x7)
#define ADXL346_2D_PORTRAIT_POS		0	/* +X */
#define ADXL346_2D_PORTRAIT_NEG		1	/* -X */
#define ADXL346_2D_LANDSCAPE_POS	2	/* +Y */
#define ADXL346_2D_LANDSCAPE_NEG	3	/* -Y */

#define ADXL346_3D_FRONT		3	/* +X */
#define ADXL346_3D_BACK			4	/* -X */
#define ADXL346_3D_RIGHT		2	/* +Y */
#define ADXL346_3D_LEFT			5	/* -Y */
#define ADXL346_3D_TOP			1	/* +Z */
#define ADXL346_3D_BOTTOM		6	/* -Z */

#define ADXL346_GINT1			EIC_ID_0
#define ADXL34X_INT_MASK		(DATA_READY | ACTIVITY | INACTIVITY)

#pragma pack(1)
struct adxl34x_data_package
{
	short y;
	short x;
	short z;
};
#pragma pack()

static int adxl34x_sensor_chip_readid(struct hua_sensor_chip *chip)
{
	int ret;
	u8 id;

	ret = chip->read_register(chip, DEVID, &id);
	if (ret < 0)
	{
		pr_red_info("hua_sensor_i2c_read_register");
		return ret;
	}

	pr_bold_info("Device ID = 0x%02x", id);

	switch (id)
	{
	case ID_ADXL345:
		chip->devid = 0x0345;
		pr_bold_info("This sendor is ADXL345");
		break;

	case ID_ADXL346:
		chip->devid = 0x0346;
		pr_bold_info("This sendor is ADXL346");
		break;

	default:
		pr_red_info("Invalid device id = 0x%02x", id);
		return -EINVAL;
	}

	return 0;
}

static int adxl34x_sensor_chip_set_power(struct hua_sensor_chip *chip, bool enable)
{
	int ret;
	u8 value;

	if (enable)
	{
		value = PCTL_AUTO_SLEEP | PCTL_LINK | PCTL_MEASURE;
	}
	else
	{
		value = 0;
	}

	pr_func_info("value = 0x%02x", value);

	ret = chip->write_register(chip, POWER_CTL, value);
	if (ret < 0)
	{
		pr_red_info("write_register POWER_CTL");
		return ret;
	}

	return 0;
}

static void adxl34x_sensor_main_loop(struct hua_sensor_chip *chip)
{
	int ret;
	u8 irq_state;
#if defined(ADXL346_GINT2) || defined(ADXL346_GINT1)
	struct completion *event_completion = &chip->event_completion;
#endif

	while (1)
	{
#if defined(ADXL346_GINT2) || defined(ADXL346_GINT1)
		enable_irq(chip->irq);
		wait_for_completion(event_completion);
#else
		msleep(chip->poll_delay);
#endif

		if (chip->state != HUA_SENSOR_THREAD_STATE_RUNNING)
		{
			pr_pos_info();
			break;
		}

		ret = chip->read_register(chip, INT_SOURCE, &irq_state);
		if (ret < 0)
		{
			pr_red_info("sensor->read_register INT_SOURCE");
			continue;
		}

		hua_sensor_chip_report_event(chip, irq_state);
	}
}

static bool adxl34x_acceleration_event_handler(struct hua_sensor_device *sensor, struct hua_sensor_chip *chip, u32 mask)
{
	int ret;
	u8 fifo_state;
	struct adxl34x_data_package package;

	if ((mask & (DATA_READY | WATERMARK)) == 0)
	{
		return false;
	}

	ret = chip->read_register(chip, FIFO_STATUS, &fifo_state);
	if (ret < 0)
	{
		pr_red_info("sensor->read_register FIFO_STATUS");
		return false;
	}

	for (fifo_state = (fifo_state & 0x3F) + 1; fifo_state; fifo_state--)
	{
		ret = chip->read_data(chip, DATAX0, &package, sizeof(package));
		if (ret < 0)
		{
			pr_red_info("hua_sensor_i2c_read_data");
			continue;
		}

		sensor->report_vector_event(chip->input, package.x, package.y, package.z);
	}

	return true;
}

static bool adxl34x_orientation_event_handler(struct hua_sensor_device *sensor, struct hua_sensor_chip *chip, u32 mask)
{
	int ret;
	int x, y, z;
	u8 orient;

	ret = chip->read_register(chip, ORIENT, &orient);
	if (ret < 0)
	{
		pr_red_info("read_register");
		return false;
	}

	if ((orient & ADXL346_3D_VALID) == 0)
	{
		return false;
	}

	x = y = z = 0;

	switch (orient & 0x07)
	{
	case 1:
		z = 1;
		break;
	case 2:
		y = 1;
		break;
	case 3:
		x = 1;
		break;
	case 4:
		x = -1;
		break;
	case 5:
		y = -1;
		break;
	case 6:
		z = -1;
		break;
	default:
		return false;
	}

	sensor->report_vector_event(chip->input, x, y, z);

	return true;
}

struct hua_sensor_device adxl34x_sensor_list[] =
{
	{
		.name = "ADXL345/346 Three-Axis Digital Accelerometer",
		.type = HUA_SENSOR_TYPE_ACCELEROMETER,
		.fuzz = 3,
		.flat = 3,
		.poll_delay = 200,
		.max_range = 16,
		.resolution = 4096,
		.power_consume = 145,
		.event_handler = adxl34x_acceleration_event_handler
	},
	{
		.name = "ADXL345/346 Three-Axis Digital Orientation",
		.type = HUA_SENSOR_TYPE_ORIENTATION,
		.fuzz = 0,
		.flat = 0,
		.poll_delay = 200,
		.max_range = 360,
		.resolution = 2,
		.power_consume = 145,
		.event_handler = adxl34x_orientation_event_handler
	}
};

static struct hua_sensor_init_data adxl34x_init_data[] =
{
	{THRESH_TAP, 35, 0},
	{OFSX, 0, 0},
	{OFSY, 0, 0},
	{OFSZ, 0, 0},
	{DUR, 3, 0},
	{LATENT, 20, 0},
	{WINDOW, 20, 0},
	{THRESH_ACT, 4, 0},
	{THRESH_INACT, 2, 0},
	{TIME_INACT, 6, 0},
	{THRESH_FF, 8, 0},
	{TIME_FF, 0x20, 0},
	{TAP_AXES, 0, 0},
	{ACT_INACT_CTL, 0xFF, 0},
	{BW_RATE, RATE(8), 0},
	{DATA_FORMAT, FULL_RES, 0},
	{FIFO_CTL, FIFO_MODE(FIFO_STREAM) | SAMPLES(0), 0},
#ifdef ADXL346_GINT2
	{INT_MAP, ADXL34X_INT_MASK | OVERRUN, 0},
#else
	{INT_MAP, 0, 0},
#endif
	{INT_ENABLE, ADXL34X_INT_MASK | OVERRUN, 0}
};

static int __devinit adxl34x_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	struct hua_sensor_chip *chip;

	pr_pos_info();

	chip = kzalloc(sizeof(*chip), GFP_KERNEL);
	if (chip == NULL)
	{
		pr_red_info("sensor == NULL");
		return -ENOMEM;
	}

#if defined(ADXL346_GINT2) || defined(ADXL346_GINT1)
#ifdef ADXL346_GINT2
	chip->irq = sprd_alloc_eic_irq(ADXL346_GINT2);
#else
	chip->irq = sprd_alloc_eic_irq(ADXL346_GINT1);
#endif
	if (chip->irq < 0)
	{
		pr_red_info("sprd_alloc_eic_irq");
		ret = chip->irq;
		goto out_kfree_sensor;
	}
	chip->min_delay = 0;
#else
	chip->irq = -1;
	chip->min_delay = 200;
#endif

	chip->name = "ADXL34X";
	chip->irq_flags = IRQF_TRIGGER_HIGH;
	chip->init_data = adxl34x_init_data;
	chip->init_data_size = ARRAY_SIZE(adxl34x_init_data);
	chip->sensor_list = adxl34x_sensor_list;
	chip->sensor_count = ARRAY_SIZE(adxl34x_sensor_list);

	chip->read_data = hua_sensor_read_data_i2c;
	chip->write_data = hua_sensor_write_data_i2c;
	chip->write_register = hua_sensor_write_register_i2c_smbus;

	chip->readid = adxl34x_sensor_chip_readid;
	chip->set_power = adxl34x_sensor_chip_set_power;
	chip->main_loop = adxl34x_sensor_main_loop;

	hua_sensor_chip_set_data(chip, client);
	i2c_set_clientdata(client, chip);

	ret = hua_sensor_register_chip(chip);
	if (ret < 0)
	{
		pr_red_info("hua_sensor_register_gsensor");
		goto out_free_eic_irq;
	}

	return 0;

out_free_eic_irq:
	if (chip->irq > 0)
	{
		sprd_free_eic_irq(chip->irq);
	}
out_kfree_sensor:
	kfree(chip);
	return ret;
}

static int __devexit adxl34x_i2c_remove(struct i2c_client *client)
{
	struct hua_sensor_chip *chip = i2c_get_clientdata(client);

	pr_pos_info();

	hua_sensor_unregister_chip(chip);

	if (chip->irq > 0)
	{
		sprd_free_eic_irq(chip->irq);
	}

	kfree(chip);

	return 0;
}

static const struct i2c_device_id adxl34x_id[] =
{
	{"adxl34x", 0}, {}
};

MODULE_DEVICE_TABLE(i2c, adxl34x_id);

static struct i2c_driver adxl34x_driver =
{
	.driver =
	{
		.name = "adxl34x",
		.owner = THIS_MODULE,
	},

	.probe = adxl34x_i2c_probe,
	.remove = adxl34x_i2c_remove,
	.id_table = adxl34x_id,
};

static int __init hua_adxl34x_init(void)
{
	pr_pos_info();

	return i2c_add_driver(&adxl34x_driver);
}

static void __exit hua_adxl34x_exit(void)
{
	pr_pos_info();

	i2c_del_driver(&adxl34x_driver);
}

module_init(hua_adxl34x_init);
module_exit(hua_adxl34x_exit);

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Huamobile ADXL345/346 Three-Axis Digital Accelerometer Driver");
MODULE_LICENSE("GPL");
