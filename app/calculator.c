/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Mon Jun 11 10:49:18 CST 2012
 */

#include <cavan.h>
#include <cavan/math.h>
#include <cavan/calculator.h>

#define FILE_CREATE_DATE "2012-06-11 10:49:18"

enum
{
	LOCAL_COMMAND_OPTION_UNKNOWN,
	LOCAL_COMMAND_OPTION_HELP,
	LOCAL_COMMAND_OPTION_VERSION,
	LOCAL_COMMAND_OPTION_BASE,
	LOCAL_COMMAND_OPTION_LENGTH,
	LOCAL_COMMAND_OPTION_PREFIX,
	LOCAL_COMMAND_OPTION_LONG,
	LOCAL_COMMAND_OPTION_MASK
};

static void show_usage(const char *command)
{
	println("Usage:");
	println("%s [option] port", command);
	println("--help, -h, -H\t\tshow this help");
	println("--version, -v, -V\tshow command version");
	println("--base, -b, -B\t\tset out radix");
	println("--length, -l, -L\tset value bit count");
	println("--prefix, -p, -P\tshow value prefix");
	println("--long\t\t\tuse my math library");
	println("--mask, -m, -M\t\tshow bit location");
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
			.name = "base",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_BASE,
		},
		{
			.name = "length",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_LENGTH,
		},
		{
			.name = "prefix",
			.has_arg = no_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_PREFIX,
		},
		{
			.name = "long",
			.has_arg = no_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_LONG,
		},
		{
			.name = "mask",
			.has_arg = no_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_MASK,
		},
		{
			0, 0, 0, 0
		},
	};
	int ret;
	char buff[1024];
	int base = 0;
	int length[2];
	int flags = 0;
	bool long_cal = false;
	bool show_bitmask = false;

	length[0] = length[1] = 0;

	while ((c = getopt_long(argc, argv, "vVhHb:B:l:L:pPmM", long_option, &option_index)) != EOF)
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

		case 'b':
		case 'B':
		case LOCAL_COMMAND_OPTION_BASE:
			base = text2value_unsigned(optarg, NULL, 10);
			break;

		case 'l':
		case 'L':
		case LOCAL_COMMAND_OPTION_LENGTH:
			length[0] = text2value_unsigned(optarg, (const char **)&optarg, 10);
			length[1] = text2value_unsigned(optarg + 1, NULL, 10);
			break;

		case 'p':
		case 'P':
		case LOCAL_COMMAND_OPTION_PREFIX:
			flags |= TEXT_FLAG_PREFIX;
			break;

		case LOCAL_COMMAND_OPTION_LONG:
			long_cal = true;
			break;

		case 'm':
		case 'M':
		case LOCAL_COMMAND_OPTION_MASK:
			show_bitmask = true;
			break;

		default:
			show_usage(argv[0]);
			return -EINVAL;
		}
	}

	assert(argc > optind);

	text_cat2(buff, argv + optind, argc - optind);
	text2lowercase(buff);

	if (long_cal)
	{
		byte result[1024];

		math_memory_calculator(buff, result, sizeof(result), base, '0', length[0]);
	}
	else
	{
		double result;

		ret = complete_calculation(buff, &result);
		if (ret < 0)
		{
			return ret;
		}

		if (base < 2 || base == 10)
		{
			if (length[0] || length[1])
			{
				char format[64];

				sprintf(format, "%%0%d.%dlf", length[0], length[1]);
				println(format, result);
			}
			else
			{
				println("%lf", result);
			}
		}
		else
		{
			double2text(&result, buff, length[0], 0, base | flags);
			println("%s", buff);
		}

		if (show_bitmask)
		{
			value2bitlist((u64)result, buff, sizeof(buff), " | ");

			println("%s", buff);
		}
	}

	return 0;
}
