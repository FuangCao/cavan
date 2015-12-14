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

#define CAVAN_I2C_FLAG_ROCKCHIP		(1 << 0)

#define CAVAN_I2C_RATE(k)			((k) * 1000)
#define CAVAN_I2C_RATE_100K			CAVAN_I2C_RATE(100)
#define CAVAN_I2C_RATE_400K			CAVAN_I2C_RATE(400)

struct i2c_msg_rockchip {
	struct i2c_msg msg;
	__u32 scl_rate;
};

struct cavan_i2c_config {
	const char *chipname;

	int addr_step;
	int addr_bytes;
	int value_bytes;
	u32 addr_first;
	u32 addr_last;
	bool addr_big_endian;
	bool value_big_endian;
};

struct cavan_i2c_client {
	int fd;
	int adapter;
	u16 slave_addr;

	int flags;
	u32 scl_rate;

	char device_path[1024];
	char driver_path[1024];
	struct cavan_i2c_config config;

	void *private_data;
};

void cavan_i2c_config_dump(const struct cavan_i2c_config *config);
struct cavan_i2c_config *cavan_i2c_find_config(const char *chipname);
int cavan_i2c_set_address(struct cavan_i2c_client *client, u16 slave_addr);
void cavan_i2c_client_init(struct cavan_i2c_client *client);
int cavan_i2c_client_open(struct cavan_i2c_client *client, int adapter, u16 slave_addr);
int cavan_i2c_client_open2(struct cavan_i2c_client *client, const char *device);
void cavan_i2c_client_close(struct cavan_i2c_client *client);
int cavan_i2c_transfer(struct cavan_i2c_client *client, struct i2c_msg *msgs, size_t count);
void cavan_i2c_detect(struct cavan_i2c_client *client);
int cavan_i2c_write_data(struct cavan_i2c_client *client, const void *addr, const void *data, size_t size);
int cavan_i2c_read_data(struct cavan_i2c_client *client, const void *addr, void *data, size_t size);
void cavan_i2c_adjust_endian(struct cavan_i2c_client *client, u8 *addr, u8 *value);
int cavan_i2c_read_register(struct cavan_i2c_client *client, u32 addr, u32 *value);
int cavan_i2c_write_register(struct cavan_i2c_client *client, u32 addr, u32 value);
int cavan_i2c_update_bits(struct cavan_i2c_client *client, u32 addr, u32 value, u32 mask);

int cavan_i2c_sysfs_get_device_path(struct cavan_i2c_client *client, char *buff, size_t size);
int cavan_i2c_sysfs_get_device_realpath(struct cavan_i2c_client *client, char *buff, size_t size);
int cavan_i2c_sysfs_get_device_name(struct cavan_i2c_client *client, char *buff, size_t size);
int cavan_i2c_sysfs_get_driver_path(struct cavan_i2c_client *client, char *buff, size_t size);
int cavan_i2c_sysfs_get_driver_realpath(struct cavan_i2c_client *client, char *buff, size_t size);
int cavan_i2c_sysfs_get_driver_name(struct cavan_i2c_client *client, char *buff, size_t size);

static inline void cavan_i2c_set_data(struct cavan_i2c_client *client, void *data)
{
	client->private_data = data;
}

static inline void *cavan_i2c_get_data(struct cavan_i2c_client *client)
{
	return client->private_data;
}

static inline int cavan_i2c_set_tenbit(struct cavan_i2c_client *client, bool enable)
{
	return ioctl(client->fd, I2C_TENBIT, enable);
}

static inline int cavan_i2c_master_send(struct cavan_i2c_client *client, const void *buff, size_t size)
{
	return write(client->fd, buff, size);
}

static inline int cavan_i2c_master_recv(struct cavan_i2c_client *client, void *buff, size_t size)
{
	return read(client->fd, buff, size);
}
