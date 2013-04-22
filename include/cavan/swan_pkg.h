#pragma once

#include <cavan.h>
#include <cavan/file.h>
#include <cavan/swan_dev.h>

// Fuang.Cao: Thu Dec 23 15:09:46 CST 2010

#define DEFAULT_CHECK_PATTERN		"eavoo_swan_i600"

#define HEADER_BIN_NAME				"header.bin"
#define BACKUP_HEADER_BIN			"/etc/swan/" HEADER_BIN_NAME

#define MKFS_MASK_VFAT				(1 << 0)
#define MKFS_MASK_USERDATA			(1 << 1)
#define MKFS_MASK_CACHE				(1 << 2)
#define MKFS_MASK_VENDOR			(1 << 3)

#define UPGRADE_FLAG_CHECK_VERSION	(1 << 0)

enum swan_image_type
{
	SWAN_IMAGE_UNKNOWN,
	SWAN_IMAGE_UBOOT,
	SWAN_IMAGE_UIMAGE,
	SWAN_IMAGE_URAMDISK,
	SWAN_IMAGE_LOGO,
	SWAN_IMAGE_BUSYBOX,
	SWAN_IMAGE_BATTERY,
	SWAN_IMAGE_SYSTEM,
	SWAN_IMAGE_RECOVERY,
	SWAN_IMAGE_USERDATA,
	SWAN_IMAGE_VENDOR,
	SWAN_IMAGE_COUNT,
};

#pragma pack(1)
struct swan_image_info
{
	u32 major;
	u32 minor;
	u32 crc32;
	u32 offset;
	u32 length;
	char filename[128];
	char device_path[128];
	char type;
	char reserved[127];
};

struct swan_package_info
{
	u32 image_count;
	u32 crc32;
	u32 resource_size;
	char volume[16];
	u32 mkfs_mask;
	u32 board_type;
	u32 upgrade_flags;
	struct swan_emmc_partition_table part_table;
	char reserved[80];
};

struct swan_file_info
{
	u32 version;
	u32 header_size;
	char md5sum[36];
	char check_pattern[16];
};
#pragma pack()

static inline ssize_t write_file_info(int pkg_fd, struct swan_file_info *file_p)
{
	return ffile_write(pkg_fd, file_p, sizeof(*file_p));
}

static inline ssize_t write_package_info(int pkg_fd, struct swan_package_info *pkg_p)
{
	return ffile_write(pkg_fd, pkg_p, sizeof(*pkg_p));
}

static inline ssize_t write_image_info(int pkg_fd, struct swan_image_info *img_p)
{
	return ffile_write(pkg_fd, img_p, sizeof(*img_p));
}

void show_file_info(struct swan_file_info *file_p);
void show_image_info(struct swan_image_info *img_p);
void show_package_info(struct swan_package_info *pkg_p);

int read_file_info(int pkg_fd, struct swan_file_info *file_p);
int read_upgrade_program(int pkg_fd, struct swan_file_info *file_p, const char *prm_name);
int open_header_bin(const char *prm_name);
int write_upgrade_program(int pkg_fd, struct swan_file_info *file_p, const char *prm_name);

int read_package_info(int pkg_fd, struct swan_package_info *pkg_p, int check_machine);
int read_resource_image(int pkg_fd, struct swan_package_info *pkg_p, const char *img_dir, int decompression, int check_machine);
int vwrite_resource_image(int pkg_fd, struct swan_package_info *pkg_p, const char *dir_name, va_list ap);
int write_resource_image(int pkg_fd, struct swan_package_info *pkg_p, const char *dir_name, ...);

int read_image_info(int pkg_fd, struct swan_image_info *img_p);
int swan_shrink_image(const char *dirname, struct swan_image_info *img_p);
int read_simple_image(int pkg_fd, int img_fd, off_t size, off_t offset);
int write_simple_image(int pkg_fd, const char *dir_name, struct swan_image_info *img_p, struct swan_emmc_partition_table *part_table);

int swan_read_md5sum(int pkg_fd, char *md5sum);
int swan_write_md5sum(int pkg_fd, char *md5sum);
int swan_calculate_md5sum(const char *pkg_path, int pkg_fd, char *md5sum);
int swan_set_md5sum(const char *pkg_path);
int swan_check_md5sum(const char *pkg_path);

const char *swan_image_type_tostring(enum swan_image_type type);
enum swan_image_type get_swan_image_type_by_name(const char *filename);

extern const char *get_resource_name_by_board_type(enum swan_board_type type);

