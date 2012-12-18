/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Sat Jan 14 14:09:55 CST 2012
 */

#include <cavan.h>
#include <cavan/tcp_dd.h>
#include <cavan/parser.h>
#include <cavan/adb.h>

#define FILE_CREATE_DATE "2012-01-14 14:09:55"

enum
{
	LOCAL_COMMAND_OPTION_UNKNOWN,
	LOCAL_COMMAND_OPTION_HELP,
	LOCAL_COMMAND_OPTION_VERSION,
};

static void show_usage(const char *command)
{
	println("Usage:");
	println("%s [option] if=input of=output", command);
	println("if=FILE\t\t\tinput file");
	println("of=FILE\t\t\toutput file");
	println("bs=BYTES\t\tblock size");
	println("seek=BLOCKS\t\toffset of output");
	println("skip=BLOCKS\t\toffset of input");
	println("--help, -h, -H\t\tshow this help");
	println("--version, -v, -V\tshow version");
	println("--ip, -i, -I\t\tserver ip address");
	println("--port, -p, -P\t\tserver port");
	println("--adb, -a, -A\t\tuse adb procotol instead of tcp");
	println("-w, -W, -s, -S\t\tsend file");
	println("-r, -R\t\t\treceive file");
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
			.name = "adb",
			.has_arg = no_argument,
			.flag = NULL,
			.val = 'a',
		},
		{
			0, 0, 0, 0
		},
	};
	struct inet_file_request file_req =
	{
		.open_connect = inet_create_tcp_link2,
		.close_connect = inet_close_tcp_socket,
	};
	int i;
	off_t bs, seek, skip, count;
	int (*handler)(struct inet_file_request *) = NULL;

	cavan_get_server_ip(file_req.ip);
	file_req.port = cavan_get_server_port(TCP_DD_DEFAULT_PORT);

	while ((c = getopt_long(argc, argv, "vVhHi:I:p:P:wWsSrRaA", long_option, &option_index)) != EOF)
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

		case 'i':
		case 'I':
			text_copy(file_req.ip, optarg);
			break;

		case 'p':
		case 'P':
			file_req.port = text2value_unsigned(optarg, NULL, 10);
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

		case 'a':
		case 'A':
			file_req.open_connect = adb_create_tcp_link2;
			break;

		default:
			show_usage(argv[0]);
			return -EINVAL;
		}
	}

	if (handler == NULL)
	{
		pr_red_info("Please select action type");
		return -EINVAL;
	}

	file_req.src_file[0] = file_req.dest_file[0] = 0;
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
				text_copy(file_req.src_file, para_value);
			}
			else if (text_cmp(p, "p") == 0)
			{
				text_copy(file_req.ip, para_value);
			}
			else
			{
				goto label_unknown_option;
			}
			break;

		case 'o':
			if (text_cmp(p, "f") == 0)
			{
				text_copy(file_req.dest_file, para_value);
				break;
			}
			goto label_unknown_option;

		case 'b':
			if (text_cmp(p, "s") == 0)
			{
				bs = text2size(para_value, NULL);
				break;
			}
			goto label_unknown_option;

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
				goto label_unknown_option;
			}
			break;

		case 'c':
			if (text_cmp(p, "ount") == 0)
			{
				count = text2size(para_value, NULL);
				break;
			}
			goto label_unknown_option;

		case 'p':
			if (text_cmp(p, "ort") == 0)
			{
				file_req.port = text2value_unsigned(para_value, NULL, 10);
				break;
			}
			goto label_unknown_option;

		default:
label_unknown_option:
			pr_red_info("unknown option `%s'", para_option);
			return -EINVAL;
		}
	}

	if (file_req.src_file[0] == 0 || file_req.dest_file[0] == 0)
	{
		pr_red_info("Please input src_file and dest_file");
		return -EINVAL;
	}

	file_req.src_offset = skip * bs;
	file_req.dest_offset = seek * bs;
	file_req.size = count * bs;

	return handler(&file_req);
}
