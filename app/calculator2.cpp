/*
 * File:		calculator2.cpp
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-07-16 18:57:37
 *
 * Copyright (c) 2015 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/calculator.h>
#include <cavan++/Math.h>

static void show_usage(const char *command)
{
	println("Usage:");
	println("%s [option] port", command);
	println("--help, -h, -H\t\tshow this help");
	println("--version, -v, -V\tshow command version");
	println("--base, -b, -B\t\tset out radix");
	println("--length, -l, -L\tset value bit count");
	println("--prefix, -p, -P\tshow value prefix");
	println("--mask, -m, -M\t\tshow bit location");
}

int main(int argc, char *argv[])
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
			.name = "base",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_BASE,
		},
		{
			.name = "length",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_LENGTH,
		},
		{
			.name = "prefix",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_PREFIX,
		},
		{
			.name = "mask",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_MASK,
		},
		{
			0, 0, 0, 0
		},
	};
	int ret;
	char buff[1024];
	int base = 0;
	int length[2];
	int flags = 0;
	bool show_bitmask = false;

	length[0] = length[1] = 0;

	while ((c = getopt_long(argc, argv, "vVhHb:B:l:L:pPmM", long_option, &option_index)) != EOF)
	{
		switch (c)
		{
		case 'v':
		case 'V':
		case CAVAN_COMMAND_OPTION_VERSION:
			show_author_info();
			return 0;

		case 'h':
		case 'H':
		case CAVAN_COMMAND_OPTION_HELP:
			show_usage(argv[0]);
			return 0;

		case 'b':
		case 'B':
		case CAVAN_COMMAND_OPTION_BASE:
			base = text2value_unsigned(optarg, NULL, 10);
			break;

		case 'p':
		case 'P':
		case CAVAN_COMMAND_OPTION_PREFIX:
			flags |= TEXT_FLAG_PREFIX;
			break;

		case 'm':
		case 'M':
		case CAVAN_COMMAND_OPTION_MASK:
			show_bitmask = true;
			break;

		default:
			show_usage(argv[0]);
			return -EINVAL;
		}
	}

	assert(argc > optind);

	text_cat2(buff, argv + optind, argc - optind);

	double result;
	Calculator calculator;
	if (!calculator.execute(buff, result))
	{
		pr_red_info("%s", calculator.getErrMsg());
		return ret;
	}

	if (base < 2 || base == 10)
	{
		if (length[0] || length[1])
		{
			char format[64];

			sprintf(format, "%%0%d.%dlf", length[0], length[1]);
			println(format, result);
		}
		else
		{
			println("%lf", result);
		}
	}
	else
	{
		double2text(&result, buff, length[0], 0, base | flags);
		println("%s", buff);
	}

	if (show_bitmask)
	{
		value2bitlist((u64) result, buff, sizeof(buff), " | ");

		println("%s", buff);
	}

	return 0;
}
