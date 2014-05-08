// Fuang.Cao <cavan.cfa@gmail.com> Fri Apr  1 16:23:05 CST 2011

#include <cavan.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cavan/tftp.h>
#include <cavan/text.h>
#include <cavan/parser.h>

int main(int argc, char *argv[])
{
	int i;
	int c;
	u16 port = 0;
	char ip[20];
	int (*tftp_handle)(const char *, u16, const char *, const char *);
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
	char temp_name[512], *p_name;

	ip[0] = 0;
	tftp_handle = NULL;

	while ((c = getopt_long(argc, argv, "rRgGwWsSpP", long_options, &option_index)) != EOF)
	{
		switch (c)
		{
		case 0:
			strcpy(ip, optarg);
			break;

		case 1:
			port = text2value(optarg, NULL, 10);
			break;

		case 'r':
		case 'R':
		case 'g':
		case 'G':
			tftp_handle = tftp_client_receive_all;
			break;

		case 'w':
		case 'W':
		case 's':
		case 'S':
		case 'p':
		case 'P':
			tftp_handle = tftp_client_send_all;
			break;

		default:
			error_msg("unknown operation");
			return -EINVAL;
		}
	}

	assert(argc - optind >= 2 && tftp_handle != NULL);

	if (ip[0] == 0)
	{
		cavan_get_server_ip(ip);
	}

	if (port == 0)
	{
		port = cavan_get_server_port(TFTP_DD_DEFAULT_PORT);
	}

	p_name = text_path_cat(temp_name, sizeof(temp_name), argv[--argc], NULL);

	for (i = optind; i < argc; i++)
	{
		int ret;

		text_basename_base(p_name, argv[i]);

		ret = tftp_handle(ip, port, argv[i], temp_name);
		if (ret < 0)
		{
			error_msg("tftp send or receive file \"%s\" failed", argv[i]);
			return ret;
		}
	}

	return 0;
}
