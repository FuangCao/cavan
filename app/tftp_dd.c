// Fuang.Cao <cavan.cfa@gmail.com> Thu Mar 31 12:06:19 CST 2011

#include <cavan.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <cavan/tftp.h>
#include <cavan/progress.h>
#include <cavan/parser.h>
#include <cavan/text.h>
#include <cavan/file.h>
#include <cavan/device.h>

static void show_usage(void)
{
	println("Usage:");
	println("tftp_dd [--ip=ip] [--port=port] -w if=local_file of=remote_file");
	println("tftp_dd [--ip=ip] [--port=port] -r if=remote_file of=local_file");
}

int main(int argc, char *argv[])
{
	int i;
	int c;
	int ret;
	const char *hostname;
	u16 port = 0;
	char input_file[128];
	char output_file[128];
	u32 bs = 1, seek = 0, skip = 0, count = 0;
	int (*handle)(const char *, u16, const char *, const char *, u32, u32, u32);
	int option_index;
	struct option long_options[] =
	{
		{
			.name = "ip",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 0,
		},
		{
			.name = "port",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 1,
		},
		{
			0, 0, 0, 0
		},
	};

	port = 0;
	hostname = NULL;
	handle = NULL;

	while ((c = getopt_long(argc, argv, "rRgGwWsSpPhH", long_options, &option_index)) != EOF)
	{
		switch (c)
		{
			case 0:
				hostname = optarg;
				break;

			case 1:
				port = text2value(optarg, NULL, 10);
				break;

			case 'r':
			case 'R':
			case 'g':
			case 'G':
				handle = tftp_client_receive_file;
				break;

			case 'w':
			case 'W':
			case 's':
			case 'S':
			case 'p':
			case 'P':
				handle = tftp_client_send_file;
				break;

			case 'h':
			case 'H':
				show_usage();
				return 0;

			default:
				error_msg("unknown option: %c", c);
				show_usage();
				return -EINVAL;
		}
	}

	if (handle == NULL)
	{
		error_msg("no function handle this action");
		return -EINVAL;
	}

	if (hostname == NULL)
	{
		hostname = cavan_get_server_hostname();
	}

	if (port == 0)
	{
		port = cavan_get_server_port(TFTP_DD_DEFAULT_PORT);
	}

	input_file[0] = 0;
	output_file[0] = 0;

	for (i = optind; i < argc; i++)
	{
		char *p;

		parse_parameter(argv[i]);

		c = para_option[0];
		p = para_option + 1;

		switch (c)
		{
		case 'i':
			if (strcmp(p, "f") == 0)
			{
				strcpy(input_file, para_value);
			}
			else if (strcmp(p, "p") == 0)
			{
				hostname = strdup(para_value);
			}
			else
			{
				goto out_unknown_option;
			}
			break;
		case 'o':
			if (strcmp(p, "f") == 0)
			{
				strcpy(output_file, para_value);
			}
			else
			{
				goto out_unknown_option;
			}
			break;
		case 'b':
			if (strcmp(p, "s") == 0)
			{
				bs = text2size(para_value, NULL);
			}
			else
			{
				goto out_unknown_option;
			}
			break;
		case 'p':
			if (strcmp(p, "ort") == 0)
			{
				port = text2value(para_value, NULL, 10);
			}
			else
			{
				goto out_unknown_option;
			}
			break;
		case 's':
			if (strcmp(p, "kip") == 0)
			{
				skip = text2size(para_value, NULL);
			}
			else if (strcmp(p, "eek") == 0)
			{
				seek = text2size(para_value, NULL);
			}
			else
			{
				goto out_unknown_option;
			}
			break;
		case 'c':
			if (strcmp(p, "ount") == 0)
			{
				count = text2size(para_value, NULL);
			}
			else
			{
				goto out_unknown_option;
			}
			break;
		default:
			goto out_unknown_option;
		}
	}

	if (input_file[0] == 0 || output_file[0] == 0)
	{
		error_msg("must specify if and of option");
		return -EINVAL;
	}

	ret = handle(hostname, port, input_file, output_file, skip * bs, seek * bs, count * bs);
	if (ret < 0)
	{
		error_msg("tftp dd failed");
		return ret;
	}

	return 0;

out_unknown_option:
	error_msg("unknown option \"%s\"", para_option);
	show_usage();
	return -EINVAL;
}
