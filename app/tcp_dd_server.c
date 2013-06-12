/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Sat Jan 14 14:09:50 CST 2012
 */

#include <cavan.h>
#include <cavan/tcp_dd.h>
#include <cavan/permission.h>

#define FILE_CREATE_DATE "2012-01-14 14:09:50"

enum
{
	LOCAL_COMMAND_OPTION_UNKNOWN,
	LOCAL_COMMAND_OPTION_HELP,
	LOCAL_COMMAND_OPTION_VERSION,
	LOCAL_COMMAND_OPTION_DAEMON,
	LOCAL_COMMAND_OPTION_DAEMON_COUNT,
	LOCAL_COMMAND_OPTION_VERBOSE,
	LOCAL_COMMAND_OPTION_SUPER,
	LOCAL_COMMAND_OPTION_PORT
};

static void show_usage(const char *command)
{
	println("Usage:");
	println("%s [option] port", command);
	println("--help, -h, -H\t\tshow this help");
	println("--super, -s, -S\t\tneed super permission");
	println("--daemon, -d, -D\trun as a daemon");
	println("--daemon_count, -c, -C\tdaemon count");
	println("--verbose, -v, -V\tshow log message");
	println("--port, -p, -P\t\tserver port");
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
			.name = "daemon_count",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_DAEMON_COUNT,
		},
		{
			.name = "verbose",
			.has_arg = no_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_VERBOSE,
		},
		{
			.name = "super",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_SUPER,
		},
		{
			.name = "port",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_PORT,
		},
		{
			0, 0, 0, 0
		},
	};
	u16 port = cavan_get_server_port(TCP_DD_DEFAULT_PORT);
	struct cavan_tcp_dd_service service =
	{
		.desc =
		{
			.name = "TCP_DD",
			.daemon_count = TCP_DD_DAEMON_COUNT,
			.as_daemon = 0,
			.show_verbose = 0,
			.super_permission = 1
		}
	};

	while ((c = getopt_long(argc, argv, "hHvVdDp:P:s:S:c:C:", long_option, &option_index)) != EOF)
	{
		switch (c)
		{

		case 'h':
		case 'H':
		case LOCAL_COMMAND_OPTION_HELP:
			show_usage(argv[0]);
			return 0;

		case LOCAL_COMMAND_OPTION_VERSION:
			show_author_info();
			println(FILE_CREATE_DATE);
			return 0;

		case 'v':
		case 'V':
		case LOCAL_COMMAND_OPTION_VERBOSE:
			service.desc.show_verbose = 1;
			break;

		case 'd':
		case 'D':
		case LOCAL_COMMAND_OPTION_DAEMON:
			service.desc.as_daemon = 1;
			break;

		case 's':
		case 'S':
		case LOCAL_COMMAND_OPTION_SUPER:
			service.desc.super_permission = text_bool_value(optarg);
			break;

		case 'p':
		case 'P':
		case LOCAL_COMMAND_OPTION_PORT:
			port = text2value_unsigned(optarg, NULL, 10);
			break;

		case 'c':
		case 'C':
		case LOCAL_COMMAND_OPTION_DAEMON_COUNT:
			service.desc.daemon_count = text2value_unsigned(optarg, NULL, 10);
			break;

		default:
			show_usage(argv[0]);
			return -EINVAL;
		}
	}

	if (argc > optind)
	{
		port = text2value_unsigned(argv[optind], NULL, 10);
	}

	return tcp_dd_service_run(&service, port);
}
