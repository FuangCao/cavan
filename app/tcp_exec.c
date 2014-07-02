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
	println("--help, -h, -H\t\tshow this help");
	println("--version, -v, -V\tshow version");
	println("--ip, -i, -I\t\tserver ip address");
	println("--local, -l, -L\t\tuse localhost ip");
	println("--port, -p, -P\t\tserver port");
	println("--adb, -a, -A\t\tuse adb procotol instead of tcp");
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
			.name = "local",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_LOCAL,
		},
		{
			0, 0, 0, 0
		},
	};
	struct inet_file_request file_req =
	{
		.open_connect = inet_create_tcp_link2,
		.close_connect = inet_close_tcp_socket
	};

	file_req.hostname = cavan_get_server_hostname();
	file_req.port = cavan_get_server_port(TCP_DD_DEFAULT_PORT);

	while ((c = getopt_long(argc, argv, "vVhHIaA:i:I:p:P:lL", long_option, &option_index)) != EOF)
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
			file_req.open_connect = adb_create_tcp_link2;
		case 'l':
		case 'L':
		case CAVAN_COMMAND_OPTION_LOCAL:
			optarg = "127.0.0.1";
		case 'i':
		case 'I':
		case CAVAN_COMMAND_OPTION_IP:
			file_req.hostname = optarg;
			break;

		case 'p':
		case 'P':
		case CAVAN_COMMAND_OPTION_PORT:
			file_req.port = text2value_unsigned(optarg, NULL, 10);
			break;

		default:
			show_usage(argv[0]);
			return -EINVAL;
		}
	}

	text_join_by_char(argv + optind, argc - optind, ' ', file_req.command, sizeof(file_req.command));

	return tcp_dd_exec_command(&file_req);
}
