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

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Huamobile IO");
MODULE_LICENSE("GPL");
