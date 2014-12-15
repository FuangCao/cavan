/*
 * File:		hua_bttest.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2014-11-25 11:55:09
 *
 * Copyright (c) 2014 Fuang.Cao <cavan.cfa@gmail.com>
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

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluedroid/bluetooth.h>
#include <sys/ioctl.h>
#include <textfile.h>
#include <alloca.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#define HUAMOBILE_MAX_RSP_NUM	255

#define NELEM(a) \
	(sizeof(a) / sizeof((a)[0]))

#define STORAGEDIR \
	"/data/misc/bluetoothd"

#define BYTE_IS_LF(b) \
	((b) == '\r' || (b) == '\n')

#define BYTE_IS_SPACE(b) \
	((b) == ' ' || (b) == '\t')

#define BYTE_IS_SEP(b) \
	BYTE_IS_SPACE(b)

#define pr_pos_info() \
	printf("%s => %s[%d]\n", __FILE__, __FUNCTION__, __LINE__)

#define pr_red_info(fmt, args ...) \
	printf("\033[31m" fmt "\033[0m\n", ##args)

#define pr_green_info(fmt, args ...) \
	printf("\033[32m" fmt "\033[0m\n", ##args)

#define pr_bold_info(fmt, args ...) \
	printf("\033[1m" fmt "\033[0m\n", ##args)

#define pr_error_info(fmt, args ...) \
	if (errno) { \
		pr_red_info("%s[%d](" fmt "): %s", __FUNCTION__, __LINE__, ##args, strerror(errno)); \
	} else { \
		pr_red_info("%s[%d]:" fmt, __FUNCTION__, __LINE__, ##args); \
	}

struct huamobile_hci_name_map
{
	char addr[32];
	char name[128];
};

static ssize_t huamobile_hci_inquiry(int hci_dev, int dev_id, struct hci_inquiry_req *req, size_t count)
{
	int ret;

	req->dev_id = dev_id;
	req->flags = 0;
	req->lap[0] = 0x33;
	req->lap[1] = 0x8B;
	req->lap[2] = 0x9E;
	req->length = 10;
	req->num_rsp = count;

	pr_bold_info("Scanning ...");

	ret = ioctl(hci_dev, HCIINQUIRY, req);
	if (ret < 0)
	{
		return ret;
	}

	return req->num_rsp;
}

static int huamobile_hci_get_pathname(int hci_dev, int dev_id, char *pathname, size_t size)
{
	int ret;
	struct hci_dev_info dev_info;
	char local_addr[64];

	memset(&dev_info, 0, sizeof(struct hci_dev_info));
	dev_info.dev_id = dev_id;
	ret = ioctl(hci_dev, HCIGETDEVINFO, &dev_info);
	if (ret < 0)
	{
		pr_error_info("HCIGETDEVINFO");
		return ret;
	}

	ba2str(&dev_info.bdaddr, local_addr);

	return snprintf(pathname, size, STORAGEDIR "/%s/names", local_addr);
}

static int huamobile_mmap(const char *pathname, void **addr, size_t *size)
{
	int ret;
	int fd;
	struct stat st;
	void *mem;

	fd = open(pathname, O_RDONLY);
	if (fd < 0)
	{
		pr_error_info("open file `%s'", pathname);
		return fd;
	}

	ret = flock(fd, LOCK_SH);
	if (ret < 0)
	{
		pr_error_info("flock");
		goto out_close_fd;
	}

	ret = fstat(fd, &st);
	if (ret < 0)
	{
		pr_error_info("fstat");
		goto out_unlock_fd;
	}

	mem = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (mem == NULL || mem == MAP_FAILED)
	{
		ret = -EFAULT;
		pr_error_info("mmap");
		goto out_unlock_fd;
	};

	*addr = mem;
	*size = st.st_size;
	return fd;

out_unlock_fd:
	flock(fd, LOCK_UN);
out_close_fd:
	close(fd);
	return ret;
}

static void huamobile_unmap(int fd, void *map, size_t size)
{
	munmap(map, size);
	flock(fd, LOCK_UN);
	close(fd);
}

static ssize_t huamobile_get_map_item(const char *start, const char *end, char *item[], size_t count)
{
	size_t i;
	char *p;

	for (i = 0; i < count && start < end; i++)
	{
		p = item[i];

		while (start < end)
		{
			if (BYTE_IS_SEP(*start))
			{
				break;
			}

			*p++ = *start++;
		}

		*p = 0;

		for (start++; start < end && BYTE_IS_SEP(*start); start++);
	}

	return i;
}

static ssize_t huamobile_load_name_map1(void *buff, size_t size, struct huamobile_hci_name_map *map, size_t count)
{
	int ret;
	char *p, *end_file, *end_line;
	struct huamobile_hci_name_map *map_bak, *end_map;
	char *item[2];

	map_bak = map;
	end_map = map + count;

	for (p = buff, end_file = p + size; p < end_file && map < end_map; p = end_line + 1)
	{
		while (p < end_file && BYTE_IS_SPACE(*p))
		{
			p++;
		}

		for (end_line = p; end_line < end_file; end_line++)
		{
			if (BYTE_IS_LF(*end_line))
			{
				break;
			}
		}

		item[0] = map->addr;
		item[1] = map->name;
		ret = huamobile_get_map_item(p, end_line, item, 2);
		if (ret == 2)
		{
			map++;
		}

		for (p = end_line; p < end_file && BYTE_IS_LF(*end_line); p++);
	}

	return map - map_bak;
}

static ssize_t huamobile_load_name_map2(const char *pathname, struct huamobile_hci_name_map *map, size_t count)
{
	int fd;
	int ret;
	void *addr;
	size_t size;

	// pr_bold_info("pathname = %s", pathname);

	fd = huamobile_mmap(pathname, &addr, &size);
	if (fd < 0)
	{
		pr_red_info("huamobile_mmap");
		return fd;
	}

	ret = huamobile_load_name_map1(addr, size, map, count);

	huamobile_unmap(fd, addr, size);

	return ret;
}

static ssize_t huamobile_load_name_map3(int hci_dev, int dev_id, struct huamobile_hci_name_map *map, size_t count)
{
	int ret;
	char pathname[PATH_MAX + 1];

	ret = huamobile_hci_get_pathname(hci_dev, dev_id, pathname, sizeof(pathname) - 1);
	if (ret < 0)
	{
		pr_red_info("huamobile_hci_get_pathname");
		return ret;
	}

	return huamobile_load_name_map2(pathname, map, count);
}

static const char *huamobile_hci_get_device_name(const char *addr, const struct huamobile_hci_name_map *map, size_t size)
{
	const struct huamobile_hci_name_map *map_end;

	for (map_end = map + size; map < map_end; map++)
	{
		if (strcmp(map->addr, addr) == 0)
		{
			return map->name;
		}
	}

	return NULL;
}

static int huamobile_hci_show_inquiry_info(int hci_dev, int dev_id, const inquiry_info *info, size_t count)
{
	int ret;
	struct huamobile_hci_name_map name_map[16], *p;
	ssize_t map_size;
	char peer_addr[32];
	const inquiry_info *info_end;
	const char *devname;

	map_size = huamobile_load_name_map3(hci_dev, dev_id, name_map, NELEM(name_map));
	if (map_size < 0)
	{
		pr_red_info("huamobile_load_name_map3");
		map_size = 0;
	}

	for (info_end = info + count; info < info_end; info++)
	{
		ba2str(&info->bdaddr, peer_addr);
		devname = huamobile_hci_get_device_name(peer_addr, name_map, map_size);
		if (devname == NULL)
		{
			devname = "UNKNOWN";
		}

		pr_bold_info("name = %s, addr = %s", devname, peer_addr);
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int ret;
	int dev_id;
	int hci_dev;
	struct hci_inquiry_req *req;
	inquiry_info *info;
	ssize_t num_rsp;

	ret = bt_is_enabled();
	if (ret < 0)
	{
		pr_red_info("bt_is_enabled");
		return ret;
	}

	if (ret == 0)
	{
		ret = bt_enable();
		if (ret < 0)
		{
			pr_red_info("bt_enable");
			return ret;
		}
	}

	dev_id = hci_get_route(NULL);
	if (dev_id < 0)
	{
		ret = dev_id;
		pr_error_info("hci_get_route");
		goto out_bt_disable;
	}

	hci_dev = hci_open_dev(dev_id);
	if (hci_dev < 0)
	{
		ret = hci_dev;
		pr_error_info("hci_open_dev");
		goto out_bt_disable;
	}

	req = alloca(sizeof(struct hci_inquiry_req) + sizeof(inquiry_info) * HUAMOBILE_MAX_RSP_NUM);
	if (req == NULL)
	{
		ret = -ENOMEM;
		pr_error_info("alloca");
		goto out_hci_close_dev;
	}

	num_rsp = huamobile_hci_inquiry(hci_dev, dev_id, req, HUAMOBILE_MAX_RSP_NUM);
	if (num_rsp < 0)
	{
		ret = num_rsp;
		pr_error_info("huamobile_hci_inquiry");
		goto out_hci_close_dev;
	}

	info = (inquiry_info *)(req + 1);
	ret = huamobile_hci_show_inquiry_info(hci_dev, dev_id, info, num_rsp);
	if (ret < 0)
	{
		pr_red_info("huamobile_hci_show_inquiry_info");
		goto out_hci_close_dev;
	}

	ret = 0;

out_hci_close_dev:
	hci_close_dev(hci_dev);
out_bt_disable:
	bt_disable();
	return ret;
}
