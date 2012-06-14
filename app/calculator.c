/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Mon Jun 11 10:49:18 CST 2012
 */

#include <cavan.h>
#include <cavan/calculator.h>

#define FILE_CREATE_DATE "2012-06-11 10:49:18"

enum
{
	LOCAL_COMMAND_OPTION_UNKNOWN,
	LOCAL_COMMAND_OPTION_HELP,
	LOCAL_COMMAND_OPTION_VERSION,
	LOCAL_COMMAND_OPTION_BASE,
	LOCAL_COMMAND_OPTION_LENGTH,
	LOCAL_COMMAND_OPTION_PREFIX
};

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
		},
	};
	int ret;
	double result;
	char buff[1024];
	int base = 0;
	int length = 0;
	int flags = 0;

	while ((c = getopt_long(argc, argv, "vVhHb:B:l:L:pP", long_option, &option_index)) != EOF)
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
			show_usage();
			return 0;

		case 'b':
		case 'B':
		case LOCAL_COMMAND_OPTION_BASE:
			base = text2value_unsigned(optarg, NULL, 10);
			break;

		case 'l':
		case 'L':
		case LOCAL_COMMAND_OPTION_LENGTH:
			length = text2value_unsigned(optarg, NULL, 10);
			break;

		case 'p':
		case 'P':
		case LOCAL_COMMAND_OPTION_PREFIX:
			flags |= TEXT_FLAG_PREFIX;
			break;

		default:
			show_usage();
			return -EINVAL;
		}
	}

	assert(argc > optind);

	text_cat2(buff, argv + optind, argc - optind);

	ret = complete_calculation(text2lowercase(buff), &result);
	if (ret < 0)
	{
		return ret;
	}

	if (base < 2)
	{
		println("%lf", result);
	}
	else
	{
		double2text(&result, buff, length, 0, base | flags);
		println(buff);
	}

	return 0;
}
