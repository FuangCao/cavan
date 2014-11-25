/*
 * File:		bootimg.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2014-11-24 13:33:51
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
#include <cavan/sha.h>
#include <cavan/bootimg.h>

void bootimg_header_dump(struct bootimg_header *hdr)
{
	char buff[128];

	println("magic = %s", text_substring((char *) hdr->magic, buff, 0, sizeof(hdr->magic)));
	println("base = 0x%x", hdr->kernel_addr - BOOTIMG_DEFAULT_KERNEL_OFFSET);
	println("kernel_size = %d", hdr->kernel_size);
	println("kernel_addr = 0x%08x", hdr->kernel_addr);
	println("ramdisk_size = %d", hdr->ramdisk_size);
	println("ramdisk_addr = 0x%08x", hdr->ramdisk_addr);
	println("second_size = %d", hdr->second_size);
	println("second_addr = 0x%08x", hdr->second_addr);
	println("tags_addr = 0x%08x", hdr->tags_addr);
	println("page_size = %d", hdr->page_size);
	println("dt_size = %d", hdr->dt_size);
	println("unused = 0x%08x 0x%08x", hdr->unused[0], hdr->unused[1]);
	println("name = \"%s\"", hdr->name);
	println("cmdline = \"%s\"", hdr->cmdline);
	println("extra_cmdline = \"%s\"", hdr->extra_cmdline);
	println("id = %s", cavan_shasum_tostring((u8 *) hdr->id, sizeof(hdr->id), buff, sizeof(buff)));
}

int bootimg_gen_repack_script(const struct bootimg_header *hdr, const char *pathname)
{
	int fd;
	int ret;
	unsigned base;

	println("automatically generated script `%s'", pathname);

	fd = open(pathname, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd < 0)
	{
		pr_red_info("open file `%s'", pathname);
		return fd;
	}

	base = hdr->kernel_addr;

	if (hdr->ramdisk_addr < base)
	{
		base = hdr->ramdisk_addr;
	}

	if (hdr->second_addr < base)
	{
		base = hdr->second_addr;
	}

	if (hdr->tags_addr < base)
	{
		base = hdr->tags_addr;
	}

	ret = ffile_puts(fd, "#!/bin/bash\n\n");
	ret |= ffile_puts(fd, "CMD_MKBOOTIMG=\"mkbootimg\"\n\n");
	ret |= ffile_printf(fd, "${CMD_MKBOOTIMG} --output boot-repack.img --pagesize %d --base 0x%x --tags_offset 0x%x", hdr->page_size, base, hdr->tags_addr - base);

	if (hdr->kernel_size > 0)
	{
		ret |= ffile_printf(fd, " --kernel kernel.bin --kernel_offset 0x%x", hdr->kernel_addr - base);
	}

	if (hdr->ramdisk_size > 0)
	{
		ret |= ffile_printf(fd, " --ramdisk ramdisk.img --ramdisk_offset 0x%x", hdr->ramdisk_addr - base);
	}

	if (hdr->second_size > 0)
	{
		ret |= ffile_printf(fd, " --second second.bin --second_offset 0x%x", hdr->second_addr - base);
	}

	if (hdr->dt_size > 0)
	{
		ret |= ffile_printf(fd, " --dt remain.bin");
	}

	if (hdr->name[0])
	{
		ret |= ffile_printf(fd, " --name \"%s\"", hdr->name);
	}

	if (hdr->cmdline[0])
	{
		ret |= ffile_printf(fd, " --cmdline \"%s%s\"", hdr->cmdline, hdr->extra_cmdline);
	}

	if (ret < 0)
	{
		goto out_close_fd;
	}

	ret = 0;
out_close_fd:
	close(fd);
	return ret;
}

int bootimg_unpack(const char *input, const char *output)
{
	int fd;
	int ret;
	int count;
	char *filename;
	char pathname[1024];
	struct bootimg_header hdr;
	struct bootimg_image images[4];

	fd = open(input, O_RDONLY);
	if (fd < 0)
	{
		pr_error_info("open file `%s'", input);
		return fd;
	}

	ret = bootimg_read_header(fd, &hdr);
	if (ret < 0)
	{
		pr_red_info("bootimg_read_header");
		goto out_close_fd;
	}

	bootimg_header_dump(&hdr);

	count = 0;

	if (hdr.kernel_size > 0)
	{
		images[count].size = hdr.kernel_size;
		images[count++].name = "kernel.bin";
	}

	if (hdr.ramdisk_size > 0)
	{
		images[count].size = hdr.ramdisk_size;
		images[count++].name = "ramdisk.img";
	}

	if (hdr.second_size > 0)
	{
		images[count].size = hdr.second_size;
		images[count++].name = "second.bin";
	}

	images[count].size = 0;
	images[count++].name = "remain.bin";

	filename = text_copy(pathname, output);
	ret = mkdir_hierarchy(pathname, 0777);
	if (ret < 0)
	{
		pr_red_info("mkdir_hierarchy");
		goto out_close_fd;
	}

	*filename++ = '/';

	if (count > 0)
	{
		struct bootimg_image *p, *p_end;

		for (p = images, p_end = p + count; p < p_end; p++)
		{
			ret = cavan_file_seek_next_page(fd, hdr.page_size);
			if (ret < 0)
			{
				pr_red_info("cavan_file_seek_next_page");
				goto out_close_fd;
			}

			strcpy(filename, p->name);
			println("%s -> %s", p->name, pathname);

			if (p->size > 0)
			{
				ret = file_ncopy2(fd, pathname, p->size, O_WRONLY | O_TRUNC | O_CREAT, 0777);
			}
			else
			{
				ret = file_copy2(fd, pathname, O_WRONLY | O_TRUNC | O_CREAT, 0777);
				if (ret != (ssize_t) hdr.dt_size)
				{
					hdr.dt_size = 0;
				}
			}

			if (ret < 0)
			{
				pr_red_info("file_copy2");
				goto out_close_fd;
			}
		}
	}

	strcpy(filename, "repack.sh");

	ret = bootimg_gen_repack_script(&hdr, pathname);
	if (ret < 0)
	{
		pr_red_info("bootimg_gen_pack_script");
		goto out_close_fd;
	}

	ret = 0;
out_close_fd:
	close(fd);
	return ret;
}
