#pragma once

/*
 * File:		bootimg.h
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

#define BOOT_MAGIC "ANDROID!"
#define BOOT_MAGIC_SIZE 8
#define BOOT_NAME_SIZE 16
#define BOOT_ARGS_SIZE 512
#define BOOT_EXTRA_ARGS_SIZE 1024

#define BOOTIMG_DEFAULT_PAGE_SIZE		2048
#define BOOTIMG_DEFAULT_BASE			0x10000000
#define BOOTIMG_DEFAULT_KERNEL_OFFSET	0x00008000
#define BOOTIMG_DEFAULT_RAMDISK_OFFSET	0x01000000
#define BOOTIMG_DEFAULT_SECOND_OFFSET	0x00f00000
#define BOOTIMG_DEFAULT_TAGS_OFFSET		0x00000100

struct bootimg_header
{
    unsigned char magic[BOOT_MAGIC_SIZE];

    unsigned kernel_size;  /* size in bytes */
    unsigned kernel_addr;  /* physical load addr */

    unsigned ramdisk_size; /* size in bytes */
    unsigned ramdisk_addr; /* physical load addr */

    unsigned second_size;  /* size in bytes */
    unsigned second_addr;  /* physical load addr */

    unsigned tags_addr;    /* physical addr for kernel tags */
    unsigned page_size;    /* flash page size we assume */

    union
    {
        unsigned dt_size;	   /* device tree in bytes */
        unsigned unused[2];       /* future expansion: should be 0 */
    };

    unsigned char name[BOOT_NAME_SIZE]; /* asciiz product name */

    unsigned char cmdline[BOOT_ARGS_SIZE];

    unsigned id[8]; /* timestamp / checksum / sha1 / etc */

    /* Supplemental command line data; kept here to maintain
     * binary compatibility with older versions of mkbootimg */
    unsigned char extra_cmdline[BOOT_EXTRA_ARGS_SIZE];
};

struct bootimg_image
{
	size_t size;
	const char *name;
	unsigned *size_addr;
};

struct bootimg_pack_option
{
	const char *kernel;
	const char *ramdisk;
	const char *second;
	const char *dt;
	const char *cmdline;
	const char *name;
	const char *output;

	u32 page_size;

	u32 base;
	u32 kernel_offset;
	u32 ramdisk_offset;
	u32 second_offset;
	u32 tags_offset;
	u32 unused[2];
};

void bootimg_header_dump(struct bootimg_header *hdr);
int bootimg_unpack(const char *input, const char *output);
int bootimg_gen_repack_script(const struct bootimg_header *hdr, const char *pathname);
int bootimg_pack(struct bootimg_pack_option *option);

static inline ssize_t bootimg_read_header(int fd, struct bootimg_header *hdr)
{
	return read(fd, hdr, sizeof(struct bootimg_header));
}
