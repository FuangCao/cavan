/*
 * File:		i2c.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-09-28 17:48:13
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

#define CAVAN_I2C_UPDATE_BITS_IMPLEMENT(bits) \
	int cavan_i2c_update_bits##bits(struct cavan_i2c_client *client, u8 addr, u##bits value, u##bits mask) { \
		int ret; \
		u##bits value_old; \
		ret = cavan_i2c_read_register##bits(client, addr, &value_old); \
		if (ret < 0) { \
			pr_red_info("cavan_i2c_read_register" #bits ": %d", ret); \
			return ret; \
		} \
		value = (value & mask) | (value_old & (~mask)); \
		if (value == value_old) { \
			return 0; \
		} \
		return cavan_i2c_write_register##bits(client, addr, value); \
	}

int cavan_i2c_init(struct cavan_i2c_client *client, int index, void *data)
{
	int fd;
	char pathname[32];

	snprintf(pathname, sizeof(pathname), "/dev/i2c-%d", index);
	println("pathname = %s", pathname);

	fd = open(pathname, O_RDWR);
	if (fd < 0) {
		pr_err_info("open %s", pathname);
		return fd;
	}

	client->fd = fd;
	client->private_data = data;

	return 0;
}

void cavan_i2c_deinit(struct cavan_i2c_client *client)
{
	close(client->fd);
}

int cavan_i2c_transfer(struct cavan_i2c_client *client, struct cavan_i2c_msg *msgs, size_t count)
{
	struct i2c_rdwr_ioctl_data data = {
		.msgs = (struct i2c_msg *) msgs,
		.nmsgs = count
	};

	return ioctl(client->fd, I2C_RDWR, &data);
}

void cavan_i2c_detect(struct cavan_i2c_client *client)
{
	u16 addr;

	for (addr = 0x01; addr < 0x80; addr++) {
		char c;
		int ret;

		ret = cavan_i2c_set_address(client, addr);
		if (ret < 0) {
			pr_err_info("cavan_i2c_set_address: %d", ret);
			continue;
		}

		ret = cavan_i2c_master_recv(client, &c, 1);
		if (ret == 1) {
			pr_green_info("client at 0x%02x ok", addr);
		}
	}
}

int cavan_i2c_write_data(struct cavan_i2c_client *client, u8 addr, const void *data, size_t size)
{
	int ret;
	char buff[size + 1];

	buff[0] = addr;
	memcpy(buff + 1, data, size);

	ret = cavan_i2c_master_send(client, buff, sizeof(buff));
	if (ret == (int) sizeof(buff)) {
		return ret - 1;
	}

	return -EFAULT;
}

int cavan_i2c_read_data(struct cavan_i2c_client *client, u8 addr, void *data, size_t size)
{
	int ret;
	struct cavan_i2c_msg msgs[] = {
		{
			.addr = client->addr,
			.flags = 0,
			.length = 1,
			.buff = &addr,
#ifdef CONFIG_I2C_ROCKCHIP_COMPAT
			.scl_rate = CAVAN_I2C_RATE_100K
#endif
		}, {
			.addr = client->addr,
			.flags = I2C_M_RD,
			.length = size,
			.buff = data,
#ifdef CONFIG_I2C_ROCKCHIP_COMPAT
			.scl_rate = CAVAN_I2C_RATE_100K
#endif
		}
	};

	ret = cavan_i2c_transfer(client, msgs, NELEM(msgs));
	if (ret == NELEM(msgs)) {
		return size;
	}

	return -EFAULT;
}

CAVAN_I2C_UPDATE_BITS_IMPLEMENT(8);
CAVAN_I2C_UPDATE_BITS_IMPLEMENT(16);
CAVAN_I2C_UPDATE_BITS_IMPLEMENT(32);
