/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Sat Jan 14 14:09:55 CST 2012
 */

#include <cavan.h>
#include <cavan/adb.h>
#include <cavan/image.h>
#include <cavan/tcp_dd.h>
#include <cavan/parser.h>
#include <cavan/command.h>

#define FILE_CREATE_DATE		"2012-01-14 14:09:55"
#define TCP_DD_MAX_IMAGE_COUNT	10

struct cavan_tcp_dd_image
{
	const char *name;
	const char *pathname;
};

static const struct option command_long_option[] =
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
		.name = "image",
		.has_arg = required_argument,
		.flag = NULL,
		.val = CAVAN_COMMAND_OPTION_IMAGE,
	},
	{
		.name = "auto",
		.has_arg = no_argument,
		.flag = NULL,
		.val = CAVAN_COMMAND_OPTION_AUTO,
	},
	{
		0, 0, 0, 0
	},
};

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
	println("-i, --ip IP\t\t\t%s", cavan_help_message_ip);
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
	println("--image [s|d|r|m|b|k|u|R]\t%s", cavan_help_message_rw_image);
	println("-I, --auto <PATHNAMES>\t\t%s ", cavan_help_message_rw_image_auto);
}

int main(int argc, char *argv[])
{
	int c;
	int ret;
	int option_index;
	int image_count;
	const char *part_name;
	const char *image_mask;
	struct network_url url;
	off_t bs, seek, skip, count;
	struct network_file_request file_req;
	struct cavan_tcp_dd_image images[TCP_DD_MAX_IMAGE_COUNT];
	int (*handler)(struct network_url *, struct network_file_request *) = NULL;

	image_count = 0;
	image_mask = NULL;
	file_req.src_file[0] = file_req.dest_file[0] = 0;

	network_url_init(&url, "tcp", NULL, TCP_DD_DEFAULT_PORT, network_get_socket_pathname());

	while ((c = getopt_long(argc, argv, "vVhHi:p:P:wWsSrRaAlLu:U:k:b:I", command_long_option, &option_index)) != EOF)
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
label_add_image:
			if (image_count < NELEM(images))
			{
				images[image_count].name = part_name;
				images[image_count].pathname = optarg;
			}
			else
			{
				pr_red_info("Too much image");
				return -ENOMEM;
			}

			image_count++;
			break;

		case CAVAN_COMMAND_OPTION_USERDATA:
			part_name = "@USERDATA@";
			goto label_add_image;

		case CAVAN_COMMAND_OPTION_RECOVERY:
			part_name = "@RECOVERY@";
			goto label_add_image;

		case CAVAN_COMMAND_OPTION_MISC:
			part_name = "@MISC@";
			goto label_add_image;

		case 'b':
		case CAVAN_COMMAND_OPTION_BOOT:
			part_name = "@BOOT@";
			goto label_add_image;

		case 'k':
		case CAVAN_COMMAND_OPTION_KERNEL:
			part_name = "@KERNEL@";
			goto label_add_image;

		case CAVAN_COMMAND_OPTION_UBOOT:
			part_name = "@UBOOT@";
			goto label_add_image;

		case CAVAN_COMMAND_OPTION_RESOURCE:
			part_name = "@RESOURCE@";
			goto label_add_image;

		case CAVAN_COMMAND_OPTION_IMAGE:
			image_mask = optarg;
			break;

		case 'I':
		case CAVAN_COMMAND_OPTION_AUTO:
			image_mask = "-";
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
	if (image_mask)
	{

		if (argc - optind < 1)
		{
			pr_red_info("Too a few argument!");
			return -EINVAL;
		}

		if (image_mask[0] == '-')
		{
			while (optind < argc)
			{
				const char *pathname = argv[optind];

				part_name = image_path_to_part_name(pathname);
				if (part_name == NULL)
				{
					pr_red_info("unknown image %s", pathname);
					return -EINVAL;
				}

				if (handler == tcp_dd_send_file)
				{
					strcpy(file_req.src_file, pathname);
					strcpy(file_req.dest_file, part_name);
				}
				else
				{
					strcpy(file_req.src_file, part_name);
					strcpy(file_req.dest_file, pathname);
				}

				file_req.size = 0;
				file_req.dest_offset = file_req.src_offset = 0;

				ret = handler(&url, &file_req);
				if (ret < 0)
				{
					return ret;
				}

				optind++;
			}
		}
		else
		{
			const char *dirname;
			const char *image_name;

			dirname = argv[optind++];

			while (*image_mask)
			{
				c = *image_mask;

				switch (c)
				{
				case 's':
					part_name = "@SYSTEM@";
					image_name = "system.img";
					break;

				case 'd':
					part_name = "@USERDATA@";
					image_name = "userdata.img";
					break;

				case 'r':
					part_name = "@RECOVERY@";
					image_name = "recovery.img";
					break;

				case 'm':
					part_name = "@MISC@";
					image_name = "misc.img";
					break;

				case 'b':
					part_name = "@BOOT@";
					image_name = "boot.img";
					break;

				case 'k':
					part_name = "@KERNEL@";
					image_name = "kernel.img";
					break;

				case 'u':
					part_name = "@UBOOT@";
					image_name = "uboot.bin";
					break;

				case 'R':
					part_name = "@RESOURCE@";
					image_name = "resource.img";
					break;

				default:
					pr_red_info("invalid image type `%c'", c);
					return -EINVAL;
				}

				if (handler == tcp_dd_send_file)
				{
					text_path_cat(file_req.src_file, sizeof(file_req.src_file), dirname, image_name);
					strcpy(file_req.dest_file, part_name);
				}
				else
				{
					strcpy(file_req.src_file, part_name);
					text_path_cat(file_req.dest_file, sizeof(file_req.src_file), dirname, image_name);
				}

				file_req.size = 0;
				file_req.dest_offset = file_req.src_offset = 0;

				ret = handler(&url, &file_req);
				if (ret < 0)
				{
					return ret;
				}

				image_mask++;
			}
		}
	}
	else
	{
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

		if (file_req.src_file[0] && file_req.dest_file[0])
		{
			file_req.src_offset = skip * bs;
			file_req.dest_offset = seek * bs;
			file_req.size = count * bs;

			ret = handler(&url, &file_req);
			if (ret < 0)
			{
				return ret;
			}
		}
		else if (image_count == 0)
		{
			pr_red_info("Please input src_file and dest_file");
			return -EINVAL;
		}
	}

	if (image_count > 0)
	{
		struct cavan_tcp_dd_image *p, *p_end;

		for (p = images, p_end = p + image_count; p < p_end; p++)
		{
			if (handler == tcp_dd_send_file)
			{
				strcpy(file_req.src_file, p->pathname);
				strcpy(file_req.dest_file, p->name);
			}
			else
			{
				strcpy(file_req.src_file, p->name);
				strcpy(file_req.dest_file, p->pathname);

				if (file_is_directory(p->pathname))
				{
					char *filename;

					for (filename = file_req.dest_file; *filename; filename++);

					*filename++ = '/';

					ret = tcp_dd_get_partition_filename(p->name, filename, sizeof(file_req.dest_file));
					if (ret <= 0)
					{
						pr_red_info("tcp_dd_get_partition_filename %s", p->name);
						return -EINVAL;
					}

					while (*filename)
					{
						filename++;
					}

					strcpy(filename, ".img");
				}
			}

			file_req.size = 0;
			file_req.dest_offset = file_req.src_offset = 0;

			ret = handler(&url, &file_req);
			if (ret < 0)
			{
				return ret;
			}
		}
	}

	return 0;
}
