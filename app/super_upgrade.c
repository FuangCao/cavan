// Fuang.Cao <cavan.cfa@gmail.com> Fri Apr 29 16:38:07 CST 2011

#include <cavan.h>
#include <cavan/swan_upgrade.h>
#include <cavan/device.h>
#include <cavan/text.h>

#define BUSYBOX_MOUNT_POINT			"/mnt/busybox"
#define BUSYBOX_DEV_MOUNT_POINT		BUSYBOX_MOUNT_POINT "/dev"
#define SIDELOAD_PACKAGE_PATH		"/tmp/sideload/package.zip"

static int swan_init(const char *mnt_point)
{
	int ret;
	unsigned int i;
	const char *sd_devices[] =
	{
		"/dev/block/mmcblk1p1",
		"/dev/mmcblk1p1",
		"/dev/block/mmcblk1",
		"/dev/mmcblk1",
		"/dev/block/mmcblk0p1",
		"/dev/mmcblk0p1",
		"/dev/block/mmcblk0k1",
		"/dev/mmcblk0",
	};

	ret = cavan_mkdir(mnt_point);
	if (ret < 0)
	{
		return ret;
	}

	if (file_access_e(SIDELOAD_PACKAGE_PATH) == false)
	{
		goto out_mount_tmp;
	}

	for (i = 0; i < ARRAY_SIZE(sd_devices); i++)
	{
		if (libc_mount(sd_devices[i], mnt_point, "vfat", 0, NULL) >= 0)
		{
			return 0;
		}
	}

out_mount_tmp:
	return libc_mount("none", mnt_point, "tmpfs", 0, "size=500m");
}

static int swan_deinit(const char *mnt_point)
{
	umount_directory2(mnt_point, MNT_DETACH);

	return 0;
}

static int swan_install(const char *dirpath)
{
	int ret;
	int fd;
	char busybox_path[1024];
	char pkg_path[1024];

	text_path_cat(busybox_path, sizeof(busybox_path), dirpath, "busybox.ext4");

	ret = file_mount_to(busybox_path, BUSYBOX_MOUNT_POINT, "ext4", 0, NULL);
	if (ret < 0)
	{
		error_msg("mount file \"%s\"", busybox_path);
		return ret;
	}

	text_path_cat(pkg_path, sizeof(pkg_path), dirpath, "upgrade.swan");

	fd = open(pkg_path, O_RDWR);
	if (fd < 0)
	{
		ret = fd;
		print_error("open upgrade file \"%s\" failed", pkg_path);
		goto out_umount_busybox;
	}

	ret = chroot(BUSYBOX_MOUNT_POINT);
	if (ret < 0)
	{
		print_error("chroot to " BUSYBOX_MOUNT_POINT " failed");
		goto out_close_fd;
	}

	ret = setenv("PATH", DEFAULT_PATH_VALUE, 1);
	if (ret < 0)
	{
		error_msg("setenv PATH failes");
		goto out_close_fd;
	}

	system_command("/etc/init.d/super_init.sh");

	print_mount_table();

	ret = fupgrade_simple(fd);

	system_command("/etc/init.d/super_deinit.sh");

out_close_fd:
	close(fd);
out_umount_busybox:
	umount_directory2(BUSYBOX_MOUNT_POINT, MNT_DETACH);

	return ret;
}

int main(int argc, char *argv[])
{
	int c;
	int option_index;
	struct option long_option[] =
	{
		{
			.name = "init",
			.has_arg = no_argument,
			.flag = NULL,
			.val = 'i',
		},
		{
			.name = "setup",
			.has_arg = no_argument,
			.flag = NULL,
			.val = 'i',
		},
		{
			.name = "deinit",
			.has_arg = no_argument,
			.flag = NULL,
			.val = 'u',
		},
		{
			0, 0, 0, 0
		},
	};
	int (*action)(const char *) = swan_install;

	while ((c = getopt_long(argc, argv, "iIsSuU", long_option, &option_index)) != EOF)
	{
		switch (c)
		{
		case 'i':
		case 'I':
		case 's':
		case 'S':
			action = swan_init;
			break;

		case 'u':
		case 'U':
			action = swan_deinit;
			break;

		default:
			error_msg("invalid argument");
			return -EINVAL;
		}
	}

	assert(optind < argc);

	return action(argv[optind]);
}
