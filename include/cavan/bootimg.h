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

#define CMD_MKBOOTIMG					"mkbootimg"
#define FILE_BOOTIMG_NAME				"boot.img"
#define FILE_BOOTIMG_REPACK_NAME		"boot-repack.img"
#define FILE_KERNEL_NAME				"kernel.bin"
#define FILE_RAMDISK_NAME				"ramdisk.img"
#define FILE_SECOND_NAME				"second.bin"
#define FILE_DT_NAME					"dt.img"
#define FILE_REMAIN_NAME				"remain.bin"
#define FILE_BOARD_NAME					"board.txt"
#define FILE_CMDLINE_NAME				"cmdline.txt"
#define FILE_REPACK_SH					"repack.sh"

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
	unsigned size;
	const char *name;
	unsigned *size_addr;
};

struct bootimg_pack_option
{
	const char *kernel;
	const char *ramdisk;
	const char *second;
	const char *dt;
	const char *remain;
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
	bool check_all;
};

void bootimg_header_dump(struct bootimg_header *hdr);
int bootimg_unpack(const char *input, const char *output, bool dt_support);
int bootimg_gen_repack_script(const struct bootimg_header *hdr, const char *pathname, bool dt_support);
int bootimg_pack(struct bootimg_pack_option *option);

static inline ssize_t bootimg_read_header(int fd, struct bootimg_header *hdr)
{
	return read(fd, hdr, sizeof(struct bootimg_header));
}
