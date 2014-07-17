/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Mon Dec 17 15:10:39 CST 2012
 */

#include <cavan.h>
#include <cavan/adb.h>
#include <cavan/command.h>
#include <cavan/tcp_proxy.h>

#define FILE_CREATE_DATE "2012-12-17 15:10:39"

static void show_usage(const char *command)
{
	println("Usage:");
	println("%s [option] [PORT]", command);
	println("--help, -H, -h\t\t\t%s", cavan_help_message_help);
	println("--version, -V, -v\t\t%s", cavan_help_message_version);
	println("--port, -P, -p PORT\t\t%s", cavan_help_message_port);
	println("--adb, -A, -a\t\t\t%s", cavan_help_message_adb);
	println("--daemon, -D, -d\t\t%s", cavan_help_message_daemon);
	println("--min, -m, -c\t\t\t%s", cavan_help_message_daemon_min);
	println("--max, -M, -C\t\t\t%s", cavan_help_message_daemon_max);
	println("--verbose\t\t\t%s", cavan_help_message_verbose);
	println("--log, -L, -l\t\t\t%s", cavan_help_message_logfile);
	println("--pip, --host HOSTNAME\t\t%s", cavan_help_message_proxy_hostname);
	println("--pport, --pp PORT\t\t%s", cavan_help_message_proxy_port);
	println("--url, -u URL\t\t\t%s", cavan_help_message_url);
	println("--purl, --pu, -U URL\t\t%s", cavan_help_message_proxy_url);
}

int main(int argc, char *argv[])
{
	int ret;
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
			.name = "port",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_PORT,
		},
		{
			.name = "host",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_PROXY_HOST,
		},
		{
			.name = "pip",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_PROXY_HOST,
		},
		{
			.name = "proxy_port",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_PROXY_PORT,
		},
		{
			.name = "pport",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_PROXY_PORT,
		},
		{
			.name = "pp",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_PROXY_PORT,
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
			.name = "adb",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_ADB,
		},
		{
			.name = "log",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_LOGFILE,
		},
		{
			.name = "url",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_URL,
		},
		{
			.name = "purl",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_PROXY_URL,
		},
		{
			.name = "pu",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_PROXY_URL,
		},
		{
			0, 0, 0, 0
		},
	};
	struct cavan_dynamic_service *service;
	struct tcp_proxy_service *proxy;

	service = cavan_dynamic_service_create(sizeof(struct tcp_proxy_service));
	if (service == NULL)
	{
		pr_red_info("cavan_dynamic_service_create");
		return -ENOMEM;
	}

	service->min = 20;
	service->max = 1000;

	proxy = cavan_dynamic_service_get_data(service);

	network_url_init(&proxy->url, "tcp", "any", CAVAN_TCP_PROXY_PORT, NULL);
	network_url_init(&proxy->url_proxy, "tcp", NULL, NETWORK_PORT_HTTP, NULL);

	while ((c = getopt_long(argc, argv, "vVhH:i:I:p:P:c:C:m:M:dDaAl:L:u:U:", long_option, &option_index)) != EOF)
	{
		switch (c)
		{
		case 'v':
		case 'V':
		case CAVAN_COMMAND_OPTION_VERSION:
			show_author_info();
			println(FILE_CREATE_DATE);
			return 0;

		case 'l':
		case 'L':
		case CAVAN_COMMAND_OPTION_LOGFILE:
			service->logfile = optarg;
			break;

		case 'h':
		case CAVAN_COMMAND_OPTION_HELP:
			show_usage(argv[0]);
			return 0;

		case 'p':
		case 'P':
		case CAVAN_COMMAND_OPTION_PORT:
			proxy->url.port = text2value_unsigned(optarg, NULL, 10);
			break;

		case CAVAN_COMMAND_OPTION_PROXY_PORT:
			proxy->url_proxy.port = text2value_unsigned(optarg, NULL, 10);
			break;

		case 'i':
		case 'I':
		case 'H':
		case CAVAN_COMMAND_OPTION_PROXY_HOST:
			proxy->url_proxy.hostname = optarg;
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

		case CAVAN_COMMAND_OPTION_VERBOSE:
			service->verbose = true;
			break;

		case 'a':
		case 'A':
		case CAVAN_COMMAND_OPTION_ADB:
			proxy->url_proxy.protocol = "adb";
			break;

		case 'u':
		case CAVAN_COMMAND_OPTION_URL:
			if (network_url_parse(&proxy->url, optarg) == NULL)
			{
				return -EINVAL;
			}
			break;

		case 'U':
		case CAVAN_COMMAND_OPTION_PROXY_URL:
			if (network_url_parse(&proxy->url_proxy, optarg) == NULL)
			{
				return -EINVAL;
			}
			break;

		default:
			show_usage(argv[0]);
			return -EINVAL;
		}
	}

	if (optind < argc)
	{
		proxy->url.port = text2value_unsigned(argv[optind], NULL, 10);
		if (proxy->url.port == 0)
		{
			pr_red_info("invalid port %s", argv[optind]);
			return -EINVAL;
		}
	}

	ret = tcp_proxy_service_run(service);
	if (ret < 0)
	{
		pr_red_info("tcp_proxy_service_run");
	}

	return ret;
}
