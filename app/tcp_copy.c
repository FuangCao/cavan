/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Tue Apr 17 16:20:46 CST 2012
 */

#include <cavan.h>
#include <cavan/adb.h>
#include <cavan/tcp_dd.h>
#include <cavan/parser.h>
#include <cavan/command.h>

#define FILE_CREATE_DATE "2012-04-17 14:23:55"

static void show_usage(const char *command)
{
	println("Usage: %s [option] <-r|-w> src_files dest_dir", command);
	println("--help, -h, -H\t\t\t%s", cavan_help_message_help);
	println("--version, -v, -V\t\t%s", cavan_help_message_version);
	println("--ip, -i, -I [IP]\t\t%s", cavan_help_message_ip);
	println("--host [HOSTNAME]\t\t%s", cavan_help_message_hostname);
	println("--local, -l, -L\t\t\t%s", cavan_help_message_local);
	println("--port, -p, -P [PORT]\t\t%s", cavan_help_message_port);
	println("--adb, -a, -A\t\t\t%s", cavan_help_message_adb);
	println("--udp\t\t\t\t%s", cavan_help_message_udp);
	println("--unix, --unix-tcp [PATHNAME]\t%s", cavan_help_message_unix_tcp);
	println("--unix-udp [PATHNAME]\t\t%s", cavan_help_message_unix_udp);
	println("--url, -u, -U [URL]\t\t%s", cavan_help_message_url);
	println("-w, -W, -s, -S\t\t\t%s", cavan_help_message_send_file);
	println("-r, -R\t\t\t\t%s", cavan_help_message_recv_file);
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
			.name = "adb",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_ADB,
		},
		{
			.name = "udp",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_UDP,
		},
		{
			.name = "local",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_LOCAL,
		},
		{
			.name = "url",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_URL,
		},
		{
			.name = "host",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_HOST,
		},
		{
			.name = "unix",
			.has_arg = optional_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_UNIX,
		},
		{
			.name = "unix-tcp",
			.has_arg = optional_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_UNIX_TCP,
		},
		{
			.name = "unix-udp",
			.has_arg = optional_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_UNIX_UDP,
		},
		{
			0, 0, 0, 0
		},
	};
	int i;
	char *pname;
	struct network_url url;
	struct network_file_request file_req;
	int (*handler)(struct network_url *, struct network_file_request *) = NULL;

	network_url_init(&url, "tcp", NULL, TCP_DD_DEFAULT_PORT, TCP_DD_DEFAULT_SOCKET);

	while ((c = getopt_long(argc, argv, "vVhHi:I:p:P:wWsSrRAalLu:U:", long_option, &option_index)) != EOF)
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
			url.protocol = "adb";
		case 'l':
		case 'L':
		case CAVAN_COMMAND_OPTION_LOCAL:
			optarg = "127.0.0.1";
		case 'i':
		case 'I':
		case CAVAN_COMMAND_OPTION_IP:
		case CAVAN_COMMAND_OPTION_HOST:
			url.hostname = optarg;
			break;

		case CAVAN_COMMAND_OPTION_UDP:
			url.protocol = "udp";
			break;

		case 'p':
		case 'P':
		case CAVAN_COMMAND_OPTION_PORT:
			url.port = text2value_unsigned(optarg, NULL, 10);
			break;

		case 'u':
		case 'U':
		case CAVAN_COMMAND_OPTION_URL:
			if (network_url_parse(&url, optarg) == NULL)
			{
				pr_red_info("invalid url %s", optarg);
				return -EINVAL;
			}
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

		case CAVAN_COMMAND_OPTION_UNIX:
		case CAVAN_COMMAND_OPTION_UNIX_TCP:
			url.protocol = "unix-tcp";
			if (optarg)
			{
				url.pathname = optarg;
			}
			break;

		case CAVAN_COMMAND_OPTION_UNIX_UDP:
			url.protocol = "unix-udp";
			if (optarg)
			{
				url.pathname = optarg;
			}
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

	assert(argc - optind > 1);

	pname = text_path_cat(file_req.dest_file, sizeof(file_req.dest_file), argv[--argc], NULL);

	for (i = optind; i < argc; i++)
	{
		int ret;

		text_basename_base(pname, argv[i]);
		text_copy(file_req.src_file, argv[i]);
		file_req.src_offset = 0;
		file_req.dest_offset = 0;
		file_req.size = 0;

		println("%s => %s", argv[i], file_req.dest_file);

		ret = handler(&url, &file_req);
		if (ret < 0)
		{
			pr_red_info("Copy file %s to %s failed!", argv[i], file_req.dest_file);
			return ret;
		}
	}

	return 0;
}

