/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Thu Apr  5 18:45:28 CST 2012
 */

#include <cavan.h>
#include <cavan/swan_ts.h>

int swan_ts_open_misc_device(const char *devpath)
{
	int fd;

	if (devpath == NULL)
	{
		devpath = SWAN_TS_MISC_DEVICE;
	}

	fd = open(devpath, 0);
	if (fd < 0)
	{
		pr_red_info("Open device \"%s\" failed", devpath);
	}

	return fd;
}

int swan_ts_calibration(const char *devpath)
{
	int fd;
	int ret;

	fd = swan_ts_open_misc_device(devpath);
	if (fd < 0)
	{
		return fd;
	}

	ret = ioctl(fd, SWAN_TS_IOCTL_CALIBRATION);
	if (ret < 0)
	{
		pr_red_info("Failed");
	}
	else
	{
		pr_green_info("OK");
	}

	close(fd);

	return ret;
}

int swan_ts_calication_main(int argc, char *argv[])
{
	return swan_ts_calibration(argc > 1 ? argv[1] : NULL);
}

int swan_ts_read_registers_fd(int fd, u16 addr, void *buff, size_t size)
{
	struct swan_ts_i2c_request req =
	{
		.offset = addr,
		.size = size,
		.data = buff
	};

	return ioctl(fd, SWAN_TS_IOCTL_READ_I2C, &req);
}

int swan_ts_read_registers(const char *devpath, u16 addr, void *buff, size_t size)
{
	int fd;
	int ret;

	fd = swan_ts_open_misc_device(devpath);
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

	fd = swan_ts_open_misc_device(devpath);
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

int swan_ts_write_registers_fd(int fd, u16 addr, const void *buff, size_t size)
{
	struct swan_ts_i2c_request req =
	{
		.offset = addr,
		.size = size,
		.data = (void *)buff
	};

	return ioctl(fd, SWAN_TS_IOCTL_WRITE_I2C, &req);
}

int swan_ts_write_registers(const char *devpath, u16 addr, const void *buff, size_t size)
{
	int fd;
	int ret;

	fd = swan_ts_open_misc_device(devpath);
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

int ft5406_firmware_upgrade_main(int argc, char *argv[])
{
	return 0;
}
