/*
 * File:			vfat.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2014-06-03 10:30:09
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
#include <cavan/vfat.h>

#define VFAT_APP_DEVICE_BLOCK_SIZE		512
#define VFAT_APP_USB_DEFAULT_READ_BYTE	0

static ssize_t vfat_app_device_read_block(struct cavan_block_device *bdev, u64 index, void *buff, size_t count)
{
	return ffile_readfrom(*(int *)bdev->context, buff, count << bdev->block_shift, index << bdev->block_shift);
}

static ssize_t vfat_app_device_write_block(struct cavan_block_device *bdev, u64 index, const void *buff, size_t count)
{
	return ffile_writeto(*(int *)bdev->context, buff, count << bdev->block_shift, index << bdev->block_shift);
}

#if VFAT_APP_USB_DEFAULT_READ_BYTE
#define vfat_app_device_read_byte		NULL
#define vfat_app_device_write_byte		NULL
#else
static ssize_t vfat_app_device_read_byte(struct cavan_block_device *bdev, u64 index, u32 offset, void *buff, size_t size)
{
	return ffile_readfrom(*(int *)bdev->context, buff, size, (index << bdev->block_shift) + offset);
}

static ssize_t vfat_app_device_write_byte(struct cavan_block_device *bdev, u64 index, u32 offset, const void *buff, size_t size)
{
	return ffile_writeto(*(int *)bdev->context, buff, size, (index << bdev->block_shift) + offset);
}
#endif

static void vfat_app_device_list_dir_handler(const struct vfat_dir_entry *entry, const char *filename, size_t namelen, void *data)
{
	pr_info("%s%s", filename, VFAT_IS_DIRECTORY(entry) ? "/" : "");
}

int main(int argc, char *argv[])
{
	int fd;
	int ret;
	char *content = NULL;
	struct cavan_vfat_fs fs;
	struct cavan_block_device bdev =
	{
		.block_shift = 0,
		.block_size = VFAT_APP_DEVICE_BLOCK_SIZE,
		.block_mask = 0,
		.read_block = vfat_app_device_read_block,
		.write_block = vfat_app_device_write_block,
		.read_byte = vfat_app_device_read_byte,
		.write_byte = vfat_app_device_write_byte
	};

	assert(argc > 1);

	fd = open(argv[1], O_RDONLY);
	if (fd < 0)
	{
		pr_error_info("open");
		return fd;
	}

	ret = cavan_block_device_init(&bdev, &fd);
	if (ret < 0)
	{
		pr_red_info("cavan_block_device_init");
		goto out_close_fd;
	}

	ret = cavan_vfat_init(&fs, &bdev);
	if (ret < 0)
	{
		pr_red_info("cavan_vfat_init");
		goto out_cavan_block_device_deinit;
	}

	if (argc > 2)
	{
		struct cavan_vfat_file *fp;

		fp = cavan_vfat_open_file(&fs, argv[2]);
		if (fp == NULL)
		{
			ret = -EFAULT;
			pr_red_info("cavan_vfat_open_file");
			goto out_cavan_vfat_deinit;
		}

		if (VFAT_IS_DIRECTORY(&fp->entry))
		{
			ret = cavan_vfat_list_dir(fp, vfat_app_device_list_dir_handler, fp);
			if (ret < 0)
			{
				pr_red_info("cavan_vfat_list_dir");
				goto out_cavan_vfat_deinit;
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
					text_basename_base(text_path_cat(path_buff, sizeof(path_buff), argv[3], NULL), argv[2]);
					pathname = path_buff;
				}
				else
				{
					pathname = argv[3];
				}

				println("%s@%s => %s", argv[1], argv[2], pathname);

				rdlen = cavan_vfat_read_file3(fp, 0, pathname, O_TRUNC);
				if (rdlen < 0)
				{
					pr_red_info("cavan_vfat_read_file3");
					goto out_cavan_vfat_deinit;
				}
			}
			else
			{
				content = malloc(fp->entry.file_size);
				if (content == NULL)
				{
					pr_error_info("malloc");
					goto out_cavan_vfat_deinit;
				}

				rdlen = cavan_vfat_read_file(fp, 0, content, fp->entry.file_size);
				if (rdlen < 0)
				{
					pr_red_info("cavan_vfat_read_file");
					goto out_free_content;
				}

				print_ntext(content, rdlen);
				print_char('\n');
			}
		}

		cavan_vfat_close_file(fp);
	}

out_free_content:
	if (content)
	{
		free(content);
	}
out_cavan_vfat_deinit:
	cavan_vfat_deinit(&fs);
out_cavan_block_device_deinit:
	cavan_block_device_deinit(&bdev);
out_close_fd:
	close(fd);
	return ret;
}
