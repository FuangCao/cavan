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
	println("%s [option] [PORT]", command);
	println("-H, -h, --help\t\t\t\t%s", cavan_help_message_help);
	println("-S, -s, --super\t\t\t\t%s", cavan_help_message_super);
	println("-D, -d, --daemon\t\t\t%s", cavan_help_message_daemon);
	println("-m, -c, --min\t\t\t\t%s", cavan_help_message_daemon_min);
	println("-M, -C, --max\t\t\t\t%s", cavan_help_message_daemon_max);
	println("-V, -v, --verbose\t\t\t%s", cavan_help_message_verbose);
	println("-p, --port PORT\t\t\t\t%s", cavan_help_message_port);
	println("-L, -l, --log [PATHNAME]\t\t%s", cavan_help_message_logfile);
	println("--udp\t\t\t\t\t%s", cavan_help_message_udp);
	println("--url [URL]\t\t\t\t%s", cavan_help_message_url);
	println("-U, -u, --unix, --unix-tcp [PATHNAME]\t%s", cavan_help_message_unix_tcp);
	println("--unix-udp [PATHNAME]\t\t\t%s", cavan_help_message_unix_udp);
	println("-P, --pt, --protocol PROTOCOL\t\t%s", cavan_help_message_protocol);
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
			.name = "unix-tcp",
			.has_arg = optional_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_UNIX_TCP,
		},
		{
			.name = "unix-udp",
			.has_arg = optional_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_UNIX_UDP,
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
			.name = "protocol",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_PROTOCOL,
		},
		{
			.name = "pt",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_PROTOCOL,
		},
		{
			0, 0, 0, 0
		},
	};
	struct network_url *url;
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
	url = &dd_service->url;
	network_url_init(url, "tcp", "any", TCP_DD_DEFAULT_PORT, NULL);

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
			service->verbose = true;
			break;

		case 'd':
		case 'D':
		case CAVAN_COMMAND_OPTION_DAEMON:
			service->as_daemon = true;
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
		case CAVAN_COMMAND_OPTION_PORT:
			url->port = text2value_unsigned(optarg, NULL, 10);
			break;

		case 'u':
		case 'U':
		case CAVAN_COMMAND_OPTION_UNIX:
		case CAVAN_COMMAND_OPTION_UNIX_TCP:
			url->protocol = "unix-tcp";
			if (optarg)
			{
				url->pathname = optarg;
			}
			break;

		case CAVAN_COMMAND_OPTION_UNIX_UDP:
			url->protocol = "unix-udp";
			if (optarg)
			{
				url->pathname = optarg;
			}
			break;

		case CAVAN_COMMAND_OPTION_UDP:
			url->protocol = "udp";
			break;

		case CAVAN_COMMAND_OPTION_URL:
			if (network_url_parse(url, optarg) == NULL)
			{
				pr_red_info("invalid url %s", optarg);
				return -EINVAL;
			}
			break;

		case 'P':
		case CAVAN_COMMAND_OPTION_PROTOCOL:
			url->protocol = optarg;
			break;

		default:
			show_usage(argv[0]);
			return -EINVAL;
		}
	}

	if (argc > optind)
	{
		url->port = text2value_unsigned(argv[optind], NULL, 10);
		if (url->port == 0)
		{
			pr_red_info("Invalid port %s", argv[optind]);
			return -EINVAL;
		}
	}

	return tcp_dd_service_run(service);
}
