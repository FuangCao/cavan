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
	println("--help, -h, -H\t\tshow this help");
	println("--version, -v, -V\tshow version");
	println("--ip, -i, -I\t\tserver ip address");
	println("--local, -l, -L\t\tuse localhost ip");
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
			.name = "local",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_LOCAL,
		},
		{
			0, 0, 0, 0
		},
	};
	struct inet_file_request file_req =
	{
		.open_connect = inet_create_tcp_link2,
		.close_connect = inet_close_tcp_socket
	};
	int i;
	char *pname;
	int (*handler)(struct inet_file_request *) = NULL;

	cavan_get_server_ip(file_req.ip);
	file_req.port = cavan_get_server_port(TCP_DD_DEFAULT_PORT);

	while ((c = getopt_long(argc, argv, "vVhHi:I:p:P:wWsSrRAalL", long_option, &option_index)) != EOF)
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
			file_req.open_connect = adb_create_tcp_link2;
		case 'l':
		case 'L':
		case CAVAN_COMMAND_OPTION_LOCAL:
			optarg = "127.0.0.1";
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

		ret = handler(&file_req);
		if (ret < 0)
		{
			pr_red_info("Copy file %s to %s failed!", argv[i], file_req.dest_file);
			return ret;
		}
	}

	return 0;
}

