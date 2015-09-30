/*
 * File:		i2c.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-09-28 17:48:30
 *
 * Copyright (c) 2015 Fuang.Cao <cavan.cfa@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <cavan.h>
#include <cavan/i2c.h>
#include <cavan/command.h>

static void show_usage(const char *command, const char *usage)
{
	if (usage) {
		println("Usage: %s <ADAPTER-SLAVE_ADDR> %s", command, usage);
	} else {
		println("Usage: %s <ADAPTER-SLAVE_ADDR> ...", command);
	}

	println("-h, --help\t\t%s", cavan_help_message_help);
	println("-v, --version\t\t%s", cavan_help_message_version);
	println("--addr-bytes\t\t%s", cavan_help_message_addr_bytes);
	println("--value-bytes\t\t%s", cavan_help_message_value_bytes);
	println("--big-endian\t\t%s", cavan_help_message_big_endian);
	println("--rk, --rockchip\tthis is rockchip i2c chip");
	println("-r, --rate\t\tSCL clock rate");
}

static int cavan_open_client_by_args(struct cavan_i2c_client *client, int argc, char *argv[], int count, const char *usage)
{
	int c;
	int option_index;
	struct option long_option[] = {
		{
			.name = "help",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_HELP,
		}, {
			.name = "version",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_VERSION,
		}, {
			.name = "addr-bytes",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_ADDR_BYTES,
		}, {
			.name = "value-bytes",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_VALUE_BYTES,
		}, {
			.name = "big-endian",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_BIG_ENDIAN,
		}, {
			.name = "rockchip",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_ROCKCHIP,
		}, {
			.name = "rk",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_ROCKCHIP,
		}, {
			.name = "rate",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_RATE,
		}, {
			0, 0, 0, 0
		},
	};
	int ret;

	cavan_i2c_client_init(client);

	while ((c = getopt_long(argc, argv, "vhbr:", long_option, &option_index)) != EOF) {
		switch (c) {
		case 'v':
		case CAVAN_COMMAND_OPTION_VERSION:
			show_author_info();
			exit(0);

		case 'h':
		case CAVAN_COMMAND_OPTION_HELP:
			show_usage(argv[0], usage);
			exit(0);

		case CAVAN_COMMAND_OPTION_ADDR_BYTES:
			client->addr_bytes = text2value_unsigned(optarg, NULL, 10);
			break;

		case CAVAN_COMMAND_OPTION_VALUE_BYTES:
			client->value_bytes = text2value_unsigned(optarg, NULL, 10);
			break;

		case 'b':
		case CAVAN_COMMAND_OPTION_BIG_ENDIAN:
			client->addr_big_endian = true;
			client->value_big_endian = true;
			break;

		case 'r':
		case CAVAN_COMMAND_OPTION_RATE:
			client->scl_rate = text2clock(optarg);
		case CAVAN_COMMAND_OPTION_ROCKCHIP:
			client->flags |= CAVAN_I2C_FLAG_ROCKCHIP;
			break;

		default:
			show_usage(argv[0], usage);
			return -EINVAL;
		}
	}

	if (argc - optind < count + 1) {
		show_usage(argv[0], usage);
		return -EINVAL;
	}

	ret = cavan_i2c_client_open2(client, argv[optind++]);
	if (ret < 0) {
		pr_red_info("cavan_i2c_client_open2: %d", ret);
		return ret;
	}

	return 0;
}


static int do_detect(int argc, char *argv[])
{
	int ret;
	int index;
	struct cavan_i2c_client client;

	if (argc <= 1) {
		println("usage: %s <ADAPTER>", argv[0]);
		return -EINVAL;
	}

	index = text2value_unsigned(argv[1], NULL, 10);

	cavan_i2c_client_init(&client);

	ret = cavan_i2c_client_open(&client, index, 0x00);
	if (ret < 0) {
		pr_red_info("cavan_i2c_client_open: %d", ret);
		return ret;
	}

	cavan_i2c_detect(&client);
	cavan_i2c_client_close(&client);

	return 0;
}

static int do_master_send(int argc, char *argv[])
{
	int i;
	int ret;
	u8 *data, *p;
	struct cavan_i2c_client client;

	ret = cavan_open_client_by_args(&client, argc, argv, 1, "<DATA> ...");
	if (ret < 0) {
		pr_red_info("cavan_open_client_by_args: %d", ret);
		return ret;
	}

	argv += optind;
	argc -= optind;

	data = alloca(argc);
	if (data == NULL) {
		pr_err_info("alloca");
		ret = -ENOMEM;
		goto out_cavan_i2c_client_close;
	}

	for (i = 0, p = data; i < argc; i++, p++) {
		*p = text2value_unsigned(argv[i], NULL, 16);
	}

	print_mem(NULL, data, argc);

	ret = cavan_i2c_master_send(&client, data, argc);
	if (ret < 0) {
		pr_red_info("cavan_i2c_master_send: %d", ret);
	}

out_cavan_i2c_client_close:
	cavan_i2c_client_close(&client);
	return ret;
}

static int do_master_recv(int argc, char *argv[])
{
	int ret;
	u8 *data;
	int count;
	struct cavan_i2c_client client;

	ret = cavan_open_client_by_args(&client, argc, argv, 1, "<COUNT>");
	if (ret < 0) {
		pr_red_info("cavan_open_client_by_args: %d", ret);
		return ret;
	}

	count = text2value_unsigned(argv[optind], NULL, 10);

	data = alloca(count);
	if (data == NULL) {
		pr_err_info("alloca");
		ret = -ENOMEM;
		goto out_cavan_i2c_client_close;
	}

	ret = cavan_i2c_master_recv(&client, data, count);
	if (ret < 0) {
		pr_red_info("cavan_i2c_master_send: %d", ret);
	} else {
		print_mem(NULL, data, count);
	}

out_cavan_i2c_client_close:
	cavan_i2c_client_close(&client);
	return ret;
}

static int do_read_data(int argc, char *argv[])
{
	int ret;
	u32 addr;
	u8 *data;
	int count;
	struct cavan_i2c_client client;

	ret = cavan_open_client_by_args(&client, argc, argv, 2, "<ADDR> <COUNT>");
	if (ret < 0) {
		pr_red_info("cavan_open_client_by_args: %d", ret);
		return ret;
	}

	addr = text2value_unsigned(argv[optind++], NULL, 16);
	count = text2value_unsigned(argv[optind++], NULL, 10);

	data = alloca(count);
	if (data == NULL) {
		pr_err_info("alloca");
		ret = -ENOMEM;
		goto out_cavan_i2c_client_close;
	}

	ret = cavan_i2c_read_data(&client, &addr, data, count);
	if (ret < 0) {
		pr_red_info("cavan_i2c_read_data: %d", ret);
	} else {
		println("addr = 0x%02x", addr);
		print_mem("data = ", data, count);
	}

out_cavan_i2c_client_close:
	cavan_i2c_client_close(&client);
	return ret;
}

static int do_write_data(int argc, char *argv[])
{
	int i;
	int ret;
	u32 addr;
	u8 *data, *p;
	struct cavan_i2c_client client;

	ret = cavan_open_client_by_args(&client, argc, argv, 2, "<ADDR> <DATA> ...");
	if (ret < 0) {
		pr_red_info("cavan_open_client_by_args: %d", ret);
		return ret;
	}

	addr = text2value_unsigned(argv[optind++], NULL, 16);

	argv += optind;
	argc -= optind;

	data = alloca(argc);
	if (data == NULL) {
		pr_err_info("alloca");
		ret = -ENOMEM;
		goto out_cavan_i2c_client_close;
	}

	for (i = 0, p = data; i < argc; i++, p++) {
		*p = text2value_unsigned(argv[i], NULL, 16);
	}

	println("addr = 0x%02x, count = %d", addr, argc);
	print_mem(NULL, data, argc);

	ret = cavan_i2c_write_data(&client, &addr, data, argc);
	if (ret < 0) {
		pr_red_info("cavan_i2c_master_send: %d", ret);
	}

out_cavan_i2c_client_close:
	cavan_i2c_client_close(&client);
	return ret;
}

static int do_read_register(int argc, char *argv[])
{
	int ret;
	u32 addr;
	u32 value;
	struct cavan_i2c_client client;

	ret = cavan_open_client_by_args(&client, argc, argv, 1, "<ADDR>");
	if (ret < 0) {
		pr_red_info("cavan_open_client_by_args: %d", ret);
		return ret;
	}

	addr = text2value_unsigned(argv[optind++], NULL, 16);

	ret = cavan_i2c_read_register(&client, addr, &value);
	if (ret < 0) {
		pr_red_info("cavan_i2c_read_register: %d", ret);
	} else {
		println("addr = 0x%08x, value = 0x%08x", addr, value);
	}

	cavan_i2c_client_close(&client);

	return ret;
}


static int do_write_register(int argc, char *argv[])
{
	int ret;
	u32 addr;
	u32 value;
	struct cavan_i2c_client client;

	ret = cavan_open_client_by_args(&client, argc, argv, 2, "<ADDR> <VALUE>");
	if (ret < 0) {
		pr_red_info("cavan_open_client_by_args: %d", ret);
		return ret;
	}

	addr = text2value_unsigned(argv[optind++], NULL, 16);
	value = text2value_unsigned(argv[optind++], NULL, 16);
	println("addr = 0x%08x, value = 0x%08x", addr, value);

	ret = cavan_i2c_write_register(&client, addr, value);
	if (ret < 0) {
		pr_red_info("cavan_i2c_read_register: %d", ret);
	}

	cavan_i2c_client_close(&client);

	return ret;
}


static int do_update_bits(int argc, char *argv[])
{
	int ret;
	u32 addr;
	int bits;
	int offset;
	u32 mask;
	u32 value;
	struct cavan_i2c_client client;

	ret = cavan_open_client_by_args(&client, argc, argv, 4, "<ADDR> <OFFSET> <BITS> <VALUE>");
	if (ret < 0) {
		pr_red_info("cavan_open_client_by_args: %d", ret);
		return ret;
	}

	addr = text2value_unsigned(argv[optind++], NULL, 16);
	offset = text2value_unsigned(argv[optind++], NULL, 10);
	bits = text2value_unsigned(argv[optind++], NULL, 10);
	value = text2value_unsigned(argv[optind++], NULL, 16);
	println("addr = 0x%08x, offset = %d, bits = %d, value = 0x%08x", addr, offset, bits, value);

	value <<= offset;
	mask = ((1 << bits) - 1) << offset;

	ret = cavan_i2c_update_bits(&client, addr, value, mask);
	if (ret < 0) {
		pr_red_info("cavan_i2c_read_register: %d", ret);
	}

	cavan_i2c_client_close(&client);

	return ret;
}

CAVAN_COMMAND_MAP_START
{ "detect", do_detect },
{ "master_send", do_master_send },
{ "master_recv", do_master_recv },
{ "read_data", do_read_data },
{ "write_data", do_write_data },
{ "read_reg", do_read_register },
{ "write_reg", do_write_register },
{ "update_bits", do_update_bits },
CAVAN_COMMAND_MAP_END
