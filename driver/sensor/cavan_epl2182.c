#include <cavan/cavan_sensor.h>
#include <cavan/cavan_i2c.h>
#include <cavan/cavan_io.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>

#define EPL2182_SUPPORT_IRQ			0

#define EPL2182_DEVICE_NAME			"epl2182"

#define PS_INTT						4
#define ALS_INTT					6		//5-8
#define P_SENSOR_LTHD				120		//100
#define P_SENSOR_HTHD				170		//500
#define LUX_PER_COUNT				440		//660 = 1.1*0.6*1000

#define EPL_SENSING_1_TIME			(0 << 5)
#define EPL_SENSING_2_TIME			(1 << 5)
#define EPL_SENSING_4_TIME			(2 << 5)
#define EPL_SENSING_8_TIME			(3 << 5)
#define EPL_SENSING_16_TIME			(4 << 5)
#define EPL_SENSING_32_TIME			(5 << 5)
#define EPL_SENSING_64_TIME			(6 << 5)
#define EPL_SENSING_128_TIME		(7 << 5)
#define EPL_C_SENSING_MODE			(0 << 4)
#define EPL_S_SENSING_MODE			(1 << 4)
#define EPL_ALS_MODE				(0 << 2)
#define EPL_PS_MODE					(1 << 2)
#define EPL_TEMP_MODE				(2 << 2)
#define EPL_H_GAIN					0
#define EPL_M_GAIN					1
#define EPL_L_GAIN					3
#define EPL_AUTO_GAIN				2


#define EPL_8BIT_ADC				0
#define EPL_10BIT_ADC				1
#define EPL_12BIT_ADC				2
#define EPL_14BIT_ADC				3

#define EPL_C_RESET					0x00
#define EPL_C_START_RUN				0x04
#define EPL_C_P_UP					0x04
#define EPL_C_P_DOWN				0x06
#define EPL_DATA_LOCK_ONLY			0x01
#define EPL_DATA_LOCK				0x05
#define EPL_DATA_UNLOCK				0x04

#define EPL_GO_MID					0x1E
#define EPL_GO_LOW					0x1E

#define EPL_DRIVE_60MA				(0 << 4)
#define EPL_DRIVE_120MA				(1 << 4)


#define EPL_INT_BINARY				0
#define EPL_INT_DISABLE				2
#define EPL_INT_ACTIVE_LOW			3
#define EPL_INT_FRAME_ENABLE		4

#define EPL_PST_1_TIME				(0 << 2)
#define EPL_PST_4_TIME				(1 << 2)
#define EPL_PST_8_TIME				(2 << 2)
#define EPL_PST_16_TIME				(3 << 2)

#define EPL_ALS_INTT_8				1
#define EPL_ALS_INTT_16				2
#define EPL_ALS_INTT_32				3
#define EPL_ALS_INTT_64				4
#define EPL_ALS_INTT_128			5
#define EPL_ALS_INTT_256			6
#define EPL_ALS_INTT_512			7
#define EPL_ALS_INTT_640			8
#define EPL_ALS_INTT_768			9
#define EPL_ALS_INTT_1024			10
#define EPL_ALS_INTT_2048			11
#define EPL_ALS_INTT_4096			12
#define EPL_ALS_INTT_6144			13
#define EPL_ALS_INTT_8192			14
#define EPL_ALS_INTT_10240			15

#define EPL_INTT_PS_32				2
#define EPL_INTT_PS_48				3
#define EPL_INTT_PS_80				4
#define EPL_INTT_PS_144				5
#define EPL_INTT_PS_272				6
#define EPL_INTT_PS_384				7
#define EPL_INTT_PS_520				8
#define EPL_INTT_PS_656				9
#define EPL_INTT_PS_784				10
#define EPL_INTT_PS_1040			11
#define EPL_INTT_PS_2064			12
#define EPL_INTT_PS_4112			13
#define EPL_INTT_PS_6160			14
#define EPL_INTT_PS_8280			15

#define EPL_CAL_LIGHT_VALUE(value) \
	(((u32) (value)) * LUX_PER_COUNT * 15 / 100000)

#define EPL_LIGHT_MAX_VALUE \
	EPL_CAL_LIGHT_VALUE(0xFFFF)

enum epl2182_mode
{
	EPL2182_MODE_NONE,
	EPL2182_MODE_ALS,
	EPL2182_MODE_PROXI,
};

enum epl2182_register_map
{
	REG_00 = 0x00,
	REG_01,
	REG_HTHDL,
	REG_HTHDH,
	REG_LTHDL,
	REG_LTHDH,
	REG_PSL,
	REG_07,
	REG_08,
	REG_09,
	REG_GO_MID,
	REG_GO_LOW,
	REG_13 = 0x0D,
	REG_CH0_DL,
	REG_CH0_DH,
	REG_CH1_DL,
	REG_CH1_DH,
	REG_REVISION = 0x13,
};

struct cavan_epl2182_device
{
	int mode;
	unsigned long light_jiffies;
	struct cavan_sensor_device proxi;
	struct cavan_sensor_device light;
};

static ssize_t epl2182_sensor_chip_read_data(struct cavan_input_chip *chip, u8 addr, void *buff, size_t size)
{
	int ret;
	struct i2c_client *client = chip->bus_data;

	addr = addr << 3 | (size - 1);
	ret = i2c_master_send(client, &addr, 1);
	if (ret < 0)
	{
		return ret;
	}

	return i2c_master_recv(client, buff, size);
}

static ssize_t epl2182_sensor_chip_write_data(struct cavan_input_chip *chip, u8 addr, const void *buff, size_t size)
{
	struct i2c_client *client = chip->bus_data;

	char mem[size + 1];

	memcpy(mem + 1, buff, size);
	mem[0] = addr << 3 | (size - 1);

	return i2c_master_send(client, mem, size + 1);
}

static int epl2182_sensor_chip_write_register(struct cavan_input_chip *chip, u8 addr, u8 value)
{
	u8 buff[] = {addr << 3, value};

	return i2c_master_send(chip->bus_data, buff, 2);
}

static int epl2182_sensor_chip_write_register16(struct cavan_input_chip *chip, u8 addr, u16 value)
{
	u8 buff[] = {addr << 3 | 1, value & 0xFF, value >> 8};

	return i2c_master_send(chip->bus_data, buff, 3);
}

static int epl2182_sensor_chip_readid(struct cavan_input_chip *chip)
{
	int ret;
	char buff[10];

	ret = chip->read_data(chip, REG_00, buff, sizeof(buff));
	if (ret < 0)
	{
		pr_red_info("chip->read_data");
	}

	cavan_input_print_memory(buff, ret);

	return 0;
}

static inline int epl2182_sensor_chip_set_active(struct cavan_input_chip *chip, bool enable)
{
	if (enable)
	{
		struct cavan_epl2182_device *epl2182 = cavan_input_chip_get_dev_data(chip);

		epl2182->mode = EPL2182_MODE_NONE;

		return 0;
	}

	return chip->write_register(chip, REG_07,  EPL_C_P_DOWN);
}

static int epl2182_sensor_chip_set_mode(struct cavan_input_chip *chip, int mode)
{
	int ret;
	struct cavan_epl2182_device *epl2182 = cavan_input_chip_get_dev_data(chip);

#if EPL2182_SUPPORT_IRQ
	ret = chip->write_register(chip, REG_09, EPL_INT_DISABLE);
	if (ret < 0)
	{
		pr_red_info("chip->write_register REG_09");
		return ret;
	}
#endif

	if (mode == EPL2182_MODE_ALS)
	{
		ret = chip->write_register(chip, REG_00, EPL_S_SENSING_MODE | EPL_SENSING_8_TIME | EPL_ALS_MODE | EPL_AUTO_GAIN);
		if (ret < 0)
		{
			pr_red_info("chip->write_register REG_00");
			return ret;
		}

		ret = chip->write_register(chip, REG_01, ALS_INTT << 4 | EPL_PST_1_TIME | EPL_10BIT_ADC);
		if (ret < 0)
		{
			pr_red_info("chip->write_register REG_01");
			return ret;
		}
	}
	else
	{
		ret = chip->write_register(chip, REG_00, EPL_S_SENSING_MODE | EPL_SENSING_8_TIME | EPL_PS_MODE | EPL_H_GAIN);
		if (ret < 0)
		{
			pr_red_info("chip->write_register REG_00");
			return ret;
		}

		ret = chip->write_register(chip, REG_01, PS_INTT << 4 | EPL_PST_1_TIME | EPL_10BIT_ADC);
		if (ret < 0)
		{
			pr_red_info("chip->write_register REG_01");
			return ret;
		}
	}

	ret = chip->write_register(chip, REG_07, EPL_C_RESET);
	if (ret < 0)
	{
		pr_red_info("chip->write_register REG_07");
		return ret;
	}

	ret = chip->write_register(chip, REG_07, EPL_C_START_RUN);
	if (ret < 0)
	{
		pr_red_info("chip->write_register REG_07");
		return ret;
	}

	epl2182->mode = mode;

	pr_bold_info("mode is %s", mode == EPL2182_MODE_ALS ? "als" : "proximity");

	return 0;
}

static int epl2182_proximity_set_enable(struct cavan_input_device *dev, bool enable)
{
	int ret;
	struct cavan_input_chip *chip;

	if (enable == false)
	{
		return 0;
	}

	chip = dev->chip;

	ret = chip->write_register16(chip, REG_HTHDL, P_SENSOR_HTHD);
	if (ret < 0)
	{
		pr_red_info("chip->write_register16 REG_HTHDL");
		return ret;
	}

	ret = chip->write_register16(chip, REG_LTHDL, P_SENSOR_LTHD);
	if (ret < 0)
	{
		pr_red_info("chip->write_register16 REG_LTHDL");
		return ret;
	}

	return 0;
}

static int epl2182_proximity_event_handler(struct cavan_input_chip *chip, struct cavan_input_device *dev)
{
	int ret;
	u8 value;
	struct cavan_epl2182_device *epl2182 = cavan_input_chip_get_dev_data(chip);

	if (epl2182->mode != EPL2182_MODE_PROXI)
	{
		ret = epl2182_sensor_chip_set_mode(chip, EPL2182_MODE_PROXI);
		if (ret < 0)
		{
			return ret;
		}

		msleep(dev->min_delay);
	}

	ret = chip->write_register(chip, REG_07, EPL_DATA_LOCK);
	if (ret < 0)
	{
		pr_red_info("chip->write_register REG_07");
		return ret;
	}

	ret = chip->read_register(chip, REG_13, &value);
	if (ret < 0)
	{
		pr_red_info("chip->write_register REG_13");
		return ret;
	}

	cavan_sensor_report_value(dev->input, (value & 0x04) == 0);

	return chip->write_register(chip, REG_07, EPL_DATA_UNLOCK);
}

static int epl2182_light_set_enable(struct cavan_input_device *dev, bool enable)
{
	int ret;
	struct cavan_input_chip *chip;

	if (enable == false)
	{
		return 0;
	}

	chip = dev->chip;

	ret = chip->write_register(chip, REG_GO_MID, EPL_GO_MID);
	if (ret < 0)
	{
		pr_red_info("chip->write_register REG_GO_MID");
		return ret;
	}

	ret = chip->write_register(chip, REG_GO_LOW, EPL_GO_LOW);
	if (ret < 0)
	{
		pr_red_info("chip->write_register REG_GO_LOW");
		return ret;
	}

	return 0;
}

static int epl2182_light_event_handler(struct cavan_input_chip *chip, struct cavan_input_device *dev)
{
	int ret;
	u16 ch1data;
	struct cavan_epl2182_device *epl2182 = cavan_input_chip_get_dev_data(chip);

	if (epl2182->mode != EPL2182_MODE_ALS)
	{
		if (jiffies - epl2182->light_jiffies < msecs_to_jiffies(dev->poll_delay))
		{
			return 0;
		}

		ret = epl2182_sensor_chip_set_mode(chip, EPL2182_MODE_ALS);
		if (ret < 0)
		{
			return ret;
		}

		msleep(dev->min_delay);
		epl2182->light_jiffies = jiffies;
	}

	ret = chip->write_register(chip, REG_07, EPL_DATA_LOCK);
	if (ret < 0)
	{
		pr_red_info("chip->write_register REG_07");
		return ret;
	}

	ret = chip->read_register16(chip, REG_CH1_DL, &ch1data);
	if (ret < 0)
	{
		pr_red_info("chip->read_register16 REG_CH1_DL");
		return ret;
	}

	cavan_sensor_report_value(dev->input, EPL_CAL_LIGHT_VALUE(ch1data));

	return chip->write_register(chip, REG_07, EPL_DATA_UNLOCK);
}

static int epl2182_input_chip_probe(struct cavan_input_chip *chip)
{
	int ret;
	struct cavan_input_device *dev;
	struct cavan_sensor_device *sensor;
	struct cavan_epl2182_device *epl2182;

	pr_pos_info();

	epl2182 = kzalloc(sizeof(*epl2182), GFP_KERNEL);
	if (epl2182 == NULL)
	{
		pr_red_info("kzalloc");
		return -ENOMEM;
	}

	cavan_input_chip_set_dev_data(chip, epl2182);

	sensor = &epl2182->proxi;
	sensor->power_consume = 145;

	dev = &sensor->dev;
	dev->name = "EPL2182 Proximity";
	dev->fuzz = 0;
	dev->flat = 0;

#if EPL2182_SUPPORT_IRQ
	dev->use_irq = chip->irq >= 0;
#else
	dev->use_irq = false;
#endif

	dev->type = CAVAN_INPUT_DEVICE_TYPE_PROXIMITY;
	dev->min_delay = 50;
	dev->poll_delay = 200;

	sensor->max_range = 1;
	sensor->resolution = 1;

	dev->set_enable = epl2182_proximity_set_enable;
	dev->event_handler = epl2182_proximity_event_handler;

	ret = cavan_input_device_register(chip, dev);
	if (ret < 0)
	{
		pr_red_info("cavan_input_device_register");
		goto out_kfree_sensor;
	}

	sensor = &epl2182->light;
	sensor->power_consume = 145;

	dev = &sensor->dev;
	dev->name = "EPL2182 Light";
	dev->fuzz = 0;
	dev->flat = 0;

#if EPL2182_SUPPORT_IRQ
	dev->use_irq = chip->irq >= 0;
#else
	dev->use_irq = false;
#endif

	dev->type = CAVAN_INPUT_DEVICE_TYPE_LIGHT;
	dev->min_delay = 55;
	dev->poll_delay = 200;
	sensor->max_range = EPL_LIGHT_MAX_VALUE;
	sensor->resolution = EPL_LIGHT_MAX_VALUE;
	dev->set_enable = epl2182_light_set_enable;
	dev->event_handler = epl2182_light_event_handler;

	ret = cavan_input_device_register(chip, dev);
	if (ret < 0)
	{
		pr_red_info("cavan_input_device_register");
		goto out_cavan_input_device_unregister_proxi;
	}

	return 0;

out_cavan_input_device_unregister_proxi:
	cavan_input_device_unregister(chip, &epl2182->proxi.dev);
out_kfree_sensor:
	kfree(sensor);
	return ret;
}

static void epl2182_input_chip_remove(struct cavan_input_chip *chip)
{
	struct cavan_epl2182_device *epl2182 = cavan_input_chip_get_dev_data(chip);

	pr_pos_info();

	cavan_input_device_unregister(chip, &epl2182->light.dev);
	cavan_input_device_unregister(chip, &epl2182->proxi.dev);
	kfree(epl2182);
}

static struct cavan_input_init_data epl2182_init_data[] =
{
	{REG_00, EPL_S_SENSING_MODE},
	{REG_07, EPL_C_P_DOWN},
	{REG_09, EPL_INT_DISABLE}
};

static int epl2182_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
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

#if EPL2182_SUPPORT_IRQ
	chip->irq = client->irq;
	chip->irq_flags = IRQF_TRIGGER_FALLING;
#else
	chip->irq = -1;
#endif

	pr_bold_info("chip->irq = %d", chip->irq);

	chip->name = "EPL2182";
	chip->flags = CAVAN_INPUT_CHIP_FLAG_POWERON_INIT;
	chip->devmask = 1 << CAVAN_INPUT_DEVICE_TYPE_PROXIMITY | 1 << CAVAN_INPUT_DEVICE_TYPE_LIGHT;
	chip->init_data = epl2182_init_data;
	chip->init_data_size = ARRAY_SIZE(epl2182_init_data);
	chip->read_data = epl2182_sensor_chip_read_data;
	chip->write_data = epl2182_sensor_chip_write_data;
	chip->write_register = epl2182_sensor_chip_write_register;
	chip->write_register16 = epl2182_sensor_chip_write_register16;
	chip->readid = epl2182_sensor_chip_readid;
	chip->set_active = epl2182_sensor_chip_set_active;

	chip->probe = epl2182_input_chip_probe;
	chip->remove = epl2182_input_chip_remove;

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

static int epl2182_i2c_remove(struct i2c_client *client)
{
	struct cavan_input_chip *chip = i2c_get_clientdata(client);

	pr_pos_info();

	cavan_input_chip_unregister(chip);
	kfree(chip);

	return 0;
}

static const struct i2c_device_id epl2182_id[] =
{
	{EPL2182_DEVICE_NAME, 0}, {}
};

MODULE_DEVICE_TABLE(i2c, epl2182_id);

static struct of_device_id epl2182_match_table[] =
{
	{
		.compatible = "elan," EPL2182_DEVICE_NAME
	},
	{}
};

static struct i2c_driver epl2182_driver =
{
	.driver =
	{
		.name = EPL2182_DEVICE_NAME,
		.owner = THIS_MODULE,
		.of_match_table = epl2182_match_table,
	},

	.probe = epl2182_i2c_probe,
	.remove = epl2182_i2c_remove,
	.id_table = epl2182_id,
};

static int __init cavan_epl2182_init(void)
{
	pr_pos_info();

	return i2c_add_driver(&epl2182_driver);
}

static void __exit cavan_epl2182_exit(void)
{
	pr_pos_info();

	i2c_del_driver(&epl2182_driver);
}

module_init(cavan_epl2182_init);
module_exit(cavan_epl2182_exit);

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Cavan EPL2182 Proximity and Light Sensor Driver");
MODULE_LICENSE("GPL");
