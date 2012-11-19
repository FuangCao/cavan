// Fuang.Cao <cavan.cfa@gmail.com> 2011-11-09 11:56:34

#include <cavan.h>

#define FILE_CREATE_DATE "2011-11-09 11:56:34"

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
			.val = 'h',
		},
		{
			.name = "version",
			.has_arg = no_argument,
			.flag = NULL,
			.val = 'v',
		},
		{
			0, 0, 0, 0
		},
	};
	int ret;

	while ((c = getopt_long(argc, argv, "vVhH", long_option, &option_index)) != EOF)
	{
		switch (c)
		{
		case 'v':
		case 'V':
			show_author_info();
			println(FILE_CREATE_DATE);
			return 0;

		case 'h':
		case 'H':
			show_usage();
			return 0;

		default:
			show_usage();
			return -EINVAL;
		}
	}

	while (optind < argc)
	{
		ret = remove_auto(argv[optind]);
		if (ret < 0)
		{
			error_msg("remove %s failed", argv[optind]);
			return ret;
		}

		optind++;
	}

	return 0;
}
