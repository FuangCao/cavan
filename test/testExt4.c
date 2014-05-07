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

struct test_ext4_device_context
{
	int fd;
};

static ssize_t test_ext4_device_read_block(struct cavan_block_device *dev, size_t index, void *buff, size_t count)
{
	off_t location = index * dev->block_size;
	struct test_ext4_device_context *context = dev->context;

	pr_bold_info("read_block: index = " PRINT_FORMAT_SIZE ", count = " PRINT_FORMAT_SIZE, index, count);
	pr_bold_info("location = " PRINT_FORMAT_OFF, location);

	return ffile_readfrom(context->fd, buff, count * dev->block_size, location);
}

static ssize_t test_ext4_device_write_block(struct cavan_block_device *dev, size_t index, const void *buff, size_t count)
{
	struct test_ext4_device_context *context = dev->context;

	pr_bold_info("write_block: index = " PRINT_FORMAT_SIZE ", count = " PRINT_FORMAT_SIZE, index, count);

	return ffile_writeto(context->fd, buff, count * dev->block_size, index * dev->block_size);
}

int main(int argc, char *argv[])
{
	int fd;
	int ret;
	struct cavan_ext4_fs fs;
	struct test_ext4_device_context context;
	struct cavan_block_device bdev =
	{
		.block_shift = 0,
		.block_size = TEST_EXT4_DEVICE_BLOCK_SIZE,
		.block_mask = 0,
		.read_block = test_ext4_device_read_block,
		.write_block = test_ext4_device_write_block,
		.read_byte = NULL,
		.write_byte = NULL
	};

	assert(argc > 1);

	fd = open(argv[1], O_RDONLY);
	if (fd < 0)
	{
		pr_error_info("open");
		return fd;
	}

	context.fd = fd;

	ret = cavan_block_device_init(&bdev, &context);
	if (ret < 0)
	{
		pr_red_info("cavan_block_device_init");
		goto out_close_fd;
	}

	ret = cavan_ext4_init(&fs, &bdev);
	if (ret < 0)
	{
		pr_red_info("cavan_ext4_init");
		goto out_cavan_block_device_deinit;
	}

	if (argc > 2)
	{
		struct cavan_ext4_file *fp;

		fp = cavan_ext4_open_file(&fs, argv[2]);
		if (fp == NULL)
		{
			pr_red_info("cavan_ext4_open_file");
		}
		else
		{
			ssize_t rdlen;
			char buff[fp->inode.i_size];

			rdlen = cavan_ext4_read_file(fp, buff, sizeof(buff));
			if (rdlen < 0)
			{
				pr_red_info("cavan_ext4_read_file");
			}
			else
			{
				println("rdlen = " PRINT_FORMAT_SIZE, rdlen);
				print_ntext(buff, rdlen);
				putchar('\n');
			}

			cavan_ext4_close_file(fp);
		}
	}

	cavan_ext4_deinit(&fs);

out_cavan_block_device_deinit:
	cavan_block_device_deinit(&bdev);
out_close_fd:
	close(fd);
	return ret;
}
