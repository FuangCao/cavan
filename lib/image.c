// Fuang.Cao <cavan.cfa@gmail.com> Tue Jan 11 23:03:28 CST 2011

#include <cavan.h>
#include <cavan/image.h>
#include <cavan/text.h>
#include <cavan/dd.h>
#include <cavan/file.h>
#include <cavan/device.h>

const char *image_type_to_text(enum image_type type)
{
	switch (type)
	{
	case IMAGE_UBOOT_BIN:
		return "u-boot.bin";

	case IMAGE_UBOOT_NO_PADDING_BIN:
		return "u-boot-no-padding.bin";

	case IMAGE_ZIMAGE:
		return "zImage";

	case IMAGE_UIMAGE:
		return "uImage";

	case IMAGE_RAMDISK_IMG:
		return "ramdisk.img";

	case IMAGE_URAMDISK_IMG:
		return "uramdisk.img";

	case IMAGE_LOGO:
		return "logo.bmp";

	case IMAGE_BUSYBOX_IMG:
		return "busybox.img";

	case IMAGE_CARTOON:
		return "charge.bmps";

	case IMAGE_SYSTEM_IMG:
		return "system.img";

	case IMAGE_RECOVERY_IMG:
		return "recovery.img";

	case IMAGE_USERDATA_IMG:
		return "userdata.img";

	default:
		return "unknown";
	}
}

enum image_type path_to_image_type(const char *img_path)
{
	char image_name[64];

	text_basename_base(image_name, img_path);

	if (text_cmp(image_name, "u-boot.bin") == 0)
	{
		return IMAGE_UBOOT_BIN;
	}
	else if (text_cmp(image_name, "u-boot-no-padding.bin") == 0)
	{
		return IMAGE_UBOOT_NO_PADDING_BIN;
	}
	else if (text_cmp(image_name, "zImage") == 0)
	{
		return IMAGE_ZIMAGE;
	}
	else if (text_cmp(image_name, "uImage") == 0)
	{
		return IMAGE_UIMAGE;
	}
	else if (text_cmp(image_name, "ramdisk.img") == 0)
	{
		return IMAGE_RAMDISK_IMG;
	}
	else if (text_cmp(image_name, "uramdisk.img") == 0)
	{
		return IMAGE_URAMDISK_IMG;
	}
	else if (text_cmp(image_name, "logo.bmp") == 0)
	{
		return IMAGE_LOGO;
	}
	else if (text_cmp(image_name, "busybox.img") == 0)
	{
		return IMAGE_BUSYBOX_IMG;
	}
	else if (text_cmp(image_name, "charge.bmps") == 0)
	{
		return IMAGE_CARTOON;
	}
	else if (text_cmp(image_name, "system.img") == 0)
	{
		return IMAGE_SYSTEM_IMG;
	}
	else if (text_cmp(image_name, "recovery.img") == 0)
	{
		return IMAGE_RECOVERY_IMG;
	}
	else if (text_cmp(image_name, "userdata.img") == 0)
	{
		return IMAGE_USERDATA_IMG;
	}
	else
	{
		return IMAGE_UNKNOWN;
	}
}

int image_type_to_device_index(enum image_type type)
{
	switch (type)
	{
	case IMAGE_UBOOT_BIN:
	case IMAGE_UBOOT_NO_PADDING_BIN:
	case IMAGE_UIMAGE:
	case IMAGE_URAMDISK_IMG:
	case IMAGE_LOGO:
	case IMAGE_BUSYBOX_IMG:
	case IMAGE_CARTOON:
		return 0;

	case IMAGE_SYSTEM_IMG:
		return 2;

	case IMAGE_RECOVERY_IMG:
		return 4;

	case IMAGE_USERDATA_IMG:
		return 5;

	default:
		ERROR_RETURN(EINVAL);
	}
}

int image_type_to_partition_path(enum image_type type, const char *dev_path, char *part_path)
{
	int part_index;
	char *p;

	part_index = image_type_to_device_index(type);
	if (part_index < 0)
	{
		return part_index;
	}

	p = text_copy(part_path, dev_path);

	if (part_index)
	{
		if (device_is_mmc(dev_path))
		{
			*p++ = 'p';
		}

		*p++ = part_index + '0';
	}

	*p = 0;

	return 0;
}

int image_type_to_dd_desc(enum image_type type, struct dd_desc *desc)
{
	switch (type)
	{
	case IMAGE_UBOOT_BIN:
		desc->bs = 1;
		desc->seek = UBOOT_OFFSET;
		desc->skip = UBOOT_PADDING_SIZE;
		desc->count = 0;
		break;

	case IMAGE_UBOOT_NO_PADDING_BIN:
		desc->bs = 1;
		desc->seek = UBOOT_OFFSET;
		desc->skip = 0;
		desc->count = 0;
		break;

	case IMAGE_UIMAGE:
		desc->bs = 1;
		desc->seek = UIMAGE_OFFSET;
		desc->skip = 0;
		desc->count = 0;
		break;

	case IMAGE_URAMDISK_IMG:
		desc->bs = 1;
		desc->seek = RAMDISK_OFFSET;
		desc->skip = 0;
		desc->count = 0;
		break;

	case IMAGE_LOGO:
		desc->bs = 1;
		desc->seek = LOGO_OFFSET;
		desc->skip = 0;
		desc->count = 0;
		break;

	case IMAGE_BUSYBOX_IMG:
		desc->bs = 1;
		desc->seek = BUSYBOX_OFFSET;
		desc->skip = 0;
		desc->count = 0;
		break;

	case IMAGE_CARTOON:
		desc->bs = 1;
		desc->seek = CARTOON_OFFSET;
		desc->skip = 0;
		desc->count = 0;
		break;

	case IMAGE_SYSTEM_IMG:
		desc->bs = 0;
		desc->seek = 0;
		desc->skip = 0;
		desc->count = 0;
		break;

	case IMAGE_RECOVERY_IMG:
		desc->bs = 0;
		desc->seek = 0;
		desc->skip = 0;
		desc->count = 0;
		break;

	case IMAGE_USERDATA_IMG:
		desc->bs = 0;
		desc->seek = 0;
		desc->skip = 0;
		desc->count = 0;
		break;

	default:
		ERROR_RETURN(EINVAL);
	}

	return 0;
}

int uramdisk2ramdisk(const char *uramdisk_path, const char *ramdisk_path)
{
	int ret;

	ret = system_command("rm %s -rfv", ramdisk_path);
	if (ret < 0)
	{
		print_error("system_command");
		return ret;
	}

	ret = cavan_dd(uramdisk_path, ramdisk_path, UIMAGE_HEADER_SIZE, 0, 0);
	if (ret < 0)
	{
		error_msg("cavan_dd");
		return ret;
	}

	return 0;
}

int ramdisk2uramdisk(const char *ramdisk_path, const char *uramdisk_path)
{
	return system_command("mkimage -A arm -O linux -T ramdisk -C none -a 0x90308000 -n \"Android Root Filesystem\" -d %s %s",
		ramdisk_path, uramdisk_path);
}

int dump_ramdisk(const char *ramdisk_path, const char *ramdisk_dir)
{
	int ret;
	char abs_ramdisk_path[1024];

	to_abs_path_base(ramdisk_path, abs_ramdisk_path, sizeof(abs_ramdisk_path));

	ret = system_command("rm %s -rfv && mkdir %s", ramdisk_dir, ramdisk_dir);
	if (ret < 0)
	{
		print_error("system_command");
		return ret;
	}

	ret = chdir_backup(ramdisk_dir);
	if (ret < 0)
	{
		print_error("chdir");
		return ret;
	}

	ret = system_command("cat %s | gzip -d | cpio -i", abs_ramdisk_path);
	if (ret < 0)
	{
		print_error("system_command");
	}

	chdir_backup(NULL);

	return ret;
}

int dump_uramdisk(const char *uramdisk_path, const char *ramdisk_dir)
{
	int ret;

	ret = uramdisk2ramdisk(uramdisk_path, TEMP_RAMDISK_PATH);
	if (ret < 0)
	{
		return ret;
	}

	return dump_ramdisk(TEMP_RAMDISK_PATH, ramdisk_dir);
}

int create_ramdisk(const char *ramdisk_dir, const char *ramdisk_path)
{
	int ret;
	char abs_ramdisk_path[1024];

	ret = system_command("rm %s -rfv", ramdisk_path);
	if (ret < 0)
	{
		print_error("system_command");
		return ret;
	}

	to_abs_path_base(ramdisk_path, abs_ramdisk_path, sizeof(abs_ramdisk_path));

	ret = chdir_backup(ramdisk_dir);
	if (ret < 0)
	{
		print_error("chdir");
		return ret;
	}

	ret = system_command("find | cpio -o -H newc | gzip > %s", abs_ramdisk_path);
	if (ret < 0)
	{
		print_error("system_command");
	}

	chdir_backup(NULL);

	return ret;
}

int create_uramdisk(const char *ramdisk_dir, const char *uramdisk_path)
{
	int ret;
	char abs_uramdisk_path[1024];

	to_abs_path_base(uramdisk_path, abs_uramdisk_path, sizeof(abs_uramdisk_path));

	ret = create_ramdisk(ramdisk_dir, TEMP_RAMDISK_PATH);
	if (ret < 0)
	{
		error_msg("create_ramdisk");
		return ret;
	}

	ret = ramdisk2uramdisk(TEMP_RAMDISK_PATH, abs_uramdisk_path);
	if (ret < 0)
	{
		error_msg("ramdisk2uramdisk");
		return ret;
	}

	return 0;
}

int image_is(const char *img_path, const char *type)
{
	if (text_kmp_find(text_basename(img_path), type) == NULL)
	{
		return 0;
	}

	return 1;
}

int uboot2uboot_np(const char *uboot_path, const char *uboot_np_path)
{
	return cavan_dd_base(uboot_path, uboot_np_path, UBOOT_PADDING_SIZE, 0, 0, O_TRUNC);
}

int uboot_np2uboot(const char *uboot_np_path, const char *uboot_path)
{
	return cavan_dd_base(uboot_np_path, uboot_path, 0, UBOOT_PADDING_SIZE, 0, O_TRUNC);
}

int burn_uboot(const char *uboot_path, const char *dev_path)
{
	umount_device(dev_path, MNT_DETACH);

	return cavan_dd(uboot_path, dev_path, UBOOT_PADDING_SIZE, UBOOT_OFFSET, 0);
}

int burn_uboot_np(const char *uboot_np_path, const char *dev_path)
{
	umount_device(dev_path, MNT_DETACH);

	return cavan_dd(uboot_np_path, dev_path, 0, UBOOT_OFFSET, 0);
}

int zImage2uImage(const char *zImage_path, const char *uImage_path)
{
	return system_command("mkimage -A arm -O linux -T kernel -C none -a 0x90008000 -e 0x90008000 -n \"Android Linux Kernel\" -d %s %s", zImage_path, uImage_path);
}

int uImage2zImage(const char *uImage_path, const char *zImage_path)
{
	return cavan_dd_base(uImage_path, zImage_path, UIMAGE_HEADER_SIZE, 0, 0, O_TRUNC);
}

int burn_uImage(const char *uImage_path, const char *dev_path)
{
	umount_device(dev_path, MNT_DETACH);

	return cavan_dd(uImage_path, dev_path, 0, UIMAGE_OFFSET, 0);
}

int burn_zImage(const char *zImage_path, const char *dev_path)
{
	int ret;

	ret = zImage2uImage(zImage_path, TEMP_UIMAGE_PATH);
	if (ret < 0)
	{
		error_msg("Convert zImage to uImage failed");
		return ret;
	}

	return burn_uImage(TEMP_UIMAGE_PATH, dev_path);
}

int burn_uramdisk(const char *uramdisk_path, const char *dev_path, int busybox)
{
	umount_device(dev_path, MNT_DETACH);

	if (busybox)
	{
		return cavan_dd(uramdisk_path, dev_path, 0, BUSYBOX_OFFSET, 0);
	}
	else
	{
		return cavan_dd(uramdisk_path, dev_path, 0, RAMDISK_OFFSET, 0);
	}
}

int burn_ramdisk(const char *ramdisk_path, const char *dev_path, int busybox)
{
	int ret;

	ret = ramdisk2uramdisk(ramdisk_path, TEMP_URAMDISK_PATH);
	if (ret < 0)
	{
		error_msg("Conver ramdisk to uramdisk failed");
		return ret;
	}

	return burn_uramdisk(TEMP_URAMDISK_PATH, dev_path, busybox);
}

int burn_directory(const char *dir_path, const char *dev_path, int busybox)
{
	int ret;

	ret = create_ramdisk(dir_path, TEMP_RAMDISK_PATH);
	if (ret < 0)
	{
		error_msg("Create ramdisk.img failed");
		return ret;
	}

	return burn_ramdisk(TEMP_RAMDISK_PATH, dev_path, busybox);
}

int image_extend(const char *img_path)
{
	int ret;

	system_sync();

	ret = system_command_retry(5, "e2fsck -fy %s", img_path);
	if (ret < 0)
	{
		error_msg("e2fsck failed");
		return ret;
	}

	ret = system_command("resize2fs -fp %s", img_path);
	if (ret < 0)
	{
		return system_command_retry(5, "e2fsck -fy %s", img_path);
	}

	return 0;
}

int image_resize(const char *img_path, u64 size)
{
	int ret;

	system_sync();

	ret = system_command_retry(5, "e2fsck -fy %s", img_path);
	if (ret < 0)
	{
		error_msg("e2fsck failed");
		return ret;
	}

#if __WORDSIZE == 64
	ret = system_command("resize2fs -fp %s %ldK", img_path, size >> 10);
#else
	ret = system_command("resize2fs -fp %s %LdK", img_path, size >> 10);
#endif
	if (ret < 0)
	{
		return system_command_retry(5, "e2fsck -fy %s", img_path);
	}

	return 0;
}

int image_shrink(const char *img_path)
{
	int ret;

	system_sync();

#if 0
	ret = file_test(img_path, "f");
	if (ret < 0)
	{
		return ret;
	}

	image_extend(img_path);
#endif

	ret = system_command_retry(5, "e2fsck -fy %s", img_path);
	if (ret < 0)
	{
		error_msg("e2fsck failed");
		return ret;
	}

	ret = system_command("resize2fs -fpM %s", img_path);
	if (ret < 0)
	{
		return system_command_retry(5, "e2fsck -fy %s", img_path);
	}

	return 0;
}

int burn_normal_image(const char *img_path, const char *dev_path)
{
	int ret;

	umount_partition(dev_path, MNT_DETACH);

	image_shrink(img_path);

	ret = cavan_dd(img_path, dev_path, 0, 0, 0);
	if (ret < 0)
	{
		error_msg("Burn image \"%s\" to device \"%s\" failed", img_path, dev_path);
		return ret;
	}

	image_extend(dev_path);

	return 0;
}

void adjust_image_device(const char **img_path, const char **dev_path)
{
	const char *tmp_path;

	if (file_test(*dev_path, "b") >= 0)
	{
		return;
	}

	tmp_path = *img_path;
	*img_path = *dev_path;
	*dev_path = tmp_path;
}

int image_is_uramdisk(const char *img_path)
{
	return image_is(img_path, "uramdisk.img") || image_is(img_path, "busybox.img");
}

int get_uramdisk(const char *dev_path, const char *file_path, int busybox)
{
	if (busybox)
	{
		return cavan_dd_base(dev_path, file_path, BUSYBOX_OFFSET, 0, BUSYBOX_MAX_SIZE, O_TRUNC);
	}
	else
	{
		return cavan_dd_base(dev_path, file_path, RAMDISK_OFFSET, 0, RAMDISK_MAX_SIZE, O_TRUNC);
	}
}

int get_dump_uramdisk(const char *dev_path, const char *ramdisk_dir, int busybox)
{
	int ret;

	ret = get_uramdisk(dev_path, TEMP_URAMDISK_PATH, busybox);
	if (ret < 0)
	{
		error_msg("Get uramdisk from device \"%s\"", dev_path);
		return ret;
	}

	return dump_uramdisk(TEMP_URAMDISK_PATH, ramdisk_dir);
}

int get_ramdisk(const char *dev_path, const char *file_path, int busybox)
{
	int ret;

	ret = get_uramdisk(dev_path, TEMP_URAMDISK_PATH, busybox);
	if (ret < 0)
	{
		error_msg("Get ramdisk from device \"%s\"", dev_path);
		return ret;
	}

	return uramdisk2ramdisk(TEMP_URAMDISK_PATH, file_path);
}

int burn_swan_image_directory(const char *dirname, const char *dest_dev)
{
	int ret;
	unsigned int i;
	struct dd_desc descs[] =
	{
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
	};

	ret = chdir_backup(dirname);
	if (ret < 0)
	{
		print_error("chdir");
		return ret;
	}

	if (file_test("ramdisk", "d") == 0)
	{
		ret = create_uramdisk("ramdisk", "uramdisk.img");
		if (ret < 0)
		{
			error_msg("create_uramdisk");
			goto out_chdir_backup;
		}
	}
	else if (file_test("ramdisk.img", "r") == 0)
	{
		ret = ramdisk2uramdisk("ramdisk.img", "uramdisk.img");
		if (ret < 0)
		{
			error_msg("ramdisk2uramdisk");
			goto out_chdir_backup;
		}
	}

	if (dest_dev && dest_dev[0])
	{
		char tmp_path[1024];

		get_device_real_path(tmp_path, dest_dev);

		for (i = 0; i < ARRAY_SIZE(descs); i++)
		{
			text_copy(descs[i].out, tmp_path);
		}
	}

	for (i = 0; i < ARRAY_SIZE(descs); i++)
	{
		if (file_test(descs[i].in, "w") < 0)
		{
			continue;
		}

		umount_partition(descs[i].out, MNT_DETACH);

		ret = cavan_dd2(descs + i);
		if (ret < 0)
		{
			error_msg("burn image \"%s\"", descs[i].in);
			goto out_chdir_backup;
		}

		if (i < 3)
		{
			ret = image_extend(descs[i].out);
			if (ret < 0)
			{
				error_msg("image_extend");
				goto out_chdir_backup;
			}
		}
	}

	ret = 0;
out_chdir_backup:
	chdir_backup(NULL);

	return ret;
}

int burn_swan_image_auto(const char *img_path, const char *dest_dev)
{
	int ret;
	enum image_type type;
	int need_extern;
	struct dd_desc desc;
	char tmp_path[1024];

	desc.in[0] = 0;

	type = path_to_image_type(img_path);
	if (type == IMAGE_UNKNOWN)
	{
		if (file_test(img_path, "d") == 0)
		{
			text_copy(desc.in, TEMP_UIMAGE_PATH);
			ret = create_uramdisk(img_path, desc.in);
			if (ret < 0)
			{
				return ret;
			}

			type = IMAGE_URAMDISK_IMG;
		}
		else
		{
			ERROR_RETURN(EINVAL);
		}
	}

	switch (type)
	{
	case IMAGE_ZIMAGE:
		text_copy(desc.in, TEMP_UIMAGE_PATH);
		ret = zImage2uImage(img_path, desc.in);
		if (ret < 0)
		{
			return ret;
		}
		type = IMAGE_UIMAGE;
		need_extern = 0;
		break;

	case IMAGE_RAMDISK_IMG:
		text_copy(desc.in, TEMP_URAMDISK_PATH);
		ret = ramdisk2uramdisk(img_path, desc.in);
		if (ret < 0)
		{
			return ret;
		}
		type = IMAGE_URAMDISK_IMG;
		need_extern = 0;
		break;

	case IMAGE_SYSTEM_IMG:
	case IMAGE_RECOVERY_IMG:
	case IMAGE_USERDATA_IMG:
		need_extern = 1;
		break;

	default:
		need_extern = 0;
	}

	if (desc.in[0] == 0)
	{
		text_copy(desc.in, img_path);
	}

	ret = image_type_to_dd_desc(type, &desc);
	if (ret < 0)
	{
		return ret;
	}

	get_device_real_path(tmp_path, dest_dev);

	ret = image_type_to_partition_path(type, tmp_path, desc.out);
	if (ret < 0)
	{
		return ret;
	}

	umount_device(tmp_path, MNT_DETACH);

	ret = cavan_dd2(&desc);
	if (ret < 0)
	{
		return ret;
	}

	if (need_extern)
	{
		return image_extend(desc.out);
	}

	return 0;
}

