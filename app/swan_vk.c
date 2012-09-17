// Fuang.Cao <cavan.cfa@gmail.com> Wed Aug 24 17:42:02 CST 2011

#include <cavan.h>
#include <cavan/input.h>
#include <cavan/file.h>
#include <cavan/event.h>
#include <poll.h>

#define SWAN_VK_PROMPT		"SWAN-VK"
#define DEVICE_SWAN_VK_DATA "/sys/devices/platform/swan_vk.0/data"
#define DEVICE_SWAN_VK_VALUE "/sys/devices/platform/swan_vk.0/value"
#define DEVICE_SWAN_TTY		"/dev/ttymxc0"
#define SWAN_VK_STOP_VALUE	0x504F5453

struct swan_vk_descriptor
{
	char *name;
	unsigned int code;
};

static const struct swan_vk_descriptor swan_vk_table[] =
{
	{"search", 217},
	{"back", 158},
	{"menu", 229},
	{"home", 102},
	{"center", 232},
	{"up", 103},
	{"down", 108},
	{"left", 105},
	{"right", 106},
	{"space", 57},
	{"tab", 15},
	{"enter", 28},
	{"power", 116},
	{"call", 61},
	{"endcall", 62},
	{"grave", 399},
	{"pound", 228},
	{"star", 227},
	{"vup", 115},
	{"vdown", 114},
	{"camera", 212},
	{"backslash", 43},
	{"del", 14}
};

// ================================================================================

static int client_active;

static void show_usage(void)
{
	println("Usage:");
}

void swan_vk_serial_client_stop_handle(int signum)
{
	pr_bold_pos();
	client_active = 0;
}

static void swan_vk_serial_send_stop_signal(int fd_tty, u32 value, int count)
{
	while (count-- && write(fd_tty, (void *)&value, sizeof(value)) > 0)
	{
		msleep(100);
	}
}

static int swan_vk_serial_client(const char *tty_path)
{
	int count;
	int fd_tty;
	struct pollfd event_fds[16];
	struct input_event events[32];
	ssize_t readlen, writelen;

	pr_bold_pos();

	fd_tty = -1;

	if (tty_path)
	{
		fd_tty = open(tty_path, O_WRONLY | O_SYNC | O_NOCTTY);
	}

	if (fd_tty < 0)
	{
		tty_path = (char *)events;
		fd_tty = file_find_and_open("/dev/ttyUSB", (char *)tty_path, 0, 10, O_WRONLY | O_SYNC);
	}

	if (fd_tty < 0)
	{
		error_msg("open tty device failed");
		return fd_tty;
	}

	println("ttypath = %s", tty_path);

	count = open_event_devices(event_fds, ARRAY_SIZE(event_fds), O_RDONLY);
	if (count <= 0)
	{
		error_msg("no input device found");
		goto out_close_tty;
	}

	signal(SIGINT, swan_vk_serial_client_stop_handle);
	client_active = 1;

	while (client_active)
	{
		readlen = poll_event_devices(event_fds, count, events, sizeof(events));
		if (readlen < 0)
		{
			error_msg("poll_event_devices");
			break;
		}

		writelen = write(fd_tty, events, readlen);
		if (writelen < 0)
		{
			error_msg("write");
			break;
		}
	}

	swan_vk_serial_send_stop_signal(fd_tty, SWAN_VK_STOP_VALUE, 2);
	close_event_devices(event_fds, count);
out_close_tty:
	close(fd_tty);

	return -1;
}

// ================================================================================

static void swan_vk_serial_server_stop_handle(int signum)
{
	restore_tty_attr(-1);
	exit(0);
}

static int swan_vk_serial_server(const char *tty_path, const char *data_path)
{
	int ret;
	int fd_tty, fd_data;
	struct input_event events[32];
	ssize_t readlen, writelen;

	pr_bold_pos();

	fd_tty = open(tty_path, O_RDONLY | O_NOCTTY);
	if (fd_tty < 0)
	{
		print_error("open tty device \"%s\" failed", tty_path);
		return fd_tty;
	}

	ret = set_tty_mode(fd_tty, 3);
	if (ret < 0)
	{
		print_error("set_tty_mode");
		goto out_close_tty;
	}

	signal(SIGKILL, swan_vk_serial_server_stop_handle);

	fd_data = open(data_path, O_WRONLY);
	if (fd_data < 0)
	{
		print_error("open data device \"%s\" failed", data_path);
		goto out_close_tty;
	}

	while (1)
	{
		readlen = read(fd_tty, events, sizeof(events));
		if (readlen < 0)
		{
			print_error("read");
			break;
		}

		if (*(u32 *)events == SWAN_VK_STOP_VALUE)
		{
			break;
		}

		writelen = write(fd_data, events, readlen);
		if (writelen < 0)
		{
			print_error("write");
			break;
		}
	}

	restore_tty_attr(fd_tty);

	close(fd_data);
out_close_tty:
	close(fd_tty);

	pr_bold_info("%s exit", __FUNCTION__);

	return -1;
}

// ================================================================================

static int swan_vk_adb_client(void)
{
	pr_pos_info();

	return 0;
}

static int swan_vk_adb_server(const char *data_path)
{
	pr_pos_info();

	return 0;
}

// ================================================================================

static void swan_vk_show_key_table(const struct swan_vk_descriptor *descs, size_t size)
{
	const struct swan_vk_descriptor *desc_end;

	for (desc_end = descs + size; descs < desc_end; descs++)
	{
		println("%s = %d", descs->name, descs->code);
	}
}

static const struct swan_vk_descriptor *swan_vk_match_key(const char *keyname, const struct swan_vk_descriptor *descs, size_t size)
{
	const struct swan_vk_descriptor *desc_end;

	for (desc_end = descs + size; descs < desc_end && text_lhcmp(keyname, descs->name); descs++);

	if (descs < desc_end)
	{
		return descs;
	}

	return NULL;
}

static int swan_vk_commadline(const char *data_path)
{
	int fd_data;
	char name[1024], *name_p;
	const struct swan_vk_descriptor *p, *old_p;
	ssize_t writelen;

	fd_data = open(data_path, O_WRONLY);
	if (fd_data < 0)
	{
		print_error("open file \"%s\" failed", data_path);
		return fd_data;
	}

	old_p = swan_vk_table;

	while (1)
	{
		print("\033[01;32m" SWAN_VK_PROMPT "\033[0m# ");

		for (name_p = name; is_empty_character((*name_p = getchar())) == 0; name_p++);

		if (name_p == name)
		{
			goto label_repo_key;
		}

		*name_p = 0;

		p = swan_vk_match_key(name, swan_vk_table, ARRAY_SIZE(swan_vk_table));
		if (p == NULL)
		{
			swan_vk_show_key_table(swan_vk_table, ARRAY_SIZE(swan_vk_table));
			continue;
		}

		old_p = p;

label_repo_key:
		pr_bold_info("repo key = %s = %d", old_p->name, old_p->code);

		writelen = write(fd_data, (void *)&old_p->code, sizeof(old_p->code));
		if (writelen < 0)
		{
			print_error("write");
			break;
		}
	}

	close(fd_data);

	return -1;
}

// ================================================================================

int main(int argc, char *argv[])
{
	int c;
	int option_index;
	int command;
	int serial;
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
		},
	};

	command = 0;
	serial = 0;

	while ((c = getopt_long(argc, argv, "vVhHCcSsUu", long_option, &option_index)) != EOF)
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
			return swan_vk_adb_client();
		}
	}
	else if (serial)
	{
		return swan_vk_serial_server(argc > optind ? argv[optind] : DEVICE_SWAN_TTY, DEVICE_SWAN_VK_DATA);
	}
	else
	{
		return swan_vk_adb_server(DEVICE_SWAN_VK_DATA);
	}
}
