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
	LOCAL_COMMAND_OPTION_DAEMON_MIN,
	LOCAL_COMMAND_OPTION_DAEMON_MAX,
	LOCAL_COMMAND_OPTION_VERBOSE,
	LOCAL_COMMAND_OPTION_SUPER,
	LOCAL_COMMAND_OPTION_LOGFILE,
	LOCAL_COMMAND_OPTION_PORT
};

static void show_usage(const char *command)
{
	println("Usage:");
	println("%s [option] port", command);
	println("--help, -h, -H\t\tshow this help");
	println("--super, -s, -S\t\tneed super permission");
	println("--daemon, -d, -D\trun as a daemon");
	println("--min, -m, -c\t\tmin daemon count");
	println("--max, -M, -C\t\tmax daemon count");
	println("--verbose, -v, -V\tshow log message");
	println("--port, -p, -P\t\tserver port");
	println("--log, -l, -L\t\tsave log to file");
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
			.name = "min",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_DAEMON_MIN,
		},
		{
			.name = "max",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_DAEMON_MAX,
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
			.name = "log",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_LOGFILE,
		},
		{
			0, 0, 0, 0
		},
	};
	struct cavan_dynamic_service *service;
	struct cavan_tcp_dd_service *dd_service;

	service = cavan_dynamic_service_create(sizeof(struct cavan_tcp_dd_service));
	if (service == NULL)
	{
		pr_red_info("cavan_dynamic_service_create");
		return -ENOMEM;
	}

	service->min = 10;
	service->max = 1000;
	service->super_permission = 1;

	dd_service = cavan_dynamic_service_get_data(service);
	dd_service->port = cavan_get_server_port(TCP_DD_DEFAULT_PORT);

	while ((c = getopt_long(argc, argv, "hHvVdDp:P:s:S:c:C:m:M:l:L:", long_option, &option_index)) != EOF)
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
			service->verbose = 1;
			break;

		case 'd':
		case 'D':
		case LOCAL_COMMAND_OPTION_DAEMON:
			service->as_daemon = 1;
			break;

		case 'c':
		case 'm':
		case LOCAL_COMMAND_OPTION_DAEMON_MIN:
			service->min = text2value_unsigned(optarg, NULL, 10);
			break;

		case 'C':
		case 'M':
		case LOCAL_COMMAND_OPTION_DAEMON_MAX:
			service->max = text2value_unsigned(optarg, NULL, 10);
			break;

		case 'l':
		case 'L':
		case LOCAL_COMMAND_OPTION_LOGFILE:
			service->logfile = optarg;
			break;


		case 's':
		case 'S':
		case LOCAL_COMMAND_OPTION_SUPER:
			service->super_permission = text_bool_value(optarg);
			break;

		case 'p':
		case 'P':
		case LOCAL_COMMAND_OPTION_PORT:
			dd_service->port = text2value_unsigned(optarg, NULL, 10);
			break;

		default:
			show_usage(argv[0]);
			return -EINVAL;
		}
	}

	if (argc > optind)
	{
		dd_service->port = text2value_unsigned(argv[optind], NULL, 10);
	}

	return tcp_dd_service_run(service);
}
