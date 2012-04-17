/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Tue Apr 17 16:20:46 CST 2012
 */
#include <cavan.h>
#include <cavan/tcp_dd.h>
#include <cavan/parser.h>

#define FILE_CREATE_DATE "2012-04-17 14:23:55"

enum
{
	LOCAL_COMMAND_OPTION_UNKNOWN,
	LOCAL_COMMAND_OPTION_HELP,
	LOCAL_COMMAND_OPTION_VERSION,
};

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
			.val = LOCAL_COMMAND_OPTION_HELP,
		},
		{
			.name = "version",
			.has_arg = no_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_VERSION,
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
	int i;
	char ip[16] = "127.0.0.1";
	u16 port = TCP_DD_SERVER_PORT;
	char dest_file[1024], *pname;
	int (*handler)(const char *, u16, const char *, off_t, const char *, off_t, off_t);

	handler = NULL;

	while ((c = getopt_long(argc, argv, "vVhHi:I:p:P:wWsSrR", long_option, &option_index)) != EOF)
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
			show_usage();
			return 0;

		case 'i':
		case 'I':
			text_copy(ip, optarg);
			break;

		case 'p':
		case 'P':
			port = text2value_unsigned(optarg, NULL, 10);
			break;

		case 'w':
		case 'W':
		case 's':
		case 'S':
			handler = tcp_dd_send_file;
			break;

		case 'r':
		case 'R':
			handler = tcp_dd_receive_file;
			break;

		default:
			show_usage();
			return -EINVAL;
		}
	}

	if (handler == NULL)
	{
		pr_red_info("Please select action type");
		return -EINVAL;
	}

	assert(argc - optind > 1);

	pname = text_path_cat(dest_file, argv[--argc], NULL);

	for (i = optind; i < argc; i++)
	{
		int ret;

		text_basename_base(pname, argv[i]);

		println("%s => %s", argv[i], dest_file);

		ret = handler(ip, port, argv[i], 0, dest_file, 0, 0);
		if (ret < 0)
		{
			pr_red_info("Copy file %s to %s failed!", argv[i], dest_file);
			return ret;
		}
	}

	return 0;
}

