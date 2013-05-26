// Fuang.Cao <cavan.cfa@gmail.com> 2011-10-28 12:46:16

#include <cavan.h>
#include <cavan/ftp.h>

#define FILE_CREATE_DATE "2011-10-28 12:46:16"

enum
{
	LOCAL_COMMAND_OPTION_UNKNOWN,
	LOCAL_COMMAND_OPTION_HELP,
	LOCAL_COMMAND_OPTION_VERSION,
	LOCAL_COMMAND_OPTION_PORT,
	LOCAL_COMMAND_OPTION_COUNT,
	LOCAL_COMMAND_OPTION_ROOT,
	LOCAL_COMMAND_OPTION_DAEMON,
	LOCAL_COMMAND_OPTION_VERBOSE,
	LOCAL_COMMAND_OPTION_SUPER
};

static void show_usage(const char *command)
{
	println("Usage:");
	println("%s [option] port", command);
	println("--help, -h, -H\t\tshow this help");
	println("--super, -s, -S\t\tneed super permission");
	println("--daemon, -d, -D\trun as a daemon");
	println("--count, -c, -C\t\tdaemon count");
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
			.name = "root",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_ROOT,
		},
		{
			.name = "port",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_PORT,
		},
		{
			.name = "count",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_COUNT,
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
			.name = "super",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_SUPER,
		},
		{0, 0, 0, 0},
	};
	u16 port = FTP_CTRL_PORT;
	struct cavan_service_description desc =
	{
		.name = "FTP",
		.daemon_count = FTP_DAEMON_COUNT,
		.as_daemon = 0,
		.show_verbose = 0,
		.super_permission = 1
	};

	while ((c = getopt_long(argc, argv, "vVhHdDp:P:c:C:r:R:s:S:", long_option, &option_index)) != EOF)
	{
		switch (c)
		{
		case LOCAL_COMMAND_OPTION_VERSION:
			show_author_info();
			println(FILE_CREATE_DATE);
			return 0;

		case 'h':
		case 'H':
		case LOCAL_COMMAND_OPTION_HELP:
			show_usage(argv[0]);
			return 0;

		case 'r':
		case 'R':
		case LOCAL_COMMAND_OPTION_ROOT:
			if (realpath(optarg, ftp_root_path) == NULL)
			{
				error_msg("Get directory `%s' realpath failed", optarg);
				return -ENOENT;
			}
			break;

		case 'p':
		case 'P':
		case LOCAL_COMMAND_OPTION_PORT:
			port = text2value_unsigned(optarg, NULL, 10);
			break;

		case 'c':
		case 'C':
		case LOCAL_COMMAND_OPTION_COUNT:
			desc.daemon_count = text2value_unsigned(optarg, NULL, 10);
			break;

		case LOCAL_COMMAND_OPTION_DAEMON:
			desc.as_daemon = 1;
			break;

		case LOCAL_COMMAND_OPTION_VERBOSE:
			desc.show_verbose = 1;
			break;

		case 's':
		case 'S':
		case LOCAL_COMMAND_OPTION_SUPER:
			desc.super_permission = text_bool_value(optarg);
			break;

		default:
			show_usage(argv[0]);
			return -EINVAL;
		}
	}

	if (argc > optind && port == FTP_CTRL_PORT)
	{
		port = text2value_unsigned(argv[optind], NULL, 10);
	}

	return ftp_service_run(&desc, port);
}
