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
	println("if=FILE\t\t\t\t%s", cavan_help_message_input_file);
	println("of=FILE\t\t\t\t%s", cavan_help_message_output_file);
	println("bs=BYTES\t\t\t%s", cavan_help_message_bs);
	println("seek=BLOCKS\t\t\t%s", cavan_help_message_seek);
	println("skip=BLOCKS\t\t\t%s", cavan_help_message_skip);
	println("-H, -h, --help\t\t\t%s", cavan_help_message_help);
	println("-V, -v, --version\t\t%s", cavan_help_message_version);
	println("-I, -i, --ip IP\t\t\t%s", cavan_help_message_ip);
	println("--host [HOSTNAME]\t\t%s", cavan_help_message_hostname);
	println("-L, ---locall\t\t\t%s", cavan_help_message_local);
	println("-p, --port PORT\t\t\t%s", cavan_help_message_port);
	println("-A, -a, --adb\t\t\t%s", cavan_help_message_adb);
	println("--udp\t\t\t\t%s", cavan_help_message_udp);
	println("--unix, --unix-tcp [PATHNAME]\t%s", cavan_help_message_unix_tcp);
	println("--unix-udp [PATHNAME]\t\t%s", cavan_help_message_unix_udp);
	println("-P, --pt, --protocol PROTOCOL\t%s", cavan_help_message_protocol);
	println("-U, -u, --url [URL]\t\t%s", cavan_help_message_url);
	println("-W, -S, -w, -s\t\t\t%s", cavan_help_message_send_file);
	println("-R, -r\t\t\t\t%s", cavan_help_message_recv_file);
	println("--system [IMAGE]\t\t%s", cavan_help_message_system);
	println("--userdata, --data [IMAGE]\t%s", cavan_help_message_userdata);
	println("--recovery [IMAGE]\t\t%s", cavan_help_message_recovery);
	println("--misc [IMAGE]\t\t\t%s", cavan_help_message_misc);
	println("--boot [IMAGE]\t\t\t%s", cavan_help_message_boot);
	println("--kernel [IMAGE]\t\t%s", cavan_help_message_kernel);
	println("--uboot [IMAGE]\t\t\t%s", cavan_help_message_uboot);
	println("--resource [IMAGE]\t\t%s", cavan_help_message_resource);
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
			.name = "protocol",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_PROTOCOL,
		},
		{
			.name = "pt",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_PROTOCOL,
		},
		{
			.name = "system",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_SYSTEM,
		},
		{
			.name = "userdata",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_USERDATA,
		},
		{
			.name = "data",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_USERDATA,
		},
		{
			.name = "recovery",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_RECOVERY,
		},
		{
			.name = "misc",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_MISC,
		},
		{
			.name = "boot",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_BOOT,
		},
		{
			.name = "kernel",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_KERNEL,
		},
		{
			.name = "uboot",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_UBOOT,
		},
		{
			.name = "resource",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_RESOURCE,
		},
		{
			0, 0, 0, 0
		},
	};
	const char *part_name;
	const char *part_image;
	struct network_url url;
	off_t bs, seek, skip, count;
	struct network_file_request file_req;
	int (*handler)(struct network_url *, struct network_file_request *) = NULL;

	part_name = part_image = NULL;
	file_req.src_file[0] = file_req.dest_file[0] = 0;

	network_url_init(&url, "tcp", NULL, TCP_DD_DEFAULT_PORT, CAVAN_NETWORK_SOCKET);

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

		case 'P':
		case CAVAN_COMMAND_OPTION_PROTOCOL:
			url.protocol = optarg;
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

		case CAVAN_COMMAND_OPTION_SYSTEM:
			part_name = "@SYSTEM@";
			part_image = optarg;
			break;

		case CAVAN_COMMAND_OPTION_USERDATA:
			part_name = "@USERDATA@";
			part_image = optarg;
			break;

		case CAVAN_COMMAND_OPTION_RECOVERY:
			part_name = "@RECOVERY@";
			part_image = optarg;
			break;

		case CAVAN_COMMAND_OPTION_MISC:
			part_name = "@MISC@";
			part_image = optarg;
			break;

		case CAVAN_COMMAND_OPTION_BOOT:
			part_name = "@BOOT@";
			part_image = optarg;
			break;

		case CAVAN_COMMAND_OPTION_KERNEL:
			part_name = "@KERNEL@";
			part_image = optarg;
			break;

		case CAVAN_COMMAND_OPTION_UBOOT:
			part_name = "@UBOOT@";
			part_image = optarg;
			break;

		case CAVAN_COMMAND_OPTION_RESOURCE:
			part_name = "@RESOURCE@";
			part_image = optarg;
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
				url.hostname = strdup(para_value);
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
				url.port = text2value_unsigned(para_value, NULL, 10);
				break;
			}
			goto label_parse_complete;

		default:
			goto label_parse_complete;
		}
	}

label_parse_complete:
	if (part_name && part_image)
	{
		if (handler == tcp_dd_send_file)
		{
			text_copy(file_req.src_file, part_image);
			text_copy(file_req.dest_file, part_name);
		}
		else
		{
			text_copy(file_req.src_file, part_name);
			text_copy(file_req.dest_file, part_image);
		}
	}

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

	return handler(&url, &file_req);
}
