// Fuang.Cao <cavan.cfa@gmail.com> Thu Mar 31 12:06:19 CST 2011

#include <cavan.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <cavan/tftp.h>
#include <cavan/progress.h>
#include <cavan/parse.h>
#include <cavan/text.h>
#include <cavan/file.h>
#include <cavan/device.h>

static void show_usage(void)
{
	println("Usage:");
	println("tftp_dd [--ip=ip_address] [--port=port] -w if=local_file of=remote_file");
	println("tftp_dd [--ip=ip_address] [--port=port] -r if=remote_file of=local_file");
}

int main(int argc, char *argv[])
{
	int i;
	int c;
	int ret;
	char ip_address[20];
	u16 port = TFTP_DEFAULT_PORT;
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
		},
	};

	ip_address[0] = 0;
	handle = NULL;

	while ((c = getopt_long(argc, argv, "rRgGwWsSpPhH", long_options, &option_index)) != EOF)
	{
		switch (c)
		{
			case 0:
				strcpy(ip_address, optarg);
				break;

			case 1:
				port = text2value(optarg, 10);
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

	if (ip_address[0] == 0)
	{
		strcpy(ip_address, TFTP_DEFAULT_IP);
	}

	input_file[0] = 0;
	output_file[0] = 0;

	for (i = optind; i < argc; i++)
	{
		parse_parameter(argv[i]);

		if (strcmp(para_option, "if") == 0)
		{
			strcpy(input_file, para_value);
		}
		else if (strcmp(para_option, "of") == 0)
		{
			strcpy(output_file, para_value);
		}
		else if (strcmp(para_option, "bs") == 0)
		{
			bs = text2size(para_value);
		}
		else if (strcmp(para_option, "ip") == 0)
		{
			strcpy(ip_address, para_value);
		}
		else if (strcmp(para_option, "port") == 0)
		{
			port = text2value(para_value, 10);
		}
		else if (strcmp(para_option, "seek") == 0)
		{
			seek = text2size(para_value);
		}
		else if (strcmp(para_option, "skip") == 0)
		{
			skip = text2size(para_value);
		}
		else if (strcmp(para_option, "count") == 0)
		{
			count = text2size(para_value);
		}
		else
		{
			error_msg("unknown option \"%s\"", para_option);
			show_usage();
			return -EINVAL;
		}
	}

	if (input_file[0] == 0 || output_file[0] == 0)
	{
		error_msg("must specify if and of option");
		return -EINVAL;
	}

	ret = handle(ip_address, port, input_file, output_file, skip * bs, seek * bs, count * bs);
	if (ret < 0)
	{
		error_msg("tftp dd failed");
		return ret;
	}

	return 0;
}
