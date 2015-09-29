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

#define CAVAN_I2C_FUNC_READ_REG			"read_reg"
#define CAVAN_I2C_FUNC_READ_REG_LEN		(sizeof(CAVAN_I2C_FUNC_READ_REG) - 1)

#define CAVAN_I2C_FUNC_WRITE_REG		"write_reg"
#define CAVAN_I2C_FUNC_WRITE_REG_LEN	(sizeof(CAVAN_I2C_FUNC_READ_REG) - 1)

#define CAVAN_I2C_FUNC_UPDATE_BITS		"update_bits"
#define CAVAN_I2C_FUNC_UPDATE_BITS_LEN	(sizeof(CAVAN_I2C_FUNC_UPDATE_BITS) - 1)

#define CAVAN_I2C_FUNC_READ_REG_IMPLEMENT(bits, format) { \
		u##bits value; \
		ret = cavan_i2c_read_register##bits(&client, addr, &value); \
		if (ret < 0) { \
			pr_red_info("cavan_i2c_read_register" #bits ": %d", ret); \
		} else { \
			println("%s: addr = 0x%02x, value = " format, argv[0], addr, value); \
		} \
	}

#define CAVAN_I2C_FUNC_WRITE_REG_IMPLEMENT(bits, format) { \
		ret = cavan_i2c_write_register##bits(&client, addr, value); \
		println("%s: addr = 0x%02x, value = " format, argv[0], addr, value); \
		if (ret < 0) { \
			pr_red_info("cavan_i2c_write_register" #bits ": %d", ret); \
		} \
	}

#define CAVAN_I2C_FUNC_UPDATE_BITS_IMPLEMENT(bits, format) { \
		ret = cavan_i2c_update_bits##bits(&client, addr, value, mask); \
		println("%s: addr = 0x%02x, value = " format ", mask = " format, argv[0], addr, value, mask); \
		if (ret < 0) { \
			pr_red_info("cavan_i2c_update_bits" #bits ": %d", ret); \
		} \
	}

#define CAVAN_I2C_FUNC_IMPLEMENT(func) \
	println("adapter = %d, client = 0x%02x, bits = %d", adapter, client_addr, bits); \
	ret = cavan_i2c_init(&client, adapter, client_addr); \
	if (ret < 0) { \
		pr_red_info("cavan_i2c_init: %d", ret); \
		return ret; \
	} \
	switch (bits) { \
	case 8: \
		func(8, "0x%02x"); \
		break; \
	case 16: \
		func(16, "0x%04x"); \
		break; \
	case 32: \
		func(32, "0x%08x"); \
		break; \
	default: \
		pr_err_info("Invalid bits = %d", bits); \
		ret = -EINVAL; \
	} \
	cavan_i2c_deinit(&client); \
	return ret;

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
	println("index = %d", index);

	ret = cavan_i2c_init(&client, index, 0x00);
	if (ret < 0) {
		pr_red_info("cavan_i2c_init: %d", ret);
		return ret;
	}

	cavan_i2c_detect(&client);
	cavan_i2c_deinit(&client);

	return 0;
}

static int do_master_send(int argc, char *argv[])
{
	int i;
	int ret;
	int count;
	int adapter;
	u8 *data, *p;
	u16 client_addr;
	struct cavan_i2c_client client;

	if (argc <= 3) {
		println("usage: %s <ADAPTER> <CLIENT_ADDR> <DATA> ...", argv[0]);
		return -EINVAL;
	}

	adapter = text2value_unsigned(argv[1], NULL, 10);
	client_addr = text2value_unsigned(argv[2], NULL, 16);

	ret = cavan_i2c_init(&client, adapter, client_addr);
	if (ret < 0) {
		pr_red_info("cavan_i2c_init: %d", ret);
		return ret;
	}

	count = argc - 3;
	data = alloca(count);
	if (data == NULL) {
		pr_err_info("alloca");
		ret = -ENOMEM;
		goto out_cavan_i2c_deinit;
	}

	for (i = 3, p = data; i < argc; i++, p++) {
		*p = text2value_unsigned(argv[i], NULL, 16);
	}

	print_mem(data, count);

	ret = cavan_i2c_master_send(&client, data, count);
	if (ret < 0) {
		pr_red_info("cavan_i2c_master_send: %d", ret);
	}

out_cavan_i2c_deinit:
	cavan_i2c_deinit(&client);
	return ret;
}

static int do_master_recv(int argc, char *argv[])
{
	int ret;
	u8 *data;
	int count;
	int adapter;
	u16 client_addr;
	struct cavan_i2c_client client;

	if (argc <= 3) {
		println("usage: %s <ADAPTER> <CLIENT_ADDR> <COUNT> ...", argv[0]);
		return -EINVAL;
	}

	adapter = text2value_unsigned(argv[1], NULL, 10);
	client_addr = text2value_unsigned(argv[2], NULL, 16);
	count = text2value_unsigned(argv[3], NULL, 10);

	ret = cavan_i2c_init(&client, adapter, client_addr);
	if (ret < 0) {
		pr_red_info("cavan_i2c_init: %d", ret);
		return ret;
	}

	data = alloca(count);
	if (data == NULL) {
		pr_err_info("alloca");
		ret = -ENOMEM;
		goto out_cavan_i2c_deinit;
	}

	ret = cavan_i2c_master_recv(&client, data, count);
	if (ret < 0) {
		pr_red_info("cavan_i2c_master_send: %d", ret);
	} else {
		print_mem(data, count);
	}

out_cavan_i2c_deinit:
	cavan_i2c_deinit(&client);
	return ret;
}

static int do_read_data(int argc, char *argv[])
{
	int ret;
	u8 addr;
	u8 *data;
	int count;
	int adapter;
	u16 client_addr;
	struct cavan_i2c_client client;

	if (argc <= 4) {
		println("usage: %s <ADAPTER> <CLIENT_ADDR> <ADDR> <COUNT> ...", argv[0]);
		return -EINVAL;
	}

	adapter = text2value_unsigned(argv[1], NULL, 10);
	client_addr = text2value_unsigned(argv[2], NULL, 16);
	addr = text2value_unsigned(argv[3], NULL, 16);
	count = text2value_unsigned(argv[4], NULL, 10);

	ret = cavan_i2c_init(&client, adapter, client_addr);
	if (ret < 0) {
		pr_red_info("cavan_i2c_init: %d", ret);
		return ret;
	}

	data = alloca(count);
	if (data == NULL) {
		pr_err_info("alloca");
		ret = -ENOMEM;
		goto out_cavan_i2c_deinit;
	}

	ret = cavan_i2c_read_data(&client, addr, data, count);
	if (ret < 0) {
		pr_red_info("cavan_i2c_read_data: %d", ret);
	} else {
		println("add = 0x%02x", addr);
		print_mem(data, count);
	}

out_cavan_i2c_deinit:
	cavan_i2c_deinit(&client);
	return ret;
}

static int do_write_data(int argc, char *argv[])
{
	int i;
	int ret;
	u8 addr;
	int count;
	int adapter;
	u8 *data, *p;
	u16 client_addr;
	struct cavan_i2c_client client;

	if (argc <= 4) {
		println("usage: %s <ADAPTER> <CLIENT_ADDR> <ADDR> <DATA> ...", argv[0]);
		return -EINVAL;
	}

	adapter = text2value_unsigned(argv[1], NULL, 10);
	client_addr = text2value_unsigned(argv[2], NULL, 16);
	addr = text2value_unsigned(argv[3], NULL, 16);

	ret = cavan_i2c_init(&client, adapter, client_addr);
	if (ret < 0) {
		pr_red_info("cavan_i2c_init: %d", ret);
		return ret;
	}

	count = argc - 4;
	data = alloca(count);
	if (data == NULL) {
		pr_err_info("alloca");
		ret = -ENOMEM;
		goto out_cavan_i2c_deinit;
	}

	for (i = 4, p = data; i < argc; i++, p++) {
		*p = text2value_unsigned(argv[i], NULL, 16);
	}

	println("addr = 0x%02x, count = %d", addr, count);
	print_mem(data, count);

	ret = cavan_i2c_write_data(&client, addr, data, count);
	if (ret < 0) {
		pr_red_info("cavan_i2c_master_send: %d", ret);
	}

out_cavan_i2c_deinit:
	cavan_i2c_deinit(&client);
	return ret;
}

static int do_read_register(int argc, char *argv[])
{
	int ret;
	u8 addr;
	int bits;
	int adapter;
	u16 client_addr;
	struct cavan_i2c_client client;

	if (argc <= 3) {
		println("usage: %s <ADAPTER> <CLIENT_ADDR> <REG_ADDR>", argv[0]);
		return -EINVAL;
	}

	bits = text2value_unsigned(argv[0] + CAVAN_I2C_FUNC_READ_REG_LEN, NULL, 10);
	adapter = text2value_unsigned(argv[1], NULL, 10);
	client_addr = text2value_unsigned(argv[2], NULL, 16);
	addr = text2value_unsigned(argv[3], NULL, 16);

	CAVAN_I2C_FUNC_IMPLEMENT(CAVAN_I2C_FUNC_READ_REG_IMPLEMENT);
}

static int do_write_register(int argc, char *argv[])
{
	int ret;
	u8 addr;
	int bits;
	u32 value;
	int adapter;
	u16 client_addr;
	struct cavan_i2c_client client;

	if (argc <= 4) {
		println("usage: %s <ADAPTER> <CLIENT_ADDR> <REG_ADDR> <REG_VALUE>", argv[0]);
		return -EINVAL;
	}

	bits = text2value_unsigned(argv[0] + CAVAN_I2C_FUNC_WRITE_REG_LEN, NULL, 10);
	adapter = text2value_unsigned(argv[1], NULL, 10);
	client_addr = text2value_unsigned(argv[2], NULL, 16);
	addr = text2value_unsigned(argv[3], NULL, 16);
	value = text2value_unsigned(argv[4], NULL, 16);

	CAVAN_I2C_FUNC_IMPLEMENT(CAVAN_I2C_FUNC_WRITE_REG_IMPLEMENT);
}

static int do_update_bits(int argc, char *argv[])
{
	int ret;
	u8 addr;
	int bits;
	u32 mask;
	u32 value;
	int offset;
	int length;
	int adapter;
	u16 client_addr;
	struct cavan_i2c_client client;

	if (argc <= 6) {
		println("usage: %s <ADAPTER> <CLIENT_ADDR> <REG_ADDR> <OFFSET> <BITS> <VALUE>", argv[0]);
		return -EINVAL;
	}

	bits = text2value_unsigned(argv[0] + CAVAN_I2C_FUNC_UPDATE_BITS_LEN, NULL, 10);
	adapter = text2value_unsigned(argv[1], NULL, 10);
	client_addr = text2value_unsigned(argv[2], NULL, 16);
	addr = text2value_unsigned(argv[3], NULL, 16);
	offset = text2value_unsigned(argv[4], NULL, 10);
	length = text2value_unsigned(argv[5], NULL, 10);
	value = text2value_unsigned(argv[6], NULL, 16);

	println("offset = %d, length = %d", offset, length);

	value <<= offset;
	mask = ((1 << length) - 1) << offset;

	CAVAN_I2C_FUNC_IMPLEMENT(CAVAN_I2C_FUNC_UPDATE_BITS_IMPLEMENT);
}

CAVAN_COMMAND_MAP_START
{ "detect", do_detect },
{ "master_send", do_master_send },
{ "master_recv", do_master_recv },
{ "read_data", do_read_data },
{ "write_data", do_write_data },
{ CAVAN_I2C_FUNC_READ_REG "8", do_read_register },
{ CAVAN_I2C_FUNC_READ_REG "16", do_read_register },
{ CAVAN_I2C_FUNC_READ_REG "32", do_read_register },
{ CAVAN_I2C_FUNC_WRITE_REG "8", do_write_register },
{ CAVAN_I2C_FUNC_WRITE_REG "16", do_write_register },
{ CAVAN_I2C_FUNC_WRITE_REG "32", do_write_register },
{ CAVAN_I2C_FUNC_UPDATE_BITS "8", do_update_bits },
{ CAVAN_I2C_FUNC_UPDATE_BITS "16", do_update_bits },
{ CAVAN_I2C_FUNC_UPDATE_BITS "32", do_update_bits },
CAVAN_COMMAND_MAP_END
