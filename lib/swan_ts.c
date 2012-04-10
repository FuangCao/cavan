/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Thu Apr  5 18:45:28 CST 2012
 */

#include <cavan.h>
#include <cavan/swan_ts.h>
#include <cavan/event.h>

int swan_ts_open_misc_device(const char *devpath, int flags)
{
	int fd;

	if (devpath == NULL)
	{
		devpath = SWAN_TS_MISC_DEVICE;
	}

	fd = open(devpath, flags);
	if (fd < 0)
	{
		pr_red_info("Open device \"%s\" failed", devpath);
	}

	return fd;
}

int swan_ts_input_calibration(const char *devname)
{
	int ret;
	struct event_desc desc;

	if (devname == NULL)
	{
		devname = SWAN_TS_DEVICE_NAME;
	}

	ret = event_init_by_name(&desc, devname);
	if (ret < 0)
	{
		pr_red_info("No input event named \"%s\"", devname);
		return ret;
	}

	pr_std_info("Device \"%s\" is named \"%s\"", desc.dev_path, devname);
	pr_bold_info("Calibration, don't touch the screen");

	ret = ioctl(desc.fd, SWAN_TS_INPUT_IOCTL_CALIBRATION);

	event_uninit(&desc);

	return ret;
}

int swan_ts_calibration(const char *devpath)
{
	int fd;
	int ret;

	fd = swan_ts_open_misc_device(devpath, 0);
	if (fd < 0)
	{
		pr_bold_info("Retry use input calibration");
		return swan_ts_input_calibration(devpath);
	}

	ret = ioctl(fd, SWAN_TS_IOCTL_CALIBRATION);

	close(fd);

	return ret;
}

int swan_ts_read_registers_fd(int fd, u16 addr, void *buff, size_t size)
{
	struct swan_ts_i2c_request req =
	{
		.offset = addr,
		.size = size,
		.data = buff
	};

	return ioctl(fd, SWAN_TS_IOCTL_READ_REGISTERS, &req);
}

int swan_ts_read_registers(const char *devpath, u16 addr, void *buff, size_t size)
{
	int fd;
	int ret;

	fd = swan_ts_open_misc_device(devpath, 0);
	if (fd < 0)
	{
		return fd;
	}

	ret = swan_ts_read_registers_fd(fd, addr, buff, size);

	close(fd);

	return ret;
}

int swan_ts_read_data_fd(int fd, void *buff, size_t size)
{
	struct swan_ts_i2c_request req =
	{
		.offset = 0,
		.size = size,
		.data = buff
	};

	return ioctl(fd, SWAN_TS_IOCTL_READ_DATA, &req);
}

int swan_ts_read_data(const char *devpath, void *buff, size_t size)
{
	int fd;
	int ret;

	fd = swan_ts_open_misc_device(devpath, 0);
	if (fd < 0)
	{
		return fd;
	}

	ret = swan_ts_read_data_fd(fd, buff, size);

	close(fd);

	return ret;
}

int swan_ts_write_registers_fd(int fd, u16 addr, const void *buff, size_t size)
{
	struct swan_ts_i2c_request req =
	{
		.offset = addr,
		.size = size,
		.data = (void *)buff
	};

	return ioctl(fd, SWAN_TS_IOCTL_WRITE_REGISTERS, &req);
}

int swan_ts_write_registers(const char *devpath, u16 addr, const void *buff, size_t size)
{
	int fd;
	int ret;

	fd = swan_ts_open_misc_device(devpath, 0);
	if (fd < 0)
	{
		return fd;
	}

	ret = swan_ts_write_registers_fd(fd, addr, buff, size);

	close(fd);

	return ret;
}

int swan_ts_write_data_fd(int fd, const void *buff, size_t size)
{
	struct swan_ts_i2c_request req =
	{
		.offset = 0,
		.size = size,
		.data = (void *)buff
	};

	return ioctl(fd, SWAN_TS_IOCTL_WRITE_DATA, &req);
}

int swan_ts_write_data(const char *devpath, const void *buff, size_t size)
{
	int fd;
	int ret;

	fd = swan_ts_open_misc_device(devpath, 0);
	if (fd < 0)
	{
		return fd;
	}

	ret = swan_ts_write_data_fd(fd, buff, size);

	close(fd);

	return ret;
}

int swan_ts_get_client_address(const char *devpath, u16 *addr)
{
	int fd;

	fd = swan_ts_open_misc_device(devpath, 0);
	if (fd < 0)
	{
		return fd;
	}

	return swan_ts_get_client_address_fd(fd, addr);
}

int swan_ts_set_client_address(const char *devpath, u16 addr)
{
	int fd;

	fd = swan_ts_open_misc_device(devpath, 0);
	if (fd < 0)
	{
		return fd;
	}

	return swan_ts_set_client_address_fd(fd, addr);
}

int swan_ts_detect_clients_fd(int fd, u16 start, u16 end)
{
	u32 args;

	if (start > end)
	{
		u16 temp;

		temp = start;
		start = end;
		end = temp;
	}

	println("Start = %d = 0x%04x, End = %d = 0x%04x", start, start, end, end);
	args = start << 16 | end;

	return ioctl(fd, SWAN_TS_IOCTL_DETECT_CLIENT, &args);
}

int swan_ts_detect_clients(const char *devpath, u16 start, u16 end)
{
	int ret;
	int fd;

	fd = swan_ts_open_misc_device(devpath, 0);
	if (fd < 0)
	{
		return fd;
	}

	ret = swan_ts_detect_clients_fd(fd, start, end);

	close(fd);

	return ret;
}

int swan_ts_test_client_fd(int fd, u16 addr)
{
	int ret;

	println("Test client %d = 0x%04x", addr, addr);

	ret = ioctl(fd, SWAN_TS_IOCTL_TEST_CLIENT, &addr);
	if (ret < 0)
	{
		pr_red_info("Failed");
	}
	else
	{
		pr_green_info("OK");
	}

	return ret;
}

int swan_ts_test_client(const char *devpath, u16 addr)
{
	int ret;
	int fd;

	fd = swan_ts_open_misc_device(devpath, 0);
	if (fd < 0)
	{
		return fd;
	}

	ret = swan_ts_test_client_fd(fd, addr);

	close(fd);

	return ret;
}

int ft5406_parse_app_file(const char *cfgpath, char *buff, size_t size)
{
	ssize_t readlen;
	char *text_buff, *p, *p_end;
	char *buff_bak, *buff_end;
	u8 value;

	readlen = file_get_size(cfgpath);
	if (readlen < 0)
	{
		pr_red_info("file_get_size %s", cfgpath);
		return readlen;
	}

	text_buff = malloc(readlen);
	if (text_buff == NULL)
	{
		pr_red_info("malloc");
		return -ENOMEM;
	}

	readlen = file_read(cfgpath, text_buff, readlen);
	if (readlen < 0)
	{
		pr_red_info("file_read %s", cfgpath);
		free(text_buff);
		return readlen;
	}

	buff_bak = buff;
	buff_end = buff + size;

	for (p = text_buff, p_end = p + readlen; p < p_end; p++)
	{
		if (*p != 'x' && *p != 'X')
		{
			continue;
		}

		for (value = 0, p++; p < p_end; p++)
		{
			int temp;

			temp = char2value(*p);
			if (temp > 15 || temp < 0)
			{
				break;
			}

			value = value << 4 | temp;
		}

		if (buff < buff_end)
		{
			*buff++ = value;
		}
		else
		{
			break;
		}
	}

	free(text_buff);

	return buff - buff_bak;
}

u8 ft5406_calculate_checksum(const void *buff, size_t size)
{
	const void *buff_end;
	u8 checksum;

	for (buff_end = buff + size, checksum = 0; buff < buff_end; buff++)
	{
		checksum ^= *(u8 *)buff;
	}

	return checksum;
}

int ft5406_firmware_write_last_data(int fd, const void *buff, size_t size)
{
	struct ft5406_firmware_data_package pkg =
	{
		.size = size,
		.data = (void *)buff
	};

	return ioctl(fd, FT5406_IOCTL_SINGLE_WRITE, &pkg);
}

int ft5406_firmware_upgrade_fd(int dev_fd, const char *cfgpath)
{
	int ret;
	char buff[KB(50)] ;
	ssize_t writelen, bufflen;
	u8 checksum[2];

	bufflen = ft5406_parse_app_file(cfgpath, buff, sizeof(buff)) - 2;
	if (bufflen < 6)
	{
		pr_red_info("ft5406_parse_app_file");
		return bufflen;
	}

	println("bufflen = %d", bufflen);

	ret = ft5406_upgrade_enter(dev_fd);
	if (ret < 0)
	{
		pr_red_info("ft5406_upgrade_start");
		return ret;
	}

	ret = ft5406_erase_app(dev_fd);
	if (ret < 0)
	{
		pr_red_info("ft5406_erase_app");
		return ret;
	}

	writelen = write(dev_fd, buff, bufflen - 6);
	if (writelen < 0)
	{
		pr_red_info("write");
		return writelen;
	}

	writelen = ft5406_firmware_write_last_data(dev_fd, buff + writelen, 6);
	if (writelen < 0)
	{
		pr_red_info("ft5406_firmware_write_last_data");
		return writelen;
	}

	ret = ft5406_read_checksum(dev_fd, checksum);
	if (ret < 0)
	{
		pr_red_info("ft5406_read_checksum");
		return ret;
	}

	checksum[1] = ft5406_calculate_checksum(buff, bufflen);

	pr_bold_info("Source checksum = 0x%02x, Dest checksum = 0x%02x", checksum[1], checksum[0]);

	if (checksum[0] != checksum[1])
	{
		pr_red_info("Checksum do't match");
		return -EFAULT;
	}

	return ft5406_upgrade_finish(dev_fd);
}

int ft5406_firmware_upgrade(const char *devpath, const char *cfgpath)
{
	int dev_fd;
	int ret;

	dev_fd = swan_ts_open_misc_device(devpath, O_WRONLY);
	if (dev_fd < 0)
	{
		return dev_fd;
	}

	ret = ft5406_firmware_upgrade_fd(dev_fd, cfgpath);

	close(dev_fd);

	return ret;
}
