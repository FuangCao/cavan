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
	LOCAL_COMMAND_OPTION_DATE,
	LOCAL_COMMAND_OPTION_REPEAT,
};

static void show_usage(const char *command)
{
	println("Usage: %s [option] command", command);
	println("--help, -h, -H\t\tshow this help");
	println("--version, -v, -V\tshow version");
	println("--daemon, -d\t\trun as a daemon");
	println("--date, -D");
	println("--repeat, -r, -R");
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
			.name = "date",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_DATE,
		},
		{
			.name = "repeat",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_REPEAT,
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

	while ((c = getopt_long(argc, argv, "vVhHr:R:dD:", long_option, &option_index)) != EOF)
	{
		switch (c)
		{
		case 'v':
		case 'V':
		case LOCAL_COMMAND_OPTION_VERSION:
			show_author_info();
			println(FILE_CREATE_DATE);
			return 0;

		case 'h':
		case 'H':
		case LOCAL_COMMAND_OPTION_HELP:
			show_usage(argv[0]);
			return 0;

		case 'd':
		case LOCAL_COMMAND_OPTION_DAEMON:
			as_daemon = true;
			break;

		case 'D':
		case LOCAL_COMMAND_OPTION_DATE:
			ret = text2date(optarg, &date);
			if (ret < 0)
			{
				pr_red_info("cavan_text2date");
				return -EINVAL;
			}
			break;

		case 'r':
		case 'R':
		case LOCAL_COMMAND_OPTION_REPEAT:
			node.repeat = text2time(optarg, NULL);
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

	pr_bold_info("command = %s", command);

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
