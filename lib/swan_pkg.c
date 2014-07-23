// Fuang.Cao: Thu Dec 23 15:09:46 CST 2010

#include <cavan.h>
#include <cavan/text.h>
#include <cavan/file.h>
#include <cavan/swan_pkg.h>
#include <cavan/image.h>
#include <cavan/swan_dev.h>

void show_file_info(struct swan_file_info *file_p)
{
	println("version = 0x%08x", file_p->version);
	print("Package build date = %s", asctime(localtime((time_t *) &file_p->version)));
	// println("check_pattern = %s", file_p->check_pattern);
	println("md5sum = %s", text_header(file_p->md5sum, MD5SUM_LEN));
	println("header_size = %s", size2text(file_p->header_size));
}

void show_image_info(struct swan_image_info *img_p)
{
	println("major = %d", img_p->major);
	println("minor = %d", img_p->minor);
	println("offset = %s", size2text(img_p->offset));
	println("length = %s", size2text(img_p->length));
	println("type = %s", swan_image_type_tostring(img_p->type));
	println("filename = %s", img_p->filename);
	println("device_path = %s", img_p->device_path);
	println("crc32 = 0x%08x", img_p->crc32);
}

void show_package_info(struct swan_package_info *pkg_p)
{
	println("image_count = %d", pkg_p->image_count);
	println("mkfs_mask = 0x%08x", pkg_p->mkfs_mask);
	println("upgrade_flags = 0x%08x", pkg_p->upgrade_flags);
	println("volume = %s", text_header(pkg_p->volume, sizeof(pkg_p->volume)));
	println("resource_size = %s", size2text(pkg_p->resource_size));
	println("crc32 = 0x%08x", pkg_p->crc32);
	println("board type = %s", swan_board_type_tostring(pkg_p->board_type));
	pr_bold_info("partition table:");
	show_swan_emmc_partation_table(&pkg_p->part_table);
}

int read_file_info(int pkg_fd, struct swan_file_info *file_p)
{
	int ret;

	ret = ffile_read(pkg_fd, file_p, sizeof(*file_p));
	if (ret < 0)
	{
		print_error("read");
		return ret;
	}

	show_file_info(file_p);

	if (strncmp(file_p->check_pattern, DEFAULT_CHECK_PATTERN, sizeof(file_p->check_pattern)) != 0)
	{
		error_msg("Bad Check Pattern");
		return -1;
	}

	right_msg("Check Pattern Is OK");

	return 0;
}

int read_upgrade_program(int pkg_fd, struct swan_file_info *file_p, const char *prm_name)
{
	int ret;
	int prm_fd;

	println("Decompression upgrade program to \"%s\"", prm_name);

	prm_fd = open(prm_name, O_WRONLY | O_CREAT | O_SYNC | O_TRUNC | O_BINARY, 0777);
	if (prm_fd < 0)
	{
		print_error("open");
		return -1;
	}

	ret = read_file_info(pkg_fd, file_p);
	if (ret < 0)
	{
		error_msg("read_file_info");
		goto out_close_prm;
	}

	ret = ffile_ncopy(pkg_fd, prm_fd, file_p->header_size);
	if (ret < 0)
	{
		error_msg("ffile_ncopy");
	}

out_close_prm:
	close(prm_fd);

	return ret;
}

int open_header_bin(const char *prm_name)
{
	int fd;

	println("upgrade_program = %s", prm_name);

	fd = open(prm_name, O_RDONLY | O_BINARY);
	if (fd >= 0)
	{
		return fd;
	}

	warning_msg("Can't open file \"%s\", try to open backup file \"%s\"", \
		prm_name, BACKUP_HEADER_BIN);

	return open(BACKUP_HEADER_BIN, O_RDONLY | O_BINARY);
}

int write_upgrade_program(int pkg_fd, struct swan_file_info *file_p, const char *prm_name)
{
	int ret;
	int prm_fd;
	struct stat st;

	prm_fd = open_header_bin(prm_name);
	if (prm_fd < 0)
	{
		print_error("open file \"%s\"", prm_name);
		return -1;
	}

	ret = fstat(prm_fd, &st);
	if (ret < 0)
	{
		print_error("get file size failed");
		goto out_close_prm;
	}

	time((time_t *) &file_p->version);
	file_p->header_size = st.st_size;
	strncpy(file_p->check_pattern, DEFAULT_CHECK_PATTERN, sizeof(file_p->check_pattern));

	ret = write_file_info(pkg_fd, file_p);
	if (ret < 0)
	{
		print_error("write_file_info");
		goto out_close_prm;
	}

	ret = ffile_copy(prm_fd, pkg_fd);
	if (ret < 0)
	{
		error_msg("ffile_copy");
	}

out_close_prm:
	close(prm_fd);

	return ret;
}

int read_package_info(int pkg_fd, struct swan_package_info *pkg_p, int check_macine)
{
	int ret;
	u32 tmp_crc32;

	ret = ffile_read(pkg_fd, pkg_p, sizeof(*pkg_p));
	if (ret < 0)
	{
		print_error("read");
		return ret;
	}

	show_package_info(pkg_p);

	tmp_crc32 = 0;
	ret = ffile_crc32_back(pkg_fd, &tmp_crc32);
	if (ret < 0)
	{
		error_msg("ffile_crc32");
		return ret;
	}

	println("pkg_crc32 = 0x%08x, tmp_crc32 = 0x%08x", pkg_p->crc32, tmp_crc32);

	if (pkg_p->crc32 ^ tmp_crc32)
	{
		error_msg("package crc32 checksum is not match");
		ERROR_RETURN(EINVAL);
	}

	right_msg("package crc32 checksum is match");

	if (check_macine == 0)
	{
		return 0;
	}

	ret = swan_board_type_check(pkg_p->board_type);
	if (ret < 0)
	{
		error_msg("board type and package type not match");
		return ret;
	}

	return 0;
}

int read_resource_image(int pkg_fd, struct swan_package_info *pkg_p, const char *img_dir, int decompression, int check_macine)
{
	int ret;
	int img_fd;
	char img_path[1024];

	ret = read_package_info(pkg_fd, pkg_p, check_macine);
	if (ret < 0)
	{
		error_msg("read_package_info");
		return ret;
	}

	if (img_dir == NULL)
	{
		ret = lseek(pkg_fd, pkg_p->resource_size, SEEK_CUR);
		if (ret < 0)
		{
			print_error("lseek");
			return ret;
		}

		return 0;
	}

	text_path_cat(img_path, sizeof(img_path), img_dir, get_resource_name_by_board_type(pkg_p->board_type));
	println("decompression resource image to \"%s\"", img_path);

	img_fd = open(img_path, O_WRONLY | O_CREAT | O_SYNC | O_TRUNC | O_BINARY, 0777);
	if (img_fd < 0)
	{
		print_error("open file \"%s\"", img_path);
		return img_fd;
	}

	ret = ffile_ncopy(pkg_fd, img_fd, pkg_p->resource_size);
	if (ret < 0)
	{
		error_msg("ffile_ncopy");
		goto out_close_img;
	}

	if (decompression)
	{
		system_command("tar -xvf %s -C /", img_path);
		remove(img_path);
	}

	ret = 0;

out_close_img:
	close(img_fd);

	return ret;
}

int vwrite_resource_image(int pkg_fd, struct swan_package_info *pkg_p, const char *dir_name, va_list ap)
{
	int ret;
	int img_fd;
	struct stat st;
	char tmp_path[1024], *p;

	p = text_path_cat(tmp_path, sizeof(tmp_path), dir_name, NULL);

	while (1)
	{
		const char *img_name = va_arg(ap, const char *);

		if (img_name == NULL)
		{
			return -1;
		}

		text_copy(p, img_name);

		img_fd = open(tmp_path, O_RDONLY | O_BINARY);
		if (img_fd >= 0)
		{
			break;
		}
	}

	println("resource path = %s", tmp_path);

	ret = fstat(img_fd, &st);
	if (ret < 0)
	{
		print_error("fstat");
		goto out_close_img;
	}

	ret = ffile_copy(img_fd, pkg_fd);
	if (ret < 0)
	{
		error_msg("ffile_copy");
		goto out_close_img;
	}

	pkg_p->resource_size = st.st_size;

out_close_img:
	close(img_fd);

	return ret;
}

int write_resource_image(int pkg_fd, struct swan_package_info *pkg_p, const char *dir_name, ...)
{
	int ret;
	va_list ap;

	va_start(ap, dir_name);
	ret = vwrite_resource_image(pkg_fd, pkg_p, dir_name, ap);
	va_end(ap);

	return ret;
}

int read_image_info(int pkg_fd, struct swan_image_info *img_p)
{
	int ret;

	ret = ffile_read(pkg_fd, img_p, sizeof(*img_p));
	if (ret < 0)
	{
		print_error("read");
		return ret;
	}

	show_image_info(img_p);

	return 0;
}

int read_simple_image(int pkg_fd, int img_fd, off_t size, off_t offset)
{
	int ret;

	ret = lseek(img_fd, offset, SEEK_SET);
	if (ret < 0)
	{
		print_error("lseek");
		return ret;
	}

	ret = ffile_ncopy(pkg_fd, img_fd, size);
	if (ret < 0)
	{
		error_msg("ffile_ncopy");
		return ret;
	}

	return 0;
}

int swan_shrink_image(const char *dirname, struct swan_image_info *img_p)
{
	char img_path[1024];

	text_path_cat(img_path, sizeof(img_path), dirname, img_p->filename);

	return image_shrink(img_path);
}

int write_simple_image(int pkg_fd, const char *dir_name, struct swan_image_info *img_p, struct swan_emmc_partition_table *part_table)
{
	int ret;
	int img_fd;
	struct stat st;
	char img_path[1024];
	ssize_t part_size;

	text_path_cat(img_path, sizeof(img_path), dir_name, img_p->filename);

	img_fd = open(img_path, O_RDONLY | O_BINARY);
	if (img_fd < 0)
	{
		error_msg("file \"%s\" don't exist", img_path);
		return img_fd;
	}

	ret = fstat(img_fd, &st);
	if (ret < 0)
	{
		print_error("fstat");
		goto out_close_img;
	}

	println("image path = %s", img_path);

	img_p->length = st.st_size;

	part_size = get_partition_size_by_type(img_p->type, part_table);
	if (part_size >= 0 && img_p->length > (u32) MB(part_size))
	{
		pr_red_info("partition size = %" PRINT_FORMAT_SIZE "MB", part_size);
		pr_red_info("image size = %s", size2text(img_p->length));
		pr_red_info("image size > partition size, please adjust image or partition size");
		ret = -EINVAL;
		goto out_close_img;
	}

	ret = ffile_crc32_back(img_fd, &img_p->crc32);
	if (ret < 0)
	{
		error_msg("ffile_crc32");
		goto out_close_img;
	}

	show_image_info(img_p);

	ret = write_image_info(pkg_fd, img_p);
	if (ret < 0)
	{
		print_error("write_image_info");
		goto out_close_img;
	}

	ret = ffile_copy(img_fd, pkg_fd);
	if (ret < 0)
	{
		error_msg("ffile_copy");
		goto out_close_img;
	}

	ret = 0;

out_close_img:
	close(img_fd);

	return ret;
}

int swan_read_md5sum(int pkg_fd, char *md5sum)
{
	int ret;

	ret = lseek(pkg_fd, MEMBER_OFFSET(struct swan_file_info, md5sum), SEEK_SET);
	if (ret < 0)
	{
		print_error("lseek");
		return ret;
	}

	return ffile_read(pkg_fd, md5sum, MD5SUM_LEN);
}

int swan_write_md5sum(int pkg_fd, char *md5sum)
{
	int ret;

	ret = lseek(pkg_fd, MEMBER_OFFSET(struct swan_file_info, md5sum), SEEK_SET);
	if (ret < 0)
	{
		print_error("lseek");
		return ret;
	}

	ret = ffile_write(pkg_fd, md5sum, MD5SUM_LEN);
	if (ret < MD5SUM_LEN)
	{
		print_error("ffile_write");
		return ret;
	}

	fsync(pkg_fd);

	return 0;
}

int swan_calculate_md5sum(const char *pkg_path, int pkg_fd, char *md5sum)
{
	int ret;

	memset(md5sum, 0, MD5SUM_LEN);

	ret = swan_write_md5sum(pkg_fd, md5sum);
	if (ret < 0)
	{
		error_msg("write");
		return ret;
	}

	ret = calculate_file_md5sum(pkg_path, md5sum);
	if (ret < 0)
	{
		error_msg("calculate_file_md5sum");
		return ret;
	}

	println("md5sum = %s", text_header(md5sum, MD5SUM_LEN));

	return 0;
}

int swan_set_md5sum(const char *pkg_path)
{
	int ret;
	int pkg_fd;
	char buff[MD5SUM_LEN];

	pkg_fd = open(pkg_path, O_WRONLY | O_BINARY);
	if (pkg_fd < 0)
	{
		print_error("open");
		return -1;
	}

	ret = swan_calculate_md5sum(pkg_path, pkg_fd, buff);
	if (ret < 0)
	{
		error_msg("swan_calculate_md5sum");
		goto out_close_pkg;
	}

	ret = swan_write_md5sum(pkg_fd, buff);
	if (ret < 0)
	{
		error_msg("swan_write_md5sum");
	}

out_close_pkg:
	close(pkg_fd);

	return ret;
}

int swan_check_md5sum(const char *pkg_path)
{
	int ret;
	int pkg_fd;
	char buff[MD5SUM_LEN];
	char md5sum[MD5SUM_LEN];

	pkg_fd = open(pkg_path, O_RDWR | O_BINARY);
	if (pkg_fd < 0)
	{
		print_error("open file \"%s\"", pkg_path);
		return -1;
	}

	ret = swan_read_md5sum(pkg_fd, md5sum);
	if (ret < 0)
	{
		error_msg("swan_read_md5sum");
		goto out_close_pkg;
	}

	println("md5sum = %s", text_header(md5sum, MD5SUM_LEN));

	memset(buff, 0, sizeof(buff));

	ret = swan_write_md5sum(pkg_fd, buff);
	if (ret < 0)
	{
		error_msg("swan_write_md5sum");
		goto out_close_pkg;
	}

	ret = check_file_md5sum(pkg_path, md5sum);
	if (ret < 0)
	{
		error_msg("check_file_md5sum");
	}

	swan_write_md5sum(pkg_fd, md5sum);

out_close_pkg:
	close(pkg_fd);

	return ret;
}

const char *swan_image_type_tostring(enum swan_image_type type)
{
	switch (type)
	{
	case SWAN_IMAGE_UBOOT:
		return "u-boot-no-padding.bin";

	case SWAN_IMAGE_UIMAGE:
		return "uImage";

	case SWAN_IMAGE_URAMDISK:
		return "uramdisk.img";

	case SWAN_IMAGE_LOGO:
		return "logo.bmp";

	case SWAN_IMAGE_BUSYBOX:
		return "busybox.img";

	case SWAN_IMAGE_BATTERY:
		return "battery.bmps";

	case SWAN_IMAGE_SYSTEM:
		return "system.img";

	case SWAN_IMAGE_RECOVERY:
		return "recovery.img";

	case SWAN_IMAGE_USERDATA:
		return "userdata.img";

	default:
		return "unknown";
	}
}

enum swan_image_type get_swan_image_type_by_name(const char *filename)
{
	if (text_cmp(filename, "u-boot-no-padding.bin") == 0 || text_cmp(filename, "u-boot") == 0)
	{
		return SWAN_IMAGE_UBOOT;
	}
	else if (text_cmp(filename, "uImage") == 0 || text_cmp(filename, "kernel") == 0)
	{
		return SWAN_IMAGE_UIMAGE;
	}
	else if (text_cmp(filename, "uramdisk.img") == 0 || text_cmp(filename, "ramdisk") == 0)
	{
		return SWAN_IMAGE_URAMDISK;
	}
	else if (text_cmp(filename, "logo.bmp") == 0 || text_cmp(filename, "logo") == 0)
	{
		return SWAN_IMAGE_LOGO;
	}
	else if (text_cmp(filename, "busybox.img") == 0 || text_cmp(filename, "busybox") == 0)
	{
		return SWAN_IMAGE_BUSYBOX;
	}
	else if (text_cmp(filename, "battery.bmps") == 0 || text_cmp(filename, "charge.bmps") == 0 || text_cmp(filename, "battery") == 0)
	{
		return SWAN_IMAGE_BATTERY;
	}
	else if (text_cmp(filename, "system.img") == 0 || text_cmp(filename, "system") == 0)
	{
		return SWAN_IMAGE_SYSTEM;
	}
	else if (text_cmp(filename, "recovery.img") == 0 || text_cmp(filename, "recovery") == 0)
	{
		return SWAN_IMAGE_RECOVERY;
	}
	else if (text_cmp(filename, "userdata.img") == 0 || text_cmp(filename, "userdata") == 0)
	{
		return SWAN_IMAGE_USERDATA;
	}
	else if (text_cmp(filename, "vendor.img") == 0 || text_cmp(filename, "vendor") == 0)
	{
		return SWAN_IMAGE_VENDOR;
	}
	else
	{
		return SWAN_IMAGE_UNKNOWN;
	}
}
