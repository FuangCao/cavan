#include <huamobile/hua_io.h>

int hua_io_write_register_masked(struct hua_input_chip *chip, u8 addr, u8 value, u8 mask)
{
	int ret;
	u8 old_value;

	ret = chip->read_register(chip, addr, &old_value);
	if (ret < 0)
	{
		pr_red_info("chip->read_register");
		return ret;
	}

	value |= old_value & (~mask);

	pr_bold_info("addr = 0x%02x, value = (0x%02x -> 0x%02x)", addr, old_value, value);

	return chip->write_register(chip, addr, value);
}

EXPORT_SYMBOL_GPL(hua_io_write_register_masked);

int hua_io_write_register16_masked(struct hua_input_chip *chip, u8 addr, u16 value, u16 mask)
{
	int ret;
	u16 old_value;

	ret = chip->read_register16(chip, addr, &old_value);
	if (ret < 0)
	{
		pr_red_info("chip->read_register");
		return ret;
	}


	value |= old_value & (~mask);

	pr_bold_info("addr = 0x%02x, value = (0x%02x -> 0x%02x)", addr, old_value, value);

	return chip->write_register16(chip, addr, value);
}

EXPORT_SYMBOL_GPL(hua_io_write_register16_masked);

ssize_t hua_io_read_write_file(const char *pathname, const char *buff, size_t size, bool store)
{
	loff_t pos;
	ssize_t rwlen;
	mm_segment_t fs;
	struct file *fp;

	fp = filp_open(pathname, store ? (O_WRONLY | O_CREAT | O_TRUNC) : O_RDONLY, 0644);
	if (IS_ERR(fp))
	{
		if (store)
		{
			pr_red_info("write file `%s' failed", pathname);
		}

		return PTR_ERR(fp);
	}

	pos = 0;
	fs = get_fs();
	set_fs(get_ds());

	rwlen =  store ? vfs_write(fp, (char __user *) buff, size, &pos) : vfs_read(fp, (char __user *) buff, size, &pos);

	set_fs(fs);

	filp_close(fp, NULL);

	return rwlen;
}

EXPORT_SYMBOL_GPL(hua_io_read_write_file);

int hua_io_gpio_set_value(int gpio, int value)
{
	if (gpio < 0)
	{
		return -EINVAL;
	}

	if (value < 0)
	{
		return gpio_direction_input(gpio);
	}
	else if (value > 0)
	{
		int ret;

		ret = gpio_direction_output(gpio, 1);
		if (ret < 0)
		{
			return ret;
		}

		gpio_set_value(gpio, 1);
	}
	else
	{
		int ret;

		ret = gpio_direction_output(gpio, 0);
		if (ret < 0)
		{
			return ret;
		}

		gpio_set_value(gpio, 0);
	}

	return 0;
}

EXPORT_SYMBOL_GPL(hua_io_gpio_set_value);

void sprd_ts_power_enable(bool enable)
{
}
EXPORT_SYMBOL_GPL(sprd_ts_power_enable);

#ifdef CONFIG_ARCH_SC8810
#include <mach/mfp.h>
#include <mach/ldo.h>

int hua_io_set_power_regulator(struct hua_input_chip *chip, bool enable)
{
	if (chip->devmask & (1 << HUA_INPUT_DEVICE_TYPE_TOUCHSCREEN))
	{
		if (enable)
		{
			LDO_SetVoltLevel(LDO_LDO_SIM2, LDO_VOLT_LEVEL0);
			LDO_TurnOnLDO(LDO_LDO_SIM2);
		}
		else
		{
			LDO_TurnOffLDO(LDO_LDO_SIM2);
		}

		return 0;
	}

	return -EFAULT;
}
#else
int hua_io_set_power_regulator(struct hua_input_chip *chip, bool enable)
{
	int ret;

	pr_pos_info();

	if (enable)
	{
		if (chip->vio)
		{
			if (chip->vio_vol_min > 0 && chip->vio_vol_max >= chip->vio_vol_min)
			{
				regulator_set_voltage(chip->vio, chip->vio_vol_min, chip->vio_vol_max);
			}

			ret = regulator_enable(chip->vio);
			if (ret < 0)
			{
				pr_red_info("regulator_enable vio");
				return ret;
			}
		}

		if (chip->vdd)
		{
			if (chip->vdd_vol_min > 0 && chip->vdd_vol_max >= chip->vdd_vol_min)
			{
				regulator_set_voltage(chip->vdd, chip->vdd_vol_min, chip->vdd_vol_max);
			}

			ret = regulator_enable(chip->vdd);
			if (ret < 0)
			{
				pr_red_info("regulator_enable vdd");
				goto label_regulator_disable_vio;
			}
		}

		return 0;
	}

	if (chip->gpio_power >= 0)
	{
		gpio_direction_output(chip->gpio_power, 0);
	}

	if (chip->vdd)
	{
		regulator_disable(chip->vdd);
	}

	ret = 0;

label_regulator_disable_vio:
	if (chip->vio)
	{
		regulator_disable(chip->vio);
	}

	return ret;
}
#endif

EXPORT_SYMBOL_GPL(hua_io_set_power_regulator);

#ifdef CONFIG_OF
int hua_input_chip_io_init(struct hua_input_chip *chip)
{
	int ret;
	u32 voltag[2];
	struct device_node *of_node = chip->dev->of_node;

	ret = of_get_named_gpio_flags(of_node, "reset-gpio-pin", 0, NULL);
	if (ret >= 0)
	{
		gpio_request(ret, "HUA-RESET");
		gpio_direction_output(chip->gpio_reset, 0);
	}

	chip->gpio_reset = ret;

	ret = of_get_named_gpio_flags(of_node, "irq-gpio-pin", 0, NULL);
	if (ret >= 0)
	{
		gpio_request(ret, "HUA-IRQ");
		gpio_direction_input(chip->gpio_irq);
	}

	chip->gpio_irq = ret;

	ret = of_get_named_gpio_flags(of_node, "power-gpio-pin", 0, NULL);
	if (ret >= 0)
	{
		gpio_request(ret, "HUA-POWER");
		gpio_direction_output(chip->gpio_power, 0);
	}

	chip->gpio_power = ret;

	pr_bold_info("%s: gpio_reset = %d, gpio_irq = %d, gpio_power = %d", chip->name, chip->gpio_reset, chip->gpio_irq, chip->gpio_power);

	chip->vdd = regulator_get(chip->dev, "vdd");
	if (IS_ERR(chip->vdd))
	{
		pr_red_info("regulator_get vdd");
		chip->vdd = NULL;
	}
	else if (regulator_count_voltages(chip->vdd) > 0 && of_property_read_u32_array(of_node, "vdd-voltage-level", voltag, ARRAY_SIZE(voltag)) >= 0)
	{
		chip->vdd_vol_min = voltag[0];
		chip->vdd_vol_max = voltag[1];
	}
	else
	{
		chip->vdd_vol_min = chip->vdd_vol_max = -1;
	}

	chip->vio = regulator_get(chip->dev, "vio");
	if (IS_ERR(chip->vio))
	{
		pr_red_info("regulator_get vio");
		chip->vio = NULL;
	}
	else if (regulator_count_voltages(chip->vio) > 0 && of_property_read_u32_array(of_node, "vio-voltage-level", voltag, ARRAY_SIZE(voltag)) >= 0)
	{
		chip->vio_vol_min = voltag[0];
		chip->vio_vol_max = voltag[1];
	}
	else
	{
		chip->vio_vol_min = chip->vio_vol_max = -1;
	}

	pr_bold_info("%s: vio = %p, vdd = %p", chip->name, chip->vio, chip->vdd);

	if (chip->vio || chip->vdd || chip->gpio_power >= 0)
	{
		chip->flags |= HUA_INPUT_CHIP_FLAG_POWERON_INIT;
	}

	return 0;
}
#else
int hua_input_chip_io_init(struct hua_input_chip *chip)
{
#ifdef CONFIG_ARCH_SC8810
extern int sprd_3rdparty_gpio_tp_rst;
extern int sprd_3rdparty_gpio_tp_irq;

	if (chip->devmask & (1 << HUA_INPUT_DEVICE_TYPE_TOUCHSCREEN))
	{
		chip->gpio_irq = sprd_3rdparty_gpio_tp_irq;
		chip->gpio_reset = sprd_3rdparty_gpio_tp_rst;
	}
	else
#endif
	{
		chip->gpio_irq = -1;
		chip->gpio_reset = -1;
	}

	chip->gpio_power = -1;

	return 0;
}
#endif

EXPORT_SYMBOL_GPL(hua_input_chip_io_init);

void hua_input_chip_io_deinit(struct hua_input_chip *chip)
{
	pr_pos_info();

	if (chip->vio)
	{
		regulator_put(chip->vio);
	}

	if (chip->vdd)
	{
		regulator_put(chip->vdd);
	}

	if (chip->gpio_power >= 0)
	{
		gpio_free(chip->gpio_power);
	}

	if (chip->gpio_reset >= 0)
	{
		gpio_free(chip->gpio_reset);
	}

	if (chip->gpio_irq >= 0)
	{
		gpio_free(chip->gpio_irq);
	}
}

EXPORT_SYMBOL_GPL(hua_input_chip_io_deinit);

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Huamobile IO");
MODULE_LICENSE("GPL");
