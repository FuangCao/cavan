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
#include <cavan/parser.h>
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

static void bootimg_copy_cmdline(struct bootimg_header *hdr, const char *cmdline)
{
	char *p, *p_end;

	for (p = (char *) hdr->cmdline, p_end = p + sizeof(hdr->cmdline) - 1; p < p_end && *cmdline; p++, cmdline++)
	{
		*p = *cmdline;
	}

	*p = 0;

	for (p = (char *) hdr->extra_cmdline, p_end = p + sizeof(hdr->extra_cmdline) - 1; p < p_end && *cmdline; p++, cmdline++)
	{
		*p = *cmdline;
	}

	*p = 0;
}

int bootimg_gen_repack_script(const struct bootimg_header *hdr, const char *pathname, bool dt_support)
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
	ret |= ffile_puts(fd, "CMD_MKBOOTIMG=\"" CMD_MKBOOTIMG "\"\n\n");
	ret |= ffile_printf(fd, "${CMD_MKBOOTIMG} --output " FILE_BOOTIMG_REPACK_NAME " --pagesize %d --base 0x%x --tags_offset 0x%x", hdr->page_size, base, hdr->tags_addr - base);

	if (hdr->kernel_size > 0)
	{
		ret |= ffile_printf(fd, " --kernel " FILE_KERNEL_NAME " --kernel_offset 0x%x", hdr->kernel_addr - base);
	}

	if (hdr->ramdisk_size > 0)
	{
		ret |= ffile_printf(fd, " --ramdisk " FILE_RAMDISK_NAME " --ramdisk_offset 0x%x", hdr->ramdisk_addr - base);
	}

	if (hdr->second_size > 0)
	{
		ret |= ffile_printf(fd, " --second " FILE_SECOND_NAME " --second_offset 0x%x", hdr->second_addr - base);
	}

	if (hdr->dt_size > 0 && dt_support)
	{
		ret |= ffile_printf(fd, " --dt " FILE_DT_NAME);
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

int bootimg_write_config_file(const struct bootimg_header *hdr, const char *pathname)
{
	int fd;
	int ret;

	println("write config file `%s'", pathname);

	fd = open(pathname, O_WRONLY | O_TRUNC | O_CREAT, 0777);
	if (fd < 0)
	{
		pr_error_info("open file %s", pathname);
		return fd;
	}

	ret = ffile_printf(fd, "kernel_addr: 0x%08x\n", hdr->kernel_addr);
	ret |= ffile_printf(fd, "ramdisk_addr: 0x%08x\n", hdr->ramdisk_addr);
	ret |= ffile_printf(fd, "second_addr: 0x%08x\n", hdr->second_addr);
	ret |= ffile_printf(fd, "tags_addr: 0x%08x\n", hdr->tags_addr);
	ret |= ffile_printf(fd, "page_size: %d\n", hdr->page_size);
	ret |= ffile_printf(fd, "unused: 0x%08x,0x%08x\n", hdr->unused[0], hdr->unused[1]);

	if (hdr->name[0])
	{
		ret |= ffile_printf(fd, "board: %s\n", hdr->name);
	}

	if (hdr->cmdline[0])
	{
		ret |= ffile_printf(fd, "cmdline: %s%s\n", hdr->cmdline, hdr->extra_cmdline);
	}

	close(fd);

	return ret;
}

static int bootimg_parse_config_handler(char *key, char *value, void *data)
{
	struct bootimg_header *hdr = data;

	if (strcmp(key, "kernel_addr") == 0)
	{
		hdr->kernel_addr = text2value_unsigned(value, NULL, 16);
	}
	else if (strcmp(key, "ramdisk_addr") == 0)
	{
		hdr->ramdisk_addr = text2value_unsigned(value, NULL, 16);
	}
	else if (strcmp(key, "second_addr") == 0)
	{
		hdr->second_addr = text2value_unsigned(value, NULL, 16);
	}
	else if (strcmp(key, "tags_addr") == 0)
	{
		hdr->tags_addr = text2value_unsigned(value, NULL, 16);
	}
	else if (strcmp(key, "page_size") == 0)
	{
		hdr->page_size = text2value_unsigned(value, NULL, 10);
	}
	else if (strcmp(key, "unused") == 0)
	{
		text2array(value, hdr->unused, 2, ',');
	}
	else if (strcmp(key, "board") == 0)
	{
		strncpy((char *) hdr->name, value, sizeof(hdr->name));
	}
	else if (strcmp(key, "cmdline") == 0)
	{
		bootimg_copy_cmdline(hdr, value);
	}
	else
	{
		pr_red_info("unknown key %s", key);
		return -EINVAL;
	}

	return 1;
}

int bootimg_parse_config_file(struct bootimg_header *hdr, const char *pathname)
{
	return parse_config_file2(pathname, ':', bootimg_parse_config_handler, hdr);
}

int bootimg_unpack(const char *input, const char *output, bool dt_support)
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

	filename = text_copy(pathname, output);
	ret = mkdir_hierarchy(pathname, 0777);
	if (ret < 0)
	{
		pr_red_info("mkdir_hierarchy");
		goto out_close_fd;
	}

	*filename++ = '/';

	count = 0;

	if (hdr.kernel_size > 0)
	{
		images[count].size = hdr.kernel_size;
		images[count++].name = FILE_KERNEL_NAME;
	}

	if (hdr.ramdisk_size > 0)
	{
		images[count].size = hdr.ramdisk_size;
		images[count++].name = FILE_RAMDISK_NAME;
	}

	if (hdr.second_size > 0)
	{
		images[count].size = hdr.second_size;
		images[count++].name = FILE_SECOND_NAME;
	}
	else
	{
		strcpy(filename, FILE_SECOND_NAME);
		unlink(filename);
	}

	if (hdr.dt_size > 0 && dt_support)
	{
		images[count].size = hdr.dt_size;
		images[count++].name = FILE_DT_NAME;
	}
	else
	{
		strcpy(filename, FILE_DT_NAME);
		unlink(filename);
	}

	images[count].size = 0;
	images[count++].name = FILE_REMAIN_NAME;

	if (count > 0)
	{
		struct bootimg_image *p, *p_end;

		for (p = images, p_end = p + count; p < p_end; p++)
		{
			strcpy(filename, p->name);
			println("%s -> %s", p->name, pathname);

			if (p->size > 0)
			{
				ret = cavan_file_seek_next_page(fd, hdr.page_size);
				if (ret < 0)
				{
					pr_red_info("cavan_file_seek_next_page");
					goto out_close_fd;
				}

				ret = file_ncopy2(fd, pathname, p->size, O_WRONLY | O_TRUNC | O_CREAT, 0777);
			}
			else
			{
				ret = file_copy2(fd, pathname, O_WRONLY | O_TRUNC | O_CREAT, 0777);
			}

			if (ret < 0)
			{
				pr_red_info("file_copy2");
				goto out_close_fd;
			}
		}
	}

	if (hdr.cmdline[0])
	{
		strcpy(filename, FILE_CMDLINE_TXT);
		println("cmdline -> %s", pathname);

		if (hdr.extra_cmdline[0])
		{
			char *p;
			char buff[sizeof(hdr.cmdline) + sizeof(hdr.extra_cmdline)];

			p = text_copy(buff, (char *) hdr.cmdline);
			p = text_copy(p, (char *) hdr.extra_cmdline);

			ret = file_writeto(pathname, buff, p - buff, 0, O_TRUNC);
			if (ret < 0)
			{
				pr_red_info("file_writeto");
				goto out_close_fd;
			}
		}
		else
		{
			ret = file_writeto(pathname, hdr.cmdline, strlen((char *) hdr.cmdline), 0, O_TRUNC);
			if (ret < 0)
			{
				pr_red_info("file_writeto");
				goto out_close_fd;
			}
		}
	}

	strcpy(filename, FILE_REPACK_SH);

	ret = bootimg_gen_repack_script(&hdr, pathname, dt_support);
	if (ret < 0)
	{
		pr_red_info("bootimg_gen_pack_script");
		goto out_close_fd;
	}

	strcpy(filename, FILE_CONFIG_TXT);

	ret = bootimg_write_config_file(&hdr, pathname);
	if (ret < 0)
	{
		pr_red_info("bootimg_write_config_file");
		goto out_close_fd;
	}

	ret = 0;
out_close_fd:
	close(fd);
	return ret;
}

static int bootimg_write_image(int fd, const char *pathname, unsigned *size, unsigned page_size, struct cavan_sha_context *context)
{
	int ret;
	int img_fd;
	ssize_t wrlen;

	ret = cavan_file_seek_next_page(fd, page_size);
	if (ret < 0)
	{
		pr_red_info("cavan_file_seek_next_page");
		return ret;
	}

	img_fd = open(pathname, O_RDONLY);
	if (img_fd < 0)
	{
		pr_error_info("open file `%s'", pathname);
		return img_fd;
	}

	wrlen = ffile_copy(img_fd, fd);
	if (wrlen < 0)
	{
		ret = wrlen;
		pr_red_info("ffile_copy");
		goto out_close_img_fd;
	}

	if (lseek(img_fd, 0, SEEK_SET) != 0)
	{
		pr_error_info("lseek");

		ret = -EFAULT;
		goto out_close_img_fd;
	}

	ret = cavan_sha_update2(context, img_fd);
	if (ret < 0)
	{
		pr_red_info("cavan_sha_update2");
		goto out_close_img_fd;
	}

	*size = wrlen;

out_close_img_fd:
	close(img_fd);
	return ret;
}

int bootimg_pack(struct bootimg_pack_option *option)
{
	int fd;
	int ret;
	int image_count;
	struct bootimg_header hdr;
	struct bootimg_image images[4];
	struct bootimg_image *p, *p_end;
	struct cavan_sha_context context;

	if (option->kernel == NULL || option->ramdisk == NULL)
	{
        pr_red_info("no kernel or ramdisk image specified");
		return -EINVAL;
	}

	fd = open(option->output, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd < 0)
	{
		pr_error_info("open file `%s' failed", option->output);
		return fd;
	}

	ret = lseek(fd, sizeof(hdr), SEEK_SET);
	if (ret < 0)
	{
		pr_red_info("cavan_file_seek_page_align");
		goto out_close_fd;
	}

	cavan_sha1_context_init(&context);

	ret = cavan_sha_init(&context);
	if (ret < 0)
	{
		pr_red_info("cavan_sha_init");
		goto out_close_fd;
	}

	memset(&hdr, 0, sizeof(hdr));
	memcpy(hdr.magic, BOOT_MAGIC, sizeof(hdr.magic));

	if (option->config)
	{
		ret = bootimg_parse_config_file(&hdr, option->config);
		if (ret < 0)
		{
			pr_red_info("bootimg_parse_config_file");
			goto out_close_fd;
		}
	}
	else
	{
		memcpy(hdr.unused, option->unused, sizeof(hdr.unused));

		hdr.page_size = option->page_size;
		hdr.kernel_addr = option->kernel_addr ?: option->base + option->kernel_offset;
		hdr.ramdisk_addr = option->ramdisk_addr ?: option->base + option->ramdisk_offset;
		hdr.second_addr = option->second_addr ?: option->base + option->second_offset;
		hdr.tags_addr = option->tags_addr ?: option->base + option->tags_offset;

		if (option->name)
		{
			strncpy((char *) hdr.name, option->name, sizeof(hdr.name));
		}

		if (option->cmdline)
		{
			bootimg_copy_cmdline(&hdr, option->cmdline);
		}
	}

	images[0].name = option->kernel;
	images[0].size_addr = &hdr.kernel_size;

	images[1].name = option->ramdisk;
	images[1].size_addr = &hdr.ramdisk_size;

	images[2].name = option->second;
	images[2].size_addr = &hdr.second_size;

	image_count = 3;

	if (option->dt)
	{
		images[image_count].name = option->dt;
		images[image_count++].size_addr = &hdr.dt_size;
	}

	for (p = images, p_end = p + image_count; p < p_end; p++)
	{
		if (p->name)
		{
			println("write image %s", p->name);

			ret = bootimg_write_image(fd, p->name, p->size_addr, hdr.page_size, &context);
			if (ret < 0)
			{
				pr_red_info("bootimg_write_image");
				goto out_close_fd;
			}
		}
		else
		{
			*p->size_addr = 0;
		}

		cavan_sha_update(&context, p->size_addr, sizeof(unsigned));
	}

	if (option->remain)
	{
		println("write remain image %s", option->remain);

		ret = file_copy3(option->remain, fd);
		if (ret < 0)
		{
			pr_red_info("file_copy3");
			goto out_close_fd;
		}
	}

	if (option->check_all)
	{
		cavan_sha_update(&context, &hdr.tags_addr, sizeof(hdr.tags_addr));
		cavan_sha_update(&context, &hdr.page_size, sizeof(hdr.page_size));
		cavan_sha_update(&context, hdr.unused, sizeof(hdr.unused));
		cavan_sha_update(&context, hdr.name, sizeof(hdr.name));
		cavan_sha_update(&context, hdr.cmdline, sizeof(hdr.cmdline));

		if (hdr.extra_cmdline[0])
		{
			cavan_sha_update(&context, hdr.extra_cmdline, sizeof(hdr.extra_cmdline));
		}
	}

	cavan_sha_finish(&context, (u8 *) hdr.id);

	bootimg_header_dump(&hdr);

	ret = ffile_writeto(fd, &hdr, sizeof(hdr), 0);
	if (ret < 0)
	{
		pr_red_info("ffile_writeto");
		goto out_close_fd;
	}

	pr_green_info("pack %s successfull", option->output);

out_close_fd:
	close(fd);
	return ret;
}
