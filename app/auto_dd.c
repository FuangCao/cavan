// Fuang.Cao <cavan.cfa@gmail.com> Sat May 14 23:11:20 CST 2011

#include <cavan.h>
#include <cavan/dd.h>
#include <cavan/uevent.h>
#include <cavan/text.h>
#include <cavan/file.h>
#include <cavan/memory.h>
#include <cavan/device.h>
#include <cavan/image.h>

static void show_usage(void)
{
	println("Usage:");
	println("auto_dd image_dir");
	println("image include:");
	println("u-boot.bin");
	println("u-boot-no-padding");
	println("uImage");
	println("uramdisk.img");
	println("logo.bmp");
	println("busybox.img");
	println("charge.bmps");
	println("system.img");
	println("recovery.img");
	println("userdata.img");
}

int main(int argc, char *argv[])
{
	int ret;
	int c;
	int delay;
	struct uevent_desc udesc;
	char dir_path[1024];
	struct dd_desc *p, *end_p, descs[] =
	{
		{
			.in = "u-boot.bin",
			.out = "/dev/sdb",
			.bs = 1,
			.seek = UBOOT_OFFSET,
			.skip = UBOOT_PADDING_SIZE,
			.count = 0,
		},
		{
			.in = "u-boot-no-padding.bin",
			.out = "/dev/sdb",
			.bs = 1,
			.seek = UBOOT_OFFSET,
			.skip = 0,
			.count = 0,
		},
		{
			.in = "uImage",
			.out = "/dev/sdb",
			.bs = 1,
			.seek = UIMAGE_OFFSET,
			.skip = 0,
			.count = 0,
		},
		{
			.in = "uramdisk.img",
			.out = "/dev/sdb",
			.bs = 1,
			.seek = RAMDISK_OFFSET,
			.skip = 0,
			.count = 0,
		},
		{
			.in = "logo.bmp",
			.out = "/dev/sdb",
			.bs = 1,
			.seek = LOGO_OFFSET,
			.skip = 0,
			.count = 0,
		},
		{
			.in = "busybox.img",
			.out = "/dev/sdb",
			.bs = 1,
			.seek = BUSYBOX_OFFSET,
			.skip = 0,
			.count = 0,
		},
		{
			.in = "charge.bmps",
			.out = "/dev/sdb",
			.bs = 1,
			.seek = CARTOON_OFFSET,
			.skip = 0,
			.count = 0,
		},
		{
			.in = "system.img",
			.out = "/dev/sdb2",
			.bs = 0,
			.seek = 0,
			.skip = 0,
			.count = 0,
		},
		{
			.in = "recovery.img",
			.out = "/dev/sdb4",
			.bs = 0,
			.seek = 0,
			.skip = 0,
			.count = 0,
		},
		{
			.in = "userdata.img",
			.out = "/dev/sdb5",
			.bs = 0,
			.seek = 0,
			.skip = 0,
			.count = 0,
		},
	};

	delay = 0;

	while ((c = getopt(argc, argv, "d:D:")) != EOF)
	{
		switch (c)
		{
		case 'd':
		case 'D':
			delay = text2value_unsigned(optarg, NULL, 10);
			break;

		default:
			show_usage();
			return -EINVAL;
		}
	}

	if (argc == optind)
	{
		show_usage();
		return -EINVAL;
	}

	if (to_abs_path_directory_base(argv[optind], dir_path, sizeof(dir_path)) == NULL)
	{
		error_msg("directory \"%s\" do't exist", argv[1]);
		return -ENOENT;
	}

	for (p = descs, end_p = descs + ARRAY_SIZE(descs); p < end_p; p++)
	{
		char path_tmp[1024];

		text_path_cat(path_tmp, dir_path, p->in);

		if (file_test(path_tmp, "r") == 0)
		{
			println("%s <=> %s", p->in, path_tmp);
			text_copy(p->in, path_tmp);
		}
		else
		{
			p->in[0] = 0;
		}
	}

	ret = uevent_init(&udesc);
	if (ret < 0)
	{
		error_msg("ueven_init");
		return ret;
	}

	while (1)
	{
		char diskpath[1024];

		ret = get_disk_add_uevent(&udesc);
		if (ret < 0)
		{
			error_msg("get_disk_add_uevent");
			break;
		}

		if (uevent_get_propery_devname(&udesc, diskpath) == NULL)
		{
			continue;
		}

		if (file_test(diskpath, "b") < 0)
		{
			continue;
		}

		println("disk \"%s\" inserted", diskpath);

		if (delay)
		{
			visual_ssleep(delay);
		}

		ret = 0;

		for (p = descs, end_p = descs + ARRAY_SIZE(descs); p < end_p; p++)
		{
			if (p->in[0] == 0)
			{
				continue;
			}

			mem_copy(p->out + 5, diskpath + 5, 3);

			if (file_test(p->out, "b") < 0)
			{
				error_msg("%s is not a block device", p->out);
				ret |= -ENODEV;
				continue;
			}

			umount_partition(p->out, MNT_DETACH);

			ret |= cavan_dd2(p);
		}

		umount_device(diskpath, MNT_DETACH);

		if (ret < 0)
		{
			println_red("Failed");
		}
		else
		{
			println_green("Burn success, you can remove the disk now");
		}
	}

	uevent_deinit(&udesc);

	return ret;
}
