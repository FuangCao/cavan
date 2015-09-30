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

#define CAVAN_I2C_DEBUG		0

int cavan_i2c_set_address(struct cavan_i2c_client *client, u16 slave_addr)
{
	int ret;

	ret = cavan_i2c_set_tenbit(client, (slave_addr & 0xFF00) != 0);
	if (ret < 0) {
		return ret;
	}

	ret = ioctl(client->fd, I2C_SLAVE_FORCE, slave_addr);
	if (ret < 0) {
		return ret;
	}

	client->slave_addr = slave_addr;

	return 0;
}

void cavan_i2c_client_init(struct cavan_i2c_client *client)
{
	memset(client, 0x00, sizeof(struct cavan_i2c_client));

	client->addr_bytes = 1;
	client->value_bytes = 1;
}

int cavan_i2c_client_open(struct cavan_i2c_client *client, int adapter, u16 slave_addr)
{
	int fd;
	int ret;
	char pathname[32];

	if (client->addr_bytes < 1 || client->addr_bytes > 4) {
		pr_red_info("Invalid addr_bytes = %d", client->addr_bytes);
		return -EINVAL;
	}

	if (client->value_bytes < 1 || client->value_bytes > 4) {
		pr_red_info("Invalid value_bytes = %d", client->value_bytes);
		return -EINVAL;
	}

	snprintf(pathname, sizeof(pathname), "/dev/i2c-%d", adapter);

#if CAVAN_I2C_DEBUG
	println("pathname = %s", pathname);
#endif

	fd = open(pathname, O_RDWR);
	if (fd < 0) {
		pr_err_info("open %s", pathname);
		return fd;
	}

	client->fd = fd;

	ret = cavan_i2c_set_address(client, slave_addr);
	if (ret < 0) {
		pr_red_info("cavan_i2c_set_address: %d", ret);
		goto out_close_fd;
	}

	if (client->addr_bytes < 2) {
		client->addr_big_endian = false;
	}

	if (client->value_bytes < 2) {
		client->value_big_endian = false;
	}

	if (file_access_e("/sys/bus/platform/drivers/rockchip_i2c")) {
		client->flags |= CAVAN_I2C_FLAG_ROCKCHIP;
	}

	if (client->scl_rate == 0) {
		client->scl_rate = CAVAN_I2C_RATE_100K;
	}

	return 0;

out_close_fd:
	close(fd);
	return ret;
}

int cavan_i2c_client_open2(struct cavan_i2c_client *client, const char *device)
{
	int adapter;
	u16 slave_addr;
	const char *p;

	adapter = text2value_unsigned(device, &p, 10);
	if (p == device || text_find("-:/@%", *p) == NULL) {
		pr_red_info("Invalid device %s", device);
		return -EINVAL;
	}

	device = p + 1;

	slave_addr = text2value_unsigned(device, &p, 16);
	if (p == device) {
		pr_red_info("Invalid client address %s", device);
		return -EINVAL;
	}

	return cavan_i2c_client_open(client, adapter, slave_addr);
}

void cavan_i2c_client_close(struct cavan_i2c_client *client)
{
	close(client->fd);
}

int cavan_i2c_transfer(struct cavan_i2c_client *client, struct i2c_msg *msgs, size_t count)
{
	struct i2c_rdwr_ioctl_data data;

	data.nmsgs = count;

	if (client->flags & CAVAN_I2C_FLAG_ROCKCHIP) {
		int i;
		struct i2c_msg_rockchip rk_msgs[count];

		for (i = count - 1; i >= 0; i--) {
			rk_msgs[i].msg = msgs[i];
			rk_msgs[i].scl_rate = client->scl_rate;
		}

		data.msgs = (struct i2c_msg *) rk_msgs;

		return ioctl(client->fd, I2C_RDWR, &data);
	} else {
		data.msgs = msgs;

		return ioctl(client->fd, I2C_RDWR, &data);
	}

}

void cavan_i2c_detect(struct cavan_i2c_client *client)
{
	u16 addr;
	int count = 0;

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
			pr_green_info("0x%02x", addr);
			count++;
		}
	}

	if (count <= 0) {
		pr_red_info("no slave found!");
	}
}

int cavan_i2c_write_data(struct cavan_i2c_client *client, const void *addr, const void *data, size_t size)
{
	int ret;
	char buff[size + client->addr_bytes];

#if CAVAN_I2C_DEBUG
	print_mem("%s: addr = ", addr, client->addr_bytes, __FUNCTION__);
	print_mem("%s: data = ", data, size, __FUNCTION__);
#endif

	memcpy(buff, addr, client->addr_bytes);
	memcpy(buff + client->addr_bytes, data, size);

	ret = cavan_i2c_master_send(client, buff, sizeof(buff));
	if (ret == (int) sizeof(buff)) {
		return size;
	}

	return -EFAULT;
}

int cavan_i2c_read_data(struct cavan_i2c_client *client, const void *addr, void *data, size_t size)
{
	int ret;
	struct i2c_msg msgs[] = {
		{
			.addr = client->slave_addr,
			.flags = 0,
			.len = client->addr_bytes,
			.buf = __UNCONST(addr),
		}, {
			.addr = client->slave_addr,
			.flags = I2C_M_RD,
			.len = size,
			.buf = data,
		}
	};

	ret = cavan_i2c_transfer(client, msgs, NELEM(msgs));
	if (ret != NELEM(msgs)) {
		return -EFAULT;
	}

#if CAVAN_I2C_DEBUG
	print_mem("%s: addr = ", addr, client->addr_bytes, __FUNCTION__);
	print_mem("%s: data = ", data, size, __FUNCTION__);
#endif

	return size;
}

int cavan_i2c_read_register(struct cavan_i2c_client *client, u32 addr, u32 *value)
{
	u8 *p;
	int ret;

	*value = 0;

	if (client->addr_big_endian) {
		p = (u8 *) &addr;
		mem_reverse_simple(p, p + client->addr_bytes - 1);
	}

	ret = cavan_i2c_read_data(client, &addr, value, client->value_bytes);
	if (ret == client->value_bytes && client->value_big_endian) {
		p = (u8 *) value;
		mem_reverse_simple(p, p + client->value_bytes - 1);
	}

	return ret;
}

int cavan_i2c_write_register(struct cavan_i2c_client *client, u32 addr, u32 value)
{
	u8 *p;

	if (client->addr_big_endian) {
		p = (u8 *) &addr;
		mem_reverse_simple(p, p + client->addr_bytes - 1);
	}

	if (client->value_big_endian) {
		p = (u8 *) &value;
		mem_reverse_simple(p, p + client->value_bytes - 1);
	}

	return cavan_i2c_write_data(client, &addr, &value, client->value_bytes);
}

int cavan_i2c_update_bits(struct cavan_i2c_client *client, u32 addr, u32 value, u32 mask)
{
	int ret;
	u32 value_old;

	ret = cavan_i2c_read_register(client, addr, &value_old);
	if (ret < 0) {
		return ret;
	}

	value = (value & mask) | (value_old & (~mask));
	if (value == value_old) {
		return 0;
	}

	println("update_bits: addr = 0x%08x, value = (0x%08x -> 0x%08x)", addr, value_old, value);

	return cavan_i2c_write_register(client, addr, value);
}
