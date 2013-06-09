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
#include <cavan/alarm.h>

#define FILE_CREATE_DATE "2013-06-09 14:27:41"

enum
{
	LOCAL_COMMAND_OPTION_UNKNOWN,
	LOCAL_COMMAND_OPTION_HELP,
	LOCAL_COMMAND_OPTION_VERSION,
	LOCAL_COMMAND_OPTION_DAEMON,
	LOCAL_COMMAND_OPTION_YEAR,
	LOCAL_COMMAND_OPTION_MON,
	LOCAL_COMMAND_OPTION_DAY,
	LOCAL_COMMAND_OPTION_HOUR,
	LOCAL_COMMAND_OPTION_MIN,
	LOCAL_COMMAND_OPTION_SEC,
	LOCAL_COMMAND_OPTION_DATE,
	LOCAL_COMMAND_OPTION_YEAR_REPEAT,
	LOCAL_COMMAND_OPTION_WEEK_REPEAT,
	LOCAL_COMMAND_OPTION_DAY_REPEAT,
	LOCAL_COMMAND_OPTION_HOUR_REPEAT,
	LOCAL_COMMAND_OPTION_MIN_REPEAT,
	LOCAL_COMMAND_OPTION_SEC_REPEAT,
};

static void show_usage(const char *command)
{
	println("Usage: %s [option] command", command);
	println("--help, -H\t\tshow this help");
	println("--version, -v, -V\tshow version");
	println("--daemon, -D\t\trun as a daemon");
	println("--year, -y\t\tdate of year");
	println("--mon, -M\t\tdate of month");
	println("--day, -d\t\tdate of day");
	println("--hour, -h\t\ttime of hoer");
	println("--min, -m\t\ttime of minutes");
	println("--sec, -s\t\ttime of second");
	println("--date");
	println("--yr, --year-repeat");
	println("--wr, --week-repeat");
	println("--dr, --day-repeat");
	println("--hr, --hour-repeat");
	println("--mr, --min-repeat");
	println("--sr, --sec-repeat");
}

static void cavan_alarm_handler(struct cavan_alarm_node *alarm, struct cavan_alarm_thread *thread, void *data)
{
	pid_t pid;

	pid = fork();
	if (pid == 0)
	{
		const char *shell_command = "sh";

		execlp(shell_command, shell_command, "-c", data, NULL);
	}
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
			.val = LOCAL_COMMAND_OPTION_HELP,
		},
		{
			.name = "version",
			.has_arg = no_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_VERSION,
		},
		{
			.name = "daemon",
			.has_arg = no_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_DAEMON,
		},
		{
			.name = "year",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_YEAR,
		},
		{
			.name = "mon",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_MON,
		},
		{
			.name = "day",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_DAY,
		},
		{
			.name = "hour",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_HOUR,
		},
		{
			.name = "min",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_MIN,
		},
		{
			.name = "sec",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_SEC,
		},
		{
			.name = "date",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_DATE,
		},
		{
			.name = "year-repeat",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_YEAR_REPEAT,
		},
		{
			.name = "yr",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_YEAR_REPEAT,
		},
		{
			.name = "week-repeat",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_WEEK_REPEAT,
		},
		{
			.name = "wr",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_WEEK_REPEAT,
		},
		{
			.name = "day-repeat",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_DAY_REPEAT,
		},
		{
			.name = "dr",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_DAY_REPEAT,
		},
		{
			.name = "hour-repeat",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_HOUR_REPEAT,
		},
		{
			.name = "hr",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_HOUR_REPEAT,
		},
		{
			.name = "min-repeat",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_MIN_REPEAT,
		},
		{
			.name = "mr",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_MIN_REPEAT,
		},
		{
			.name = "sec-repeat",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_SEC_REPEAT,
		},
		{
			.name = "sr",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_SEC_REPEAT,
		},
		{
			0, 0, 0, 0
		},
	};
	int ret;
	bool as_daemon = false;
	char command[1024];
	struct tm date;
	struct cavan_alarm_thread thread;
	struct cavan_alarm_node node;

	cavan_alarm_node_init(&node, command, cavan_alarm_handler);

	node.time = time(NULL);
	if (node.time == ((time_t)-1))
	{
		pr_error_info("time");
		return -EFAULT;
	}

	localtime_r(&node.time, &date);

	while ((c = getopt_long(argc, argv, "vVHDy:h:M:d:h:m:s:", long_option, &option_index)) != EOF)
	{
		switch (c)
		{
		case 'v':
		case 'V':
		case LOCAL_COMMAND_OPTION_VERSION:
			show_author_info();
			println(FILE_CREATE_DATE);
			return 0;

		case 'H':
		case LOCAL_COMMAND_OPTION_HELP:
			show_usage(argv[0]);
			return 0;

		case 'D':
		case LOCAL_COMMAND_OPTION_DAEMON:
			as_daemon = true;
			break;

		case 'y':
		case LOCAL_COMMAND_OPTION_YEAR:
			date.tm_year = text2value_unsigned(optarg, NULL, 10) - 1900;
			break;

		case 'M':
		case LOCAL_COMMAND_OPTION_MON:
			date.tm_mon = text2value_unsigned(optarg, NULL, 10) - 1;
			break;

		case 'd':
		case LOCAL_COMMAND_OPTION_DAY:
			date.tm_mday = text2value_unsigned(optarg, NULL, 10);
			break;

		case 'h':
		case LOCAL_COMMAND_OPTION_HOUR:
			date.tm_hour = text2value_unsigned(optarg, NULL, 10);
			break;

		case 'm':
		case LOCAL_COMMAND_OPTION_MIN:
			date.tm_min = text2value_unsigned(optarg, NULL, 10);
			break;

		case 's':
		case LOCAL_COMMAND_OPTION_SEC:
			date.tm_sec = text2value_unsigned(optarg, NULL, 10);
			break;

		case LOCAL_COMMAND_OPTION_DATE:
			ret = cavan_text2date(optarg, &date);
			if (ret < 0)
			{
				pr_red_info("cavan_text2date");
				return -EINVAL;
			}
			break;

		case LOCAL_COMMAND_OPTION_YEAR_REPEAT:
			cavan_alarm_set_year_repeat(&node, text2value_unsigned(optarg, NULL, 10));
			break;

		case LOCAL_COMMAND_OPTION_WEEK_REPEAT:
			cavan_alarm_set_week_repeat(&node, text2value_unsigned(optarg, NULL, 10));
			break;

		case LOCAL_COMMAND_OPTION_DAY_REPEAT:
			cavan_alarm_set_day_repeat(&node, text2value_unsigned(optarg, NULL, 10));
			break;

		case LOCAL_COMMAND_OPTION_HOUR_REPEAT:
			cavan_alarm_set_hour_repeat(&node, text2value_unsigned(optarg, NULL, 10));
			break;

		case LOCAL_COMMAND_OPTION_MIN_REPEAT:
			cavan_alarm_set_min_repeat(&node, text2value_unsigned(optarg, NULL, 10));
			break;

		case LOCAL_COMMAND_OPTION_SEC_REPEAT:
			cavan_alarm_set_sec_repeat(&node, text2value_unsigned(optarg, NULL, 10));
			break;

		default:
			show_usage(argv[0]);
			return -EINVAL;
		}
	}

	if (optind < argc)
	{
		text_join_by_char(argv + optind, argc - optind, ' ', command, sizeof(command));
	}
	else
	{
		pr_red_info("Please give a command");
		show_usage(argv[0]);
		return -EINVAL;
	}

	ret = cavan_alarm_thread_init(&thread);
	if (ret < 0)
	{
		pr_red_info("cavan_alarm_thread_init");
		return ret;
	}

	if (as_daemon)
	{
		ret = daemon(0, 0);
		if (ret < 0)
		{
			pr_warning_info("daemon");
		}
	}

	ret = cavan_alarm_thread_start(&thread);
	if (ret < 0)
	{
		pr_red_info("cavan_alarm_thread_start");
		goto out_cavan_alarm_thread_deinit;
	}

	ret = cavan_alarm_insert_node(&thread, &node, &date);
	if (ret < 0)
	{
		pr_red_info("cavan_alarm_add_node");
		goto out_cavan_alarm_thread_stop;
	}

	ret = cavan_alarm_thread_join(&thread);

out_cavan_alarm_thread_stop:
	cavan_alarm_thread_stop(&thread);
out_cavan_alarm_thread_deinit:
	cavan_alarm_thread_deinit(&thread);
	return ret;
}
