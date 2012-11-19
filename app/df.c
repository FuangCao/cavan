// Fuang.Cao <cavan.cfa@gmail.com> Tue Aug  2 09:56:18 CST 2011

#include <cavan.h>
#include <cavan/device.h>

#define FILE_CREATE_DATE "Tue Aug  2 09:56:18 CST 2011"

static void show_usage(void)
{
	println("Usage:");
	println("df dev_path");
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
	struct statfs stfs;

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

	assert(optind < argc);

	ret = get_device_statfs(argv[optind], NULL, &stfs);
	if (ret < 0)
	{
		error_msg("get_device_statfs");
		return ret;
	}

	show_statfs(&stfs);

	return 0;
}
