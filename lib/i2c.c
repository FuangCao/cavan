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

int cavan_i2c_transfer(struct cavan_i2c_client *client, struct i2c_msg *msgs, size_t count)
{
	struct i2c_rdwr_ioctl_data data = {
		.msgs = msgs,
		.nmsgs = count
	};

	return ioctl(client->fd, I2C_RDWR, &data);
}

int cavan_i2c_master_send(struct cavan_i2c_client *client, const void *buff, size_t size)
{
	struct i2c_msg msg = {
		.addr = client->addr,
		.flags = 0,
		.buf = __UNCONST(buff),
		.len = size
	};

	return cavan_i2c_transfer(client, &msg, 1);
}

int cavan_i2c_master_recv(struct cavan_i2c_client *client, void *buff, size_t size)
{
	struct i2c_msg msg = {
		.addr = client->addr,
		.flags = I2C_M_RD,
		.buf = buff,
		.len = size
	};

	return cavan_i2c_transfer(client, &msg, 1);
}

void cavan_i2c_detect(struct cavan_i2c_client *client)
{
	u16 addr;

	for (addr = 0x01; addr < 0x80; addr++) {
		char c;
		int ret;

		ret = cavan_i2c_set_address(client, addr);

		ret = cavan_i2c_master_recv(client, &c, 1);
		if (ret < 0) {
			pr_red_info("client at 0x%02x fault", addr);
		} else {
			pr_green_info("client at 0x%02x ok", addr);
		}
	}
}
