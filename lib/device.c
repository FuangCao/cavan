#include <cavan.h>
#include <cavan/device.h>
#include <cavan/text.h>
#include <cavan/file.h>
#include <sys/vfs.h>

#define MAX_MOUNT_COUNT		100

int fget_device_size(int dev_fd, u64 *size)
{
	int ret;
	long sec_count;

	ret = ioctl(dev_fd, BLKGETSIZE64, size);
	if (ret >= 0)
	{
		return ret;
	}

	ret = ioctl(dev_fd, BLKGETSIZE, &sec_count);
	if (ret < 0)
	{
		return ret;
	}

	size[0] = sec_count * 512;

	return 0;
}

int get_device_size(const char *dev_path, u64 *size)
{
	int ret;
	int dev_fd;

	dev_fd = open(dev_path, 0);
	if (dev_fd < 0)
	{
		print_error("open");
		return -1;
	}

	ret = fget_device_size(dev_fd, size);

	close(dev_fd);

	return ret;
}

int umount_directory_wait(const char *mnt_point)
{
	int ret;
	int count;

	print("%s => umount ...", mnt_point);

	sync();

	for (count = 0; (ret = umount(mnt_point)) == 0 || errno == EBUSY; count++)
	{
		print_text(".");
	}

	if (count)
	{
		update_mount_table();
		print_string(" OK");
	}
	else
	{
		print_string(" Failed");
	}

	return count ? 0 : ret;
}

int umount_directory2(const char *mnt_point, int flags)
{
	int ret;
	int count;

	print("%s => umount ...", mnt_point);

	sync();

	for (count = 0; (ret = umount2(mnt_point, flags)) == 0; count++)
	{
		print_text(".");
	}

	if (count)
	{
		update_mount_table();
		print_string(" OK");
	}
	else
	{
		print_string(" Failed");
	}

	return count ? 0 : ret;
}

int umount_partition(const char *dev_path, int flags)
{
	struct mount_table *mtab, *p, *end_p;
	ssize_t readlen;
	char abs_path[1024];

	if (dev_path == NULL)
	{
		pr_red_info("dev_path == NULL");
		ERROR_RETURN(EINVAL);
	}

	if (to_abs_path2_base(dev_path, abs_path, sizeof(abs_path)) == NULL)
	{
		ERROR_RETURN(ENOENT);
	}

	mtab = alloca(sizeof(*mtab) * MAX_MOUNT_COUNT);
	if (mtab == NULL)
	{
		pr_error_info("alloca");
		return -EFAULT;
	}

	readlen = read_mount_table(mtab, MAX_MOUNT_COUNT);
	if (readlen < 0)
	{
		return readlen;
	}

	for (p = mtab, end_p = mtab + readlen; p < end_p; p++)
	{
		if (strcmp(p->source, abs_path) == 0)
		{
			int ret;

			ret = umount_directory2(p->target, flags);
			if (ret < 0)
			{
				return ret;
			}

			if (text_lhcmp("/dev/loop", abs_path) == 0)
			{
				loop_clr_fd(abs_path);
			}
		}
	}

	return 0;
}

int umount_by_key_text(const char *key_text, int flags)
{
	int ret;
	char *file_mem;
	size_t file_size;
	const char *p, *end_p;

	file_mem = file_read_mounts(&file_size);
	if (file_mem == NULL)
	{
		pr_red_info("file_read_mounts");
		return -EFAULT;
	}

	p = file_mem;
	end_p = file_mem + file_size;

	while (p < end_p)
	{
		struct mount_table mtab;

		p = find_mount_table_base(p, &mtab, key_text);
		if (p == NULL)
		{
			break;
		}

		ret = umount_directory2(mtab.target, flags);
		if (ret < 0)
		{
			goto out_free_file_mem;
		}

		while (p < end_p && *p != '\n')
		{
			p++;
		}

		if (p < end_p)
		{
			p++;
		}
		else
		{
			break;
		}
	}

	ret = 0;

out_free_file_mem:
	free(file_mem);
	return ret;
}

int device_is_mounted_base(const char *dev_abs_path)
{
	ssize_t readlen;
	struct mount_table *mtab, *p, *end_p;

	mtab = alloca(sizeof(*mtab) * MAX_MOUNT_COUNT);
	if (mtab == NULL)
	{
		pr_error_info("alloca");
		return -EFAULT;
	}

	readlen = read_mount_table(mtab, MAX_MOUNT_COUNT);
	if (readlen < 0)
	{
		error_msg("read mount table failed");
		return readlen;
	}

	for (p = mtab, end_p = mtab + readlen; p < end_p; p++)
	{
		if (text_lhcmp(dev_abs_path, p->source) == 0)
		{
			return 1;
		}
	}

	return 0;
}

int device_is_mounted(const char *dev_path)
{
	char abs_path[1024];

	if (to_abs_path2_base(dev_path, abs_path, sizeof(abs_path)) == NULL)
	{
		error_msg("path \"%s\" do't exist", dev_path);
		ERROR_RETURN(ENOENT);
	}

	return device_is_mounted_base(abs_path);
}

int umount_device(const char *dev_path, int flags)
{
	ssize_t readlen;
	char abs_path[1024];
	struct mount_table *mtab, *p, *end_p;

	if (dev_path == NULL)
	{
		pr_red_info("dev_path == NULL");
		ERROR_RETURN(EINVAL);
	}
	else
	{
		char c;

		c = text_get_char(dev_path, -1);
		if (IS_NUMBER(c))
		{
			return umount_partition(dev_path, flags);
		}
	}

	if (to_abs_path2_base(dev_path, abs_path, sizeof(abs_path)) == NULL)
	{
		error_msg("path \"%s\" do't exist", dev_path);
		ERROR_RETURN(ENOENT);
	}

	mtab = alloca(sizeof(*mtab) * MAX_MOUNT_COUNT);
	if (mtab == NULL)
	{
		pr_error_info("alloca");
		return -EFAULT;
	}

	readlen = read_mount_table(mtab, MAX_MOUNT_COUNT);
	if (readlen < 0)
	{
		error_msg("read mount table failed");
		return readlen;
	}

	for (p = mtab, end_p = mtab + readlen; p < end_p; p++)
	{
		if (text_lhcmp(abs_path, p->source) == 0)
		{
			umount_directory2(p->target, flags);

			if (text_lhcmp("/dev/loop", abs_path) == 0)
			{
				loop_clr_fd(abs_path);
			}
		}
	}

	return device_is_mounted_base(abs_path) ? -EFAULT : 0;
}

int umount_abs_path(const char *abs_path, int flags)
{
	ssize_t readlen;
	struct mount_table *mtab, *p, *end_p;

	mtab = alloca(sizeof(*mtab) * MAX_MOUNT_COUNT);
	if (mtab == NULL)
	{
		pr_error_info("alloca");
		return -EFAULT;
	}

	readlen = read_mount_table(mtab, MAX_MOUNT_COUNT);
	if (readlen < 0)
	{
		error_msg("read mount table failed");
		return readlen;
	}

	for (p = mtab, end_p = mtab + readlen; p < end_p; p++)
	{
		if (text_lhcmp(abs_path, p->source) == 0 || text_lhcmp(abs_path, p->target) == 0)
		{
			umount_directory2(p->target, flags);

			if (text_lhcmp("/dev/loop", p->source) == 0)
			{
				loop_clr_fd(p->source);
			}
		}
	}

	return device_is_mounted_base(abs_path) ? -EFAULT : 0;
}

int umount_main(const char *pathname, int flags)
{
	char abs_path[1024];

	if (file_test(pathname, "e") < 0 && pathname[0] != '/')
	{
		sprintf(abs_path, "/dev/sd%s", pathname);

		if (file_test(abs_path, "e") < 0)
		{
			sprintf(abs_path, "/dev/%s", pathname);
		}
	}
	else if (to_abs_path2_base(pathname, abs_path, sizeof(abs_path)) == NULL)
	{
		error_msg("path \"%s\" do't exist", pathname);
		ERROR_RETURN(ENOENT);
	}

	println("Device path = %s", abs_path);

	return umount_abs_path(abs_path, flags);
}

int umount_all1(char *devs[], size_t count, int flags)
{
	size_t i;
	int ret, ret_tmp;

	for (i = 0, ret = 0; i < count; i++)
	{
		if (devs[i] == NULL)
		{
			continue;
		}

		ret_tmp = umount_main(devs[i], flags);
		if (ret_tmp < 0)
		{
			if (errno == EPERM)
			{
				return ret_tmp;
			}

			ret = ret_tmp;
		}
	}

	return ret;
}

int umount_all2(char *devs[], int flags)
{
	int i;
	int ret;

	for (i = 0; devs[i]; i++)
	{
		ret = umount_main(devs[i], flags);
		if (ret < 0)
		{
			return ret;
		}
	}

	return 0;
}

int vumount_all3(int flags, va_list ap)
{
	while (1)
	{
		int ret;
		const char *devpath = va_arg(ap, const char *);

		if (devpath == NULL)
		{
			break;
		}

		ret = umount_main(devpath, flags);
		if (ret < 0)
		{
			return ret;
		}
	}

	return 0;
}

int umount_all3(int flags, ...)
{
	int ret;
	va_list ap;

	va_start(ap, flags);
	ret = vumount_all3(flags, ap);
	va_end(ap);

	return ret;
}

int cavan_mkdir(const char *dir_path)
{
	remove(dir_path);

	return mkdir_hierarchy(dir_path, 0777);
}

int system_mount(const char *mnt_dev, const char *mnt_point, const void *data)
{
	int ret;

	ret = cavan_mkdir(mnt_point);
	if (ret < 0)
	{
		return ret;
	}

	if (file_test(mnt_dev, "b") == 0)
	{
		if (data && *(const char *)data)
		{
			return system_command("mount %s %s -o %s", mnt_dev, mnt_point, (const char *)data);
		}
		else
		{
			return system_command("mount %s %s", mnt_dev, mnt_point);
		}
	}
	else
	{
		if (data && *(const char *)data)
		{
			return system_command("mount %s %s -o loop,%s", mnt_dev, mnt_point, (const char *)data);
		}
		else
		{
			return system_command("mount %s %s -o loop", mnt_dev, mnt_point);
		}
	}
}

ssize_t parse_filesystems(int fd, char (*fstypes)[FSTYPE_NAME_LEN], size_t fstype_size)
{
	char (*fstypes_bak)[FSTYPE_NAME_LEN] = fstypes;
	char (*end_fstypes)[FSTYPE_NAME_LEN] = fstypes + fstype_size;

	while (fstypes < end_fstypes)
	{
		ssize_t rdlen;
		char buff[512];
		char temp[FSTYPE_NAME_LEN];

		rdlen = file_read_line(fd, buff, sizeof(buff));
		if (rdlen < 1)
		{
			if (rdlen < 0)
			{
				pr_red_info("file_read_line");
				return rdlen;
			}

			break;
		}

		rdlen = sscanf(buff, "%s %s", *fstypes, temp);
		if (rdlen == 1 || text_lhcmp("nodev", *fstypes))
		{
			fstypes++;
		}
	}

	return fstypes - fstypes_bak;
}

ssize_t read_filesystems(char (*fstypes)[FSTYPE_NAME_LEN], size_t fstype_size)
{
	int fd;
	ssize_t count;

	fd = open(FILE_PROC_FILESYSTEMS, O_RDONLY);
	if (fd < 0)
	{
		pr_error_info("open file `%s' failed", FILE_PROC_FILESYSTEMS);
		return fd;
	}

	count = parse_filesystems(fd, fstypes, fstype_size);
	close(fd);

	return count;
}

int partition_is_mounted(const char *source, const char *target)
{
	char mnt_point[1024];

	if (to_abs_path2_base(target, mnt_point, sizeof(mnt_point)) == NULL)
	{
		return 0;
	}

	return text_cmp(get_mount_target(source), mnt_point) == 0;
}

int mount_update_mtab(const char *source, const char *target, const char *fstype, unsigned long flags, const void *data)
{
	int ret;

	ret = mount(source, target, fstype, flags, data);
	if (ret < 0)
	{
		if (errno == EBUSY && partition_is_mounted(source, target))
		{
			right_msg("partition \"%s\" is already mounted to \"%s\"", source, target);
			return 0;
		}
		else
		{
			return ret;
		}
	}

	update_mount_table();

	return 0;
}

int libc_mount_auto(const char *source, const char *target, unsigned long flags, const void *data)
{
	unsigned int i;
	int ret;
	const char *fstypes[] = {"ext4", "vfat", "fuseblk", "ext3", "ext2"};

	for (i = 0; i < ARRAY_SIZE(fstypes); i++)
	{
		ret = mount_update_mtab(source, target, fstypes[i], flags, data);
		if (ret == 0)
		{
			println("filesystem type = %s", fstypes[i]);
			return 0;
		}
	}

	return -1;
}

int libc_mount(const char *source, const char *target, const char *fstype, unsigned long flags, const void *data)
{
	if (fstype)
	{
		int ret;

		ret = mount_update_mtab(source, target, fstype, flags, data);
		if (ret >= 0)
		{
			return 0;
		}
	}

	return libc_mount_auto(source, target, flags, data);
}

int libc_mount_to(const char *source, const char *target, const char *fstype, unsigned long flags, const void *data)
{
	int ret;

	ret = cavan_mkdir(target);
	if (ret < 0)
	{
		return ret;
	}

	ret = libc_mount(source, target, fstype, flags, data);
	if (ret >= 0)
	{
		return 0;
	}

#ifdef CAVAN_DEBUG
	warning_msg("rw mount failed, retry use ro");
#endif

	return libc_mount(source, target, fstype, flags | MS_RDONLY, data);
}

int mount_to(const char *mnt_dev, const char *mnt_point, const char *fstype, const void *data)
{
	int ret;
	struct stat st;

	if (fstype && fstype[0] == 0)
	{
		fstype = NULL;
	}

	println("%s [mount]-> %s", mnt_dev, mnt_point);

	if (file_stat2(mnt_dev, &st) < 0)
	{
		ret =  libc_mount_to(mnt_dev, mnt_point, fstype, 0, data);
		goto label_check;
	}

	switch (st.st_mode & S_IFMT)
	{
	case S_IFBLK:
		ret = libc_mount_to(mnt_dev, mnt_point, fstype, 0, data);
		break;

	case S_IFREG:
	case S_IFLNK:
		ret = file_mount_to(mnt_dev, mnt_point, fstype, 0, data);
		break;

	default:
		warning_msg("unknown deivce type");
		ret = -1;
	}

label_check:
	if (ret >= 0 || errno == EPERM)
	{
		return ret;
	}

	return system_mount(mnt_dev, mnt_point, data);
}

int mount_main(const char *mnt_dev, const char *mnt_point, const char *fstype, const void *data)
{
	char source[1024], target[1024];

	if (mnt_point == NULL || mnt_point[0] == 0)
	{
		int ret;
		char part_label[32];

		ret = partition_read_label_auto(mnt_dev, part_label, sizeof(part_label));
		if (ret < 0 || part_label[0] == 0)
		{
			text_basename_base(text_copy(target, "/mnt/"), mnt_dev);
		}
		else
		{
			sprintf(target, "/mnt/%s", part_label);
		}

		text_copy(source, mnt_dev);
	}
	else if (file_test(mnt_dev, "bfl") == 0)
	{
		text_copy(source, mnt_dev);
		text_copy(target, mnt_point);
	}
	else if (file_test(mnt_point, "bfl") == 0)
	{
		text_copy(source, mnt_point);
		text_copy(target, mnt_dev);
	}
	else
	{
		error_msg("No mount device find");
		ERROR_RETURN(ENODEV);
	}

	return mount_to(source, target, fstype, data);
}

int can_mount(const char *mnt_dev, const char *fstype)
{
	int ret;

	ret = mount_main(mnt_dev, TEMP_MOUNT_POINT, fstype, NULL);
	if (ret < 0)
	{
		return 0;
	}

	umount_directory2(TEMP_MOUNT_POINT, 0);

	return 1;
}

int simple_ioctl(const char *dev_path, int request)
{
	int fd;
	int ret;

	fd = open(dev_path, 0);
	if (fd < 0)
	{
		print_error("open device \"%s\"", dev_path);
		return -1;
	}

	ret = fsimple_ioctl(fd, request);

	close(fd);

	return ret;
}

int reread_part_table(const char *dev_path)
{
	int ret;
	int dev_fd;

	println("Re-Read Partition Table From \"%s\"", dev_path);

	dev_fd = open(dev_path, 0);
	if (dev_fd < 0)
	{
		print_error("open device \"%s\"", dev_path);
		return -1;
	}

	umount_device(dev_path, MNT_DETACH);

	ret = freread_part_table(dev_fd);

	close(dev_fd);

	if (ret < 0)
	{
		return ret;
	}

	system_sync();

	return 0;
}

int freread_part_table_retry(int dev_fd, int count)
{
	int ret = 0;

	while (count--)
	{
		ret = freread_part_table(dev_fd);
		if (ret < 0)
		{
			println("retry remaining count = %d", count);
			sleep(2);
		}
		else
		{
			break;
		}
	}

	fsync(dev_fd);

	return ret;
}

int reread_part_table_retry(const char *dev_path, int count)
{
	int ret;
	int dev_fd;

	println("Re-Read Partition Table From \"%s\", Max Retry Count = %d", dev_path, count);

	dev_fd = open(dev_path, 0);
	if (dev_fd < 0)
	{
		print_error("open device \"%s\"", dev_path);
		return -1;
	}

	umount_device(dev_path, MNT_DETACH);

	ret = freread_part_table_retry(dev_fd, count);

	close(dev_fd);

	return ret;
}

void show_disk_address(struct disk_address *addr)
{
	println("cylinder_number = %d, header_number = %d, sector_number = %d", \
		addr->cylinder_number_hsb << 8 | addr->cylinder_number_lsb, addr->header_number, addr->sector_number);
}

void show_partition_table(struct disk_partition_table *ptable_p)
{
	print_sep(60);
	println("abs_start_sector_number = %d", ptable_p->abs_start_sector_number);
	println("active_mark = 0x%02x", ptable_p->active_mark);
	println("file_system_mark = 0x%02x", ptable_p->file_system_mark);
	println("total_sector_count = %d", ptable_p->total_sector_count);
	print_string("start address:");
	show_disk_address(&ptable_p->start_addr);
	print_string("end address:");
	show_disk_address(&ptable_p->end_addr);
	print_sep(60);
}

void show_partition_tables(struct disk_partition_table *ptables_p)
{
	int i;

	for (i = 0; i < 4; i++)
	{
		show_partition_table(ptables_p + i);
	}
}

void show_master_boot_sector(struct master_boot_sector *mbs_p)
{
	println("magic_number = 0x%04x", mbs_p->magic_number);

	if (mbs_p->magic_number != 0xAA55)
	{
		error_msg("Bad Magic Number");
		return;
	}

	show_partition_tables(mbs_p->disk_part_tables);
}

int fread_master_boot_sector(int dev_fd, struct master_boot_sector *mbs_p)
{
	int ret;

	ret = lseek(dev_fd, 0, SEEK_SET);
	if (ret < 0)
	{
		print_error("lseek");
		return ret;
	}

	ret = ffile_read(dev_fd, mbs_p, sizeof(*mbs_p));
	if (ret < 0)
	{
		print_error("read master boot sector");
		return ret;
	}

	return 0;
}

int read_master_boot_sector(const char *dev_path, struct master_boot_sector *mbs_p)
{
	int ret;
	int dev_fd;

	dev_fd = open(dev_path, O_RDONLY | O_BINARY);
	if (dev_fd < 0)
	{
		print_error("open deive \"%s\"", dev_path);
		return -1;
	}

	ret = fread_master_boot_sector(dev_fd, mbs_p);
	close(dev_fd);
	if (ret < 0)
	{
		error_msg("fread_master_boot_sector");
		return ret;
	}

	return 0;
}

int fwrite_master_boot_sector(int dev_fd, struct master_boot_sector *mbs_p)
{
	int ret;

	show_master_boot_sector(mbs_p);

	ret = lseek(dev_fd, 0, SEEK_SET);
	if (ret < 0)
	{
		print_error("lseek");
		return ret;
	}

	ret = ffile_write(dev_fd, mbs_p, sizeof(*mbs_p));
	if (ret < (int)sizeof(*mbs_p))
	{
		print_error("ffile_write");
		return ret;
	}

	return 0;
}

int write_master_boot_sector(const char *dev_path, struct master_boot_sector *mbs_p)
{
	int dev_fd;
	int ret;

	dev_fd = open(dev_path, O_WRONLY | O_BINARY);
	if (dev_fd < 0)
	{
		print_error("open deivce \"%s\"", dev_path);
		return -1;
	}

	ret = fwrite_master_boot_sector(dev_fd, mbs_p);
	if (ret < 0)
	{
		error_msg("fwrite_master_boot_sector");
	}

	close(dev_fd);

	return ret;
}

int ioctl_get32(const char *dev_path, int request, u32 *val_p)
{
	int ret;
	int dev_fd;

	dev_fd = open(dev_path, 0);
	if (dev_fd < 0)
	{
		print_error("open device \"%s\"", dev_path);
		return -1;
	}

	ret = fioctl_get32(dev_fd, request, val_p);

	close(dev_fd);

	return ret;
}

int ioctl_set32(const char *dev_path, int request, u32 val)
{
	int ret;
	int dev_fd;

	dev_fd = open(dev_path, 0);
	if (dev_fd < 0)
	{
		print_error("open device \"%s\"", dev_path);
		return -1;
	}

	ret = fioctl_set32(dev_fd, request, val);

	close(dev_fd);

	return ret;
}

void show_geometry(struct hd_geometry *geo_p)
{
	println("cylinders = %d", geo_p->cylinders);
	println("heads = %d", geo_p->heads);
	println("sectors = %d", geo_p->sectors);
	println("start = %ld", geo_p->start);
}

int get_device_geometry(const char *dev_path, struct hd_geometry *geo_p)
{
	int ret;
	int dev_fd;

	dev_fd = open(dev_path, 0);
	if (dev_fd < 0)
	{
		print_error("open deivce \"%s\"", dev_path);
		return -1;
	}

	ret = fget_device_geometry(dev_fd, geo_p);

	close(dev_fd);

	return ret;
}

void sector_index2disk_address(struct hd_geometry *geo_p, u32 sec_index, struct disk_address *disk_addr)
{
	u32 cyl_index;

	cyl_index = sec_index / (geo_p->sectors * geo_p->heads);
	disk_addr->cylinder_number_hsb = (cyl_index >> 8) & 0x03;
	disk_addr->cylinder_number_lsb = cyl_index & 0xFF;
	sec_index -= cyl_index * geo_p->sectors * geo_p->heads;

	disk_addr->header_number = sec_index / geo_p->sectors;
	sec_index -= disk_addr->header_number * geo_p->sectors;

	disk_addr->sector_number = sec_index + 1;
}

u32 disk_address2sector_index(struct hd_geometry *geo_p, struct disk_address *disk_addr)
{
	u32 sec_index;
	u32 cyl_index;

	show_disk_address(disk_addr);

	sec_index = disk_addr->sector_number;
	sec_index += disk_addr->header_number * geo_p->sectors;
	cyl_index = disk_addr->cylinder_number_hsb << 8 |  disk_addr->cylinder_number_lsb;
	sec_index += cyl_index * geo_p->sectors * geo_p->heads;

	return sec_index - 1;
}

u32 sector_cylinder_halignment(u32 sec_index, struct hd_geometry *geo_p)
{
	u32 sec_per_cyl;

	sec_per_cyl = geo_p->heads * geo_p->sectors;

	return (sec_index + sec_per_cyl) / sec_per_cyl * sec_per_cyl;
}

u32 sector_cylinder_lalignment(u32 sec_index, struct hd_geometry *geo_p)
{
	u32 sec_per_cyl;

	sec_per_cyl = geo_p->heads * geo_p->sectors;

	return sec_index / sec_per_cyl * sec_per_cyl;
}

u32 sector_cylinder_alignment_auto(u32 sec_index, struct hd_geometry *geo_p)
{
	u32 sec_per_cyl;
	u32 sec_index_new;

	sec_per_cyl = geo_p->heads * geo_p->sectors;

	sec_index_new = sec_index / sec_per_cyl * sec_per_cyl;

	if ((sec_index % sec_per_cyl) > (sec_per_cyl >> 1))
	{
		return sec_index_new + sec_per_cyl;
	}

	return sec_index_new;
}

void set_part_address(struct disk_partition_table *part_table_p, struct hd_geometry *geo_p, u32 sec_index, u32 sec_count)
{
	part_table_p->abs_start_sector_number = sec_index;
	sector_index2disk_address(geo_p, sec_index, &part_table_p->start_addr);

	part_table_p->total_sector_count = sec_count;
	sector_index2disk_address(geo_p, sec_index + sec_count - 1, &part_table_p->end_addr);
}

ssize_t fget_first_partition_offset(int fd, off_t *offset)
{
	int ret;
	int i;
	struct master_boot_sector mbs;
	u32 sec_size;

	ret = fread_master_boot_sector(fd, &mbs);
	if (ret < 0)
	{
		return ret;
	}

	ret = get_sector_size(fd, &sec_size);
	if (ret < 0)
	{
		return ret;
	}

	for (i = 0; i < 4; i++)
	{
		if (mbs.disk_part_tables[i].total_sector_count)
		{
			*offset = mbs.disk_part_tables[i].abs_start_sector_number * sec_size;
			return 0;
		}
	}

	*offset = 0;

	return 0;
}

ssize_t get_first_partition_offset(const char *dev_path, off_t *offset)
{
	int fd;
	int ret;

	fd = open(dev_path, O_RDONLY);
	if (fd < 0)
	{
		return fd;
	}

	ret = fget_first_partition_offset(fd, offset);

	close(fd);

	return ret;
}

void visual_usleep(u32 usec)
{
	while (usec)
	{
		print("Delay remain time %d(us)  \r", usec);
		usleep(1);
		usec--;
	}
}

void visual_msleep(u32 msec)
{
	while (msec)
	{
		print("Delay remain time %d(ms)  \r", msec);
		usleep(1000);
		msec--;
	}
}

void visual_ssleep(u32 sec)
{
	while (sec)
	{
		print("Delay remain time %d(s)  \r", sec);
		sleep(1);
		sec--;
	}
}

struct filesystem_desc fs_table[] =
{
	{
		.type = FS_EXT2,
		.name = "ext2",
		.mkfs_cmd = "mkfs.ext2 -F",
		.label_cmd = "e2label",
		.mkfs_label_opt = 'L',
	},
	{
		.type = FS_EXT3,
		.name = "ext3",
		.mkfs_cmd = "mkfs.ext3 -F",
		.label_cmd = "e2label",
		.mkfs_label_opt = 'L',
	},
	{
		.type = FS_EXT4,
		.name = "ext4",
		.mkfs_cmd = "mkfs.ext4 -F",
		.label_cmd = "e2label",
		.mkfs_label_opt = 'L',
	},
	{
		.type = FS_VFAT,
		.name = "vfat",
		.mkfs_cmd = "mkfs.vfat",
		.label_cmd = "dosfslabel",
		.mkfs_label_opt = 'n',
	},
	{
		.type = FS_NTFS,
		.name = "ntfs",
		.mkfs_cmd = "mkfs.ntfs -F",
		.label_cmd = "ntfslabel",
		.mkfs_label_opt = 'L',
	},
	{
		.type = FS_FUSE,
		.name = "fuse",
		.mkfs_cmd = "mkfs.ntfs -F",
		.label_cmd = "ntfslabel",
		.mkfs_label_opt = 'L',
	},
	{
		.type = FS_SYS,
		.name = "sysfs",
	},
	{
		.type = FS_PROC,
		.name = "proc",
	},
	{
		.type = FS_TMP,
		.name = "tmpfs",
	},
};

struct filesystem_desc *get_fsdesc_by_name(const char *name)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(fs_table); i++)
	{
		if (strcmp(fs_table[i].name, name) == 0)
		{
			return fs_table + i;
		}
	}

	error_msg("unknown filesystem");

	return NULL;
}

struct filesystem_desc *get_fsdesc_by_type(enum filesystem_type type)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(fs_table); i++)
	{
		if (fs_table[i].type == type)
		{
			return fs_table + i;
		}
	}

	return NULL;
}

int partition_mount_to(struct partition_desc *part_desc, const char *mnt_point, const void *data)
{
	int ret;
	const struct filesystem_desc *fsdesc;

	ret = partition_test(part_desc);
	if (ret < 0)
	{
		return ret;
	}

	fsdesc = get_fsdesc_by_type(part_desc->type);
	if (fsdesc == NULL)
	{
		ERROR_RETURN(EINVAL);
	}

	ret = libc_mount_to(part_desc->path, mnt_point, fsdesc->name, 0, data);
	if (ret < 0)
	{
		print_error("mount \"%s\" to \"%s\"", part_desc->path, mnt_point);
		return ret;
	}

	return 0;
}

int partition_test_mount(struct partition_desc *part_desc)
{
	int ret;

	ret = partition_mount_to(part_desc, TEMP_MOUNT_POINT, NULL);
	if (ret < 0)
	{
		return ret;
	}

	umount_directory2(TEMP_MOUNT_POINT, 0);

	return 0;
}

int partition_test(struct partition_desc *part_desc)
{
	int ret;

	if (part_desc == NULL || part_desc->path[0] == 0)
	{
		ERROR_RETURN(EINVAL);
	}

	if (file_wait(part_desc->path, "bf", 5) == 0)
	{
		return 0;
	}

	if (part_desc->major == 0)
	{
		ERROR_RETURN(ENOENT);
	}

	warning_msg("block device \"%s\" don't exist, try to mknod", part_desc->path);

	ret = remknod(part_desc->path, 0666, makedev(part_desc->major, part_desc->minor));
	if (ret < 0)
	{
		return ret;
	}

	return file_test_read(part_desc->path);
}

ssize_t partition_read_label_base(const char *dev_path, const char *read_cmd, char *buff, size_t buff_len)
{
	ssize_t readlen;

	readlen = buff_command2(buff, buff_len, "%s %s", read_cmd, dev_path);
	if (readlen < 0)
	{
		return readlen;
	}

	buff[readlen] = 0;
	text_tok(buff, " \n");

	return readlen;
}

ssize_t partition_read_label(struct partition_desc *part_desc)
{
	const struct filesystem_desc *fsdesc;

	fsdesc = get_fsdesc_by_type(part_desc->type);
	if (fsdesc == NULL)
	{
		ERROR_RETURN(EINVAL);
	}

	return partition_read_label_base(part_desc->path, fsdesc->label_cmd, part_desc->label, sizeof(part_desc->label));
}

ssize_t partition_read_label_auto(const char *dev_path, char *buff, size_t buff_len)
{
	unsigned int i;
	const char *label_cmds[] = {"e2label", "dosfslabel", "ntfslabel"};

	for (i = 0; i < ARRAY_SIZE(label_cmds); i++)
	{
		ssize_t readlen;

		readlen = partition_read_label_base(dev_path, label_cmds[i], buff, buff_len);
		if (readlen >= 0)
		{
			return readlen;
		}
	}

	return -1;
}

int partition_change_label(struct partition_desc *part_desc)
{
	int ret;
	const struct filesystem_desc *fsdesc;

	ret = partition_test(part_desc);
	if (ret < 0)
	{
		return ret;
	}

	if (part_desc->label[0] == 0)
	{
		warning_msg("label text is empty");
		return 0;
	}

	fsdesc = get_fsdesc_by_type(part_desc->type);
	if (fsdesc == NULL)
	{
		ERROR_RETURN(EINVAL);
	}

	return system_command("%s %s %s", fsdesc->label_cmd, part_desc->path, part_desc->label);
}

int partition_change_label_auto(struct partition_desc *part_desc)
{
	int i;

	for (i = FS_START + 1; i < FS_END; i++)
	{
		part_desc->type = i;
		if (partition_change_label(part_desc) == 0)
		{
			return 0;
		}
	}

	return -1;
}

void show_partition_desc(struct partition_desc *part_desc)
{
	const struct filesystem_desc *fsdesc;

	print_sep(60);
	println("flags = 0x%08x", part_desc->flags);
	println("label = %s", part_desc->label);
	println("major = %d", part_desc->major);
	println("minor = %d", part_desc->minor);
	println("path = %s", part_desc->path);

	fsdesc = get_fsdesc_by_type(part_desc->type);
	if (fsdesc == NULL)
	{
		println("type = unknown");
	}
	else
	{
		println("type = %s", fsdesc->name);
	}

	print_sep(60);
}

int partition_mkfs(struct partition_desc *part_desc)
{
	int ret;
	struct filesystem_desc *fs_desc;

	show_partition_desc(part_desc);

	ret = partition_test(part_desc);
	if (ret < 0)
	{
		return ret;
	}

	fs_desc = get_fsdesc_by_type(part_desc->type);
	if (fs_desc == NULL)
	{
		ERROR_RETURN(EINVAL);
	}

	visual_ssleep(2);

	ret = umount_partition(part_desc->path, MNT_DETACH);
	if (ret < 0 || ((part_desc->flags & MKFS_FLAG_TEST) && partition_test_mount(part_desc) == 0))
	{
		right_msg("partition \"%s\" is ok, try to change label", part_desc->path);

		ret = partition_change_label(part_desc);
		if (ret < 0)
		{
			warning_msg("change label failed, mkfs now");
		}
		else
		{
			right_msg("change label is ok, don't need mkfs");
			return 0;
		}
	}

	if (part_desc->label[0] == 0)
	{
		warning_msg("the label of partition \"%s\" is empty, read it now", part_desc->path);

		ret = partition_read_label(part_desc);
		if (ret < 0)
		{
			error_msg("read label from partition \"%s\" failed", part_desc->path);
			part_desc->label[0] = 0;
		}
		else
		{
			right_msg("the label of partition \"%s\" is \"%s\"", part_desc->path, part_desc->label);
		}
	}

	if (part_desc->label[0])
	{
		return system_command("%s %s -%c %s", fs_desc->mkfs_cmd, part_desc->path, fs_desc->mkfs_label_opt, part_desc->label);
	}
	else
	{
		return system_command("%s %s", fs_desc->mkfs_cmd, part_desc->path);
	}
}

int mount_system_devices(void)
{
	int ret;

	ret = libc_mount_to("sys", "/sys", "sysfs", 0, NULL);
	if (ret < 0)
	{
		print_error("mount sysfs");
		return ret;
	}

	ret = libc_mount_to("proc", "/proc", "proc", 0, NULL);
	if (ret < 0)
	{
		print_error("mount proc");
		return ret;
	}

	ret = libc_mount_to("tmpfs", "/tmp", "tmpfs", 0, NULL);
	if (ret < 0)
	{
		print_error("mount tmp");
		return ret;
	}

	ret = libc_mount_to("tmpfs", "/dev", "tmpfs", 0, NULL);
	if (ret < 0)
	{
		print_error("mount dev");
		return ret;
	}

	return 0;
}

void show_mount_table(struct mount_table *mtab)
{
	print_sep(60);
	println("source = %s", mtab->source);
	println("target = %s", mtab->target);
	println("fstype = %s", mtab->fstype);
	println("option = %s", mtab->option);
}

ssize_t parse_mount_table(int fd, struct mount_table *mtab, size_t mtab_size)
{
	struct mount_table *end_mtab = mtab + mtab_size, *mtab_bak = mtab;;

	while (mtab < end_mtab)
	{
		ssize_t rdlen;
		char buff[1024];

		rdlen = file_read_line(fd, buff, sizeof(buff));
		if (rdlen < 1)
		{
			if (rdlen == 0)
			{
				break;
			}

			pr_red_info("file_read_line");

			return rdlen;
		}

		rdlen = parse_mount_table_simple(buff, mtab);
		if (rdlen < 0)
		{
			pr_red_info("parse_mount_table_simple");
			return rdlen;
		}

		mtab++;
	}

	return mtab - mtab_bak;
}

ssize_t read_mount_table(struct mount_table *mtab, size_t size)
{
	int fd;
	ssize_t count;

	fd = open(FILE_PROC_MOUNTS, O_RDONLY);
	if (fd < 0)
	{
		pr_error_info("open file `%s' failed", FILE_PROC_MOUNTS);
		return fd;
	}

	count = parse_mount_table(fd, mtab, size);
	close(fd);

	return count;
}

void print_mount_table_base(struct mount_table *mtab, size_t size)
{
	struct mount_table *p, *end_p;

	for (p = mtab, end_p = mtab + size; p < end_p; p++)
	{
		println("%s on %s [%s] (%s)", p->source, p->target, p->fstype, p->option);
	}
}

int print_mount_table(void)
{
	ssize_t readlen;
	struct mount_table *mtab;

	mtab = alloca(sizeof(*mtab) * MAX_MOUNT_COUNT);
	if (mtab == NULL)
	{
		pr_error_info("alloca");
		return -EFAULT;
	}

	readlen = read_mount_table(mtab, MAX_MOUNT_COUNT);
	if (readlen < 0)
	{
		error_msg("read_mount_table failed");
		return readlen;
	}

	print_mount_table_base(mtab, readlen);

	return 0;
}

const char *find_mount_table_item(const char *mounts, const char *item)
{
	const char *p;

	p = text_kmp_find(mounts, item);
	if (p == NULL)
	{
		return NULL;
	}

	while (p >= mounts && *p != '\n')
	{
		p--;
	}

	return p + 1;
}

const char *find_mount_table_base(const char *mounts, struct mount_table *mtab, const char *text)
{
	const char *p;
	int ret;

	p = find_mount_table_item(mounts, text);
	if (p == NULL)
	{
		return NULL;
	}

	ret = parse_mount_table_simple(p, mtab);
	if (ret < 0)
	{
		return NULL;
	}

	return p;
}

int find_mount_table(struct mount_table *mtab, const char *text)
{
	ssize_t count;
	char *file_mem;
	size_t file_size;

	file_mem = file_read_mounts(&file_size);
	if (file_mem == NULL)
	{
		pr_red_info("file_read_mounts");
		return -EFAULT;
	}

	count = find_mount_table_base(file_mem, mtab, text) ? 0 : -ENODATA;
	free(file_mem);

	return count;
}

char *get_mount_source_base(const char *target, char *buff, size_t size)
{
	struct mount_table *mtab, *p, *end_p;
	ssize_t mtab_size;
	char target_abs[1024];

	if (to_abs_path_directory_base(target, target_abs, sizeof(target_abs)) == NULL)
	{
		return NULL;
	}

	mtab = alloca(sizeof(*mtab) * MAX_MOUNT_COUNT);
	if (mtab == NULL)
	{
		pr_error_info("alloca");
		return NULL;
	}

	mtab_size = read_mount_table(mtab, MAX_MOUNT_COUNT);
	if (mtab_size < 0)
	{
		return NULL;
	}

	p = mtab;
	end_p = mtab + mtab_size;

	while (p < end_p)
	{
		if (strcmp(p->target, target_abs) == 0)
		{
			return text_ncopy(buff, p->source, size);
		}

		p++;
	}

	return NULL;
}

char *get_mount_source(const char *target)
{
	static char buff[1024];

	if (get_mount_source_base(target, buff, sizeof(buff)) == NULL)
	{
		buff[0] = 0;
	}

	return buff;
}

char *get_mount_target_base(const char *source, char *buff, size_t size)
{
	struct mount_table *mtab, *p, *end_p;
	ssize_t mtab_size;
	char source_abs[1024];

	if (to_abs_path2_base(source, source_abs, sizeof(source_abs)) == NULL)
	{
		return NULL;
	}

	mtab = alloca(sizeof(*mtab) * MAX_MOUNT_COUNT);
	if (mtab == NULL)
	{
		pr_error_info("alloca");
		return NULL;
	}

	mtab_size = read_mount_table(mtab, MAX_MOUNT_COUNT);
	if (mtab_size < 0)
	{
		return NULL;
	}

	p = mtab;
	end_p = mtab + mtab_size;

	while (p < end_p)
	{
		if (strcmp(p->source, source_abs) == 0)
		{
			return text_ncopy(buff, p->target, size);
		}

		p++;
	}

	return NULL;
}

char *get_mount_target(const char *source)
{
	static char buff[1024];

	if (get_mount_target_base(source, buff, sizeof(buff)) == NULL)
	{
		buff[0] = 0;
	}

	return buff;
}

int loop_get_fd(const char *filename, char *loop_path, u64 offset)
{
	int i;
	int ret;
	int loop_fd;
	char *p;
	char file_abs_path[1024];

	if (to_abs_path2_base(filename, file_abs_path, sizeof(file_abs_path)) == NULL)
	{
		ERROR_RETURN(ENOENT);
	}

	p = text_copy(loop_path, "/dev/loop");

	for (i = 0; i < 100; i++)
	{
		struct loop_info64 loopinfo;

		value2text_base(i, p, 0, 0, 10);

#ifdef CAVAN_DEBUG
		println("loop_dev = %s", loop_path);
#endif

		remknod(loop_path, 0666, makedev(7, i));

		loop_fd = file_open_rw_ro(loop_path, 0);
		if (loop_fd < 0)
		{
			return -ENOENT;
		}

		ret = ioctl(loop_fd, LOOP_GET_STATUS64, &loopinfo);
		if (ret < 0 && errno == ENXIO)
		{
			return loop_fd;
		}

		if (text_cmp((char *)loopinfo.lo_file_name, file_abs_path) == 0 && loopinfo.lo_offset == offset)
		{
			if (ioctl(loop_fd, LOOP_CLR_FD, 0) == 0)
			{
				return loop_fd;
			}
		}

		close(loop_fd);
	}

	return -ENOENT;
}

int system_init(const char *path, char *argv[])
{
	int ret;
	unsigned int i;
	const char *init_paths[] = {"/sbin/init", "/bin/init", "/init", "/bin/sh", "/bin/bash"};

	ret = mount_system_devices();
	if (ret < 0)
	{
		error_msg("mount_system_devices failed");
		return ret;
	}

	ret = setenv("PATH", DEFAULT_PATH_VALUE, 1);
	if (ret < 0)
	{
		print_error("setenv PATH");
		return ret;
	}

	print_mount_table();

	if (path)
	{
		ret = execv(path, argv);
		if (ret < 0)
		{
			print_error("execv \"%s\"", path);
			return ret;
		}
	}

	argv[0] = NULL;

	for (i = 0; i < ARRAY_SIZE(init_paths); i++)
	{
		println("execute application \"%s\"", init_paths[i]);
		execv(init_paths[i], argv);
	}

	return -EINVAL;
}

ssize_t device_read_blocks(int fd, void *buff, size_t block_size, off_t start, size_t count)
{
	off_t seek_result;
	ssize_t readlen;

	seek_result = lseek(fd, block_size * start, SEEK_SET);
	if (seek_result < 0)
	{
		return seek_result;
	}

	readlen = read(fd, buff, block_size * count);
	if (readlen < 0)
	{
		return readlen;
	}

	return readlen / block_size;
}

ssize_t device_write_blocks(int fd, void *buff, size_t block_size, off_t start, size_t count)
{
	off_t seek_result;
	ssize_t writelen;

	seek_result = lseek(fd, block_size * start, SEEK_SET);
	if (seek_result < 0)
	{
		return seek_result;
	}

	writelen = write(fd, buff, block_size * count);
	if (writelen < 0)
	{
		return writelen;
	}

	return writelen / block_size;
}

int device_is_mmc(const char *dev_path)
{
	char dev_name[64];

	text_basename_base(dev_name, dev_path);

	return text_lhcmp("mmcblk", dev_name) == 0;
}

char *get_device_real_path(char *dest, const char *src)
{
	if (src == NULL || src[0] == 0)
	{
		text_copy(dest, "/dev/sdb");
	}
	else if (file_test(src, "bf") == 0)
	{
		text_copy(dest, src);
	}
	else
	{
		text_cat5(dest, "/dev/sd", src, NULL);
	}

	return dest;
}

void show_statfs(struct statfs *stfs)
{
	u64 fs_size, fs_free, fs_avail;

	fs_size = stfs->f_blocks * stfs->f_bsize;
	fs_free = stfs->f_bfree * stfs->f_bsize;
	fs_avail = stfs->f_bavail * stfs->f_bsize;

	println("size = %s", size2text(fs_size));
	println("avail = %s", size2text(fs_avail));

#if __WORDSIZE == 64
	println("used = %ld%%", ((fs_size - fs_free) * 100) / fs_size);
	println("f_bavail = %ld", stfs->f_bavail);
	println("f_bfree = %ld", stfs->f_bfree);
	println("f_blocks = %ld", stfs->f_blocks);
	println("f_bsize = %ld", stfs->f_bsize);
	println("f_ffree = %ld", stfs->f_ffree);
	println("f_files = %ld", stfs->f_files);
	println("f_frsize = %ld", stfs->f_frsize);
	println("f_namelen = %ld", stfs->f_namelen);
	println("f_type = %ld", stfs->f_type);
#else
	println("used = %Ld%%", ((fs_size - fs_free) * 100) / fs_size);
	println("f_bavail = %Ld", stfs->f_bavail);
	println("f_bfree = %Ld", stfs->f_bfree);
	println("f_blocks = %Ld", stfs->f_blocks);
	println("f_bsize = %d", stfs->f_bsize);
	println("f_ffree = %Ld", stfs->f_ffree);
	println("f_files = %Ld", stfs->f_files);
	println("f_frsize = %d", stfs->f_frsize);
	println("f_namelen = %d", stfs->f_namelen);
	println("f_type = %d", stfs->f_type);
#endif
}

int get_device_statfs(const char *devpath, const char *fstype, struct statfs *stfs)
{
	int ret;
	char mnt_point[1024];

	if (get_mount_target_base(devpath, mnt_point, sizeof(mnt_point)))
	{
		return statfs(mnt_point, stfs);
	}

	ret = mount_main(devpath, TEMP_MOUNT_POINT, fstype, NULL);
	if (ret < 0)
	{
		return ret;
	}

	ret = statfs(TEMP_MOUNT_POINT, stfs);

	umount_directory2(TEMP_MOUNT_POINT, 0);

	return ret;
}
