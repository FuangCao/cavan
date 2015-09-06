#include <cavan.h>
#include <cavan/swan_dev.h>
#include <cavan/text.h>
#include <cavan/swan_upgrade.h>
#include <cavan/file.h>
#include <cavan/dd.h>
#include <cavan/device.h>
#include <cavan/image.h>
#include <cavan/parser.h>
#include <sys/utsname.h>

void show_swan_emmc_partation_table(struct swan_emmc_partition_table *part_table)
{
	println("system = %dMB", part_table->system_size);
	println("recovery = %dMB", part_table->recovery_size);
	println("userdata = %dMB", part_table->userdata_size);
	println("cache = %dMB", part_table->cache_size);
	println("vendor = %dMB", part_table->vendor_size);
}

ssize_t get_partition_size_by_type(enum swan_image_type type, struct swan_emmc_partition_table *part_table)
{
	switch (type)
	{
	case SWAN_IMAGE_SYSTEM:
		return part_table->system_size;
	case SWAN_IMAGE_RECOVERY:
		return part_table->recovery_size;
	case SWAN_IMAGE_USERDATA:
		return part_table->userdata_size;
	case SWAN_IMAGE_VENDOR:
		return part_table->vendor_size;
	default:
		return -EINVAL;
	}
}

void get_default_emmc_partition_table(struct swan_emmc_partition_table *part_table)
{
	part_table->system_size = SYSTEM_SIZE;
	part_table->recovery_size = RECOVERY_SIZE;
	part_table->userdata_size = USERDATA_SIZE;
	part_table->cache_size = CACHE_SIZE;
}

int fix_emmc_partition_table(struct swan_emmc_partition_table *part_table)
{
	int ret = 0;

	if (part_table->system_size < SYSTEM_MIN_SIZE)
	{
		ret++;
		warning_msg("system partition size is too small");
		part_table->system_size = SYSTEM_MIN_SIZE;
	}

	if (part_table->recovery_size < RECOVERY_MIN_SIZE)
	{
		ret++;
		warning_msg("recovery partition size is too small");
		part_table->recovery_size = RECOVERY_MIN_SIZE;
	}

	if (part_table->userdata_size < USERDATA_MIN_SIZE)
	{
		ret++;
		warning_msg("userdata partition size is too small");
		part_table->userdata_size = USERDATA_MIN_SIZE;
	}

	if (part_table->cache_size < CACHE_MIN_SIZE)
	{
		ret++;
		warning_msg("cache partition size is too small");
		part_table->cache_size = CACHE_MIN_SIZE;
	}

	return ret;
}

int swan_sfdisk(struct partition_desc *dev_desc, struct swan_emmc_partition_table *part_table)
{
	u64 total_size;
	int vfat_size, expand_size;
	int ret;

	ret = partition_test(dev_desc);
	if (ret < 0)
	{
		return ret;
	}

	umount_device(dev_desc->path, MNT_DETACH);

	ret = cavan_dd("/dev/zero", dev_desc->path, 0, 0, 512);
	if (ret < 0)
	{
		error_msg("cavan_dd");
		return ret;
	}

	ret = get_device_size(dev_desc->path, &total_size);
	if (ret < 0)
	{
		error_msg("get_device_size");
		return ret;
	}

	println("Total Size = %s", size2text(total_size));

	total_size = BM(total_size);

	ret = fix_emmc_partition_table(part_table);
	if (ret)
	{
		warning_msg("some partition size is invalid");
	}

	if (part_table->system_size + part_table->recovery_size + part_table->userdata_size + part_table->cache_size + part_table->vendor_size > total_size)
	{
		get_default_emmc_partition_table(part_table);
	}

	expand_size = part_table->userdata_size + part_table->cache_size + part_table->vendor_size;
	vfat_size = total_size - part_table->system_size - expand_size - part_table->recovery_size;

	if (part_table->vendor_size)
	{
		ret = system_command("sfdisk %s -uM -f << EOF\n,%d,L\n,%d,L,*\n,%d,E\n,,L\n,%d,L\n,%d,L\n,,L\nEOF\n", \
			dev_desc->path, vfat_size, part_table->system_size, expand_size, part_table->userdata_size, part_table->cache_size);
	}
	else
	{
		ret = system_command("sfdisk %s -uM -f << EOF\n,%d,L\n,%d,L,*\n,%d,E\n,,L\n,%d,L\n,,L\nEOF\n", \
			dev_desc->path, vfat_size, part_table->system_size, expand_size, part_table->userdata_size);
	}

	if (ret < 0)
	{
		print_error("sfdisk");
		return ret;
	}

	ret = system_command("sfdisk %s -uM -N1 -f << EOF\n%d,%d,\nEOF\n", \
		dev_desc->path, BOOT_SIZE, vfat_size - BOOT_SIZE);
	if (ret < 0)
	{
		print_error("sfdisk");
		return ret;
	}

	ret = reread_part_table_retry(dev_desc->path, 100);
	if (ret < 0)
	{
		error_msg("reread_part_table_retry");
		return ret;
	}

	return 0;
}

int swan_mkfs(struct partition_desc *dev_desc, struct partition_desc *part_descs, int count)
{
	int ret;
	struct partition_desc *p;

	ret = partition_test(dev_desc);
	if (ret < 0)
	{
		return ret;
	}

#if 0
	ret = reread_part_table_retry(dev_desc->path, 100);
	if (ret < 0)
	{
		error_msg("reread_part_table_retry");
		return ret;
	}
#endif

	for (p = part_descs + count - 1; p >= part_descs; p--)
	{
		int i;

		for (i = 5; i > 0 && partition_mkfs(p) < 0; i--);

		if (i <= 0)
		{
			warning_msg("format partition \"%s\" failed, try to change label", p->path);
			partition_change_label(p);
		}
	}

	return 0;
}

static int swan_copy_base(const char *dev_path, const char *cache_file)
{
	int ret;

	ret = mount_main(dev_path, MOUNT_POINT, "vfat", NULL);
	if (ret < 0)
	{
		error_msg("mount_main");
		return ret;
	}

	ret = file_copy(UPGRADE_FILE_PATH, cache_file, O_SYNC);
	if (ret < 0)
	{
		error_msg("file_copy");
	}

	umount_directory_wait(MOUNT_POINT);

	return ret;
}

int swan_copy(const char *cache_file)
{
	unsigned int i;
	int ret;
	const char *dev_list[] = { SDCARD_DEVICE "p1", SDCARD_DEVICE, EMMC_DEVICE "p1", EMMC_DEVICE };

	for (i = 0; i < ARRAY_SIZE(dev_list); i++)
	{
		ret = swan_copy_base(dev_list[i], cache_file);
		if (ret >= 0)
		{
			return 0;
		}
	}

	return -1;
}

int swan_find_upgrade_file_from(const char *dev_path, const char *tmp_file)
{
	int ret;

	ret = mount_main(dev_path, MOUNT_POINT, "vfat", NULL);
	if (ret < 0)
	{
		warning_msg("mount device \"%s\" failed", dev_path);
		return ret;
	}

	ret = system_command("find %s -maxdepth 1 -name \"%s*.%s\" | tee %s", MOUNT_POINT, UPGRADE_FILE_PREFIX, UPGRADE_FILE_TYPE, tmp_file);
	if (ret < 0)
	{
		warning_msg("no upgrade package find");
		umount_directory2(MOUNT_POINT, MNT_DETACH);
	}
	else
	{
		right_msg("find a upgrade package from device \"%s\"", dev_path);
	}

	return ret;
}

int swan_find_upgrade_file(const char *tmp_file)
{
	unsigned int i;
	int ret;
	const char *dev_list[] = { SDCARD_DEVICE "p1", SDCARD_DEVICE, EMMC_DEVICE "p1", EMMC_DEVICE };

	system_command("rm /tmp/* %s -rf", tmp_file);

	for (i = 0; i < ARRAY_SIZE(dev_list); i++)
	{
		ret = swan_find_upgrade_file_from(dev_list[i], tmp_file);
		if (ret >= 0)
		{
			return 0;
		}
	}

	return -1;
}

int destroy_environment(const char *dev_path)
{
	stand_msg("Destory Uboot Environment Variable");

	return cavan_dd("/dev/zero", dev_path, 0, KB(200), KB(800));
}

int swan_check(const char *dev_path, const char *list_partition)
{
	while (*list_partition)
	{
		image_extend(format_text("%sp%c", dev_path, *list_partition));
		list_partition++;
	}

	return 0;
}

int fset_brightness_shadow_increase(int fd, int current, int target, int step, u32 msec)
{
	char buff[32];
	int writelen;

	while (current + step < target)
	{
		current += step;

		writelen = value2text_base(current, buff, 0, 0, 10) - buff;
		writelen = write(fd, buff, writelen);
		if (writelen < 0)
		{
			return writelen;
		}

		msleep(msec);
	}

	writelen = value2text_base(target, buff, 0, 0, 10) - buff;

	return write(fd, buff, writelen);
}

int fset_brightness_shadow_decrease(int fd, int current, int target, int step, u32 msec)
{
	char buff[32];
	int writelen;

	while (current - step > target)
	{
		current -= step;

		writelen = value2text_base(current, buff, 0, 0, 10) - buff;
		writelen = write(fd, buff, writelen);
		if (writelen < 0)
		{
			return writelen;
		}

		msleep(msec);
	}

	writelen = value2text_base(target, buff, 0, 0, 10) - buff;

	return write(fd, buff, writelen);
}

int set_brightness_shadow(int brightness, int step, u32 msec)
{
	int fd;
	char buff[32];
	int ret;
	ssize_t readlen;
	int current_brightness;

	fd = open(BRIGHTNESS_PATH, O_RDWR | O_SYNC | O_BINARY);
	if (fd < 0)
	{
		print_error("open file \"" BRIGHTNESS_PATH "\" failed");
		return fd;
	}

	readlen = read(fd, buff, sizeof(buff));
	if (readlen < 0)
	{
		ret = readlen;
		print_error("read");
		goto out_close_fd;
	}

	buff[readlen] = 0;

	current_brightness = text2value_unsigned(buff, NULL, 10);

	if (current_brightness > brightness)
	{
		ret = fset_brightness_shadow_decrease(fd, current_brightness, brightness, step, msec);
	}
	else if (current_brightness < brightness)
	{
		ret = fset_brightness_shadow_increase(fd, current_brightness, brightness, step, msec);
	}
	else
	{
		ret = 0;
	}

out_close_fd:
	close(fd);

	return ret;
}

int display_config(int text_mode)
{
	file_write(BRIGHTNESS_PATH, "0", 1);
	file_write(IPU_ENABLE_FILE, "on", 2);

	if (text_mode)
	{
		println("Set console mode to text mode");
		switch2graph_mode(SWAN_CONSOLE_DEVICE);
		switch2text_mode(SWAN_CONSOLE_DEVICE);
	}

	set_brightness_shadow(255, 20, 100);

	return 0;
}

static struct swan_machine swan_machine_table[] =
{
	{
		.type = SWAN_BOARD_I200,
		.name = "I2xx Serials Boards",
	},
	{
		.type = SWAN_BOARD_I600,
		.name = "I6xx Serials Boards",
	},
	{
		.type = SWAN_BOARD_I600,
		.name = "Eavoo MX51 Swan Board",
	},
	{
		.type = SWAN_BOARD_I700,
		.name = "I7xx Serials Boards",
	},
};

int swan_board_name_to_type(const char *name)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(swan_machine_table); i++)
	{
		if (text_lhcmp(swan_machine_table[i].name, name) == 0)
		{
			return swan_machine_table[i].type;
		}
	}

	return SWAN_BOARD_UNKNOWN;
}

const char *swan_board_type_tostring(enum swan_board_type type)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(swan_machine_table); i++)
	{
		if (swan_machine_table[i].type == type)
		{
			return swan_machine_table[i].name;
		}
	}

	return "unknown";
}

enum swan_board_type get_swan_board_type(void)
{
	struct equation lines[100], *p, *p_end;
	ssize_t count;

	count = parse_config_file_simple(FILE_PROC_CPUINFO, ':', lines, ARRAY_SIZE(lines));
	if (count < 0)
	{
		error_msg("parse_config_file2");
		return count;
	}

	for (p = lines, p_end = lines + count; p < p_end; p++)
	{
		if (text_cmp(p->option, CPUINFO_LINE_MACHINE_NAME) == 0)
		{
			println("Machine name = %s", p->value);
			return swan_board_name_to_type(p->value);
		}
	}

	return SWAN_BOARD_UNKNOWN;
}

int swan_board_type_check(int pkg_type)
{
	int board_type;

	if (pkg_type == SWAN_BOARD_UNKNOWN)
	{
		return 0;
	}

	board_type = get_swan_board_type();
	if (board_type == SWAN_BOARD_UNKNOWN)
	{
		return 0;
	}

	if (pkg_type != board_type)
	{
		ERROR_RETURN(EINVAL);
	}

	return 0;
}

int swan_check_emmc_device(void)
{
	if (file_access_e("/dev/mmcblk1") == 0 || file_access_e("/dev/mmcblk0p2") == 0)
	{
		return 0;
	}

	if (file_access_e("/dev/block/mmcblk1") == 0 || file_access_e("/dev/block/mmcblk0p2") == 0)
	{
		return 0;
	}

	pr_red_info("EMMC device maby bad!");

	return -ENOENT;
}

int swan_check_uname(void)
{
	int ret;
	struct utsname uts;

	ret = uname(&uts);
	if (ret < 0)
	{
		pr_blue_info("get utsname failed");
		return 0;
	}

	println("%s %s %s %s %s", uts.sysname, uts.nodename, uts.release, uts.version, uts.machine);

	if (text_cmp("Linux", uts.sysname) || text_lhcmp("arm", uts.machine))
	{
		pr_red_info("This machine is not arm linux");
		return -EFAULT;
	}

	return 0;
}

