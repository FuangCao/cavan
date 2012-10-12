/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Fri Oct 12 09:36:56 CST 2012
 */

#include <cavan.h>

#define FILE_CREATE_DATE "2012-10-12 09:36:56"

enum
{
	LOCAL_COMMAND_OPTION_UNKNOWN,
	LOCAL_COMMAND_OPTION_HELP,
	LOCAL_COMMAND_OPTION_VERSION,
};

static void show_usage(const char *command)
{
	println("Usage:");
	println("%s [option] command", command);
	println("delay, sleep, -d, -s: delay after command terminal");
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
			.val = LOCAL_COMMAND_OPTION_HELP,
		},
		{
			.name = "version",
			.has_arg = no_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_VERSION,
		},
		{
			.name = "sleep",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 's',
		},
		{
			.name = "delay",
			.has_arg = required_argument,
			.flag = NULL,
			.val = 'd',
		},
		{
		},
	};
	u32 delay;
	char command[1024];
	int ret;

	delay = 0;

	while ((c = getopt_long(argc, argv, "vVhHs:S:d:D:", long_option, &option_index)) != EOF)
	{
		switch (c)
		{
		case 'v':
		case 'V':
		case LOCAL_COMMAND_OPTION_VERSION:
			show_author_info();
			println(FILE_CREATE_DATE);
			return 0;

		case 'h':
		case 'H':
		case LOCAL_COMMAND_OPTION_HELP:
			show_usage(argv[0]);
			return 0;

		case 's':
		case 'S':
		case 'd':
		case 'D':
			delay = text2value_unsigned(optarg, NULL, 10);
			break;

		default:
			show_usage(argv[0]);
			return -EINVAL;
		}
	}

	if (optind < argc)
	{
		char *p = command;

		while (1)
		{
			p = text_copy(p, argv[optind++]);
			if (optind < argc)
			{
				*p++ = ' ';
			}
			else
			{
				break;
			}
		}

		*p = 0;
	}
	else
	{
		pr_red_info("Please give a command");
		show_usage(argv[0]);
		return -EINVAL;
	}

	pr_bold_info("command = `%s'", command);

	while (1)
	{
		ret = system(command);
		if (ret)
		{
			pr_red_info("Failed");
		}
		else
		{
			pr_green_info("OK");
		}

		if (delay)
		{
			sleep(delay);
		}
		else
		{
			msleep(100);
		}
	}

	return 0;
}
