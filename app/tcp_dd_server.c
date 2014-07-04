/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Sat Jan 14 14:09:50 CST 2012
 */

#include <cavan.h>
#include <cavan/tcp_dd.h>
#include <cavan/command.h>
#include <cavan/permission.h>

#define FILE_CREATE_DATE "2012-01-14 14:09:50"

static void show_usage(const char *command)
{
	println("Usage:");
	println("%s [option] port", command);
	println("--help, -h, -H\t\t\tshow this help");
	println("--super, -s, -S\t\t\tneed super permission");
	println("--daemon, -d, -D\t\trun as a daemon");
	println("--min, -m, -c\t\t\tmin daemon count");
	println("--max, -M, -C\t\t\tmax daemon count");
	println("--verbose, -v, -V\t\tshow log message");
	println("--port, -p, -P\t\t\tserver port");
	println("--log, -l, -L\t\t\tsave log to file");
	println("--udp\t\t\t\trun as udp service");
	println("--url\t\t\t\tservice url");
	println("--unix, -u, -U [PATHNAME]\tlisten to a named socket, default path is %s", TCP_DD_DEFAULT_SOCKET);
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
			.name = "daemon",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_DAEMON,
		},
		{
			.name = "min",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_DAEMON_MIN,
		},
		{
			.name = "max",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_DAEMON_MAX,
		},
		{
			.name = "verbose",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_VERBOSE,
		},
		{
			.name = "super",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_SUPER,
		},
		{
			.name = "port",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_PORT,
		},
		{
			.name = "log",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_LOGFILE,
		},
		{
			.name = "unix",
			.has_arg = optional_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_UNIX,
		},
		{
			.name = "udp",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_UDP,
		},
		{
			.name = "url",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_URL,
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
	dd_service->sun_path = NULL;
	dd_service->type = NETWORK_CONNECT_TCP;
	dd_service->port = cavan_get_server_port(TCP_DD_DEFAULT_PORT);

	while ((c = getopt_long(argc, argv, "hHvVdDp:P:s:S:c:C:m:M:l:L:u::U::", long_option, &option_index)) != EOF)
	{
		switch (c)
		{
		case 'h':
		case 'H':
		case CAVAN_COMMAND_OPTION_HELP:
			show_usage(argv[0]);
			return 0;

		case CAVAN_COMMAND_OPTION_VERSION:
			show_author_info();
			println(FILE_CREATE_DATE);
			return 0;

		case 'v':
		case 'V':
		case CAVAN_COMMAND_OPTION_VERBOSE:
			service->verbose = 1;
			break;

		case 'd':
		case 'D':
		case CAVAN_COMMAND_OPTION_DAEMON:
			service->as_daemon = 1;
			break;

		case 'c':
		case 'm':
		case CAVAN_COMMAND_OPTION_DAEMON_MIN:
			service->min = text2value_unsigned(optarg, NULL, 10);
			break;

		case 'C':
		case 'M':
		case CAVAN_COMMAND_OPTION_DAEMON_MAX:
			service->max = text2value_unsigned(optarg, NULL, 10);
			break;

		case 'l':
		case 'L':
		case CAVAN_COMMAND_OPTION_LOGFILE:
			service->logfile = optarg;
			break;


		case 's':
		case 'S':
		case CAVAN_COMMAND_OPTION_SUPER:
			service->super_permission = text_bool_value(optarg);
			break;

		case 'p':
		case 'P':
		case CAVAN_COMMAND_OPTION_PORT:
			dd_service->port = text2value_unsigned(optarg, NULL, 10);
			break;

		case 'u':
		case 'U':
		case CAVAN_COMMAND_OPTION_UNIX:
			dd_service->type = NETWORK_CONNECT_UNIX_TCP;

			if (optarg)
			{
				dd_service->sun_path = optarg;
			}
			else
			{
				dd_service->sun_path = TCP_DD_DEFAULT_SOCKET;
			}
			break;

		case CAVAN_COMMAND_OPTION_UDP:
			dd_service->type = NETWORK_CONNECT_UDP;
			break;

		case CAVAN_COMMAND_OPTION_URL:
			dd_service->url = optarg;
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
