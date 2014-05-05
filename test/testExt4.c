/*
 * File:		testExt4.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2014-05-05 10:30:09
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

#include <cavan.h>
#include <cavan/ext4.h>

#define TEST_EXT4_DEVICE_BLOCK_SIZE		512

struct test_ext4_device
{
	int fd;
};

static ssize_t test_ext4_device_read_block(struct cavan_ext4_fs *fs, size_t index, void *buff, size_t count)
{
	struct test_ext4_device *dev = fs->hw_data;

	pr_bold_info("read_block: index = " PRINT_FORMAT_SIZE ", count = " PRINT_FORMAT_SIZE, index, count);

	return ffile_readfrom(dev->fd, buff, count * fs->hw_block_size, index * fs->hw_block_size);
}

static ssize_t test_ext4_device_write_block(struct cavan_ext4_fs *fs, size_t index, const void *buff, size_t count)
{
	struct test_ext4_device *dev = fs->hw_data;

	pr_bold_info("write_block: index = " PRINT_FORMAT_SIZE ", count = " PRINT_FORMAT_SIZE, index, count);

	return ffile_writeto(dev->fd, buff, count * fs->hw_block_size, index * fs->hw_block_size);
}

int main(int argc, char *argv[])
{
	int fd;
	int ret;
	struct cavan_ext4_fs fs;
	struct test_ext4_device dev;

	assert(argc > 1);

	fd = open(argv[1], O_RDONLY);
	if (fd < 0)
	{
		pr_error_info("open");
		return fd;
	}

	dev.fd = fd;
	fs.hw_data = &dev;
	fs.hw_block_shift = 0;
	fs.hw_block_size = TEST_EXT4_DEVICE_BLOCK_SIZE;
	fs.read_block = test_ext4_device_read_block;
	fs.write_block = test_ext4_device_write_block;

	ret = cavan_ext4_init(&fs);
	if (ret < 0)
	{
		pr_red_info("cavan_ext4_init");
		goto out_close_fd;
	}

	cavan_ext4_deinit(&fs);

out_close_fd:
	close(fd);
	return ret;
}
