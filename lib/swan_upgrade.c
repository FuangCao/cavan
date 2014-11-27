// Fuang.Cao <cavan.cfa@gmail.com> Wed Jun 29 10:02:20 CST 2011

#include <cavan.h>
#include <cavan/swan_upgrade.h>
#include <cavan/text.h>
#include <cavan/progress.h>
#include <cavan/file.h>
#include <cavan/device.h>
#include <cavan/bmp.h>
#include <cavan/dd.h>
#include <cavan/swan_dev.h>
#include <cavan/swan_mac.h>
#include <cavan/swan_ts.h>
#include <cavan/swan_pkg.h>
#include <sys/reboot.h>
#include <pthread.h>
#include <cavan/event.h>
#include <cavan/image.h>
#include <cavan/parser.h>
#include <cavan/list.h>
#include <cavan/input.h>

char swan_vfat_volume[128] = EMMC_VFAT_DEFAULT_LABEL;
u32 swan_mkfs_mask = MKFS_MASK_USERDATA | MKFS_MASK_CACHE;
u32 swan_machine_type = SWAN_BOARD_UNKNOWN;
u32 swan_upgrade_flags = 0;
u32 swan_package_flags = 0;
enum swan_image_type swan_exclude_images[16];
int swan_exclude_image_count;
int swan_need_shrink = 1;
struct swan_emmc_partition_table swan_emmc_part_table =
{
	.system_size = SYSTEM_SIZE,
	.recovery_size = RECOVERY_SIZE,
	.userdata_size = USERDATA_SIZE,
	.cache_size = CACHE_SIZE,
	.vendor_size = 0,
};

struct swan_image_info swan_images[] =
{
	{
		.major = 179,
		.minor = FIRST_MINOR + 7,
		.offset = 0,
		.type = SWAN_IMAGE_VENDOR,
		.filename = "vendor.img",
		.device_path = EMMC_DEVICE "p7",
	},
	{
		.major = 179,
		.minor = FIRST_MINOR + 5,
		.offset = 0,
		.type = SWAN_IMAGE_USERDATA,
		.filename = "userdata.img",
		.device_path = EMMC_DEVICE "p5",
	},
	{
		.major = 179,
		.minor = FIRST_MINOR + 4,
		.offset = 0,
		.type = SWAN_IMAGE_RECOVERY,
		.filename = "recovery.img",
		.device_path = EMMC_DEVICE "p4",
	},
	{
		.major = 179,
		.minor = FIRST_MINOR + 2,
		.offset = 0,
		.type = SWAN_IMAGE_SYSTEM,
		.filename = "system.img",
		.device_path = EMMC_DEVICE "p2",
	},
	{
		.major = 179,
		.minor = FIRST_MINOR + 0,
		.offset = CARTOON_OFFSET,
		.type = SWAN_IMAGE_BATTERY,
		.filename = "charge.bmps",
		.device_path = EMMC_DEVICE,
	},
	{
		.major = 179,
		.minor = FIRST_MINOR + 0,
		.offset = CARTOON_OFFSET,
		.type = SWAN_IMAGE_BATTERY,
		.filename = "battery.bmps",
		.device_path = EMMC_DEVICE,
	},
	{
		.major = 179,
		.minor = FIRST_MINOR + 0,
		.offset = BUSYBOX_OFFSET,
		.type = SWAN_IMAGE_BUSYBOX,
		.filename = "busybox.img",
		.device_path = EMMC_DEVICE,
	},
	{
		.major = 179,
		.minor = FIRST_MINOR + 0,
		.offset = LOGO_OFFSET,
		.type = SWAN_IMAGE_LOGO,
		.filename = "logo.bmp",
		.device_path = EMMC_DEVICE,
	},
	{
		.major = 179,
		.minor = FIRST_MINOR + 0,
		.offset = RAMDISK_OFFSET,
		.type = SWAN_IMAGE_URAMDISK,
		.filename = "uramdisk.img",
		.device_path = EMMC_DEVICE,
	},
	{
		.major = 179,
		.minor = FIRST_MINOR + 0,
		.offset = UIMAGE_OFFSET,
		.type = SWAN_IMAGE_UIMAGE,
		.filename = "uImage",
		.device_path = EMMC_DEVICE,
	},
	{
		.major = 179,
		.minor = FIRST_MINOR + 0,
		.offset = UBOOT_OFFSET,
		.type = SWAN_IMAGE_UBOOT,
		.filename = "u-boot-no-padding.bin",
		.device_path = EMMC_DEVICE,
	},
};

static bool swan_keypad_match(struct cavan_event_matcher *matcher, void *data)
{
	return cavan_event_name_matcher(matcher->devname, data, NULL);
}

static void swan_keypad_handler(cavan_input_message_t *message, void *data)
{
	struct cavan_input_message_key *key;

	if (message->type != CAVAN_INPUT_MESSAGE_KEY)
	{
		return;
	}

	key = &message->key;

	if (text_cmp(key->name, "POWER") == 0 || key->code == SWAN_KEY_POWER)
	{
		print_string("Power key was pressed, reset the system ...");
		sync();
		reboot(RB_AUTOBOOT);
	}
}

static void swan_show_picture(const char *state, int reset)
{
	unsigned int i;
	const char *fb_devices[] = {"/dev/fb0", "/dev/graphice/fb0", "/dev/fb1", "/dev/graphice/fb1"};
	struct cavan_input_service service;

	close_console();

	for (i = 0; i < ARRAY_SIZE(fb_devices); i++)
	{
		if (access(fb_devices[i], W_OK) == 0)
		{
			break;
		}
	}

	if (i >= ARRAY_SIZE(fb_devices))
	{
		error_msg("no fb device find");
		goto out_loop;
	}

	println("fb device = %s", fb_devices[i]);

	if (bmp_view(format_text(BMP_PATH "/%s.bmp", state), fb_devices[i]) < 0)
	{
		bmp_view(format_text(BACKUP_BMP_PATH "/%s.bmp", state), fb_devices[i]);
	}

out_loop:
	if (reset == 0)
	{
		sleep(5);
		return;
	}

	print_string("Press \"Power\" reset the system");
	cavan_input_service_init(&service, swan_keypad_match);
	service.handler = swan_keypad_handler;
	cavan_input_service_start(&service, NULL);

	print_string("Press \"Enter\" into command line");
	for (i = REBOOT_TIMEOUT; i > 0; i--)
	{
		int c;

		print("Reboot remain time %d(s)  \r", i);

		c = timeout_getchar(1, 0);

		if (c == '\n')
		{
			cavan_input_service_stop(&service);
			return;
		}
	}

	println("\nSystem Rebooting ...");
	sync();
	reboot(RB_AUTOBOOT);
}

int package(const char *pkg_name, const char *dir_name)
{
	int ret;
	int pkg_fd;
	struct swan_package_info pkg_info;
	struct swan_file_info file_info;
	char tmp_path[1024], *name_p;
	int shrink_image_table[] =
	{
		SWAN_IMAGE_SYSTEM,
		SWAN_IMAGE_RECOVERY,
		SWAN_IMAGE_USERDATA,
		SWAN_IMAGE_VENDOR,
	};
	struct swan_image_info *p, *p_end;

	name_p = text_path_cat(tmp_path, sizeof(tmp_path), dir_name, NULL);

	if (swan_machine_type == SWAN_BOARD_UNKNOWN)
	{
		text_copy(name_p, "system.img");
		swan_machine_type = get_swan_board_type_by_system_img(tmp_path);
	}

	if (swan_machine_type == SWAN_BOARD_UNKNOWN)
	{
		error_msg("Machine type no set");
	}
	else
	{
		struct swan_image_info *img_info;

		img_info = get_swan_image_info_by_type(SWAN_IMAGE_LOGO);
		if (img_info)
		{
			text_copy(img_info->filename, get_logo_name_by_board_type(swan_machine_type));
		}

		img_info = get_swan_image_info_by_type(SWAN_IMAGE_BUSYBOX);
		if (img_info)
		{
			text_copy(img_info->filename, get_busybox_name_by_board_type(swan_machine_type));
		}
	}

	pkg_fd = open(pkg_name, O_RDWR | O_CREAT | O_SYNC | O_TRUNC | O_BINARY, 0777);
	if (pkg_fd < 0)
	{
		print_error("open file \"%s\"", pkg_name);
		return -1;
	}

	text_copy(name_p, HEADER_BIN_NAME);
	ret = write_upgrade_program(pkg_fd, &file_info, tmp_path);
	if (ret < 0)
	{
		error_msg("write_upgrade_program");
		goto out_close_pkg;
	}

	ret = lseek(pkg_fd, sizeof(pkg_info), SEEK_CUR);
	if (ret < 0)
	{
		print_error("lseek");
		goto out_close_pkg;
	}

	ret = write_resource_image(pkg_fd, &pkg_info, dir_name, get_resource_name_by_board_type(swan_machine_type),
		RESOURCE_IMAGE_NAME, I600_RESOURCE_NAME, I200_RESOURCE_NAME, MODEM_RESOURCE_NAME, NULL);
	if (ret < 0)
	{
		error_msg("write_resource_image");
		goto out_close_pkg;
	}

	pkg_info.image_count = 0;

	for (p = swan_images, p_end = p + ARRAY_SIZE(swan_images); p < p_end; p++)
	{
		text_copy(name_p, p->filename);

		if (array_has_element(p->type, (int *) swan_exclude_images, swan_exclude_image_count) || file_test(tmp_path, "r") < 0)
		{
			println_cyan("exclude image \"%s\"", tmp_path);
			continue;
		}

		if (swan_need_shrink && array_has_element(p->type, shrink_image_table, ARRAY_SIZE(shrink_image_table)))
		{
			ret = swan_shrink_image(dir_name, p);
			if (ret < 0)
			{
				error_msg("image_shrink");
				goto out_close_pkg;
			}
		}

		ret = write_simple_image(pkg_fd, dir_name, p, &swan_emmc_part_table);
		if (ret < 0)
		{
			error_msg("write_image");
			goto out_close_pkg;
		}

		pkg_info.image_count++;
	}

	pkg_info.crc32 = 0;
	ret = ffile_crc32_seek(pkg_fd, sizeof(file_info) + file_info.header_size + sizeof(pkg_info), 0, &pkg_info.crc32);
	if (ret < 0)
	{
		error_msg("ffile_crc32");
		goto out_close_pkg;
	}

	strncpy(pkg_info.volume, swan_vfat_volume, sizeof(pkg_info.volume));
	pkg_info.mkfs_mask = swan_mkfs_mask;
	pkg_info.board_type = swan_machine_type;
	pkg_info.upgrade_flags = swan_upgrade_flags;
	pkg_info.part_table = swan_emmc_part_table;

	show_package_info(&pkg_info);

	ret = lseek(pkg_fd, sizeof(file_info) + file_info.header_size, SEEK_SET);
	if (ret < 0)
	{
		print_error("lseek");
		goto out_close_pkg;
	}

	ret = write_package_info(pkg_fd, &pkg_info);
	if (ret < 0)
	{
		print_error("write_package_info");
		goto out_close_pkg;
	}

	ret = swan_set_md5sum(pkg_name);
	if (ret < 0)
	{
		error_msg("swan_set_md5sum");
	}

out_close_pkg:
	close(pkg_fd);

	if (ret < 0)
	{
		remove(pkg_name);
		return ret;
	}

	return 0;
}

int unpack(const char *pkg_name, const char *dir_name)
{
	int ret;
	int pkg_fd;
	int img_fd;
	struct swan_file_info file_info;
	struct swan_package_info pkg_info;
	struct swan_image_info img_info;
	char tmp_path[1024];

	stand_msg("unpack upgrade file \"%s\" -> \"%s\"", pkg_name, dir_name);

	ret = swan_check_md5sum(pkg_name);
	if (ret < 0)
	{
		error_msg("swan_check_md5sum");
		return ret;
	}

	pkg_fd = open(pkg_name, O_RDONLY | O_BINARY);
	if (pkg_fd < 0)
	{
		print_error("open file \"%s\"", pkg_name);
		return -1;
	}

	ret = cavan_mkdir(dir_name);
	if (ret < 0)
	{
		error_msg("cavan_mkdir");
		goto out_close_pkg;
	}

	text_path_cat(tmp_path, sizeof(tmp_path), dir_name, HEADER_BIN_NAME);
	ret = read_upgrade_program(pkg_fd, &file_info, tmp_path);
	if (ret < 0)
	{
		error_msg("get_upgrade_program");
		goto out_close_pkg;
	}

	ret = read_resource_image(pkg_fd, &pkg_info, dir_name, 0, 0);
	if (ret < 0)
	{
		error_msg("get_resource_image");
		goto out_close_pkg;
	}

	while (pkg_info.image_count)
	{
		ret = read_image_info(pkg_fd, &img_info);
		if (ret < 0)
		{
			error_msg("read_image_info");
			goto out_close_pkg;
		}

		text_path_cat(tmp_path, sizeof(tmp_path), dir_name, img_info.filename);
		img_fd = open(tmp_path, O_WRONLY | O_CREAT | O_SYNC | O_TRUNC | O_BINARY, 0777);
		if (img_fd < 0)
		{
			print_error("open");
			goto out_close_pkg;
		}

		ret = read_simple_image(pkg_fd, img_fd, img_info.length, 0);
		if (ret < 0)
		{
			error_msg("get_image");
			goto out_close_img;
		}

		close(img_fd);
		pkg_info.image_count--;
	}

	println_green("Decompression upgrade package \"%s\" to \"%s\" is ok", pkg_name, dir_name);

	ret = 0;
	goto out_close_pkg;

out_close_img:
	close(img_fd);
out_close_pkg:
	close(pkg_fd);

	if (ret < 0)
	{
		rmdir(dir_name);
	}

	return ret;
}

static int open_dest_device(struct swan_image_info *img_info)
{
	int fd;
	int ret;

	fd = open(img_info->device_path, O_RDWR | O_SYNC | O_BINARY);
	if (fd >= 0)
	{
		return fd;
	}

	println("Can't open target device, try to mknod");

	ret = remknod(TEMP_DEVICE_PATH, 0666, makedev(img_info->major, img_info->minor));
	if (ret < 0)
	{
		print_error("remknod");
		return ret;
	}

	return open(TEMP_DEVICE_PATH, O_RDWR | O_SYNC | O_BINARY);
}

static int write2emmc(int pkg_fd, int img_count, int retry_count, enum swan_image_type *skip_types, size_t skip_img_count)
{
	int ret;
	int img_fd;
	u32 tmp_crc32;
	struct swan_image_info img_info;

	while (img_count)
	{
		int retry;

		ret = read_image_info(pkg_fd, &img_info);
		if (ret < 0)
		{
			error_msg("read_image_info");
			return ret;
		}

		if (array_has_element(img_info.type, (int *) skip_types, skip_img_count))
		{
			ret = lseek(pkg_fd, img_info.length, SEEK_CUR);
			if (ret < 0)
			{
				print_error("lseek");
				return ret;
			}

			warning_msg("skipping image \"%s\" ...", swan_image_type_tostring(img_info.type));
			continue;
		}

		img_fd = open_dest_device(&img_info);
		if (img_fd < 0)
		{
			error_msg("open_dest_device");
			return ret;
		}

		retry = retry_count;

		while (retry--)
		{
			off_t pkg_pointer_bak;

			ret = get_file_pointer(pkg_fd, &pkg_pointer_bak);
			if (ret < 0)
			{
				error_msg("can't backup file pointer");
				goto out_close_img;
			}

			ret = read_simple_image(pkg_fd, img_fd, img_info.length, img_info.offset);
			if (ret < 0)
			{
				error_msg("get_image");
				goto out_close_img;
			}

			ret = lseek(img_fd, img_info.offset, SEEK_SET);
			if (ret < 0)
			{
				print_error("lseek");
				goto out_close_img;
			}

			fsync(img_fd);

			println("check image crc32 checksum");

			tmp_crc32 = 0;

			ret = ffile_ncrc32(img_fd, img_info.length, &tmp_crc32);
			if (ret < 0)
			{
				error_msg("ffile_ncrc32");
				goto out_close_img;
			}

			println("img_crc32 = 0x%08x, tmp_crc32 = 0x%08x", img_info.crc32, tmp_crc32);

			if (img_info.crc32 ^ tmp_crc32)
			{
				error_msg("image crc32 checksum is not match, retry = %d", retry);
			}
			else
			{
				right_msg("image crc32 checksum is match");
				break;
			}

			ret = lseek(pkg_fd, pkg_pointer_bak, SEEK_SET);
			if (ret < 0)
			{
				print_error("recovery package file pointer");
				goto out_close_img;
			}
		}

		if (retry < 0)
		{
			ret = -EIO;
			goto out_close_img;
		}

		close(img_fd);
		img_count--;
	}

	return 0;

out_close_img:
	close(img_fd);

	return ret;
}

int upgrade(const char *pkg_name, const char *dir_name)
{
	int ret;
	unsigned int i;
	int pkg_fd;
	struct swan_package_info pkg_info;
	struct swan_file_info file_info;
	struct partition_desc part_descs[] =
	{
		{
			.major = 179,
			.minor = FIRST_MINOR + 1,
			.path = EMMC_DEVICE "p1",
			.label = EMMC_VFAT_DEFAULT_LABEL,
			.type = FS_VFAT,
		},
		{
			.major = 179,
			.minor = FIRST_MINOR + 5,
			.path = EMMC_DEVICE "p5",
			.label = "data",
			.type = FS_EXT4,
		},
		{
			.major = 179,
			.minor = FIRST_MINOR + 6,
			.path = EMMC_DEVICE "p6",
			.label = "cache",
			.type = FS_EXT3,
		},
		{
			.major = 179,
			.minor = FIRST_MINOR + 7,
			.path = EMMC_DEVICE "p7",
			.label = "vendor",
			.type = FS_EXT4,
		},
	};
	struct partition_desc emmc_dev_desc =
	{
		.major = 179,
		.minor = FIRST_MINOR,
		.path = EMMC_DEVICE,
	};
	u32 mkfs_mask_tmp;
	u32 mkfs_mask_table[] =
	{
		MKFS_MASK_VFAT, MKFS_MASK_USERDATA, MKFS_MASK_CACHE, MKFS_MASK_VENDOR
	};

	ret = swan_check_uname();
	if (ret < 0)
	{
		return ret;
	}

	open_console(SWAN_CONSOLE_DEVICE);

	ret = swan_check_emmc_device();
	if (ret < 0)
	{
		swan_show_picture("upgrade", 0);
		return ret;
	}

	ret = swan_check_md5sum(pkg_name);
	if (ret < 0)
	{
		swan_show_picture("upgrade", 0);
		return ret;
	}

	pkg_fd = open(pkg_name, O_RDONLY | O_BINARY);
	if (pkg_fd < 0)
	{
		print_error("open \"%s\"", pkg_name);
		swan_show_picture("upgrade", 0);
		return -1;
	}

	ret = read_upgrade_program(pkg_fd, &file_info, UPGRADE_PROGRAM_PATH ".bak");
	if (ret < 0)
	{
		error_msg("get_upgrade_program");
		swan_show_picture("upgrade", 0);
		goto out_close_pkg;
	}

	ret = read_resource_image(pkg_fd, &pkg_info, "/tmp", 1, 1);
	if (ret < 0)
	{
		error_msg("get_resource_image");
		swan_show_picture("upgrade", 0);
		goto out_close_pkg;
	}

	if ((pkg_info.upgrade_flags & UPGRADE_FLAG_CHECK_VERSION) && swan_if_need_upgrade(EMMC_DEVICE "p2", file_info.version) == 0)
	{
		println_blue("current version is newest, do't need upgrade");
		goto out_upgrade_success;
	}

	if (file_test(UPDATER_SCRIPT_PATH, "x") == 0)
	{
		system_command("%s | tee %s", UPDATER_SCRIPT_PATH, SWAN_CONSOLE_DEVICE);
	}

	if (pkg_info.volume[0])
	{
		strncpy(part_descs[0].label, pkg_info.volume, sizeof(part_descs[0].label));
	}

	ret = swan_sfdisk(&emmc_dev_desc, &pkg_info.part_table);
	if (ret < 0)
	{
		error_msg("swan_sfdisk");
		swan_show_picture("sfdisk", 0);
		goto out_close_pkg;
	}

	mkfs_mask_tmp = pkg_info.mkfs_mask;

	for (i = 0; i < ARRAY_SIZE(mkfs_mask_table); i++)
	{
		if (mkfs_mask_tmp & mkfs_mask_table[i])
		{
			part_descs[i].flags = 0;
		}
		else
		{
			part_descs[i].flags = MKFS_FLAG_TEST;
		}
	}

	if (pkg_info.part_table.vendor_size)
	{
		ret = swan_mkfs(&emmc_dev_desc, part_descs, ARRAY_SIZE(part_descs));
	}
	else
	{
		ret = swan_mkfs(&emmc_dev_desc, part_descs, ARRAY_SIZE(part_descs) - 1);
	}

	if (ret < 0)
	{
		error_msg("swan_mkfs");
		swan_show_picture("mkfs", 0);
		goto out_close_pkg;
	}

	ret = write2emmc(pkg_fd, pkg_info.image_count, 5, NULL, 0);
	if (ret < 0)
	{
		error_msg("write2emmc");
		swan_show_picture("upgrade", 0);
		goto out_close_pkg;
	}

	ret = destroy_environment(EMMC_DEVICE);
	if (ret < 0)
	{
		error_msg("destroy_environment");
		swan_show_picture("check", 0);
		goto out_close_pkg;
	}

	ret = swan_check(EMMC_DEVICE, "245");
	if (ret < 0)
	{
		error_msg("swan_check");
		swan_show_picture("check", 0);
		goto out_close_pkg;
	}

	ret = swan_config_system(EMMC_DEVICE, file_info.version);
	if (ret < 0)
	{
		error_msg("swan_check");
		swan_show_picture("check", 0);
		goto out_close_pkg;
	}

out_upgrade_success:
	swan_ts_calibration(NULL);

	right_msg("Upgrade Successed");
	swan_show_picture("success", 1);

	ret = 0;

out_close_pkg:
	close(pkg_fd);

	return ret;
}

int fupgrade_simple(int pkg_fd)
{
	unsigned int i;
	int ret;
	struct swan_package_info pkg_info;
	struct swan_file_info file_info;
	enum swan_image_type skip_imgs[] = {SWAN_IMAGE_RECOVERY};
	struct partition_desc part_descs[] =
	{
		{
			.major = 179,
			.minor = FIRST_MINOR + 1,
			.path = EMMC_DEVICE "p1",
			.label = EMMC_VFAT_DEFAULT_LABEL,
			.type = FS_VFAT,
		},
		{
			.major = 179,
			.minor = FIRST_MINOR + 5,
			.path = EMMC_DEVICE "p5",
			.label = "data",
			.type = FS_EXT3,
		},
		{
			.major = 179,
			.minor = FIRST_MINOR + 6,
			.path = EMMC_DEVICE "p6",
			.label = "cache",
			.type = FS_EXT3,
		},
	};
	struct partition_desc emmc_dev_desc =
	{
		.major = 179,
		.minor = FIRST_MINOR,
		.path = EMMC_DEVICE,
	};
	u32 mkfs_mask_tmp;
	u32 mkfs_mask_table[] =
	{
		MKFS_MASK_VFAT, MKFS_MASK_USERDATA, MKFS_MASK_CACHE,
	};

	ret = read_upgrade_program(pkg_fd, &file_info, UPGRADE_PROGRAM_PATH ".bak");
	if (ret < 0)
	{
		error_msg("get_upgrade_program");
		return ret;
	}

	ret = read_resource_image(pkg_fd, &pkg_info, NULL, 0, 1);
	if (ret < 0)
	{
		error_msg("get_resource_image");
		return ret;
	}

	if ((pkg_info.upgrade_flags & UPGRADE_FLAG_CHECK_VERSION) && swan_if_need_upgrade(EMMC_DEVICE "p2", file_info.version) == 0)
	{
		right_msg("current version is newest, do't need upgrade");
		goto out_upgrade_success;
	}

	if (pkg_info.volume[0])
	{
		strncpy(part_descs[0].label, pkg_info.volume, sizeof(part_descs[0].label));
	}

	mkfs_mask_tmp = pkg_info.mkfs_mask;

	for (i = 0; i < ARRAY_SIZE(mkfs_mask_table); i++)
	{
		if (mkfs_mask_tmp & mkfs_mask_table[i])
		{
			part_descs[i].flags = 0;
		}
		else
		{
			part_descs[i].flags = MKFS_FLAG_TEST;
		}
	}

	ret = swan_mkfs(&emmc_dev_desc, part_descs, ARRAY_SIZE(part_descs));
	if (ret < 0)
	{
		error_msg("swan_mkfs");
		return ret;
	}

	ret = write2emmc(pkg_fd, pkg_info.image_count, 5, skip_imgs, ARRAY_SIZE(skip_imgs));
	if (ret < 0)
	{
		error_msg("write2emmc");
		return ret;
	}

	ret = swan_check(EMMC_DEVICE, "25");
	if (ret < 0)
	{
		error_msg("swan_check");
		return ret;
	}

	ret = swan_config_system(EMMC_DEVICE, file_info.version);
	if (ret < 0)
	{
		error_msg("swan_check");
		return ret;
	}

	swan_ts_calibration(NULL);

	ret = destroy_environment(EMMC_DEVICE);
	if (ret < 0)
	{
		error_msg("destroy_environment");
		return ret;
	}

out_upgrade_success:
	right_msg("Upgrade Successed");

	return 0;
}

int upgrade_simple(const char *pkg_name, const char *dir_name)
{
	int ret;
	int pkg_fd;

	ret = swan_check_uname();
	if (ret < 0)
	{
		return ret;
	}

	ret = swan_check_emmc_device();
	if (ret < 0)
	{
		swan_show_picture("upgrade", 0);
		return ret;
	}

	ret = swan_check_md5sum(pkg_name);
	if (ret < 0)
	{
		return ret;
	}

	pkg_fd = open(pkg_name, O_RDONLY | O_BINARY);
	if (pkg_fd < 0)
	{
		print_error("open");
		return -1;
	}

	ret = fupgrade_simple(pkg_fd);

	close(pkg_fd);

	return ret;
}

int auto_upgrade(const char *pkg_name)
{
	int ret;
	int pkg_fd;
	struct swan_file_info file_info;

	ret = swan_check_uname();
	if (ret < 0)
	{
		return ret;
	}

	open_console(SWAN_CONSOLE_DEVICE);

	ret = swan_check_emmc_device();
	if (ret < 0)
	{
		swan_show_picture("upgrade", 0);
		return ret;
	}

	if (pkg_name == NULL || file_test(pkg_name, "f") < 0)
	{
		warning_msg("Upgrade packet do't exist, find it now");

		ret = swan_copy(CACHE_FILE_PATH);
		if (ret < 0)
		{
			error_msg("swan_find_file");
			swan_show_picture("copy", 0);
			return ret;
		}

		pkg_name = CACHE_FILE_PATH;
	}

	stand_msg("pkg_name = %s", pkg_name);

	pkg_fd = open(pkg_name, O_RDONLY | O_BINARY);
	if (pkg_fd < 0)
	{
		print_error("open");
		swan_show_picture("copy", 0);
		return -1;
	}

	ret = read_upgrade_program(pkg_fd, &file_info, UPGRADE_PROGRAM_PATH);
	close(pkg_fd);
	if (ret < 0)
	{
		error_msg("get_upgrade_program");
		swan_show_picture("copy", 0);
		return ret;
	}

	close_console();

	return execl(UPGRADE_PROGRAM_PATH, "swan_upgrade", "-u", pkg_name, NULL);
}

enum swan_board_type get_swan_board_type_by_build_prop(const char *build_prop)
{
	struct equation lines[200], *p, *p_end;
	ssize_t count;

	count = parse_config_file_simple(build_prop, '=', lines, ARRAY_SIZE(lines));
	if (count < 0)
	{
		error_msg("parse_config_file2");
		return SWAN_BOARD_UNKNOWN;
	}

	for (p = lines, p_end = lines + count; p < p_end; p++)
	{
		if (text_cmp(p->option, BUILD_PROP_PRODUCT) == 0)
		{
			println("Product name = %s", p->value);

			if (text_rhcmp(p->value, "I200") == 0 || text_rhcmp(p->value, "i200") == 0)
			{
				return SWAN_BOARD_I200;
			}
			else if (text_rhcmp(p->value, "I600") == 0 || text_rhcmp(p->value, "i600") == 0)
			{
				return SWAN_BOARD_I600;
			}
			else if (text_rhcmp(p->value, "I700") == 0 || text_rhcmp(p->value, "i700") == 0)
			{
				return SWAN_BOARD_I700;
			}
			else
			{
				return SWAN_BOARD_UNKNOWN;
			}
		}
	}

	return SWAN_BOARD_UNKNOWN;
}

enum swan_board_type get_swan_board_type_by_system_img(const char *system_img)
{
	int ret;
	enum swan_board_type type;

	ret = file_mount_to(system_img, TEMP_MOUNT_POINT, "ext4", 0, NULL);
	if (ret < 0)
	{
		return SWAN_BOARD_UNKNOWN;
	}

	type = get_swan_board_type_by_build_prop(TEMP_MOUNT_POINT "/" BUILD_PROP_NAME);

	umount_directory2(TEMP_MOUNT_POINT, MNT_DETACH);

	return type;
}

struct swan_image_info *get_swan_image_info_by_name(const char *filename)
{
	struct swan_image_info *p, *end_p;

	for (p = swan_images, end_p = p + ARRAY_SIZE(swan_images); p < end_p; p++)
	{
		if (text_cmp(filename, p->filename) == 0)
		{
			return p;
		}
	}

	return NULL;
}

struct swan_image_info *get_swan_image_info_by_type(enum swan_image_type type)
{
	struct swan_image_info *p, *end_p;

	for (p = swan_images, end_p = p + ARRAY_SIZE(swan_images); p < end_p; p++)
	{
		if ((enum swan_image_type) p->type == type)
		{
			return p;
		}
	}

	return NULL;
}

const char *get_resource_name_by_board_type(enum swan_board_type type)
{
	if (swan_package_flags & SWAN_FLAGS_MODEM_UPGRADE)
	{
		return MODEM_RESOURCE_NAME;
	}

	switch (type)
	{
	case SWAN_BOARD_I200:
		return I200_RESOURCE_NAME;

	case SWAN_BOARD_I600:
		return I600_RESOURCE_NAME;

	case SWAN_BOARD_I700:
		return I700_RESOURCE_NAME;

	default:
		return RESOURCE_IMAGE_NAME;
	}
}

const char *get_logo_name_by_board_type(enum swan_board_type type)
{
	switch (type)
	{
	case SWAN_BOARD_I200:
		return I200_LOGO_NAME;

	case SWAN_BOARD_I600:
		return I600_LOGO_NAME;

	case SWAN_BOARD_I700:
		return I700_LOGO_NAME;

	default:
		return SWAN_LOGO_NAME;
	}
}

const char *get_busybox_name_by_board_type(enum swan_board_type type)
{
	switch (type)
	{
	case SWAN_BOARD_I200:
		return I200_BUSYBOX_NAME;

	case SWAN_BOARD_I600:
		return I600_BUSYBOX_NAME;

	case SWAN_BOARD_I700:
		return I700_BUSYBOX_NAME;

	default:
		return SWAN_LOGO_NAME;
	}
}

u32 swan_read_version(const char *system_dev)
{
	ssize_t readlen;
	char buff[128];

	if (mount_to(system_dev, SYSTEM_MNT_POINT, "ext4", NULL) < 0)
	{
		error_msg("mount deivce \"%s\"", system_dev);
		return 0;
	}

	readlen = file_read(SYSTEM_MNT_POINT "/" SWAN_VERSION_PATH, buff, sizeof(buff));

	umount_partition(system_dev, MNT_DETACH);

	if (readlen <= 0)
	{
		return 0;
	}

	buff[readlen] = 0;

	return text2value_unsigned(buff, NULL, 16);
}

ssize_t swan_write_version(const char *system_mnt_point, u32 version)
{
	ssize_t writelen;
	char buff[128];
	char version_file[1024];

	text_path_cat(version_file, sizeof(version_file), system_mnt_point, SWAN_VERSION_PATH);

	writelen = value2text_base(version, buff, 0, 0, 16) - buff;
	writelen = file_writeto(version_file, buff, writelen, 0, O_TRUNC);
	if (writelen < 0)
	{
		error_msg("write version failed");
	}

	return writelen;
}

int swan_if_need_upgrade(const char *system_dev, u32 version_new)
{
	u32 version_old;

	version_old = swan_read_version(system_dev);

	println("old version = 0x%08x", version_old);
	println("new version = 0x%08x", version_new);

#if 0
	return version_old < version_new;
#else
	return version_old != version_new;
#endif
}

int swan_config_system(const char *emmc_dev, u32 version)
{
	int ret;
	char system_dev[1024];
	const char *system_mnt_point = SYSTEM_MNT_POINT;

	text_copy(text_copy(system_dev, emmc_dev), "p2");

	ret = mount_to(system_dev, system_mnt_point, "ext4", NULL);
	if (ret < 0)
	{
		error_msg("mount system device failed");
		return ret;
	}

	ret = swan_write_version(system_mnt_point, version);
	if (ret < 0)
	{
		error_msg("swan_write_version");
		goto out_umount_system;
	}

	ret = write_mac_address(emmc_dev, system_mnt_point);
	if (ret < 0)
	{
		error_msg("write mac address faild");
	}

out_umount_system:
	umount_partition(system_dev, 0);

	return ret;
}

