/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Sat Jan 14 14:09:55 CST 2012
 */

#include <cavan.h>
#include <cavan/adb.h>
#include <cavan/tcp_dd.h>
#include <cavan/parser.h>
#include <cavan/command.h>

#define FILE_CREATE_DATE "2012-01-14 14:09:55"

static void show_usage(const char *command)
{
	println("Usage: %s [option] <-w|-r> if=input of=output [src] [dest]", command);
	println("if=FILE\t\t\tinput file");
	println("of=FILE\t\t\toutput file");
	println("bs=BYTES\t\tblock size");
	println("seek=BLOCKS\t\toffset of output");
	println("skip=BLOCKS\t\toffset of input");
	println("--help, -h, -H\t\tshow this help");
	println("--version, -v, -V\tshow version");
	println("--ip, -i, -I\t\tserver ip address");
	println("--local, -l, -L\t\tuse localhost ip");
	println("--port, -p, -P\t\tserver port");
	println("--adb, -a, -A\t\tuse adb procotol instead of tcp");
	println("--url, -u, -A [URL]\t\tservice url");
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
			.val = CAVAN_COMMAND_OPTION_HELP,
		},
		{
			.name = "version",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_VERSION,
		},
		{
			.name = "ip",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_IP,
		},
		{
			.name = "port",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_PORT,
		},
		{
			.name = "url",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_URL,
		},
		{
			.name = "adb",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_ADB,
		},
		{
			.name = "local",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_LOCAL,
		},
		{
			0, 0, 0, 0
		},
	};
	u16 port;
	const char *url;
	char url_buff[1024];
	const char *protocol;
	const char *hostname;
	off_t bs, seek, skip, count;
	struct network_file_request file_req;
	int (*handler)(struct network_file_request *, const char *) = NULL;

	url = NULL;
	protocol = "tcp";
	hostname = cavan_get_server_hostname();
	port = cavan_get_server_port(TCP_DD_DEFAULT_PORT);

	while ((c = getopt_long(argc, argv, "vVhHi:I:p:P:wWsSrRaAlLu:U:", long_option, &option_index)) != EOF)
	{
		switch (c)
		{
		case 'v':
		case 'V':
		case CAVAN_COMMAND_OPTION_VERSION:
			show_author_info();
			println(FILE_CREATE_DATE);
			return 0;

		case 'h':
		case 'H':
		case CAVAN_COMMAND_OPTION_HELP:
			show_usage(argv[0]);
			return 0;

		case 'a':
		case 'A':
		case CAVAN_COMMAND_OPTION_ADB:
			protocol = "adb";
		case 'l':
		case 'L':
		case CAVAN_COMMAND_OPTION_LOCAL:
			optarg = "127.0.0.1";
		case 'i':
		case 'I':
		case CAVAN_COMMAND_OPTION_IP:
			hostname = optarg;
			break;

		case 'p':
		case 'P':
		case CAVAN_COMMAND_OPTION_PORT:
			port = text2value_unsigned(optarg, NULL, 10);
			break;

		case 'u':
		case 'U':
		case CAVAN_COMMAND_OPTION_URL:
			url = optarg;
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
			show_usage(argv[0]);
			return -EINVAL;
		}
	}

	if (handler == NULL)
	{
		pr_red_info("Please select action type");
		return -EINVAL;
	}

	if (url == NULL)
	{
		network_url_build(url_buff, sizeof(url_buff), protocol, hostname, port, NULL);
		url = url_buff;
	}

	file_req.src_file[0] = file_req.dest_file[0] = 0;

	for (bs = 1, count = seek = skip = 0; optind < argc; optind++)
	{
		char c, *p;

		parse_parameter(argv[optind]);

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
				hostname = strdup(para_value);
			}
			else
			{
				goto label_parse_complete;
			}
			break;

		case 'o':
			if (text_cmp(p, "f") == 0)
			{
				text_copy(file_req.dest_file, para_value);
				break;
			}
			goto label_parse_complete;

		case 'b':
			if (text_cmp(p, "s") == 0)
			{
				bs = text2size(para_value, NULL);
				break;
			}
			goto label_parse_complete;

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
				goto label_parse_complete;
			}
			break;

		case 'c':
			if (text_cmp(p, "ount") == 0)
			{
				count = text2size(para_value, NULL);
				break;
			}
			goto label_parse_complete;

		case 'p':
			if (text_cmp(p, "ort") == 0)
			{
				port = text2value_unsigned(para_value, NULL, 10);
				break;
			}
			goto label_parse_complete;

		default:
			goto label_parse_complete;
		}
	}

label_parse_complete:
	switch (argc - optind)
	{
	case 2:
		text_copy(file_req.src_file, argv[optind++]);
	case 1:
		text_copy(file_req.dest_file, argv[optind++]);
	case 0:
		break;

	default:
		show_usage(argv[0]);
		return -EINVAL;
	}

	if (file_req.src_file[0] == 0 || file_req.dest_file[0] == 0)
	{
		pr_red_info("Please input src_file and dest_file");
		return -EINVAL;
	}

	file_req.src_offset = skip * bs;
	file_req.dest_offset = seek * bs;
	file_req.size = count * bs;

	return handler(&file_req, url);
}
