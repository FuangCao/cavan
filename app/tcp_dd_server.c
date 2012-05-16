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
	LOCAL_COMMAND_OPTION_VERBOSE
};

static void show_usage(void)
{
	println("Usage:");
}

int main(int argc, char *argv[])
{
	int c;
	int option_index;
	int ret;
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
			.name = "verbose",
			.has_arg = no_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_VERBOSE,
		},
		{
		},
	};
	u16 port;
	struct cavan_service_description desc =
	{
		.name = "TCP_DD",
		.daemon_count = TCP_DD_DAEMON_COUNT,
		.as_daemon = 0,
		.show_verbose = 0,
	};

	ret = is_super_user(NULL);
	if (ret < 0)
	{
		return ret;
	}

	while ((c = getopt_long(argc, argv, "hHvVdD", long_option, &option_index)) != EOF)
	{
		switch (c)
		{

		case 'h':
		case 'H':
		case LOCAL_COMMAND_OPTION_HELP:
			show_usage();
			return 0;

		case LOCAL_COMMAND_OPTION_VERSION:
			show_author_info();
			println(FILE_CREATE_DATE);
			return 0;

		case 'v':
		case 'V':
		case LOCAL_COMMAND_OPTION_VERBOSE:
			desc.show_verbose = 1;
			break;

		case 'd':
		case 'D':
		case LOCAL_COMMAND_OPTION_DAEMON:
			desc.as_daemon = 1;
			break;

		default:
			show_usage();
			return -EINVAL;
		}
	}

	port = argc > 1 ? text2value_unsigned(argv[1], NULL, 10) : cavan_get_server_port(TCP_DD_DEFAULT_PORT);

	return tcp_dd_service_run(&desc, port);
}
