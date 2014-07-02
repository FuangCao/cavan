/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Thu Jul 19 10:30:57 CST 2012
 */

#include <cavan.h>
#include <cavan/adb.h>
#include <cavan/command.h>

#define FILE_CREATE_DATE "2012-07-19 10:30:57"

static void show_usage(void)
{
	println("Usage:");
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
		{0, 0, 0, 0},
	};
	int sockfd;
	char ip[32];
	u16 port;

	ip[0] = 0;
	port = ADB_SMS_TRANSLATOR_PORT;

	while ((c = getopt_long(argc, argv, "a:A:p:P:vVhH", long_option, &option_index)) != EOF)
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
			show_usage();
			return 0;

		case 'a':
		case 'A':
		case CAVAN_COMMAND_OPTION_IP:
			text_copy(ip, optarg);
			break;

		case 'p':
		case 'P':
		case CAVAN_COMMAND_OPTION_PORT:
			port = text2value_unsigned(optarg, NULL, 10);
			break;

		default:
			show_usage();
			return -EINVAL;
		}
	}

	if (optind >= argc)
	{
		pr_red_info("Please give a filename");
		return -EINVAL;
	}

	sockfd = adb_create_tcp_link(ip[0] == 0 ? NULL : ip, 0, port);
	if (sockfd < 0)
	{
		pr_red_info("adb_connect_service2");
		return sockfd;
	}

	return sms_receive_and_write(sockfd, argv[optind]);
}
