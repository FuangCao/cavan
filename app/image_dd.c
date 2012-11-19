// Fuang.Cao <cavan.cfa@gmail.com> Mon May 16 19:37:23 CST 2011

#include <cavan.h>
#include <cavan/image.h>
#include <cavan/dd.h>
#include <cavan/device.h>

#define FILE_CREATE_DATE "Mon May 16 19:37:23 CST 2011"

static void show_usage(void)
{
	println("Usage:");
	println("image_dd [-o dev_path] img_path");
	println("image_dd [-o dev_path] -a img_dir");
}

int main(int argc, char *argv[])
{
	int c;
	int ret;
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
		{0, 0, 0, 0},
	};
	char dest_dev[1024];
	int burn_image_dir;

	dest_dev[0] = 0;
	burn_image_dir = 0;

	while ((c = getopt_long(argc, argv, "vVhHo:O:aA", long_option, &option_index)) != EOF)
	{
		switch (c)
		{
		case 'o':
		case 'O':
			text_copy(dest_dev, optarg);
			break;

		case 'a':
		case 'A':
			burn_image_dir = 1;
			break;

		case 'v':
		case 'V':
			show_author_info();
			println("%s", FILE_CREATE_DATE);
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

	if (argc == optind)
	{
		show_usage();
		ERROR_RETURN(EINVAL);
	}

	if (burn_image_dir && file_test(argv[optind], "d") == 0)
	{
		ret = burn_swan_image_directory(argv[optind], dest_dev);
		if (ret < 0)
		{
			error_msg("burn_swan_images");
			return ret;
		}
	}
	else
	{
		ret = burn_swan_image_auto(argv[optind], dest_dev);
		if (ret < 0)
		{
			error_msg("burn_swan_image_auto");
			return ret;
		}
	}

	return 0;
}
