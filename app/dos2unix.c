// Fuang.Cao <cavan.cfa@gmail.com> Mon Jun 13 17:22:48 CST 2011

#include <cavan.h>

#define FILE_CREATE_DATE "Mon Jun 13 17:22:48 CST 2011"

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

	assert(argc == 2);

	ret = file_dos2unix(argv[1]);
	if (ret < 0)
	{
		print_error("file_dos2unix");
		return ret;
	}

	println_green("success");

	return 0;
}
