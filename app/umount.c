// Fuang.Cao <cavan.cfa@gmail.com> Thu Apr 28 19:30:39 CST 2011

#include <cavan.h>
#include <cavan/device.h>
#include <cavan/permission.h>

static void show_usage(void)
{
	println("Usage:");
	println("umount_from [-l] devices");
	println("umount_from [-l] mnt_points");
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
		{
			0, 0, 0, 0
		},
	};
	int flags = 0;

	ret = has_super_permission(NULL);
	if (ret < 0)
	{
		return ret;
	}

	while ((c = getopt_long(argc, argv, "vVhHlLfFeE", long_option, &option_index)) != EOF)
	{
		switch (c)
		{
		case 'v':
		case 'V':
			show_author_info();
			return 0;

		case 'h':
		case 'H':
			show_usage();
			return 0;

		case 'l':
		case 'L':
			flags |= MNT_DETACH;
			break;

		case 'f':
		case 'F':
			flags |= MNT_FORCE;
			break;

		case 'e':
		case 'E':
			flags |= MNT_EXPIRE;
			break;

		default:
			show_usage();
			return -EINVAL;
		}
	}

	assert(optind < argc);

	println("flags = 0x%08x", flags);

	ret = umount_all1(argv + optind, argc - optind, flags);
	if (ret < 0)
	{
		print_error("umount_all1");
		return ret;
	}

	return 0;
}
