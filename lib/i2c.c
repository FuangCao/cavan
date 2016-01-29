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
#include <cavan/ctype.h>

#define CAVAN_I2C_DEBUG		0

struct cavan_i2c_config cavan_i2c_config_table[] = {
	{
		.chipname = "a1v1",
		.addr_bytes = 1,
		.value_bytes = 1,
		.addr_big_endian = false,
		.value_big_endian = false,
	}, {
		.chipname = "a1v2",
		.addr_bytes = 1,
		.value_bytes = 2,
		.addr_big_endian = false,
		.value_big_endian = false,
	}, {
		.chipname = "a2v2",
		.addr_bytes = 2,
		.value_bytes = 2,
		.addr_big_endian = false,
		.value_big_endian = false,
	}, {
		.chipname = "bq27320",
		.addr_step = 2,
		.addr_bytes = 1,
		.value_bytes = 2,
		.addr_first = 0x00,
		.addr_last = 0x2F,
		.addr_big_endian = false,
		.value_big_endian = false,
	}, {
		.chipname = "bq27520",
		.addr_step = 2,
		.addr_bytes = 1,
		.value_bytes = 2,
		.addr_first = 0x00,
		.addr_last = 0x2F,
		.addr_big_endian = false,
		.value_big_endian = false,
	}, {
		.chipname = "bq24296",
		.addr_step = 1,
		.addr_bytes = 1,
		.value_bytes = 1,
		.addr_first = 0x00,
		.addr_last = 0x0A,
		.addr_big_endian = false,
		.value_big_endian = false,
	}, {
		.chipname = "bq24192",
		.addr_step = 1,
		.addr_bytes = 1,
		.value_bytes = 1,
		.addr_first = 0x00,
		.addr_last = 0x0A,
		.addr_big_endian = false,
		.value_big_endian = false,
	}, {
		.chipname = "wm8962",
		.addr_step = 1,
		.addr_bytes = 2,
		.value_bytes = 2,
		.addr_first = 0x0000,
		.addr_last = 0x5293,
		.addr_big_endian = true,
		.value_big_endian = true,
	}, {
		.chipname = "rt5670",
		.addr_step = 1,
		.addr_bytes = 1,
		.value_bytes = 2,
		.addr_first = 0x00,
		.addr_last = 0xFE,
		.addr_big_endian = true,
		.value_big_endian = true,
	}, {
		.chipname = "alc5671",
		.addr_step = 1,
		.addr_bytes = 1,
		.value_bytes = 2,
		.addr_first = 0x00,
		.addr_last = 0xFE,
		.addr_big_endian = true,
		.value_big_endian = true,
	}, {
		.chipname = "tca9535",
		.addr_step = 2,
		.addr_bytes = 1,
		.value_bytes = 2,
		.addr_first = 0x00,
		.addr_last = 0x06,
		.addr_big_endian = false,
		.value_big_endian = false,
	}, {
		.chipname = "tc358762",
		.addr_step = 2,
		.addr_bytes = 2,
		.value_bytes = 4,
		.addr_big_endian = true,
		.value_big_endian = false,
	}, {
		.chipname = "tc358775",
		.addr_step = 2,
		.addr_bytes = 2,
		.value_bytes = 4,
		.addr_big_endian = true,
		.value_big_endian = false,
	}, {
		.chipname = "tc358748",
		.addr_step = 2,
		.addr_bytes = 2,
		.value_bytes = 2,
		.addr_big_endian = true,
		.value_big_endian = true,
	}, {
		.chipname = "tc358749",
		.addr_step = 2,
		.addr_bytes = 2,
		.value_bytes = 4,
		.addr_big_endian = true,
		.value_big_endian = false,
	}, {
		.chipname = "tc358779",
		.addr_step = 2,
		.addr_bytes = 2,
		.value_bytes = 4,
		.addr_big_endian = true,
		.value_big_endian = false,
	}, {
		.chipname = "tc358768",
		.addr_step = 2,
		.addr_bytes = 2,
		.value_bytes = 2,
		.addr_big_endian = true,
		.value_big_endian = false,
	}, {
		.chipname = "adv7513",
		.addr_step = 1,
		.addr_bytes = 1,
		.value_bytes = 1,
		.addr_first = 0x00,
		.addr_last = 0xFF,
		.addr_big_endian = false,
		.value_big_endian = false,
	}
};

void cavan_i2c_config_dump(const struct cavan_i2c_config *config)
{
	if (config->chipname) {
		println("chipname = %s", config->chipname);
	}

	println("addr_step = %d", config->addr_step);
	println("addr_bytes = %d", config->addr_bytes);
	println("value_bytes = %d", config->value_bytes);
	println("addr_big_endian = %s", cavan_bool_tostring(config->addr_big_endian));
	println("value_big_endian = %s", cavan_bool_tostring(config->value_big_endian));
}

struct cavan_i2c_config *cavan_i2c_find_config(const char *chipname)
{
	int count;
	int offset = 0;
	struct cavan_i2c_config *config, *p, *p_end = cavan_i2c_config_table + NELEM(cavan_i2c_config_table);

	while (1) {
		count = 0;

		for (p = cavan_i2c_config_table; p < p_end; p++) {
			if (strlen(p->chipname) < (size_t) offset) {
				continue;
			}

			if (strcmp(p->chipname + offset, chipname) == 0) {
				return p;
			}

			count++;
		}

		if (count == 0) {
			break;
		}

		offset++;
	}

	count = 0;
	config = NULL;

	for (p = cavan_i2c_config_table; p < p_end; p++) {
		if (text_hcmp(chipname, p->chipname) == 0) {
			config = p;
			count++;
		}
	}

	if (count == 1) {
		return config;
	}

	return NULL;
}

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
	struct cavan_i2c_config *config = &client->config;

	memset(client, 0x00, sizeof(struct cavan_i2c_client));

	config->addr_step = 1;
	config->addr_bytes = 1;
	config->value_bytes = 1;
}

int cavan_i2c_client_open(struct cavan_i2c_client *client, int adapter, u16 slave_addr)
{
	int fd;
	int ret;
	char pathname[32];
	struct cavan_i2c_config *chip_config = NULL;
	struct cavan_i2c_config *config = &client->config;

	if (config->chipname && (chip_config = cavan_i2c_find_config(config->chipname)) == NULL) {
		pr_red_info("No config found for chip: %s", config->chipname);
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
	client->adapter = adapter;

	ret = cavan_i2c_set_address(client, slave_addr);
	if (ret < 0) {
		pr_red_info("cavan_i2c_set_address: %d", ret);
		goto out_close_fd;
	}

	if (client->slave_addr > 0) {
		if (cavan_i2c_sysfs_get_device_realpath(client, client->device_path, sizeof(client->device_path)) < 0) {
			client->device_path[0] = 0;
		} else {
			println("device_path = %s", client->device_path);
		}

		if (cavan_i2c_sysfs_get_driver_realpath(client, client->driver_path, sizeof(client->driver_path)) < 0) {
			client->driver_path[0] = 0;
		} else {
			println("driver_path = %s", client->driver_path);

			if (chip_config == NULL) {
				config->chipname = text_basename(client->driver_path);
				chip_config = cavan_i2c_find_config(config->chipname);
			}
		}
	}

	if (chip_config) {
		println("chipname = %s", chip_config->chipname);
		memcpy(&client->config, chip_config, sizeof(client->config));
	} else {
		if (config->addr_bytes < 1 || config->addr_bytes > 4) {
			pr_red_info("Invalid addr_bytes = %d", config->addr_bytes);
			ret = -EINVAL;
			goto out_close_fd;
		}

		if (config->value_bytes < 1 || config->value_bytes > 4) {
			pr_red_info("Invalid value_bytes = %d", config->value_bytes);
			ret = -EINVAL;
			goto out_close_fd;
		}

		if (config->addr_step < 1 || config->addr_step > 4) {
			pr_warn_info("Invalid addr_step = %d, set to default %d now", config->addr_step, config->addr_bytes);
			config->addr_step = config->value_bytes;
		}
	}

	if (config->addr_bytes < 2) {
		config->addr_big_endian = false;
	}

	if (config->value_bytes < 2) {
		config->value_big_endian = false;
	}

	if (file_access_e("/sys/bus/platform/drivers/rockchip_i2c")) {
		client->flags |= CAVAN_I2C_FLAG_ROCKCHIP;
	}

	if (client->scl_rate == 0) {
		client->scl_rate = CAVAN_I2C_RATE_100K;
	}

#if CAVAN_I2C_DEBUG
	cavan_i2c_config_dump(config);
#endif

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
		int ret;
		char rd_value;
		char wr_value = 0;

		ret = cavan_i2c_set_address(client, addr);
		if (ret < 0) {
			pr_err_info("cavan_i2c_set_address: %d", ret);
			continue;
		}

		if (cavan_i2c_master_recv(client, &rd_value, 1) == 1 || cavan_i2c_master_send(client, &wr_value, 1) == 1) {
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
	char buff[size + client->config.addr_bytes];

#if CAVAN_I2C_DEBUG
	print_mem("%s: addr = ", addr, client->config.addr_bytes, __FUNCTION__);
	print_mem("%s: data = ", data, size, __FUNCTION__);
#endif

	memcpy(buff, addr, client->config.addr_bytes);
	memcpy(buff + client->config.addr_bytes, data, size);

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
			.len = client->config.addr_bytes,
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
	print_mem("%s: addr = ", addr, client->config.addr_bytes, __FUNCTION__);
	print_mem("%s: data = ", data, size, __FUNCTION__);
#endif

	return size;
}

int cavan_i2c_read_register(struct cavan_i2c_client *client, u32 addr, u32 *value)
{
	u8 *p;
	int ret;

	*value = 0;

	if (client->config.addr_big_endian) {
		p = (u8 *) &addr;
		mem_reverse_simple(p, p + client->config.addr_bytes - 1);
	}

	ret = cavan_i2c_read_data(client, &addr, value, client->config.value_bytes);
	if (ret == client->config.value_bytes && client->config.value_big_endian) {
		p = (u8 *) value;
		mem_reverse_simple(p, p + client->config.value_bytes - 1);
	}

	return ret;
}

int cavan_i2c_write_register(struct cavan_i2c_client *client, u32 addr, u32 value)
{
	u8 *p;

	if (client->config.addr_big_endian) {
		p = (u8 *) &addr;
		mem_reverse_simple(p, p + client->config.addr_bytes - 1);
	}

	if (client->config.value_big_endian) {
		p = (u8 *) &value;
		mem_reverse_simple(p, p + client->config.value_bytes - 1);
	}

	return cavan_i2c_write_data(client, &addr, &value, client->config.value_bytes);
}

int cavan_i2c_update_bits(struct cavan_i2c_client *client, u32 addr, u32 value, u32 mask)
{
	int ret;
	u32 value_old;

	ret = cavan_i2c_read_register(client, addr, &value_old);
	if (ret < 0) {
		return ret;
	}

	print_bit_mask(value_old, "mask: ");

	value = (value & mask) | (value_old & (~mask));
	if (value == value_old) {
		return 0;
	}

	println("update: addr = 0x%08x, value = (0x%08x -> 0x%08x)", addr, value_old, value);
	print_bit_mask(value, "mask: ");

	return cavan_i2c_write_register(client, addr, value);
}

int cavan_i2c_sysfs_get_device_path(struct cavan_i2c_client *client, char *buff, size_t size)
{
	return snprintf(buff, size, "/sys/bus/i2c/devices/%d-%04x", client->adapter, client->slave_addr);
}

int cavan_i2c_sysfs_get_device_realpath(struct cavan_i2c_client *client, char *buff, size_t size)
{
	char pathname[1024];

	cavan_i2c_sysfs_get_device_path(client, pathname, sizeof(pathname));

	if (realpath(pathname, buff) == NULL) {
#if CAVAN_I2C_DEBUG
		pr_err_info("realpath: pathname = %s", pathname);
#endif
		return -ENOENT;
	}

	return 0;
}

int cavan_i2c_sysfs_get_device_name(struct cavan_i2c_client *client, char *buff, size_t size)
{
	int ret;
	char *p;
	int length;
	ssize_t rdlen;
	char pathname[1024];

	ret = cavan_i2c_sysfs_get_device_path(client, pathname, sizeof(pathname));
	if (ret < 0) {
		return ret;
	}

	length = strlen(pathname);
	strncpy(pathname + length, "/name", sizeof(pathname) - length);

	rdlen = file_read(pathname, buff, size);
	if (rdlen < 0) {
		return rdlen;
	}

	for (p = buff + rdlen - 1; p > buff && cavan_isspace(*p); p--) {
		*p = 0;
	}

	return p - buff;
}

int cavan_i2c_sysfs_get_driver_path(struct cavan_i2c_client *client, char *buff, size_t size)
{
	int length;
	ssize_t rdlen;
	char *filename;

	length = cavan_i2c_sysfs_get_device_path(client, buff, size);
	filename = buff + length;
	*filename++ = '/';
	length = size - length - 1;
	strncpy(filename, "driver", length);

	rdlen = readlink(buff, filename, length);
	if (rdlen < 0) {
#if CAVAN_I2C_DEBUG
		pr_err_info("readlink: pathname = %s", buff);
#endif
		return rdlen;
	}

	return 0;
}

int cavan_i2c_sysfs_get_driver_realpath(struct cavan_i2c_client *client, char *buff, size_t size)
{
	int ret;
	char pathname[1024];

	ret = cavan_i2c_sysfs_get_driver_path(client, pathname, sizeof(pathname));
	if (ret < 0) {
		return ret;
	}

	if (realpath(pathname, buff) == NULL) {
#if CAVAN_I2C_DEBUG
		pr_err_info("realpath: pathname = %s", pathname);
#endif
		return -ENOENT;
	}

	return 0;
}

int cavan_i2c_sysfs_get_driver_name(struct cavan_i2c_client *client, char *buff, size_t size)
{
	int ret;
	char pathname[1024];
	const char *basename;

	ret = cavan_i2c_sysfs_get_driver_path(client, pathname, sizeof(pathname));
	if (ret < 0) {
		return ret;
	}

	basename = text_basename_simple(pathname);
	strncpy(buff, basename, size);

	return 0;
}
