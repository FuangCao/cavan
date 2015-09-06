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
	println("--dt, -d\t\thas dt.img");
}

static int cavan_bootimg_unpack(int argc, char *argv[])
{
	int c;
	int option_index;
	bool dt_support = false;
	const char *command = argv[0];
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
			.name = "dt",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_DT,
		},
		{
			0, 0, 0, 0
		},
	};

	while ((c = getopt_long(argc, argv, "vVhHd", long_option, &option_index)) != EOF)
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
			show_usage_unpack(command);
			return 0;

		case 'd':
		case CAVAN_COMMAND_OPTION_DT:
			dt_support = true;
			break;

		default:
			show_usage_unpack(command);
			return -EINVAL;
		}
	}

	argv += optind;
	argc -= optind;

	if (argc < 1)
	{
		show_usage_unpack(command);
		return -EINVAL;
	}

	return bootimg_unpack(argv[0], argc > 1 ? argv[1] : ".", dt_support);
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
	println("--help, -h, -H\t\t\t\t\t%s", cavan_help_message_help);
	println("--version, -v, -V\t\t\t\t%s", cavan_help_message_version);
	println("--name, --board, -n <boardname>");
	println("--cmdline, -c <kernel-cmdline>");
	println("--kernel, -k <filename>");
	println("--ramdisk, -r <filename>");
	println("--second, -s <filename>");
	println("--dt, -d <filename>");
	println("--remain <filename>");
	println("--config <filename>");
	println("--unused, u <value,value>");
	println("--page_size, --pagesize, --ps, -p <size>");
	println("--check_all, --check-all, --ca, -a\t\tcheck header full sha1sum");
	println("--base, -b <address>\t\t\t\tbase load address");
	println("--kernel_offset, --ko <address>\t\t\toffset of base address");
	println("--ramdisk_offset, --ro <address>\t\toffset of base address");
	println("--second_offset, --so <address>\t\t\toffset of base address");
	println("--tags_offset, --to <address>\t\t\toffset of base address");
	println("--kernel_addr, --ka <address>\t\t\tphysical load addr");
	println("--ramdisk_addr, --ra <address>\t\t\tphysical load addr");
	println("--second_addr, --sa <address>\t\t\tphysical load addr");
	println("--tags_addr, --ta <address>\t\t\tphysical load addr");
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
			.name = "kernel_addr",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_KERNEL_ADDR,
		},
		{
			.name = "ka",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_KERNEL_ADDR,
		},
		{
			.name = "ramdisk_addr",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_RAMDISK_ADDR,
		},
		{
			.name = "ra",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_RAMDISK_ADDR,
		},
		{
			.name = "second_addr",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_SECOND_ADDR,
		},
		{
			.name = "sa",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_SECOND_ADDR,
		},
		{
			.name = "tags_addr",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_TAGS_ADDR,
		},
		{
			.name = "ta",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_TAGS_ADDR,
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
			.name = "remain",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_REMAIN,
		},
		{
			.name = "config",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_CONFIG,
		},
		{
			.name = "check_all",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_CHECK_ALL,
		},
		{
			.name = "check-all",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_CHECK_ALL,
		},
		{
			.name = "ca",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_CHECK_ALL,
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
		.config = NULL,
		.page_size = BOOTIMG_DEFAULT_PAGE_SIZE,
		.base = BOOTIMG_DEFAULT_BASE,
		.kernel_offset = BOOTIMG_DEFAULT_KERNEL_OFFSET,
		.ramdisk_offset = BOOTIMG_DEFAULT_RAMDISK_OFFSET,
		.second_offset = BOOTIMG_DEFAULT_SECOND_OFFSET,
		.tags_offset = BOOTIMG_DEFAULT_TAGS_OFFSET,
		.kernel_addr = 0,
		.ramdisk_addr = 0,
		.second_addr = 0,
		.tags_addr = 0,
		.unused = { 0, 0 },
		.check_all = false
	};

	while ((c = getopt_long(argc, argv, "vVhHn:c:k:r:s:d:u:p:b:a", long_option, &option_index)) != EOF)
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

		case CAVAN_COMMAND_OPTION_REMAIN:
			option.remain = optarg;
			break;

		case CAVAN_COMMAND_OPTION_CONFIG:
			option.config = optarg;
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

		case 'a':
		case CAVAN_COMMAND_OPTION_CHECK_ALL:
			option.check_all = true;
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

		case CAVAN_COMMAND_OPTION_KERNEL_ADDR:
			option.kernel_addr = text2value_unsigned(optarg, NULL, 10);
			break;

		case CAVAN_COMMAND_OPTION_RAMDISK_ADDR:
			option.ramdisk_addr = text2value_unsigned(optarg, NULL, 10);
			break;

		case CAVAN_COMMAND_OPTION_SECOND_ADDR:
			option.second_addr = text2value_unsigned(optarg, NULL, 10);
			break;

		case CAVAN_COMMAND_OPTION_TAGS_ADDR:
			option.tags_addr = text2value_unsigned(optarg, NULL, 10);
			break;

		default:
			show_usage_pack(argv[0]);
			return -EINVAL;
		}
	}

	if (option.kernel == NULL && option.ramdisk == NULL)
	{
		if (file_access_e(FILE_KERNEL_NAME))
		{
			option.kernel = FILE_KERNEL_NAME;
		}

		if (file_access_e(FILE_RAMDISK_NAME))
		{
			option.ramdisk = FILE_RAMDISK_NAME;
		}

		if (option.second == NULL && file_access_e(FILE_SECOND_NAME))
		{
			option.second = FILE_SECOND_NAME;
		}

		if (option.dt == NULL && file_access_e(FILE_DT_NAME))
		{
			option.dt = FILE_DT_NAME;
		}

		if (option.remain == NULL && file_access_e(FILE_REMAIN_NAME))
		{
			option.remain = FILE_REMAIN_NAME;
		}

		if (option.config == NULL && file_access_e(FILE_CONFIG_TXT))
		{
			option.config = FILE_CONFIG_TXT;
		}
	}

	if (optind < argc)
	{
		option.output = argv[optind++];
	}
	else
	{
		option.output = FILE_BOOTIMG_NAME;
	}

	return bootimg_pack(&option);
}

// ============================================================

static int cavan_bootimg_info(int argc, char *argv[])
{
	int ret;
	const char *pathname;
	struct bootimg_header hdr;

	if (argc > 1)
	{
		pathname = argv[1];
	}
	else
	{
		pathname = "boot.img";
	}

	ret = file_read(pathname, &hdr, sizeof(hdr));
	if (ret < 0)
	{
		pr_error_info("read file %s", pathname);
		return ret;
	}

	bootimg_header_dump(&hdr);

	return 0;
}

// ============================================================

static struct cavan_command_map cmd_map[] =
{
	{"unpack", cavan_bootimg_unpack},
	{"repack", cavan_bootimg_repack},
	{"info", cavan_bootimg_info},
	{"pack", cavan_bootimg_pack}
};

FIND_EXEC_COMMAND_MAIN(cmd_map);
