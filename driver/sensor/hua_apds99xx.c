#include <huamobile/hua_sensor.h>
#include <huamobile/hua_i2c.h>
#include <huamobile/hua_io.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>

#define APDS99XX_SUPPORT_IRQ			1

#define APDS99XX_LIGHT_STEP				5
#define APDS99XX_LIGHT_MAX_VALUE		30000
#define APDS99XX_LIGHT_Bx100			223
#define APDS99XX_LIGHT_Cx100			70
#define APDS99XX_LIGHT_Dx100			142
#define APDS99XX_LIGHT_GAx100			48
#define APDS99XX_LIGHT_DF				52
#define APDS99XX_LIGHT_SHIFT			2

#define APDS99XX_PROXI_STEP				5
#define APDS99XX_PROXI_THRESHOLD		500
#define APDS99XX_PROXI_MAX_VALUE		1023

#define APDS99XX_ID_9900				0x29
#define APDS99XX_ID_9901				0x20
#define APDS99XX_ID_9930				0x39
#define APDS99XX_ID_9931				0x30

#define APDS99XX_CMD_BYTE				0x80
#define APDS99XX_CMD_WORD				0xA0
#define APDS99XX_CMD_SPEC				0xE0

#define APDS99XX_ENABLE_SAI				(1 << 6)
#define APDS99XX_ENABLE_PIEN			(1 << 5)
#define APDS99XX_ENABLE_AIEN			(1 << 4)
#define APDS99XX_ENABLE_WEN				(1 << 3)
#define APDS99XX_ENABLE_PEN				(1 << 2)
#define APDS99XX_ENABLE_AEN				(1 << 1)
#define APDS99XX_ENABLE_PON				(1 << 0)

#define APDS99XX_STATE_PSAT				(1 << 6)
#define APDS99XX_STATE_PINT				(1 << 5)
#define APDS99XX_STATE_AINT				(1 << 4)
#define APDS99XX_STATE_PVALID			(1 << 1)
#define APDS99XX_STATE_AVALID			(1 << 0)

#define APDS99XX_CONFIG_AGL				(1 << 2)
#define APDS99XX_CONFIG_WLONG			(1 << 1)
#define APDS99XX_CONFIG_PDL				(1 << 0)

#define APDS99XX_CONTROL_PDRIVE_OFFSET	6
#define APDS99XX_CONTROL_PDRIVE_MASK	(0x03 << APDS99XX_CONTROL_PDRIVE_OFFSET)
#define APDS99XX_CONTROL_PDIODE_OFFSET	4
#define APDS99XX_CONTROL_PDIODE_MASK	(0x03 << APDS99XX_CONTROL_PDIODE_OFFSET)
#define APDS99XX_CONTROL_PGAIN_OFFSET	2
#define APDS99XX_CONTROL_PGAIN_MASK		(0x03 << APDS99XX_CONTROL_PGAIN_OFFSET)
#define APDS99XX_CONTROL_AGAIN_OFFSET	0
#define APDS99XX_CONTROL_AGAIN_MASK		(0x03 << APDS99XX_CONTROL_AGAIN_OFFSET)

#define APDS99XX_COMMAND_IPCLEAR		0x05
#define APDS99XX_COMMAND_IACLEAR		0x06
#define APDS99XX_COMMAND_IAPCLEAR		0x07

#define APDS99XX_PINT_MASK				(/* APDS99XX_STATE_PINT | */ APDS99XX_STATE_PVALID)
#define APDS99XX_AINT_MASK				(/* APDS99XX_STATE_AINT | */ APDS99XX_STATE_AVALID)

#define APDS99XX_DEVICE_NAME			"apds99xx"

enum apds99xx_register_map
{
	REG_ENABLE = 0x00,
	REG_ATIME,
	REG_PTIME,
	REG_WTIME,
	REG_AILTL,
	REG_AILTH,
	REG_AIHTL,
	REG_AIHTH,
	REG_PILTL,
	REG_PILTH,
	REG_PIHTL,
	REG_PIHTH,
	REG_PERS,
	REG_CONFIG,
	REG_PPCOUNT,
	REG_CONTROL,
	REG_REV = 0x11,
	REG_ID,
	REG_STATUS,
	REG_CH0DATAL,
	REG_CH0DATAH,
	REG_CH1DATAL,
	REG_CH1DATAH,
	REG_PDATAL,
	REG_PDATAH,
	REG_POFFSET = 0x1E,
};

struct hua_apds99xx_chip
{
	struct hua_input_chip chip;
	struct regulator *vdd;
	struct regulator *vio;
};

struct hua_apds99xx_device
{
	u32 alpc;
	u32 adelay;
	bool areduce;

	u8 again;
	u8 areduce_value;

	u16 adata_high;
	u16 adata_low;

	struct hua_sensor_device proxi;
	struct hua_sensor_device light;
};

static struct hua_sensor_rate_table_node apds99xx_arate_table[] =
{
	{0xF6, 27200},
	{0xED, 51680},
	{0xDB, 100640}
};

static const u32 apds99xx_pgain_table[] = {1, 2, 4, 8};
static const u8 apds99xx_again_table[] = {1, 8, 16, 120};
static const u32 apds99xx_ares_table[] = {10240, 19456, 37888};

static const char *apds99xx_get_chip_name_by_id(u8 id)
{
	switch (id)
	{
	case APDS99XX_ID_9900:
		return "APDS9900";
	case APDS99XX_ID_9901:
		return "APDS9901";
	case APDS99XX_ID_9930:
		return "APDS9930";
	case APDS99XX_ID_9931:
		return "APDS9931";
	default:
		return NULL;
	}
}

static inline int apds99xx_read_register(struct hua_input_chip *chip, u8 addr, u8 *value)
{
	return hua_input_read_register_i2c_smbus(chip, APDS99XX_CMD_BYTE | addr, value);
}

static inline int apds99xx_write_register(struct hua_input_chip *chip, u8 addr, u8 value)
{
	return hua_input_write_register_i2c_smbus(chip, APDS99XX_CMD_BYTE | addr, value);
}

static inline int apds99xx_read_register16(struct hua_input_chip *chip, u8 addr, u16 *value)
{
	return hua_input_read_register16_i2c_smbus(chip, APDS99XX_CMD_WORD | addr, value);
}

static inline int apds99xx_write_register16(struct hua_input_chip *chip, u8 addr, u16 value)
{
	return hua_input_write_register16_i2c_smbus(chip, APDS99XX_CMD_WORD | addr, value);
}

static inline int apds99xx_send_command(struct hua_input_chip *chip, u8 command)
{
	command |= APDS99XX_CMD_SPEC;

	return chip->master_send(chip, &command, 1);
}

static int apds99xx_sensor_chip_readid(struct hua_input_chip *chip)
{
	int ret;
	u8 value;
	const char *name;

	ret = chip->write_register(chip, REG_ENABLE, 0);
	if (ret < 0)
	{
		pr_red_info("apds99xx_write_enable_register");
		return ret;
	}

	ret = chip->read_register(chip, REG_ID, &value);
	if (ret < 0)
	{
		pr_red_info("chip->read_register");
		return ret;
	}

	name = apds99xx_get_chip_name_by_id(value);
	if (name == NULL)
	{
		pr_red_info("unknown chip id = 0x%02x", value);
		return -EINVAL;
	}

	chip->name = name;
	chip->devid = value;

	return 0;
}

static int apds99xx_sensor_chip_set_power(struct hua_input_chip *chip, bool enable)
{
	struct hua_apds99xx_chip *apds99xx = (struct hua_apds99xx_chip *) chip;

	if (enable)
	{
		if (apds99xx->vio)
		{
			regulator_enable(apds99xx->vio);
		}

		if (apds99xx->vdd)
		{
			regulator_enable(apds99xx->vdd);
		}

		msleep(20);
	}
	else
	{
		if (apds99xx->vio)
		{
			regulator_disable(apds99xx->vio);
		}

		if (apds99xx->vdd)
		{
			regulator_disable(apds99xx->vdd);
		}
	}

	return 0;
}

static int apds99xx_sensor_chip_set_active(struct hua_input_chip *chip, bool enable)
{
	int ret;

	pr_pos_info();

	if (enable == false)
	{
		return chip->write_register(chip, REG_ENABLE, 0);
	}

	ret = hua_io_write_register_masked(chip, REG_ENABLE, APDS99XX_ENABLE_PON, APDS99XX_ENABLE_PON);
	if (ret < 0)
	{
		pr_red_info("hua_io_write_register_masked");
		return ret;
	}

	return 0;
}

static int apds99xx_proximity_set_enable(struct hua_input_device *dev, bool enable)
{
	int ret;
	u8 value;
	struct hua_input_chip *chip = dev->chip;

	pr_pos_info();

	if (enable == false)
	{
		value = 0;
	}
#if APDS99XX_SUPPORT_IRQ
	else if (dev->use_irq)
	{
		value = APDS99XX_ENABLE_PEN | APDS99XX_ENABLE_PIEN;
	}
#endif
	else
	{
		value = APDS99XX_ENABLE_PEN;
	}

	pr_bold_info("value = 0x%02x", value);

	ret = hua_io_write_register_masked(chip, REG_ENABLE, value, APDS99XX_ENABLE_PEN | APDS99XX_ENABLE_PIEN);
	if (ret < 0)
	{
		pr_red_info("hua_io_write_register_masked");
		return ret;
	}

	msleep(20);
	chip->event_handler(chip);

	return 0;
}

static int apds99xx_proximity_set_delay(struct hua_input_device *dev, unsigned int delay)
{
	pr_pos_info();

	return dev->chip->write_register(dev->chip, REG_PTIME, 0xFF);
}

static ssize_t apds99xx_proximity_calibration(struct hua_input_device *dev, char *buff, size_t size, bool store)
{
	int ret;
	s8 offset;
	struct hua_input_chip *chip = dev->chip;

	pr_pos_info();

	if (store)
	{
		offset = simple_strtol(buff, NULL, 10);

		pr_bold_info("offset = %d", offset);

		ret = chip->write_register(chip, REG_POFFSET, offset);
		if (ret < 0)
		{
			pr_red_info("chip->write_register");
			return ret;
		}

		return size;
	}
	else
	{
		offset = 0;

		while (1)
		{
			int i;
			u8 status;
			u16 value;
			int retry;
			u32 avg_value;

			ret = chip->write_register(chip, REG_POFFSET, offset);
			if (ret < 0)
			{
				pr_red_info("chip->write_register");
				return ret;
			}

			for (i = 0, avg_value = 0; i < 5; i++)
			{
				apds99xx_send_command(chip, APDS99XX_COMMAND_IAPCLEAR);

				retry = 100;

				while (1)
				{
					msleep(10);

					ret = chip->read_register(chip, REG_STATUS, &status);
					if (ret >= 0 && (status & APDS99XX_STATE_PVALID))
					{
						break;
					}

					if (--retry < 1)
					{
						return -ETIMEDOUT;
					}
				}

				ret = chip->read_register16(chip, REG_PDATAL, &value);
				if (ret < 0)
				{
					pr_red_info("chip->read_register16");
					return ret;
				}

				if (i == 0)
				{
					avg_value = value;
				}
				else
				{
					avg_value = (avg_value + value) >> 1;
				}
			}

			if (avg_value > 0 && avg_value < 10)
			{
				break;
			}

			if (avg_value > 0)
			{
				if (offset < 0)
				{
					if (offset > -127)
					{
						offset--;
					}
					else
					{
						offset = 0;
					}
				}
				else
				{
					if (offset < 127)
					{
						offset++;
					}
					else
					{
						break;
					}
				}
			}
			else
			{
				if (offset < 0)
				{
					if (offset < -1)
					{
						offset++;
					}
					else
					{
						break;
					}
				}
				else
				{
					if (offset > 0)
					{
						offset--;
					}
					else
					{
						offset = -127;
					}
				}
			}
		}

		return sprintf(buff, "%d\n", offset);
	}
}

static int apds99xx_proximity_event_handler(struct hua_input_chip *chip, struct hua_input_device *dev)
{
	int ret;
	u16 value;
#if APDS99XX_SUPPORT_IRQ
	int pilt, piht;

	if ((chip->irq_state & APDS99XX_PINT_MASK) != APDS99XX_PINT_MASK)
	{
		return 0;
	}
#endif

	ret = chip->read_register16(chip, REG_PDATAL, &value);
	if (ret < 0)
	{
		pr_red_info("chip->read_register16");
		return ret;
	}

	hua_sensor_report_value(dev->input, (APDS99XX_PROXI_MAX_VALUE - value) >> APDS99XX_PROXI_STEP);

#if APDS99XX_SUPPORT_IRQ
	pilt = value > (1 << APDS99XX_PROXI_STEP) ? (value - (1 << APDS99XX_PROXI_STEP)) : 0;
	piht = value + (1 << APDS99XX_PROXI_STEP);

	chip->write_register16(chip, REG_PILTL, pilt);
	chip->write_register16(chip, REG_PIHTL, piht);
#endif

	return 0;
}

static void apds99xx_update_alpc(struct hua_apds99xx_device *apds99xx)
{
	int again = apds99xx_again_table[apds99xx->again];

	apds99xx->alpc = (apds99xx->adelay * again * apds99xx->areduce_value) >> APDS99XX_LIGHT_SHIFT;

	pr_bold_info("again[%d] = %d, adelay = %d, areduce_value = %d, alpc = %d", apds99xx->again, again, apds99xx->adelay, apds99xx->areduce_value, apds99xx->alpc);
}

static int apds99xx_set_again(struct hua_apds99xx_device *apds99xx, struct hua_input_chip *chip, int again)
{
	int ret;

	ret = hua_io_write_register_masked(chip, REG_CONTROL, again << APDS99XX_CONTROL_AGAIN_OFFSET, APDS99XX_CONTROL_AGAIN_MASK);
	if (ret < 0)
	{
		pr_red_info("hua_io_write_register_masked");
		return ret;
	}

	apds99xx->again = again;
	apds99xx_update_alpc(apds99xx);

	return 0;
}

static int apds99xx_set_areduce(struct hua_apds99xx_device *apds99xx, struct hua_input_chip *chip, bool reduce)
{
	int ret;

	ret = hua_io_write_register_masked(chip, REG_CONFIG, reduce ? APDS99XX_CONFIG_AGL : 0, APDS99XX_CONFIG_AGL);
	if (ret < 0)
	{
		pr_red_info("hua_io_write_register_masked");
		return ret;
	}

	apds99xx->areduce = reduce;
	apds99xx->areduce_value = reduce ? 16 : 100;
	apds99xx_update_alpc(apds99xx);

	return 0;
}

static int apds99xx_light_set_enable(struct hua_input_device *dev, bool enable)
{
	int ret;
	u8 value;
	struct hua_input_chip *chip = dev->chip;
	struct hua_apds99xx_device *apds99xx = hua_input_chip_get_dev_data(chip);

	pr_pos_info();

	if (enable == false)
	{
		value = 0;
	}
#if APDS99XX_SUPPORT_IRQ
	else
	{
		if (dev->use_irq)
		{
			chip->write_register16(chip, REG_AILTL, 0xFFFF);
			chip->write_register16(chip, REG_AIHTL, 0x0000);

			value = APDS99XX_ENABLE_AEN | APDS99XX_ENABLE_AIEN;
		}
#endif
		else
		{
			value = APDS99XX_ENABLE_AEN;
		}

		if (apds99xx->adelay == 0)
		{
			dev->set_delay(dev, dev->poll_delay);
		}

		apds99xx_set_again(apds99xx, chip, 0);
		apds99xx_set_areduce(apds99xx, chip, false);
	}

	pr_bold_info("value = 0x%02x", value);

	ret = hua_io_write_register_masked(chip, REG_ENABLE, value, APDS99XX_ENABLE_AEN | APDS99XX_ENABLE_AIEN);
	if (ret < 0)
	{
		pr_red_info("hua_io_write_register_masked");
		return ret;
	}

	return 0;
}

static int apds99xx_light_set_delay(struct hua_input_device *dev, unsigned int delay)
{
	int ret;
	u16 ares;
	struct hua_apds99xx_device *apds99xx;
	const struct hua_sensor_rate_table_node *p = hua_sensor_find_rate_value(apds99xx_arate_table, ARRAY_SIZE(apds99xx_arate_table), delay * 1000);

	ret = dev->chip->write_register(dev->chip, REG_ATIME, p->value);
	if (ret < 0)
	{
		pr_red_info("dev->chip->write_register");
		return ret;
	}

	apds99xx = hua_input_chip_get_dev_data(dev->chip);
	apds99xx->adelay = p->delay_ns / 10;
	apds99xx_update_alpc(apds99xx);

	ares = apds99xx_ares_table[p - apds99xx_arate_table];
	apds99xx->adata_low = ares * 10 / 100;
	apds99xx->adata_high = ares - apds99xx->adata_low;
	pr_bold_info("ares = %d, adata_high = %d, adata_low = %d", ares, apds99xx->adata_high, apds99xx->adata_low);

	return 0;
}

static void apds99xx_light_report_value(struct hua_input_device *dev, struct hua_apds99xx_device *apds99xx, u16 ch0data, u16 ch1data)
{
	int iac1, iac2, iac, lux;

	iac1 = ch0data * 100 - APDS99XX_LIGHT_Bx100 * ch1data;
	iac2 = APDS99XX_LIGHT_Cx100 * ch0data - APDS99XX_LIGHT_Dx100 * ch1data;
	iac = iac1 > iac2 ? iac1 : iac2;
	if (iac < 0)
	{
		if (apds99xx->areduce == false)
		{
			return;
		}

		lux = APDS99XX_LIGHT_MAX_VALUE;
	}
	else
	{
		lux = ((((u32) iac) >> APDS99XX_LIGHT_SHIFT) * APDS99XX_LIGHT_GAx100 * APDS99XX_LIGHT_DF) / apds99xx->alpc;
	}

	hua_sensor_report_value(dev->input, lux);
}

#if APDS99XX_SUPPORT_IRQ
static void apds99xx_set_ait(struct hua_input_chip *chip, u32 data)
{
	u32 ailt, aiht;

	ailt = (data * ((1 << APDS99XX_LIGHT_STEP) - 1)) >> APDS99XX_LIGHT_STEP;
	aiht = (data * ((1 << APDS99XX_LIGHT_STEP) + 1)) >> APDS99XX_LIGHT_STEP;

	chip->write_register16(chip, REG_AILTL, ailt);
	chip->write_register16(chip, REG_AIHTL, aiht);
}
#endif

static int apds99xx_light_event_handler(struct hua_input_chip *chip, struct hua_input_device *dev)
{
	int ret;
	u16 ch0data, ch1data;
	struct hua_apds99xx_device *apds99xx = hua_input_chip_get_dev_data(chip);

#if APDS99XX_SUPPORT_IRQ
	if ((chip->irq_state & APDS99XX_AINT_MASK) != APDS99XX_AINT_MASK)
	{
		return 0;
	}
#endif

	ret = chip->read_register16(chip, REG_CH0DATAL, &ch0data);
	if (ret < 0)
	{
		pr_red_info("chip->read_register16 REG_CH0DATAL");
		return ret;
	}

	ret = chip->read_register16(chip, REG_CH1DATAL, &ch1data);
	if (ret < 0)
	{
		pr_red_info("chip->read_register16 REG_CH1DATAL");
		return ret;
	}

	apds99xx_light_report_value(dev, apds99xx, ch0data, ch1data);

	if (ch0data > apds99xx->adata_high)
	{
		if (apds99xx->again > 0)
		{
			apds99xx_set_again(apds99xx, chip, apds99xx->again - 1);
		}
		else if (apds99xx->areduce == false)
		{
			apds99xx_set_areduce(apds99xx, chip, true);
		}
#if APDS99XX_SUPPORT_IRQ
		else
		{
			apds99xx_set_ait(chip, ch0data);
		}
#endif
	}
	else if (ch0data < apds99xx->adata_low)
	{
		if (apds99xx->areduce)
		{
			apds99xx_set_areduce(apds99xx, chip, false);
		}
		else if (apds99xx->again < ARRAY_SIZE(apds99xx_again_table) - 1)
		{
			apds99xx_set_again(apds99xx, chip, apds99xx->again + 1);
		}
#if APDS99XX_SUPPORT_IRQ
		else
		{
			apds99xx_set_ait(chip, ch0data);
		}
#endif
	}
#if APDS99XX_SUPPORT_IRQ
	else
	{
		apds99xx_set_ait(chip, ch0data);
	}
#endif

	return 0;
}

#if APDS99XX_SUPPORT_IRQ
static int apds99xx_chip_event_handler(struct hua_input_chip *chip)
{
	int ret;

	ret = chip->read_register(chip, REG_STATUS, (u8 *) &chip->irq_state);
	if (ret < 0)
	{
		pr_red_info("chip->read_register");
		return ret;
	}

	ret = hua_input_chip_report_events(chip, &chip->isr_list);
	if (ret < 0)
	{
		pr_red_info("hua_input_chip_report_events");
		return ret;
	}

	apds99xx_send_command(chip, APDS99XX_COMMAND_IAPCLEAR);

	return 0;
}
#endif

static int apds99xx_input_chip_probe(struct hua_input_chip *chip)
{
	int ret;
	struct hua_input_device *dev;
	struct hua_sensor_device *sensor;
	struct hua_apds99xx_device *apds99xx;

	pr_pos_info();

	apds99xx = kzalloc(sizeof(*apds99xx), GFP_KERNEL);
	if (apds99xx == NULL)
	{
		pr_red_info("kzalloc");
		return -ENOMEM;
	}

	hua_input_chip_set_dev_data(chip, apds99xx);

	sensor = &apds99xx->proxi;
	sensor->min_delay = 2;
	sensor->power_consume = 145;

	dev = &sensor->dev;
	dev->name = "ADPS99XX-PS";
	dev->fuzz = 0;
	dev->flat = 0;

#if APDS99XX_SUPPORT_IRQ
	dev->use_irq = chip->irq >= 0;
#else
	dev->use_irq = false;
#endif

	dev->type = HUA_INPUT_DEVICE_TYPE_PROXIMITY;
	dev->poll_delay = 200;

	sensor->max_range = APDS99XX_PROXI_MAX_VALUE >> APDS99XX_PROXI_STEP;
	sensor->resolution = APDS99XX_PROXI_MAX_VALUE >> APDS99XX_PROXI_STEP;

	dev->set_enable = apds99xx_proximity_set_enable;
	dev->set_delay = apds99xx_proximity_set_delay;
	dev->calibration = apds99xx_proximity_calibration;
	dev->event_handler = apds99xx_proximity_event_handler;

	ret = hua_input_device_register(chip, dev);
	if (ret < 0)
	{
		pr_red_info("hua_input_device_register");
		goto out_kfree_sensor;
	}

	sensor = &apds99xx->light;
	sensor->min_delay = 2;
	sensor->power_consume = 145;

	dev = &sensor->dev;
	dev->name = "ADPS99XX-LS";
	dev->fuzz = 0;
	dev->flat = 0;

#if APDS99XX_SUPPORT_IRQ
	dev->use_irq = chip->irq >= 0;
#else
	dev->use_irq = false;
#endif

	dev->type = HUA_INPUT_DEVICE_TYPE_LIGHT;
	dev->poll_delay = 200;
	sensor->max_range = APDS99XX_LIGHT_MAX_VALUE;
	sensor->resolution = APDS99XX_LIGHT_MAX_VALUE;
	dev->set_enable = apds99xx_light_set_enable;
	dev->set_delay = apds99xx_light_set_delay;
	dev->event_handler = apds99xx_light_event_handler;

	ret = hua_input_device_register(chip, dev);
	if (ret < 0)
	{
		pr_red_info("hua_input_device_register");
		goto out_hua_input_device_unregister_proxi;
	}

	return 0;

out_hua_input_device_unregister_proxi:
	hua_input_device_unregister(chip, &apds99xx->proxi.dev);
out_kfree_sensor:
	kfree(sensor);
	return ret;
}

static void apds99xx_input_chip_remove(struct hua_input_chip *chip)
{
	struct hua_apds99xx_device *apds99xx = hua_input_chip_get_dev_data(chip);

	pr_pos_info();

	hua_input_device_unregister(chip, &apds99xx->light.dev);
	hua_input_device_unregister(chip, &apds99xx->proxi.dev);
	kfree(apds99xx);
}

static struct hua_input_init_data apds99xx_init_data[] =
{
	{REG_ENABLE, 0x00},
	{REG_WTIME, 0xFF},
	{REG_PPCOUNT, 0x08},
	{REG_CONFIG, 0x00},
	{REG_CONTROL, 0x21},
	{REG_PERS, 0x11},
};

static int apds99xx_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	struct hua_input_chip *chip;
	struct hua_apds99xx_chip *apds99xx;

	pr_pos_info();

	apds99xx = kzalloc(sizeof(*apds99xx), GFP_KERNEL);
	if (apds99xx == NULL)
	{
		pr_red_info("kzalloc");
		return -ENOMEM;
	}

	apds99xx->vdd = regulator_get(&client->dev, "vdd");
	apds99xx->vio = regulator_get(&client->dev, "vio");

	i2c_set_clientdata(client, apds99xx);
	chip = &apds99xx->chip;
	hua_input_chip_set_bus_data(chip, client);

#if APDS99XX_SUPPORT_IRQ
	chip->irq = client->irq;
	chip->irq_flags = IRQF_TRIGGER_FALLING;
#else
	chip->irq = -1;
#endif

	pr_bold_info("chip->irq = %d", chip->irq);

	chip->name = "APDS99XX";
	chip->poweron_init = true;
	chip->devmask = 1 << HUA_INPUT_DEVICE_TYPE_PROXIMITY | 1 << HUA_INPUT_DEVICE_TYPE_LIGHT;
	chip->init_data = apds99xx_init_data;
	chip->init_data_size = ARRAY_SIZE(apds99xx_init_data);
	chip->read_data = hua_input_read_data_i2c;
	chip->read_register = apds99xx_read_register;
	chip->read_register16 = apds99xx_read_register16;
	chip->write_data = hua_input_write_data_i2c;
	chip->write_register = apds99xx_write_register;
	chip->write_register16 = apds99xx_write_register16;
	chip->readid = apds99xx_sensor_chip_readid;
	chip->set_active = apds99xx_sensor_chip_set_active;
	chip->set_power = apds99xx_sensor_chip_set_power;

#if APDS99XX_SUPPORT_IRQ
	chip->event_handler = apds99xx_chip_event_handler;
#endif

	chip->probe = apds99xx_input_chip_probe;
	chip->remove = apds99xx_input_chip_remove;

	ret = hua_input_chip_register(chip);
	if (ret < 0)
	{
		pr_red_info("hua_input_chip_register");
		goto out_regulator_put;
	}

	return 0;

out_regulator_put:
	if (apds99xx->vio)
	{
		regulator_put(apds99xx->vio);
	}

	if (apds99xx->vdd)
	{
		regulator_put(apds99xx->vdd);
	}

	kfree(chip);
	return ret;
}

static int apds99xx_i2c_remove(struct i2c_client *client)
{
	struct hua_apds99xx_chip *apds99xx = i2c_get_clientdata(client);
	struct hua_input_chip *chip = &apds99xx->chip;

	pr_pos_info();

	hua_input_chip_unregister(chip);

	if (apds99xx->vio)
	{
		regulator_put(apds99xx->vio);
	}

	if (apds99xx->vdd)
	{
		regulator_put(apds99xx->vdd);
	}

	kfree(apds99xx);

	return 0;
}

static const struct i2c_device_id apds99xx_id[] =
{
	{APDS99XX_DEVICE_NAME, 0}, {}
};

MODULE_DEVICE_TABLE(i2c, apds99xx_id);

static struct of_device_id apds99xx_match_table[] =
{
	{
		.compatible = "avago," APDS99XX_DEVICE_NAME
	},
	{}
};

static struct i2c_driver apds99xx_driver =
{
	.driver =
	{
		.name = APDS99XX_DEVICE_NAME,
		.owner = THIS_MODULE,
		.of_match_table = apds99xx_match_table,
	},

	.probe = apds99xx_i2c_probe,
	.remove = apds99xx_i2c_remove,
	.id_table = apds99xx_id,
};

static int __init hua_apds99xx_init(void)
{
	pr_pos_info();

	return i2c_add_driver(&apds99xx_driver);
}

static void __exit hua_apds99xx_exit(void)
{
	pr_pos_info();

	i2c_del_driver(&apds99xx_driver);
}

module_init(hua_apds99xx_init);
module_exit(hua_apds99xx_exit);

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Huamobile apds99xx Three-Axis Digital Accelerometer Driver");
MODULE_LICENSE("GPL");
