/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Tue Jan 29 13:54:19 CST 2013
 */

#include <cavan.h>
#include <cavan/sprd_diag.h>

#define FILE_CREATE_DATE "2013-01-29 13:54:19"

enum
{
	LOCAL_COMMAND_OPTION_UNKNOWN,
	LOCAL_COMMAND_OPTION_HELP,
	LOCAL_COMMAND_OPTION_VERSION,
};

static void show_usage(const char *command)
{
	println("Usage: %s [option] [devpath] command [argument]", command);
	println("--help, -h, -H\t\tshow this help");
	println("--version, -v, -V\tshow version");
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
			0, 0, 0, 0
		},
	};
	int fd;
	int ret;
	ssize_t rwlen;
	char reply[1024];
	const char *devpath = SPRD_DEFAULT_AT_DEVICE;

	while ((c = getopt_long(argc, argv, "vVhH", long_option, &option_index)) != EOF)
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

		default:
			show_usage(argv[0]);
			return -EINVAL;
		}
	}

	if (optind + 1 < argc)
	{
		devpath = argv[optind++];
	}
	else if (optind >= argc)
	{
		show_usage(argv[0]);
		return -EINVAL;
	}

	fd = open(devpath, O_RDWR | O_SYNC);
	if (fd < 0)
	{
		pr_error_info("open %s", devpath);
		return fd;
	}

	rwlen = sprd_modem_send_at_command(fd, reply, sizeof(reply), "%s\r", argv[optind++]);
	if (rwlen < 0)
	{
		ret = rwlen;
		pr_red_info("sprd_modem_send_at_command");
		goto out_close_fd;
	}

	if (optind < argc)
	{
		rwlen = sprd_modem_send_at_command(fd, reply, sizeof(reply), "%s\032", argv[optind++]);
		if (rwlen < 0)
		{
			ret = rwlen;
			pr_red_info("sprd_modem_send_at_command");
			goto out_close_fd;
		}
	}

	ret = 0;
out_close_fd:
	close(fd);
	return ret;
}
