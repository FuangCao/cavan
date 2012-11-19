#include <cavan.h>
#include <cavan/copy.h>
#include <cavan/file.h>
#include <cavan/device.h>

static void show_usage(void)
{
	println("Usage:");
	println("copy src_file dest_file");
	println("copy src_files dest_dir");
}

int main(int argc, char *argv[])
{
	int ret;
	int i;
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
			.name = "umount",
			.has_arg = no_argument,
			.flag = NULL,
			.val = 'u',
		},
		{
			.name = "sync",
			.has_arg = no_argument,
			.flag = NULL,
			.val = 's',
		},
		{
			.name = "nosync",
			.has_arg = no_argument,
			.flag = NULL,
			.val = 'n',
		},
		{
			0, 0, 0, 0
		},
	};
	char dest_path[1024];
	int need_umount = 0;

	while ((c = getopt_long(argc, argv, "vVhHuUsSNn", long_option, &option_index)) != EOF)
	{
		switch (c)
		{
		case 'v':
		case 'V':
		case 'h':
		case 'H':
			show_usage();
			return 0;

		case 'u':
		case 'U':
			need_umount = 1;
			break;

		case 's':
		case 'S':
			write_flags |= O_SYNC;
			break;

		case 'n':
		case 'N':
			write_flags &= ~O_SYNC;
			break;

		default:
			show_usage();
			return -EINVAL;
		}
	}

	argc = argc - optind - 1;
	argv += optind;

	assert(argc > 0);

	if (file_test(argv[argc], "b") == 0)
	{
		if (get_partition_mount_point_base(argv[argc], dest_path, sizeof(dest_path)) == NULL)
		{
			text_copy(dest_path, TEMP_MOUNT_POINT);

			if (mount_to(argv[argc], dest_path, "ext4", NULL) < 0)
			{
				text_copy(dest_path, argv[argc]);
			}
			else
			{
				need_umount = 1;
			}
		}
	}
	else
	{
		text_copy(dest_path, argv[argc]);
	}

	for (i = 0; i < argc; i++)
	{
		ret = copy_main(argv[i], dest_path);
		if (ret < 0)
		{
			error_msg("copy \"%s\" to \"%s\" failed", argv[i], dest_path);
			goto out_umount_dest;
		}
	}

	ret = 0;

out_umount_dest:
	if (need_umount)
	{
		umount_directory2(dest_path, 0);
	}

	return ret;
}
