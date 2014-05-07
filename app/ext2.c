/*
 * File:			ext2.c
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

#define EXT2_APP_DEVICE_BLOCK_SIZE		512

struct ext2_app_device_context
{
	int fd;
};

static ssize_t ext2_app_device_read_block(struct cavan_block_device *bdev, size_t index, void *buff, size_t count)
{
	struct ext2_app_device_context *context = bdev->context;
	return ffile_readfrom(context->fd, buff, count * bdev->block_size, index * bdev->block_size);
}

static ssize_t ext2_app_device_write_block(struct cavan_block_device *bdev, size_t index, const void *buff, size_t count)
{
	struct ext2_app_device_context *context = bdev->context;
	return ffile_writeto(context->fd, buff, count * bdev->block_size, index * bdev->block_size);
}

static ssize_t ext2_app_device_read_byte(struct cavan_block_device *bdev, off_t offset, void *buff, size_t size)
{
	struct ext2_app_device_context *context = bdev->context;
	return ffile_readfrom(context->fd, buff, size, offset);
}

static ssize_t ext2_app_device_write_byte(struct cavan_block_device *bdev, off_t offset, const void *buff, size_t size)
{
	struct ext2_app_device_context *context = bdev->context;
	return ffile_writeto(context->fd, buff, size, offset);
}

static void ext2_app_device_list_dir_handler(struct ext2_dir_entry_2 *entry, void *data)
{
	print_ntext(entry->name, entry->name_len);

	if (entry->file_type == EXT2_FT_DIR)
	{
		print_char('/');
	}

	print_char('\n');
}

int main(int argc, char *argv[])
{
	int fd;
	int ret;
	char *content = NULL;
	struct cavan_ext4_fs fs;
	struct ext2_app_device_context context;
	struct cavan_block_device bdev =
	{
		.block_shift = 0,
		.block_size = EXT2_APP_DEVICE_BLOCK_SIZE,
		.block_mask = 0,
		.read_block = ext2_app_device_read_block,
		.write_block = ext2_app_device_write_block,
		.read_byte = ext2_app_device_read_byte,
		.write_byte = ext2_app_device_write_byte
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
			ret = -EFAULT;
			pr_red_info("cavan_ext4_open_file");
			goto out_cavan_ext4_deinit;
		}

		if (S_ISDIR(fp->inode.i_mode))
		{
			ret = cavan_ext4_list_dir(fp, ext2_app_device_list_dir_handler, fp);
			if (ret < 0)
			{
				pr_red_info("cavan_ext4_list_dir");
				goto out_cavan_ext4_deinit;
			}
		}
		else
		{
			ssize_t rdlen;

			if (argc > 3)
			{
				char path_buff[1024];
				const char *pathname;

				if (file_type_test(argv[3], S_IFDIR))
				{
					text_basename_base(text_path_cat(path_buff, argv[3], NULL), argv[2]);
					pathname = path_buff;
				}
				else
				{
					pathname = argv[3];
				}

				println("%s@%s => %s", argv[1], argv[2], pathname);

				rdlen = cavan_ext4_read_file3(fp, pathname, O_TRUNC);
				if (rdlen < 0)
				{
					pr_red_info("cavan_ext4_read_file3");
					goto out_cavan_ext4_deinit;
				}
			}
			else
			{
				content = malloc(fp->inode.i_size);
				if (content == NULL)
				{
					pr_error_info("malloc");
					goto out_cavan_ext4_deinit;
				}

				rdlen = cavan_ext4_read_file(fp, content, fp->inode.i_size);
				if (rdlen < 0)
				{
					pr_red_info("cavan_ext4_read_file");
					goto out_free_content;
				}

				print_ntext(content, rdlen);
			}
		}

		cavan_ext4_close_file(fp);
	}

out_free_content:
	if (content)
	{
		free(content);
	}
out_cavan_ext4_deinit:
	cavan_ext4_deinit(&fs);
out_cavan_block_device_deinit:
	cavan_block_device_deinit(&bdev);
out_close_fd:
	close(context.fd);
	return ret;
}
