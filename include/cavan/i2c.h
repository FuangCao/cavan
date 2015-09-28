#pragma once

/*
 * File:		i2c.h
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
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

struct cavan_i2c_client
{
	int fd;
	u16 addr;
	void *private_data;
};

int cavan_i2c_init(struct cavan_i2c_client *client, int index, void *data);
void cavan_i2c_deinit(struct cavan_i2c_client *client);
int cavan_i2c_transfer(struct cavan_i2c_client *client, struct i2c_msg *msgs, size_t count);
int cavan_i2c_master_send(struct cavan_i2c_client *client, const void *buff, size_t size);
int cavan_i2c_master_recv(struct cavan_i2c_client *client, void *buff, size_t size);
void cavan_i2c_detect(struct cavan_i2c_client *client);

static inline int cavan_i2c_set_address(struct cavan_i2c_client *client, u16 addr)
{
	int ret = ioctl(client->fd, I2C_SLAVE, addr);
	if (ret < 0) {
		return ret;
	}

	client->addr = addr;

	return 0;
}

static inline int cavan_i2c_set_tenbit(struct cavan_i2c_client *client, bool enable)
{
	return ioctl(client->fd, I2C_TENBIT, enable);
}
