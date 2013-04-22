#pragma once

// Fuang.Cao <cavan.cfa@gmail.com> Tue Jan 11 23:03:28 CST 2011

#include <cavan.h>
#include <cavan/dd.h>

#define UIMAGE_HEADER_SIZE	64
#define UBOOT_PADDING_SIZE	KB(1)

#define UBOOT_OFFSET		KB(1)
#define UIMAGE_OFFSET		MB(1)
#define RAMDISK_OFFSET		MB(5)
#define SN_OFFSET			MB(10)
#define LOGO_OFFSET			MB(12)
#define BUSYBOX_OFFSET		MB(14)
#define CARTOON_OFFSET		MB(30)

#define UBOOT_MAX_SIZE		(UIMAGE_OFFSET - UBOOT_OFFSET)
#define UIMAGE_MAX_SIZE		(RAMDISK_OFFSET - UIMAGE_OFFSET)
#define RAMDISK_MAX_SIZE	(SN_OFFSET - RAMDISK_OFFSET)
#define LOGO_MAX_SIZE		(BUSYBOX_OFFSET - LOGO_OFFSET)
#define BUSYBOX_MAX_SIZE	(CARTOON_OFFSET - BUSYBOX_OFFSET)
#define CARTOON_MAX_SIZE	(MB(BOOT_SIZE) - CARTOON_OFFSET)

#define TEMP_RAMDISK_PATH	"/tmp/ramdisk.img"
#define TEMP_URAMDISK_PATH	"/tmp/uramdisk.img"
#define TEMP_UIMAGE_PATH	"/tmp/uImage"

enum image_type
{
	IMAGE_UNKNOWN,
	IMAGE_UBOOT_BIN,
	IMAGE_UBOOT_NO_PADDING_BIN,
	IMAGE_ZIMAGE,
	IMAGE_UIMAGE,
	IMAGE_RAMDISK_IMG,
	IMAGE_URAMDISK_IMG,
	IMAGE_LOGO,
	IMAGE_BUSYBOX_IMG,
	IMAGE_CARTOON,
	IMAGE_SYSTEM_IMG,
	IMAGE_RECOVERY_IMG,
	IMAGE_USERDATA_IMG,
};

const char *image_type_to_text(enum image_type type);
enum image_type path_to_image_type(const char *img_path);
int image_type_to_device_index(enum image_type type);
int image_type_to_partition_path(enum image_type type, const char *dev_path, char *part_path);
int image_type_to_dd_desc(enum image_type type, struct dd_desc *desc);

int uramdisk2ramdisk(const char *uramdisk_path, const char *ramdisk_path);
int ramdisk2uramdisk(const char *ramdisk_path, const char *uramdisk_path);
int dump_ramdisk(const char *ramdisk_path, const char *ramdisk_dir);
int dump_uramdisk(const char *uramdisk_path, const char *ramdisk_dir);
int create_ramdisk(const char *ramdisk_dir, const char *ramdisk_path);
int create_uramdisk(const char *ramdisk_dir, const char *uramdisk_path);
int image_is(const char *img_path, const char *type);

int image_resize(const char *img_path, u64 size);
int image_extend(const char *img_path);
int image_shrink(const char *img_path);

int uboot2uboot_np(const char *uboot_path, const char *uboot_np_path);
int uboot_np2uboot(const char *uboot_np_path, const char *uboot_path);
int burn_uboot(const char *uboot_path, const char *dev_path);
int burn_uboot_np(const char *uboot_np_path, const char *dev_path);
int zImage2uImage(const char *zImage_path, const char *uImage_path);
int uImage2zImage(const char *uImage_path, const char *zImage_path);
int burn_uImage(const char *uImage_path, const char *dev_path);
int burn_zImage(const char *zImage_path, const char *dev_path);
int burn_uramdisk(const char *uramdisk_path, const char *dev_path, int busybox);
int burn_ramdisk(const char *ramdisk_path, const char *dev_path, int busybox);
int burn_directory(const char *dir_path, const char *dev_path, int busybox);
int burn_normal_image(const char *img_path, const char *dev_path);
void adjust_image_device(const char **img_path, const char **dev_path);
int image_is_uramdisk(const char *img_path);

int get_uramdisk(const char *dev_path, const char *file_path, int busybox);
int get_dump_uramdisk(const char *dev_path, const char *ramdisk_dir, int busybox);
int get_ramdisk(const char *dev_path, const char *file_path, int busybox);

int burn_swan_image_directory(const char *dirname, const char *dest_dev);
int burn_swan_image_auto(const char *img_path, const char *dest_dev);

