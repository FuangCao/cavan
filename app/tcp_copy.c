#define CAVAN_CMD_NAME tcp_copy

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Tue Apr 17 16:20:46 CST 2012
 */

#include <cavan.h>
#include <cavan/adb.h>
#include <cavan/file.h>
#include <cavan/tcp_dd.h>
#include <cavan/parser.h>
#include <cavan/command.h>

#define FILE_CREATE_DATE "2012-04-17 14:23:55"

static void show_usage(const char *command)
{
	println("Usage: %s [option] <-r|-w> src_files dest_dir", command);
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
	println("-w, -W\t\t\t\t%s", cavan_help_message_send_file);
	println("-r, -R\t\t\t\t%s", cavan_help_message_recv_file);
	println("-s, -S, --same\t\t\t%s", "local and remote use same path");
	println("-f, -o, --force, --override\t%s", "force override");
}

int main(int argc, char *argv[])
{
	int c;
	int option_index;
	struct option long_option[] = {
		{
			.name = "help",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_HELP,
		}, {
			.name = "version",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_VERSION,
		}, {
			.name = "ip",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_IP,
		}, {
			.name = "port",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_PORT,
		}, {
			.name = "adb",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_ADB,
		}, {
			.name = "udp",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_UDP,
		}, {
			.name = "local",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_LOCAL,
		}, {
			.name = "url",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_URL,
		}, {
			.name = "host",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_HOST,
		}, {
			.name = "unix",
			.has_arg = optional_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_UNIX,
		}, {
			.name = "unix-tcp",
			.has_arg = optional_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_UNIX_TCP,
		}, {
			.name = "unix-udp",
			.has_arg = optional_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_UNIX_UDP,
		}, {
			.name = "protocol",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_PROTOCOL,
		}, {
			.name = "pt",
			.has_arg = required_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_PROTOCOL,
		}, {
			.name = "same",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_SAME,
		}, {
			.name = "force",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_FORCE,
		}, {
			.name = "override",
			.has_arg = no_argument,
			.flag = NULL,
			.val = CAVAN_COMMAND_OPTION_OVERRIDE,
		}, {
			0, 0, 0, 0
		},
	};
	int i;
	int ret;
	int count;
	char *filename;
	bool same = false;
	struct network_url url;
	tcp_dd_handler_t handler = NULL;
	struct network_file_request file_req;
	u32 flags = TCP_DDF_BREAKPOINT_RESUME;

	network_url_init(&url, "tcp", NULL, TCP_DD_DEFAULT_PORT, network_get_socket_pathname());

	while ((c = getopt_long(argc, argv, "vVhHi:I:p:P:wWsSrRAalLu:U:fo", long_option, &option_index)) != EOF) {
		switch (c) {
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
			if (network_url_parse(&url, optarg) == NULL) {
				pr_red_info("invalid url %s", optarg);
				return -EINVAL;
			}
			break;

		case 'w':
		case 'W':
			handler = tcp_dd_send_file;
			break;

		case 'r':
		case 'R':
			handler = tcp_dd_receive_file;
			break;

		case CAVAN_COMMAND_OPTION_UNIX:
		case CAVAN_COMMAND_OPTION_UNIX_TCP:
			url.protocol = "unix-tcp";
			if (optarg) {
				url.pathname = optarg;
			}
			break;

		case CAVAN_COMMAND_OPTION_UNIX_UDP:
			url.protocol = "unix-udp";
			if (optarg) {
				url.pathname = optarg;
			}
			break;

		case 'P':
		case CAVAN_COMMAND_OPTION_PROTOCOL:
			url.protocol = optarg;
			break;

		case 's':
		case 'S':
		case CAVAN_COMMAND_OPTION_SAME:
			same = true;
			break;

		case 'f':
		case 'o':
		case CAVAN_COMMAND_OPTION_FORCE:
		case CAVAN_COMMAND_OPTION_OVERRIDE:
			flags &= ~TCP_DDF_BREAKPOINT_RESUME;
			break;

		default:
			show_usage(argv[0]);
			return -EINVAL;
		}
	}

	if (handler == NULL) {
		pr_red_info("Please select action type");
		return -EINVAL;
	}

	count = argc - optind;
	if (count < 1) {
		show_usage(argv[0]);
		return -EINVAL;
	}

	if (count == 1 || same) {
		filename = NULL;
	} else {
		filename = cavan_path_copy(file_req.dest_file, sizeof(file_req.dest_file), argv[--argc], true);
	}

	for (i = optind; i < argc; i++) {
		if (filename) {
			cavan_path_basename(filename, argv[i]);
			text_copy(file_req.src_file, argv[i]);
		} else {
			file_abs_path_simple(argv[i], file_req.src_file, sizeof(file_req.src_file), true);
			strncpy(file_req.dest_file, file_req.src_file, sizeof(file_req.dest_file));
		}

		file_req.src_offset = 0;
		file_req.dest_offset = 0;
		file_req.size = 0;

		// println("%s => %s", file_req.src_file, file_req.dest_file);

		ret = handler(&url, &file_req, flags);
		if (ret < 0) {
			pr_red_info("Failed to copy file %s to %s", file_req.src_file, file_req.dest_file);
			return ret;
		}
	}

	return 0;
}
