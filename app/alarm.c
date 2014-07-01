/*
 * File:		alarm.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2013-06-09 14:27:41
 *
 * Copyright (c) 2013 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <time.h>
#include <cavan/adb.h>
#include <cavan/alarm.h>
#include <cavan/tcp_dd.h>
#include <cavan/network.h>
#include <cavan/command.h>

#define FILE_CREATE_DATE "2013-06-09 14:27:41"

static void show_usage(const char *command)
{
	println("Usage: %s [add|remove|list] <option> [command|index]", command);
	println("--help, -h, -H\t\tshow this help");
	println("--version, -v, -V\tshow version");
	println("--ip, -i, -I\t\tserver ip address");
	println("--local, -l, -L\t\tuse localhost ip");
	println("--port, -p, -P\t\tserver port");
	println("--adb, -a, -A\t\tuse adb procotol instead of tcp");
	println("--date, -d, -D");
	println("--time, -t, -T");
	println("--repeat, -r, -R");
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
			.name = "date",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 'd',
		},
		{
			.name = "time",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 't',
		},
		{
			.name = "repeat",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 'r',
		},
		{
			.name = "ip",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 'i',
		},
		{
			.name = "port",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 'p',
		},
		{
			.name = "adb",
			.has_arg = no_argument,
			.flag = NULL,
			.val = 'a',
		},
		{
			.name = "local",
			.has_arg = no_argument,
			.flag = NULL,
			.val = 'l',
		},
		{
			0, 0, 0, 0
		},
	};
	int ret;
	time_t curr_time, repeat;
	struct tm date;
	const char *subcmd;
	struct inet_file_request file_req =
	{
		.open_connect = inet_create_tcp_link2,
		.close_connect = inet_close_tcp_socket,
	};

	curr_time = time(NULL);
	if (curr_time == ((time_t)-1))
	{
		pr_error_info("time");
		return -EFAULT;
	}

	repeat = 0;
	localtime_r(&curr_time, &date);

	cavan_get_server_ip(file_req.ip);
	file_req.port = cavan_get_server_port(TCP_DD_DEFAULT_PORT);

	while ((c = getopt_long(argc, argv, "vVhHlLaAr:R:d:D:t:T:i:I:p:P:", long_option, &option_index)) != EOF)
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
			show_usage(argv[0]);
			return 0;

		case 'd':
		case 'D':
			ret = text2date(optarg, &date, "%Y-%m-%d", "%m-%d", NULL);
			if (ret < 0)
			{
				pr_red_info("cavan_text2date");
				return -EINVAL;
			}
			break;

		case 't':
		case 'T':
			date.tm_sec = 0;
			ret = text2date(optarg, &date, "%H:%M:%S", "%H:%M", NULL);
			if (ret < 0)
			{
				pr_red_info("cavan_text2date");
				return -EINVAL;
			}
			break;

		case 'r':
		case 'R':
			repeat = text2time(optarg, NULL);
			break;

		case 'a':
		case 'A':
			file_req.open_connect = adb_create_tcp_link2;
		case 'l':
		case 'L':
			optarg = "127.0.0.1";
		case 'i':
		case 'I':
			text_copy(file_req.ip, optarg);
			break;

		case 'p':
		case 'P':
			file_req.port = text2value_unsigned(optarg, NULL, 10);
			break;

		default:
			show_usage(argv[0]);
			return -EINVAL;
		}
	}

	if (optind >= argc)
	{
		pr_red_info("Please give a subcmd");
		show_usage(argv[0]);
		return -EINVAL;
	}

	subcmd = argv[optind++];

	if (strcmp(subcmd, "add") == 0)
	{
		if (optind < argc)
		{
			text_join_by_char(argv + optind, argc - optind, ' ', file_req.command, sizeof(file_req.command));
			ret = tcp_alarm_add(&file_req, mktime(&date), repeat);
			if (ret < 0)
			{
				pr_red_info("cavan_tcp_alarm_add");
			}
		}
		else
		{
			pr_red_info("Please give a command");
			show_usage(argv[0]);
			return -EINVAL;
		}
	}
	else if (strcmp(subcmd, "remove") == 0)
	{
		if (optind < argc && text_is_number(argv[optind]))
		{
			int index = text2value_unsigned(argv[optind], NULL, 10);

			ret = tcp_alarm_remove(&file_req, index);
			if (ret < 0)
			{
				pr_red_info("cavan_tcp_alarm_remove");
			}
		}
		else
		{
			pr_red_info("Please give a command");
			show_usage(argv[0]);
			return -EINVAL;
		}
	}
	else if (strcmp(subcmd, "list") == 0)
	{
		int index = -1;

		if (optind < argc)
		{
			if (text_is_number(argv[optind]))
			{
				index = text2value_unsigned(argv[optind], NULL, 10);
			}
			else
			{
				pr_red_info("The alarm index is not a number");
				show_usage(argv[0]);
				return -EINVAL;
			}
		}

		ret = tcp_alarm_list(&file_req, index);
		if (ret < 0)
		{
			pr_red_info("cavan_tcp_alarm_list");
		}
	}
	else
	{
		pr_red_info("Invalid subcmd `%s'", subcmd);
		show_usage(argv[0]);
		return -EINVAL;
	}

	return ret;
}
