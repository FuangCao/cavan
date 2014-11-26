/*
 * File:		bootimg.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2014-11-24 13:35:23
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
#include <cavan/command.h>
#include <cavan/bootimg.h>

#define FILE_CREATE_DATE "2014-11-24 13:35:23"

static void show_usage_unpack(const char *command)
{
	println("Usage: %s [option] boot.img [output]", command);
	println("--help, -h, -H\t\t%s", cavan_help_message_help);
	println("--version, -v, -V\t%s", cavan_help_message_version);
}

static int cavan_bootimg_unpack(int argc, char *argv[])
{
	int c;
	int option_index;
	struct option long_option[] =
	{
		{
			.name = "help",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_HELP,
		},
		{
			.name = "version",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_VERSION,
		},
		{
			0, 0, 0, 0
		},
	};

	while ((c = getopt_long(argc, argv, "vVhH", long_option, &option_index)) != EOF)
	{
		switch (c)
		{
		case 'v':
		case 'V':
		case CAVAN_COMMAND_OPTION_VERSION:
			show_author_info();
			println(FILE_CREATE_DATE);
			return 0;

		case 'h':
		case 'H':
		case CAVAN_COMMAND_OPTION_HELP:
			show_usage_unpack(argv[0]);
			return 0;

		default:
			show_usage_unpack(argv[0]);
			return -EINVAL;
		}
	}

	argv += optind;
	argv -= optind;

	if (argc < 2)
	{
		show_usage_unpack(argv[0]);
		return -EINVAL;
	}

	return bootimg_unpack(argv[1], argc > 2 ? argv[2] : ".");
}

// ============================================================

static int cavan_bootimg_repack(int argc, char *argv[])
{
	pr_pos_info();

	return 0;
}

// ============================================================

static void show_usage_pack(const char *command)
{
	println("Usage: %s [option] [input] [output]", command);
	println("--help, -h, -H\t\t\t\t%s", cavan_help_message_help);
	println("--version, -v, -V\t\t\t%s", cavan_help_message_version);
	println("--name, --board, -n <boardname>");
	println("--cmdline, -c <kernel-cmdline>");
	println("--kernel, -k <filename>");
	println("--ramdisk, -r <filename>");
	println("--second, -s <filename>");
	println("--dt, -d <filename>");
	println("--unused, u <value,value>");
	println("--page_size, --pagesize, --ps, -p <size>");
	println("--base, -b <address>\t\t\tbase load address");
	println("--kernel_offset, --ko <address>\t\toffset address of --base");
	println("--ramdisk_offset, --ro <address>\toffset address of --base");
	println("--second_offset, --so <address>\t\toffset address of --base");
	println("--tags_offset, --to <address>\t\toffset address of --base");
}

static int cavan_bootimg_pack(int argc, char *argv[])
{
	int c;
	int option_index;
	struct option long_option[] =
	{
		{
			.name = "help",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_HELP,
		},
		{
			.name = "version",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_VERSION,
		},
		{
			.name = "kernel",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_KERNEL,
		},
		{
			.name = "kernel_offset",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_KERNEL_OFFSET,
		},
		{
			.name = "ko",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_KERNEL_OFFSET,
		},
		{
			.name = "ramdisk",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_RAMDISK,
		},
		{
			.name = "ramdisk_offset",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_RAMDISK_OFFSET,
		},
		{
			.name = "ro",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_RAMDISK_OFFSET,
		},
		{
			.name = "second",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_SECOND,
		},
		{
			.name = "second_offset",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_SECOND_OFFSET,
		},
		{
			.name = "so",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_SECOND_OFFSET,
		},
		{
			.name = "tags_offset",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_TAGS_OFFSET,
		},
		{
			.name = "to",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_TAGS_OFFSET,
		},
		{
			.name = "dt",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_DT,
		},
		{
			.name = "base",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_BASE,
		},
		{
			.name = "cmdline",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_CMDLINE,
		},
		{
			.name = "name",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_NAME,
		},
		{
			.name = "board",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_BOARD,
		},
		{
			.name = "page_size",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_PAGE_SIZE,
		},
		{
			.name = "pagesize",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_PAGE_SIZE,
		},
		{
			.name = "ps",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_PAGE_SIZE,
		},
		{
			.name = "unused",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_UNUSED,
		},
		{
			0, 0, 0, 0
		},
	};
	struct bootimg_pack_option option =
	{
		.kernel = NULL,
		.ramdisk = NULL,
		.second = NULL,
		.dt = NULL,
		.cmdline = NULL,
		.name = NULL,
		.page_size = BOOTIMG_DEFAULT_PAGE_SIZE,
		.base = BOOTIMG_DEFAULT_BASE,
		.kernel_offset = BOOTIMG_DEFAULT_KERNEL_OFFSET,
		.ramdisk_offset = BOOTIMG_DEFAULT_RAMDISK_OFFSET,
		.second_offset = BOOTIMG_DEFAULT_SECOND_OFFSET,
		.tags_offset = BOOTIMG_DEFAULT_TAGS_OFFSET,
		.unused = {0, 0}
	};

	while ((c = getopt_long(argc, argv, "vVhHn:c:k:r:s:d:u:p:b:", long_option, &option_index)) != EOF)
	{
		switch (c)
		{
		case 'v':
		case 'V':
		case CAVAN_COMMAND_OPTION_VERSION:
			show_author_info();
			println(FILE_CREATE_DATE);
			return 0;

		case 'h':
		case 'H':
		case CAVAN_COMMAND_OPTION_HELP:
			show_usage_pack(argv[0]);
			return 0;

		case 'n':
		case CAVAN_COMMAND_OPTION_NAME:
		case CAVAN_COMMAND_OPTION_BOARD:
			option.name = optarg;
			break;

		case 'c':
		case CAVAN_COMMAND_OPTION_CMDLINE:
			option.cmdline = optarg;
			break;

		case 'k':
		case CAVAN_COMMAND_OPTION_KERNEL:
			option.kernel = optarg;
			break;

		case 'r':
		case CAVAN_COMMAND_OPTION_RAMDISK:
			option.ramdisk = optarg;
			break;

		case 's':
		case CAVAN_COMMAND_OPTION_SECOND:
			option.second = optarg;
			break;

		case 'd':
		case CAVAN_COMMAND_OPTION_DT:
			option.dt = optarg;
			break;

		case 'u':
		case CAVAN_COMMAND_OPTION_UNUSED:
			text2array(optarg, option.unused, NELEM(option.unused), ',');
			break;

		case 'p':
		case CAVAN_COMMAND_OPTION_PAGE_SIZE:
			option.page_size = text2value_unsigned(optarg, NULL, 10);
			break;

		case 'b':
		case CAVAN_COMMAND_OPTION_BASE:
			option.base = text2value_unsigned(optarg, NULL, 10);
			break;

		case CAVAN_COMMAND_OPTION_KERNEL_OFFSET:
			option.kernel_offset = text2value_unsigned(optarg, NULL, 10);
			break;

		case CAVAN_COMMAND_OPTION_RAMDISK_OFFSET:
			option.ramdisk_offset = text2value_unsigned(optarg, NULL, 10);
			break;

		case CAVAN_COMMAND_OPTION_SECOND_OFFSET:
			option.second_offset = text2value_unsigned(optarg, NULL, 10);
			break;

		case CAVAN_COMMAND_OPTION_TAGS_OFFSET:
			option.tags_offset = text2value_unsigned(optarg, NULL, 10);
			break;

		default:
			show_usage_pack(argv[0]);
			return -EINVAL;
		}
	}

	println("unused = 0x%08x 0x%08x", option.unused[0], option.unused[1]);

	if (option.kernel == NULL && option.ramdisk == NULL)
	{
		if (file_access_e("kernel.bin"))
		{
			option.kernel = "kernel.bin";
		}

		if (file_access_e("ramdisk.img"))
		{
			option.ramdisk = "ramdisk.img";
		}

		if (file_access_e("second.bin"))
		{
			option.second = "second.bin";
		}

		if (file_access_e("dt.bin"))
		{
			option.dt = "dt.bin";
		}
	}

	if (optind < argc)
	{
		option.output = argv[optind++];
	}
	else
	{
		option.output = "boot.img";
	}

	return bootimg_pack(&option);
}

// ============================================================

static struct cavan_command_map cmd_map[] =
{
	{"unpack", cavan_bootimg_unpack},
	{"repack", cavan_bootimg_repack},
	{"pack", cavan_bootimg_pack}
};

FIND_EXEC_COMMAND_MAIN(cmd_map);
