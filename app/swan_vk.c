// Fuang.Cao <cavan.cfa@gmail.com> Wed Aug 24 17:42:02 CST 2011

#include <cavan.h>
#include <cavan/swan_vk.h>

static void show_usage(void)
{
	println("Usage:");
}

int main(int argc, char *argv[])
{
	int c;
	int option_index;
	int command = 0;
	int serial = 0;
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
			.name = "command",
			.has_arg = no_argument,
			.flag = NULL,
			.val = 'c',
		},
		{
			.name = "serial",
			.has_arg = no_argument,
			.flag = NULL,
			.val = 's',
		},
		{
			.name = "usb",
			.has_arg = no_argument,
			.flag = NULL,
			.val = 'u',
		},
		{
			.name = "adb",
			.has_arg = no_argument,
			.flag = NULL,
			.val = 'u',
		},
		{
			.name = "daemon",
			.has_arg = no_argument,
			.flag = NULL,
			.val = 'd',
		},
		{
			.name = "ip",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 'i',
		},
		{
			.name = "port",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 'p',
		},
		{
		},
	};
	struct cavan_service_description desc =
	{
		.name = "SWAN_VK",
		.as_daemon = 0,
		.daemon_count = 5,
	};
	char ip[32] = "127.0.0.1";
	u16 port = SWAN_VK_TCP_PORT;

	while ((c = getopt_long(argc, argv, "vVhHCcSsUuDdP:p:I:i:", long_option, &option_index)) != EOF)
	{
		switch (c)
		{
		case 'v':
		case 'V':
			show_author_info();
			return 0;

		case 'h':
		case 'H':
			show_usage();
			return 0;

		case 'c':
		case 'C':
			command = 1;
			break;

		case 's':
		case 'S':
			serial = 1;
			break;

		case 'u':
		case 'U':
			serial = 0;
			break;

		case 'd':
		case 'D':
			desc.as_daemon = 1;
			break;

		case 'i':
		case 'I':
			text_copy(ip, optarg);
			break;

		case 'p':
		case 'P':
			port = text2value_unsigned(optarg, NULL, 10);
			break;

		default:
			show_usage();
			return -EINVAL;
		}
	}

	if (command)
	{
		if (argc > optind)
		{
			return swan_vk_commadline(argv[optind]);
		}
		else
		{
			return swan_vk_commadline(DEVICE_SWAN_VK_VALUE);
		}
	}

	if (access(DEVICE_SWAN_VK_DATA, F_OK) < 0)
	{
		if (serial)
		{
			return swan_vk_serial_client(argc > optind ? argv[optind] : NULL);
		}
		else
		{
			return swan_vk_adb_client(ip, port);
		}
	}
	else if (serial)
	{
		return swan_vk_serial_server(argc > optind ? argv[optind] : DEVICE_SWAN_TTY, DEVICE_SWAN_VK_DATA);
	}
	else
	{
		return swan_vk_adb_server(&desc, DEVICE_SWAN_VK_DATA, port);
	}
}
