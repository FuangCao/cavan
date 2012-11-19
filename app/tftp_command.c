// Fuang.Cao <cavan.cfa@gmail.com> Thu May  5 10:20:40 CST 2011

#include <cavan.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cavan/tftp.h>
#include <cavan/text.h>
#include <cavan/parser.h>

static void show_usage(void)
{
	println("Usage:");
	println("tftp_command [--ip=ip --port=port] command");
}

int main(int argc, char *argv[])
	{
		int i;
		int ret;
		u16 port = 0;
		char ip[20];
		char command[1024], *p;
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
		int c;
		int option_index;

		ip[0] = 0;

		while ((c = getopt_long(argc, argv, "hH", long_options, &option_index)) != EOF)
		{
			switch (c)
			{
				case 0:
					strcpy(ip, optarg);
					break;

				case 1:
					port = text2value(optarg, NULL, 10);
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

		assert(argc > optind);

		if (ip[0] == 0)
		{
			cavan_get_server_ip(ip);
		}

		if (port == 0)
		{
			port = cavan_get_server_port(TFTP_DD_DEFAULT_PORT);
		}

		p = command;
		i = optind;

		while (1)
		{
			char *argv_p = argv[i++];

			while (*argv_p)
			{
				*p++ = *argv_p++;
			}

			if (i < argc)
			{
				*p++ = ' ';
			}
			else
			{
				break;
			}
		}

		*p = 0;

		if (command[0] == 0)
		{
			error_msg("please input a command");
			return -EINVAL;
		}

		ret = send_command_request_show(ip, port, command);
		if (ret < 0)
		{
			error_msg("Send command request failed");
			return ret;
		}

		return 0;
	}

