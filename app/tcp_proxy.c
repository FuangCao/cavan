/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Mon Dec 17 15:10:39 CST 2012
 */

#include <cavan.h>
#include <cavan/adb.h>
#include <cavan/tcp_proxy.h>

#define FILE_CREATE_DATE "2012-12-17 15:10:39"

enum
{
	LOCAL_COMMAND_OPTION_UNKNOWN,
	LOCAL_COMMAND_OPTION_HELP,
	LOCAL_COMMAND_OPTION_VERSION,
	LOCAL_COMMAND_OPTION_PORT,
	LOCAL_COMMAND_OPTION_PROXY_PORT,
	LOCAL_COMMAND_OPTION_PROXY_IP,
	LOCAL_COMMAND_OPTION_DAEMON,
	LOCAL_COMMAND_OPTION_DAEMON_COUNT,
	LOCAL_COMMAND_OPTION_VERBOSE,
	LOCAL_COMMAND_OPTION_ADB
};

static void show_usage(const char *command)
{
	println("Usage:");
	println("%s [option] proxy_port", command);
	println("--help, -h, -H\t\t\tshow this help");
	println("--version, -v, -V\t\tshow version");
	println("--port, -p, -P\t\t\tserver port");
	println("--proxy_ip, --pip, -i, -I\tproxy ip address");
	println("--proxy_port, --pport\t\tproxy port");
	println("--adb, -a, -A\t\t\tuse adb procotol instead of tcp");
	println("--daemon, -d, -D\t\trun as a daemon");
	println("--daemon_count, -c, -C\t\tdaemon count");
	println("--verbose\t\t\tshow log message");
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
			.val = LOCAL_COMMAND_OPTION_HELP,
		},
		{
			.name = "version",
			.has_arg = no_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_VERSION,
		},
		{
			.name = "port",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_PORT,
		},
		{
			.name = "proxy_ip",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_PROXY_IP,
		},
		{
			.name = "pip",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_PROXY_IP,
		},
		{
			.name = "proxy_port",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_PROXY_PORT,
		},
		{
			.name = "pport",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_PROXY_PORT,
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
			.name = "adb",
			.has_arg = no_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_ADB,
		},
		{
			0, 0, 0, 0
		},
	};
	struct tcp_proxy_service proxy_service =
	{
		.service =
		{
			.name = "TCP_PROXY",
			.daemon_count = 10,
			.as_daemon = 0,
			.show_verbose = 1,
			.super_permission = 0
		},
		.port = 8888,
		.proxy_port = 8888,
		.proxy_ip = "127.0.0.1",
		.open_connect = inet_create_tcp_link2,
		.close_connect = inet_close_tcp_socket
	};

	while ((c = getopt_long(argc, argv, "vVhHi:I:p:P:c:C:dDaA", long_option, &option_index)) != EOF)
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

		case 'p':
		case 'P':
		case LOCAL_COMMAND_OPTION_PORT:
			proxy_service.port = text2value_unsigned(optarg, NULL, 10);
			break;

		case LOCAL_COMMAND_OPTION_PROXY_PORT:
			proxy_service.proxy_port = text2value_unsigned(optarg, NULL, 10);
			break;

		case 'i':
		case 'I':
		case LOCAL_COMMAND_OPTION_PROXY_IP:
			text_copy(proxy_service.proxy_ip, optarg);
			break;

		case 'd':
		case 'D':
		case LOCAL_COMMAND_OPTION_DAEMON:
			proxy_service.service.as_daemon = 1;
			break;

		case 'c':
		case 'C':
		case LOCAL_COMMAND_OPTION_DAEMON_COUNT:
			proxy_service.service.daemon_count = text2value_unsigned(optarg, NULL, 10);
			break;

		case LOCAL_COMMAND_OPTION_VERBOSE:
			proxy_service.service.show_verbose = 1;
			break;

		case 'a':
		case 'A':
		case LOCAL_COMMAND_OPTION_ADB:
			proxy_service.open_connect = adb_create_tcp_link2;
			break;

		default:
			show_usage(argv[0]);
			return -EINVAL;
		}
	}

	if (optind < argc)
	{
		proxy_service.proxy_port = text2value_unsigned(argv[optind], NULL, 10);
	}

	if (proxy_service.open_connect != adb_create_tcp_link2 && proxy_service.proxy_port == proxy_service.port && inet_addr(proxy_service.proxy_ip) == htonl(INADDR_LOOPBACK))
	{
		pr_red_info("Can't proxy yourself, please change proxy port");
		return -EINVAL;
	}

	ret = tcp_proxy_service_run(&proxy_service);
	if (ret < 0)
	{
		pr_red_info("tcp_proxy_service_run");
	}

	return ret;
}
