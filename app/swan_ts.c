/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Thu Apr  5 18:35:30 CST 2012
 */

#include <cavan.h>
#include <cavan/swan_ts.h>
#include <cavan/command.h>

static int swan_ts_calication_main(int argc, char *argv[])
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

static int swan_ts_read_registers_main(int argc, char *argv[])
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

	print_mem((u8 *) buff, count);

out_free_buff:
	free(buff);

	return ret;
}

static int swan_ts_poll_registers_main(int argc, char *argv[])
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
	devpath = argc > 4 ? argv[4] : SWAN_TS_DEFAULT_DEVICE;

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

		print_mem((u8 *) buff, count);
	}

	close(fd);

out_free_buff:
	free(buff);

	return ret;
}

static int swan_ts_write_registers_main(int argc, char *argv[])
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
	print_mem((u8 *) buff, count);

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

static int swan_ts_read_data_main(int argc, char *argv[])
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

	print_mem((u8 *) buff, count);

out_free_buff:
	free(buff);

	return ret;
}

static int swan_ts_poll_data_main(int argc, char *argv[])
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
	devpath = argc > 3 ? argv[3] : SWAN_TS_DEFAULT_DEVICE;

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

		print_mem((u8 *) buff, count);
	}

	close(fd);

out_free_buff:
	free(buff);

	return ret;
}

static int swan_ts_write_data_main(int argc, char *argv[])
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
	print_mem((u8 *) buff, count);

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

static int swan_ts_get_client_address_main(int argc, char *argv[])
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

static int swan_ts_set_client_address_main(int argc, char *argv[])
{
	u16 addr;

	assert(argc > 1);

	addr = text2value_unsigned(argv[1], NULL, 10);

	println("Client address = %d = 0x%04x", addr, addr);

	return swan_ts_set_client_address(argc > 2 ? argv[2] : NULL, addr);
}

static int swan_ts_detect_clients_main(int argc, char *argv[])
{
	u16 start;
	u16 end;

	start = argc > 1 ? text2value_unsigned(argv[1], NULL, 10) : 1;
	end = argc > 2 ? text2value_unsigned(argv[2], NULL, 10) : 0x7F;

	return swan_ts_detect_clients(argc > 3 ? argv[3] : NULL, start, end);
}

static int swan_ts_test_client_main(int argc, char *argv[])
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

static int ft5406_firmware_upgrade_main(int argc, char *argv[])
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

static struct cavan_command_map cmd_map[] =
{
	{"calibration",		swan_ts_calication_main},
	{"read_registers",	swan_ts_read_registers_main},
	{"poll_registers",	swan_ts_poll_registers_main,},
	{"write_registers",	swan_ts_write_registers_main},
	{"read_data",		swan_ts_read_data_main},
	{"poll_data",		swan_ts_poll_data_main},
	{"write_data",		swan_ts_write_data_main},
	{"get_address",		swan_ts_get_client_address_main},
	{"set_address",		swan_ts_set_client_address_main},
	{"test_client",		swan_ts_test_client_main},
	{"detect_clients",	swan_ts_detect_clients_main},
	{"ft5406_upgrade",	ft5406_firmware_upgrade_main}
};

FIND_EXEC_COMMAND_MAIN(cmd_map, false);
