#include <cavan.h>
#include <cavan/device.h>
#include <cavan/file.h>

static void show_usage(void)
{
	println("Usage:");
	println("mount_to");
	println("mount_to -t fs_type device mount_point");
}

int main(int argc, char *argv[])
{
	int ret;
	char devpath[1024];
	char fs_type[64];
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
			0, 0, 0, 0
		},
	};

	fs_type[0] = 0;

	while ((c = getopt_long(argc, argv, "t:T:hH", long_option, &option_index)) != EOF)
	{
		switch (c)
		{
		case 't':
		case 'T':
			text_copy(fs_type, optarg);
			break;

		case 'h':
		case 'H':
			show_usage();
			return 0;

		default:
			show_usage();
			return -EINVAL;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc < 1)
	{
		return print_mount_table();
	}

	if (file_test(argv[0], "e") < 0 && argv[0][0] != '/')
	{
		sprintf(devpath, "/dev/sd%s", argv[0]);

		if (file_test(devpath, "e") < 0)
		{
			sprintf(devpath, "/dev/%s", argv[0]);
		}
	}
	else
	{
		text_copy(devpath, argv[0]);
	}

	println("devpath = %s", devpath);

	if (argc < 2)
	{
		ret = mount_main(devpath, NULL, fs_type, NULL);
	}
	else if (argc < 3 || fs_type[0])
	{
		ret = mount_main(devpath, argv[1], fs_type, NULL);
	}
	else
	{
		ret = mount_main(devpath, argv[1], argv[2], NULL);
	}

	if (ret < 0)
	{
		print_error("mount deivce \"%s\" failed", argv[0]);
		return ret;
	}

	return 0;
}
