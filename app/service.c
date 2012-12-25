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
	LOCAL_COMMAND_OPTION_DAEMON,
	LOCAL_COMMAND_OPTION_SUPER
};

static void show_usage(const char *command)
{
	println("Usage: %s [--start|--stop] [option] <--exec command>", command);
	println("--start:\t\tstart a server");
	println("--stop:\t\t\tstop a server");
	println("--help, -h, -H\t\tshow this help");
	println("--version, -v, -V\tshow version");
	println("--daemon, -d, -D\trun as a daemon");
	println("--verbose, -v, -V\tshow log message");
	println("--super, -s, -S\t\tneed super permission");
	println("--exec, -e, -E\t\tservice command name");
	println("--pidfile, -p, -P\tsave process id to file");
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
			.name = "daemon",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_DAEMON,
		},
		{
			.name = "super",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_SUPER,
		},
	};
	int (*handler)(struct cavan_daemon_description *);
	unsigned int i;
	int ret;
	struct cavan_daemon_description desc =
	{
		.as_daemon = 1,
		.super_permission = 1
	};

	desc.cmdfile[0] = 0;
	desc.pidfile[0] = 0;
	handler = NULL;

	while ((c = getopt_long(argc, argv, "vVhHe:E:p:P:s:S:", long_option, &option_index)) != EOF)
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

		case 'e':
		case 'E':
		case LOCAL_COMMAND_OPTION_EXEC:
			text_copy(desc.cmdfile, optarg);
			break;

		case 'p':
		case 'P':
		case LOCAL_COMMAND_OPTION_PIDFILE:
			text_copy(desc.pidfile, optarg);
			break;

		case 'd':
		case 'D':
		case LOCAL_COMMAND_OPTION_DAEMON:
			desc.as_daemon = text_bool_value(optarg);
			break;

		case 's':
		case 'S':
		case LOCAL_COMMAND_OPTION_SUPER:
			desc.super_permission = text_bool_value(optarg);
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

	if (handler == NULL)
	{
		pr_red_info("Please specify --start or --stop option");
		show_usage(argv[0]);
		return -EINVAL;
	}

	for (desc.argv[0] = desc.cmdfile, i = 1; optind < argc && i < (NELEM(desc.argv) - 2); i++, optind++)
	{
		desc.argv[i] = argv[optind];
	}

	desc.argv[i] = NULL;

	ret = handler(&desc);
	if (ret < 0)
	{
		pr_red_info("Failed");
		return ret;
	}

	pr_green_info("OK");

	return 0;
}
