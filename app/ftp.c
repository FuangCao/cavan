// Fuang.Cao <cavan.cfa@gmail.com> 2011-10-28 17:42:37

#include <cavan.h>
#include <cavan/ftp.h>

#define FILE_CREATE_DATE "2011-10-28 17:42:37"

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
			0, 0, 0, 0
		},
	};
	const char *server_ip;
	u16 server_port;

	while ((c = getopt_long(argc, argv, "vVhH", long_option, &option_index)) != EOF)
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

		default:
			show_usage();
			return -EINVAL;
		}
	}

	argv += optind;

	switch (argc - optind)
	{
	case 0:
		server_ip = "127.0.0.1";
		server_port = FTP_CTRL_PORT;
		break;

	case 1:
		server_ip = argv[0];
		server_port = FTP_CTRL_PORT;
		break;

	default:
		server_ip = argv[0];
		server_port = text2value_unsigned(argv[1], NULL, 10);
	}

	return ftp_client_run(server_ip, server_port);
}
