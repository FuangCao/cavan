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
			.name = "user",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 'u',
		},
		{
			.name = "username",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 'u',
		},
		{
			.name = "pass",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 'p',
		},
		{
			.name = "password",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 'p',
		},
		{
			0, 0, 0, 0
		},
	};
	struct network_url url;
	const char *username = NULL;
	const char *password = NULL;

	while ((c = getopt_long(argc, argv, "vVhHu:U:p:P:", long_option, &option_index)) != EOF)
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

		case 'u':
		case 'U':
			username = optarg;
			break;

		case 'p':
		case 'P':
			password = optarg;
			break;

		default:
			show_usage();
			return -EINVAL;
		}
	}

	argv += optind;

	switch (argc - optind)
	{
	case 0:
		url.hostname = "127.0.0.1";
		url.port = FTP_CTRL_PORT;
		break;

	case 1:
		url.hostname = argv[0];
		url.port = FTP_CTRL_PORT;
		break;

	default:
		url.hostname = argv[0];
		url.port = text2value_unsigned(argv[1], NULL, 10);
	}

	url.protocol = "ftp";
	url.pathname = NULL;

	return ftp_client_run(&url, username, password);
}
