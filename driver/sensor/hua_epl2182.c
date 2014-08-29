#include <huamobile/hua_sensor.h>
#include <huamobile/hua_i2c.h>
#include <huamobile/hua_io.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>

#define EPL2182_SUPPORT_IRQ			0

#define EPL2182_DEVICE_NAME			"epl2182"

#define EPL_SENSOR_MIN_DELAY		55
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

enum epl2182_register_map
{
	REG_00 = 0x00,
	REG_01,
	REG_02,
	REG_03,
	REG_04,
	REG_05,
	REG_06,
	REG_07,
	REG_08,
	REG_09,
	REG_10,
	REG_11,
	REG_12,
	REG_13,
	REG_14,
	REG_15,
	REG_16,
	REG_17,
	REG_18,
	REG_19,
	REG_20,
	REG_21,
};

struct hua_epl2182_device
{
	int proxi_count;
	struct hua_sensor_device proxi;
	struct hua_sensor_device light;
};

static ssize_t epl2182_sensor_chip_read_data(struct hua_input_chip *chip, u8 addr, void *buff, size_t size)
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

static ssize_t epl2182_sensor_chip_write_data(struct hua_input_chip *chip, u8 addr, const void *buff, size_t size)
{
	struct i2c_client *client = chip->bus_data;

	char mem[size + 1];

	memcpy(mem + 1, buff, size);
	mem[0] = addr << 3 | (size - 1);

	return i2c_master_send(client, mem, size + 1);
}

static int epl2182_sensor_chip_write_register(struct hua_input_chip *chip, u8 addr, u8 value)
{
	u8 buff[] = {addr << 3, value};

	return i2c_master_send(chip->bus_data, buff, 2);
}

static int epl2182_sensor_chip_write_register16(struct hua_input_chip *chip, u8 addr, u16 value)
{
	u8 buff[] = {addr << 3 | 1, value & 0xFF, value >> 8};

	return i2c_master_send(chip->bus_data, buff, 3);
}

static int epl2182_sensor_chip_readid(struct hua_input_chip *chip)
{
	int ret;
	char buff[10];

	ret = chip->read_data(chip, REG_00, buff, sizeof(buff));
	if (ret < 0)
	{
		pr_red_info("chip->read_data");
	}

	hua_input_print_memory(buff, ret);

	return 0;
}

static inline int epl2182_sensor_chip_set_active(struct hua_input_chip *chip, bool enable)
{
	if (enable)
	{
		return 0;
	}

	return chip->write_register(chip, REG_07,  EPL_C_P_DOWN);
}

static int epl2182_sensor_chip_set_mode(struct hua_input_chip *chip, bool als)
{
	int ret;
	struct hua_epl2182_device *epl2182 = hua_input_chip_get_dev_data(chip);

#if EPL2182_SUPPORT_IRQ
	ret = chip->write_register(chip, REG_09, EPL_INT_DISABLE);
	if (ret < 0)
	{
		pr_red_info("chip->write_register REG_09");
		return ret;
	}
#endif

	if (als)
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

		epl2182->proxi_count = 0;
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

		epl2182->proxi_count = 5;
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

	msleep(EPL_SENSOR_MIN_DELAY);

	return 0;
}

static int epl2182_proximity_set_enable(struct hua_input_device *dev, bool enable)
{
	int ret;
	struct hua_input_chip *chip;

	if (enable == false)
	{
		return 0;
	}

	chip = dev->chip;

	ret = chip->write_register16(chip, REG_02, P_SENSOR_HTHD);
	if (ret < 0)
	{
		pr_red_info("chip->write_register16 REG_02");
		return ret;
	}

	ret = chip->write_register16(chip, REG_04, P_SENSOR_LTHD);
	if (ret < 0)
	{
		pr_red_info("chip->write_register16 REG_04");
		return ret;
	}

	return epl2182_sensor_chip_set_mode(chip, false);
}

static int epl2182_proximity_event_handler(struct hua_input_chip *chip, struct hua_input_device *dev)
{
	int ret;
	u8 value;
	struct hua_epl2182_device *epl2182 = hua_input_chip_get_dev_data(chip);

	if (epl2182->proxi_count == 0)
	{
		if (epl2182->light.dev.enabled)
		{
			return 0;
		}

		return epl2182_sensor_chip_set_mode(chip, false);
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

	hua_sensor_report_value(dev->input, (value & 0x04) == 0);

	if (epl2182->light.dev.enabled)
	{
		if (epl2182->proxi_count > 1)
		{
			epl2182->proxi_count--;
		}
		else
		{
			return epl2182_sensor_chip_set_mode(chip, true);
		}
	}

	return chip->write_register(chip, REG_07, EPL_DATA_UNLOCK);
}

static int epl2182_light_set_enable(struct hua_input_device *dev, bool enable)
{
	int ret;
	struct hua_input_chip *chip;

	if (enable == false)
	{
		return 0;
	}

	chip = dev->chip;

	ret = chip->write_register(chip, REG_10, EPL_GO_MID);
	if (ret < 0)
	{
		pr_red_info("chip->write_register REG_10");
		return ret;
	}

	ret = chip->write_register(chip, REG_11, EPL_GO_LOW);
	if (ret < 0)
	{
		pr_red_info("chip->write_register REG_11");
		return ret;
	}

	return epl2182_sensor_chip_set_mode(chip, true);
}

static int epl2182_light_event_handler(struct hua_input_chip *chip, struct hua_input_device *dev)
{
	int ret;
	u16 value16;
	struct hua_epl2182_device *epl2182 = hua_input_chip_get_dev_data(chip);

	if (epl2182->proxi_count > 0)
	{
		if (epl2182->proxi.dev.enabled)
		{
			return 0;
		}

		return epl2182_sensor_chip_set_mode(chip, true);
	}

	ret = chip->write_register(chip, REG_07, EPL_DATA_LOCK);
	if (ret < 0)
	{
		pr_red_info("chip->write_register REG_07");
		return ret;
	}

	ret = chip->read_register16(chip, REG_16, &value16);
	if (ret < 0)
	{
		pr_red_info("chip->read_register16 REG_16");
		return ret;
	}

	hua_sensor_report_value(dev->input, EPL_CAL_LIGHT_VALUE(value16));

	if (epl2182->proxi.dev.enabled)
	{
		return epl2182_sensor_chip_set_mode(chip, false);
	}

	return chip->write_register(chip, REG_07, EPL_DATA_UNLOCK);
}

static int epl2182_input_chip_probe(struct hua_input_chip *chip)
{
	int ret;
	struct hua_input_device *dev;
	struct hua_sensor_device *sensor;
	struct hua_epl2182_device *epl2182;

	pr_pos_info();

	epl2182 = kzalloc(sizeof(*epl2182), GFP_KERNEL);
	if (epl2182 == NULL)
	{
		pr_red_info("kzalloc");
		return -ENOMEM;
	}

	hua_input_chip_set_dev_data(chip, epl2182);

	sensor = &epl2182->proxi;
	sensor->min_delay = EPL_SENSOR_MIN_DELAY;
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

	dev->type = HUA_INPUT_DEVICE_TYPE_PROXIMITY;
	dev->poll_delay = 200;

	sensor->max_range = 1;
	sensor->resolution = 1;

	dev->set_enable = epl2182_proximity_set_enable;
	dev->event_handler = epl2182_proximity_event_handler;

	ret = hua_input_device_register(chip, dev);
	if (ret < 0)
	{
		pr_red_info("hua_input_device_register");
		goto out_kfree_sensor;
	}

	sensor = &epl2182->light;
	sensor->min_delay = EPL_SENSOR_MIN_DELAY;
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

	dev->type = HUA_INPUT_DEVICE_TYPE_LIGHT;
	dev->poll_delay = 200;
	sensor->max_range = EPL_LIGHT_MAX_VALUE;
	sensor->resolution = EPL_LIGHT_MAX_VALUE;
	dev->set_enable = epl2182_light_set_enable;
	dev->event_handler = epl2182_light_event_handler;

	ret = hua_input_device_register(chip, dev);
	if (ret < 0)
	{
		pr_red_info("hua_input_device_register");
		goto out_hua_input_device_unregister_proxi;
	}

	return 0;

out_hua_input_device_unregister_proxi:
	hua_input_device_unregister(chip, &epl2182->proxi.dev);
out_kfree_sensor:
	kfree(sensor);
	return ret;
}

static void epl2182_input_chip_remove(struct hua_input_chip *chip)
{
	struct hua_epl2182_device *epl2182 = hua_input_chip_get_dev_data(chip);

	pr_pos_info();

	hua_input_device_unregister(chip, &epl2182->light.dev);
	hua_input_device_unregister(chip, &epl2182->proxi.dev);
	kfree(epl2182);
}

static struct hua_input_init_data epl2182_init_data[] =
{
	{REG_00, EPL_S_SENSING_MODE},
	{REG_07, EPL_C_P_DOWN},
	{REG_09, EPL_INT_DISABLE}
};

static int epl2182_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	struct hua_input_chip *chip;

	pr_pos_info();

	chip = kzalloc(sizeof(*chip), GFP_KERNEL);
	if (chip == NULL)
	{
		pr_red_info("kzalloc");
		return -ENOMEM;
	}

	i2c_set_clientdata(client, chip);
	hua_input_chip_set_bus_data(chip, client);

#if EPL2182_SUPPORT_IRQ
	chip->irq = client->irq;
	chip->irq_flags = IRQF_TRIGGER_FALLING;
#else
	chip->irq = -1;
#endif

	pr_bold_info("chip->irq = %d", chip->irq);

	chip->name = "EPL2182";
	chip->poweron_init = true;
	chip->devmask = 1 << HUA_INPUT_DEVICE_TYPE_PROXIMITY | 1 << HUA_INPUT_DEVICE_TYPE_LIGHT;
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

	ret = hua_input_chip_register(chip, &client->dev);
	if (ret < 0)
	{
		pr_red_info("hua_input_chip_register");
		goto out_kfree_chip;
	}

	return 0;

out_kfree_chip:
	kfree(chip);
	return ret;
}

static int epl2182_i2c_remove(struct i2c_client *client)
{
	struct hua_input_chip *chip = i2c_get_clientdata(client);

	pr_pos_info();

	hua_input_chip_unregister(chip);
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

static int __init hua_epl2182_init(void)
{
	pr_pos_info();

	return i2c_add_driver(&epl2182_driver);
}

static void __exit hua_epl2182_exit(void)
{
	pr_pos_info();

	i2c_del_driver(&epl2182_driver);
}

module_init(hua_epl2182_init);
module_exit(hua_epl2182_exit);

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Huamobile EPL2182 Proximity and Light Sensor Driver");
MODULE_LICENSE("GPL");
