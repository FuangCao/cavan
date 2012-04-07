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

int swan_ts_calication_main(int argc, char *argv[])
{
	int ret;

	ret = swan_ts_calibration(argc > 1 ? argv[1] : NULL);
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

int swan_ts_read_registers_main(int argc, char *argv[])
{
	int ret;
	u16 offset;
	u16 count;
	char *buff;

	assert(argc > 1);

	offset = text2value_unsigned(argv[1], NULL, 10);
	count = argc > 2 ? text2value_unsigned(argv[2], NULL, 10) : 1;

	buff = malloc(count);
	if (buff == NULL)
	{
		pr_red_info("malloc");
		return -ENOMEM;
	}

	println("offset = %d, count = %d", offset, count);

	ret = swan_ts_read_registers(NULL, offset, buff, count);
	if (ret < 0)
	{
		pr_red_info("swan_ts_read_registers");
		goto out_free_buff;
	}

	print_mem(buff, count);

out_free_buff:
	free(buff);

	return ret;
}

int swan_ts_poll_registers_main(int argc, char *argv[])
{
	int ret;
	u16 offset;
	u16 count;
	u32 timeout;
	int fd;
	char *buff;
	const char *devpath;

	assert(argc > 2);

	offset = text2value_unsigned(argv[1], NULL, 10);
	count = text2value_unsigned(argv[2], NULL, 10);
	timeout = argc > 3 ? text2value_unsigned(argv[3], NULL, 10) : 200;
	devpath = argc > 4 ? argv[4] : SWAN_TS_MISC_DEVICE;

	println("offset = %d, count = %d, timeout = %d, devpath = %s", offset, count, timeout, devpath);

	buff = malloc(count);
	if (buff == NULL)
	{
		pr_red_info("malloc");
		return -ENOMEM;
	}

	fd = swan_ts_open_misc_device(devpath, 0);
	if (fd < 0)
	{
		pr_red_info("swan_ts_open_misc_device");
		ret = fd;
		goto out_free_buff;
	}

	while (1)
	{
		msleep(timeout);

		ret = swan_ts_read_registers_fd(fd, offset, buff, count);
		if (ret < 0)
		{
			continue;
		}

		print_mem(buff, count);
	}

	close(fd);

out_free_buff:
	free(buff);

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

int swan_ts_read_data_main(int argc, char *argv[])
{
	int ret;
	u16 count;
	char *buff;

	count = argc > 1 ? text2value_unsigned(argv[1], NULL, 10) : 1;

	buff = malloc(count);
	if (buff == NULL)
	{
		pr_red_info("malloc");
		return -ENOMEM;
	}

	println("count = %d", count);

	ret = swan_ts_read_data(NULL, buff, count);
	if (ret < 0)
	{
		pr_red_info("swan_ts_read_data");
		goto out_free_buff;
	}

	print_mem(buff, count);

out_free_buff:
	free(buff);

	return ret;
}

int swan_ts_poll_data_main(int argc, char *argv[])
{
	int ret;
	u16 count;
	u32 timeout;
	int fd;
	char *buff;
	const char *devpath;

	assert(argc > 1);

	count = text2value_unsigned(argv[1], NULL, 10);
	timeout = argc > 2 ? text2value_unsigned(argv[2], NULL, 10) : 200;
	devpath = argc > 3 ? argv[3] : SWAN_TS_MISC_DEVICE;

	println("count = %d, timeout = %d, devpath = %s", count, timeout, devpath);

	buff = malloc(count);
	if (buff == NULL)
	{
		pr_red_info("malloc");
		return -ENOMEM;
	}

	fd = swan_ts_open_misc_device(devpath, 0);
	if (fd < 0)
	{
		pr_red_info("swan_ts_open_misc_device");
		ret = fd;
		goto out_free_buff;
	}

	while (1)
	{
		msleep(timeout);

		ret = swan_ts_read_data_fd(fd, buff, count);
		if (ret < 0)
		{
			continue;
		}

		print_mem(buff, count);
	}

	close(fd);

out_free_buff:
	free(buff);

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

int swan_ts_write_registers_main(int argc, char *argv[])
{
	int i;
	int ret;
	char *buff;
	u16 offset;
	u16 count;

	assert(argc > 2);

	offset = text2value_unsigned(argv[1], NULL, 10);
	count = argc - 2;

	buff = malloc(count);

	for (i = 0; i < count; i++)
	{
		buff[i] = text2value_unsigned(argv[i + 2], NULL, 10);
	}

	println("offset = %d, count = %d", offset, count);
	println("The data is:");
	print_mem(buff, count);

	ret = swan_ts_write_registers(NULL, offset, buff, count);
	if (ret < 0)
	{
		pr_red_info("Failed");
	}
	else
	{
		pr_green_info("OK");
	}

	free(buff);

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

int swan_ts_write_data_main(int argc, char *argv[])
{
	int i;
	int ret;
	char *buff;
	u16 count;

	assert(argc > 1);

	count = argc - 1;

	buff = malloc(count);

	for (i = 0; i < count; i++)
	{
		buff[i] = text2value_unsigned(argv[i + 1], NULL, 10);
	}

	println("count = %d", count);
	println("The data is:");
	print_mem(buff, count);

	ret = swan_ts_write_data(NULL, buff, count);
	if (ret < 0)
	{
		pr_red_info("Failed");
	}
	else
	{
		pr_green_info("OK");
	}

	free(buff);

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

int swan_ts_get_client_address_main(int argc, char *argv[])
{
	int ret;
	u16 addr;

	ret = swan_ts_get_client_address(argc > 1 ? argv[1] : NULL, &addr);
	if (ret < 0)
	{
		pr_red_info("swan_ts_get_client_address");
		return ret;
	}

	pr_bold_info("Client address = %d = 0x%04x", addr, addr);

	return 0;
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

int swan_ts_set_client_address_main(int argc, char *argv[])
{
	u16 addr;

	assert(argc > 1);

	addr = text2value_unsigned(argv[1], NULL, 10);

	println("Client address = %d = 0x%04x", addr, addr);

	return swan_ts_set_client_address(argc > 2 ? argv[2] : NULL, addr);
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

int swan_ts_detect_clients_main(int argc, char *argv[])
{
	u16 start;
	u16 end;

	start = argc > 1 ? text2value_unsigned(argv[1], NULL, 10) : 1;
	end = argc > 2 ? text2value_unsigned(argv[2], NULL, 10) : 0x7F;

	return swan_ts_detect_clients(argc > 3 ? argv[3] : NULL, start, end);
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

int swan_ts_test_client_main(int argc, char *argv[])
{
	int i;
	u16 addr;

	for (i = 1; i < argc; i++)
	{
		addr = text2value_unsigned(argv[i], NULL, 10);

		swan_ts_test_client(NULL, addr);
	}

	return 0;
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

int ft5406_firmware_upgrade_fd(int dev_fd, const char *cfgpath)
{
	int ret;
	char buff[KB(50)];
	ssize_t writelen, bufflen;

	bufflen = ft5406_parse_app_file(cfgpath, buff, sizeof(buff));
	if (bufflen < 0)
	{
		pr_red_info("ft5406_parse_app_file");
		return bufflen;
	}

	println("bufflen = %d", bufflen);

	ret = ft5406_upgrade_start(dev_fd);
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

	ret = lseek(dev_fd, 0x6FFA, SEEK_SET);
	if (ret < 0)
	{
		pr_red_info("lseek");
		return ret;
	}

	writelen = write(dev_fd, buff + writelen, 6);
	if (writelen < 0)
	{
		pr_red_info("write");
		return writelen;
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

int ft5406_firmware_upgrade_main(int argc, char *argv[])
{
	int ret;
	const char *cfgpath, *devpath;

	assert(argc > 1);

	cfgpath = argv[1];
	devpath = argc > 2 ? argv[2] : NULL;

	ret = ft5406_firmware_upgrade(devpath, cfgpath);
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
