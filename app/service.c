/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Thu May 17 11:18:13 CST 2012
 */

#include <cavan.h>
#include <cavan/service.h>
#include <cavan/text.h>

#define FILE_CREATE_DATE "2012-05-17 11:18:13"

enum
{
	LOCAL_COMMAND_OPTION_UNKNOWN,
	LOCAL_COMMAND_OPTION_HELP,
	LOCAL_COMMAND_OPTION_VERSION,
	LOCAL_COMMAND_OPTION_START,
	LOCAL_COMMAND_OPTION_STOP,
	LOCAL_COMMAND_OPTION_EXEC,
	LOCAL_COMMAND_OPTION_PIDFILE,
	LOCAL_COMMAND_OPTION_LOGFILE,
	LOCAL_COMMAND_OPTION_SUPER,
	LOCAL_COMMAND_OPTION_VERBOSE
};

static void show_usage(const char *command)
{
	println("Usage: %s [--start|--stop] [option] [<--exec command>|<command>]", command);
	println("--start:\t\tstart a server");
	println("--stop:\t\t\tstop a server");
	println("--help, -h, -H\t\tshow this help");
	println("--version\t\tshow version");
	println("--verbose, -v, -V\tshow log message");
	println("--super, -s, -S\t\tneed super permission");
	println("--exec, -e, -E\t\tservice command name");
	println("--pidfile, -p, -P\tsave process id to file");
	println("--logfile, -l, -L\tredirect stdout and stderr to file");
	println("--verbose, -v, -V\tshow log message");
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
			.name = "start",
			.has_arg = no_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_START,
		},
		{
			.name = "stop",
			.has_arg = no_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_STOP,
		},
		{
			.name = "exec",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_EXEC,
		},
		{
			.name = "pidfile",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_PIDFILE,
		},
		{
			.name = "logfile",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_LOGFILE,
		},
		{
			.name = "super",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_SUPER,
		},
		{
			.name = "verbose",
			.has_arg = no_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_VERBOSE,
		},
	};
	int ret;
	char command[1024];
	int (*handler)(struct cavan_daemon_description *);
	struct cavan_daemon_description desc =
	{
		.verbose = 0,
		.as_daemon = 1,
		.super_permission = 1,
		.command = NULL,
		.pidfile = NULL,
		.logfile = NULL,
	};

	handler = cavan_daemon_run;

	while ((c = getopt_long(argc, argv, "vVhHe:E:p:P:s:S:l:L:", long_option, &option_index)) != EOF)
	{
		switch (c)
		{
		case LOCAL_COMMAND_OPTION_VERSION:
			show_author_info();
			println(FILE_CREATE_DATE);
			return 0;

		case 'h':
		case 'H':
		case LOCAL_COMMAND_OPTION_HELP:
			show_usage(argv[0]);
			return 0;

		case 'e':
		case 'E':
		case LOCAL_COMMAND_OPTION_EXEC:
			desc.command = optarg;
			break;

		case 'p':
		case 'P':
		case LOCAL_COMMAND_OPTION_PIDFILE:
			desc.pidfile = optarg;
			break;

		case 'l':
		case 'L':
		case LOCAL_COMMAND_OPTION_LOGFILE:
			desc.logfile = optarg;
			break;

		case 's':
		case 'S':
		case LOCAL_COMMAND_OPTION_SUPER:
			desc.super_permission = text_bool_value(optarg);
			break;

		case 'v':
		case 'V':
		case LOCAL_COMMAND_OPTION_VERBOSE:
			desc.verbose = 1;
			break;

		case LOCAL_COMMAND_OPTION_START:
			handler = cavan_daemon_run;
			break;

		case LOCAL_COMMAND_OPTION_STOP:
			handler = cavan_daemon_stop;
			break;

		default:
			show_usage(argv[0]);
			return -EINVAL;
		}
	}

	if (argc > optind && desc.command == NULL)
	{
		text_join_by_char(argv + optind, argc - optind, ' ', command, sizeof(command));
		desc.command = command;
	}

	ret = handler(&desc);
	if (ret < 0)
	{
		pr_red_info("Failed");
		return ret;
	}

	pr_green_info("OK");

	return 0;
}
