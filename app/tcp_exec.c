/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Tue Dec 18 15:10:21 CST 2012
 */

#include <cavan.h>
#include <cavan/adb.h>
#include <cavan/tcp_dd.h>
#include <cavan/command.h>

#define FILE_CREATE_DATE "2012-12-18 15:10:21"

static void show_usage(const char *command)
{
	println("Usage: %s [option] command", command);
	println("--help, -h, -H\t\t\t%s", cavan_help_message_help);
	println("--version, -v, -V\t\t%s", cavan_help_message_version);
	println("--ip, -i, -I [IP]\t\t%s", cavan_help_message_ip);
	println("--host [HOSTNAME]\t\t%s", cavan_help_message_hostname);
	println("--local, -l, -L\t\t\t%s", cavan_help_message_local);
	println("--port, -p, -P [PORT]\t\t%s", cavan_help_message_port);
	println("--adb, -a, -A\t\t\t%s", cavan_help_message_adb);
	println("--udp\t\t\t\t%s", cavan_help_message_udp);
	println("--unix, --unix-tcp [PATHNAME]\t%s", cavan_help_message_unix_tcp);
	println("--unix-udp [PATHNAME]\t\t%s", cavan_help_message_unix_udp);
	println("--url, -u, -U [URL]\t\t%s", cavan_help_message_url);
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
			.name = "ip",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_IP,
		},
		{
			.name = "port",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_PORT,
		},
		{
			.name = "adb",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_ADB,
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
			.name = "local",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_LOCAL,
		},
		{
			.name = "host",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_HOST,
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
			0, 0, 0, 0
		},
	};
	char command[1024];
	struct network_url url;

	network_url_init(&url, "tcp", NULL, TCP_DD_DEFAULT_PORT, TCP_DD_DEFAULT_SOCKET);

	while ((c = getopt_long(argc, argv, "vVhHIaA:i:I:p:P:lLu:U:", long_option, &option_index)) != EOF)
	{
		switch (c)
		{
		case 'v':
		case 'V':
		case CAVAN_COMMAND_OPTION_VERSION:
			show_author_info();
			println(FILE_CREATE_DATE);
			return 0;

		case 'h':
		case 'H':
		case CAVAN_COMMAND_OPTION_HELP:
			show_usage(argv[0]);
			return 0;

		case 'a':
		case 'A':
		case CAVAN_COMMAND_OPTION_ADB:
			url.protocol = "adb";
		case 'l':
		case 'L':
		case CAVAN_COMMAND_OPTION_LOCAL:
			optarg = "127.0.0.1";
		case 'i':
		case 'I':
		case CAVAN_COMMAND_OPTION_IP:
		case CAVAN_COMMAND_OPTION_HOST:
			url.hostname = optarg;
			break;

		case CAVAN_COMMAND_OPTION_UDP:
			url.protocol = "udp";
			break;

		case 'p':
		case 'P':
		case CAVAN_COMMAND_OPTION_PORT:
			url.port = text2value_unsigned(optarg, NULL, 10);
			break;

		case 'u':
		case 'U':
		case CAVAN_COMMAND_OPTION_URL:
			if (network_url_parse(&url, optarg) == NULL)
			{
				pr_red_info("invalid url %s", optarg);
				return -EINVAL;
			}
			break;

		case CAVAN_COMMAND_OPTION_UNIX:
		case CAVAN_COMMAND_OPTION_UNIX_TCP:
			url.protocol = "unix-tcp";
			if (optarg)
			{
				url.pathname = optarg;
			}
			break;

		case CAVAN_COMMAND_OPTION_UNIX_UDP:
			url.protocol = "unix-udp";
			if (optarg)
			{
				url.pathname = optarg;
			}
			break;

		default:
			show_usage(argv[0]);
			return -EINVAL;
		}
	}

	text_join_by_char(argv + optind, argc - optind, ' ', command, sizeof(command));

	return tcp_dd_exec_command(&url, command);
}
