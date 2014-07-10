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

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Huamobile IO");
MODULE_LICENSE("GPL");
