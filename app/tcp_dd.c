/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Sat Jan 14 14:09:55 CST 2012
 */

#include <cavan.h>
#include <cavan/tcp_dd.h>
#include <cavan/parser.h>

#define FILE_CREATE_DATE "2012-01-14 14:09:55"

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
	char ip[16];
	u16 port = 0;
	char src_file[1024], dest_file[1024];
	off_t bs, seek, skip, count;
	int (*handler)(const char *, u16, const char *, off_t, const char *, off_t, off_t);

	handler = NULL;
	ip[0] = 0;

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

	src_file[0] = dest_file[0] = 0;
	bs = 1;
	count = seek = skip = 0;

	for (i = optind; i < argc; i++)
	{
		char c, *p;

		parse_parameter(argv[i]);

		c = para_option[0];
		p = para_option + 1;

		switch (c)
		{
		case 'i':
			if (text_cmp(p, "f") == 0)
			{
				text_copy(src_file, para_value);
			}
			else if (text_cmp(p, "p") == 0)
			{
				text_copy(ip, para_value);
			}
			else
			{
				goto label__unknown_option;
			}
			break;

		case 'o':
			if (text_cmp(p, "f") == 0)
			{
				text_copy(dest_file, para_value);
				break;
			}
			goto label__unknown_option;

		case 'b':
			if (text_cmp(p, "s") == 0)
			{
				bs = text2size(para_value, NULL);
				break;
			}
			goto label__unknown_option;

		case 's':
			if (text_cmp(p, "kip") == 0)
			{
				skip = text2size(para_value, NULL);
			}
			else if (text_cmp(p, "eek") == 0)
			{
				seek = text2size(para_value, NULL);
			}
			else
			{
				goto label__unknown_option;
			}
			break;

		case 'c':
			if (text_cmp(p, "ount") == 0)
			{
				count = text2size(para_value, NULL);
				break;
			}
			goto label__unknown_option;

		case 'p':
			if (text_cmp(p, "ort") == 0)
			{
				port = text2value_unsigned(para_value, NULL, 10);
				break;
			}
			goto label__unknown_option;

		default:
label__unknown_option:
			pr_red_info("unknown option `%s'", para_option);
			return -EINVAL;
		}
	}

	if (src_file[0] == 0 || dest_file[0] == 0)
	{
		pr_red_info("Please input src_file and dest_file");
		return -EINVAL;
	}

	if (ip[0] == 0)
	{
		cavan_get_server_ip(ip);
	}

	if (port == 0)
	{
		port = cavan_get_server_port(TCP_DD_DEFAULT_PORT);
	}

	return handler(ip, port, src_file, skip * bs, dest_file, seek * bs, count * bs);
}
