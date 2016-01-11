/*
 * File:		mkfs.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2016-01-11 11:42:13
 *
 * Copyright (c) 2016 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/device.h>
#include <cavan/command.h>
#include <android/filesystem.h>

#define FILE_CREATE_DATE "2016-01-11 11:42:13"

static void show_usage(const char *command)
{
	println("Usage: %s <volume> [option]", command);
	println("-h, -H, --help\t\tshow this help");
	println("-v, -V, --version\tshow version");
	println("-t, --type <FS>\t\tfilesystem type");
	println("--fstab <PATH>\t\tfstab path");
}

int main(int argc, char *argv[])
{
	int c;
	int ret;
	int option_index;
	struct option long_option[] = {
		{
			.name = "help",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_HELP,
		}, {
			.name = "version",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_VERSION,
		}, {
			.name = "type",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_TYPE,
		}, {
			.name = "fstab",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_FSTAB,
		}, {
			0, 0, 0, 0
		},
	};
	const char *volume;
	const char *fstab = NULL;
	const char *fs_type = NULL;

	while ((c = getopt_long(argc, argv, "vVhHt:", long_option, &option_index)) != EOF) {
		switch (c) {
		case 'v':
		case 'V':
		case CAVAN_COMMAND_OPTION_VERSION:
			show_author_info();
			println(FILE_CREATE_DATE);
			return 0;

		case 'h':
		case 'H':
		case CAVAN_COMMAND_OPTION_HELP:
			show_usage(argv[0]);
			return 0;

		case 't':
		case CAVAN_COMMAND_OPTION_TYPE:
			fs_type = optarg;
			break;

		case CAVAN_COMMAND_OPTION_FSTAB:
			fstab = optarg;
			break;

		default:
			show_usage(argv[0]);
			return -EINVAL;
		}
	}

	android_stop_all();

	while (optind < argc) {
		if (!fs_volume_format2(argv[optind++], fs_type, fstab, true)) {
			pr_red_info("fs_volume_format2");
			return -EFAULT;
		}

		pr_green_info("OK");
	}

	return 0;
}
