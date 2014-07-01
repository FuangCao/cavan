/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Fri Oct 12 09:36:56 CST 2012
 */

#include <cavan.h>
#include <sys/wait.h>
#include <cavan/command.h>

#define FILE_CREATE_DATE "2012-10-12 09:36:56"

static void show_usage(const char *command)
{
	println("Usage:");
	println("%s [option] command", command);
	println("delay, sleep, -d, -s: delay after command terminal");
	println("wait, -w: exec wait-for-devices before command");
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
			.name = "wait",
			.has_arg = no_argument,
			.flag = NULL,
			.val = 'w',
		},
		{0, 0, 0, 0},
	};
	u32 delay;
	bool wait_for_devices;
	char command[1024];
	int ret;

	delay = 1;
	wait_for_devices = false;

	while ((c = getopt_long(argc, argv, "vVhHswW:S:d:D:", long_option, &option_index)) != EOF)
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

		case 's':
		case 'S':
		case 'd':
		case 'D':
			delay = text2value_unsigned(optarg, NULL, 10);
			break;

		case 'w':
		case 'W':
			wait_for_devices = true;
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

	while (1)
	{
		if (wait_for_devices)
		{
			// pr_bold_info("Adb Wait For Devices");
			if (system("adb wait-for-devices"))
			{
				break;
			}
		}

		// println("%s", command);
		ret = system(command);
		if (ret)
		{
			pr_red_info("Failed");

			if (WIFSIGNALED(ret))
			{
				break;
			}
		}

		if (delay)
		{
			sleep(delay);
		}
	}

	return 0;
}
