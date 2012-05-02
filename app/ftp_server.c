// Fuang.Cao <cavan.cfa@gmail.com> 2011-10-28 12:46:16

#include <cavan.h>
#include <cavan/ftp.h>

#define FILE_CREATE_DATE "2011-10-28 12:46:16"

static void show_usage(void)
{
	println("Usage:");
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
			.val = 'h',
		},
		{
			.name = "version",
			.has_arg = no_argument,
			.flag = NULL,
			.val = 'v',
		},
		{
			.name = "dev",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 'd',
		},
		{
			.name = "root",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 'r',
		},
		{
			.name = "port",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 'p',
		},
		{
			.name = "count",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 'c',
		},
		{
		},
	};
	u16 port = FTP_CTRL_PORT;
	int count = FTP_DAEMON_COUNT;

	while ((c = getopt_long(argc, argv, "vVhHd:D:p:P:c:C:r:R:", long_option, &option_index)) != EOF)
	{
		switch (c)
		{
		case 'v':
		case 'V':
			show_author_info();
			println(FILE_CREATE_DATE);
			return 0;

		case 'h':
		case 'H':
			show_usage();
			return 0;

		case 'd':
		case 'D':
			text_copy(ftp_netdev_name, optarg);
			break;

		case 'r':
		case 'R':
			if (realpath(optarg, ftp_root_path) == NULL)
			{
				error_msg("Get directory `%s' realpath failed", optarg);
				return -ENOENT;
			}
			break;

		case 'p':
		case 'P':
			port = text2value_unsigned(optarg, NULL, 10);
			break;

		case 'c':
		case 'C':
			count = text2value_unsigned(optarg, NULL, 10);
			break;

		default:
			show_usage();
			return -EINVAL;
		}
	}

	if (argc > optind && port == FTP_CTRL_PORT)
	{
		port = text2value_unsigned(argv[optind], NULL, 10);
	}

	return ftp_service_run(port, count);
}
