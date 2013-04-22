#pragma once

#include <cavan.h>
#include <cavan/text.h>
#include <cavan/swan_pkg.h>

#define EMMC_DEVICE				"/dev/mmcblk0"
#define SDCARD_DEVICE			"/dev/mmcblk1"
#define MOUNT_POINT				"/mnt/sdcard"
#define IPU_ENABLE_FILE			"/sys/swan/power/swan_ipu_fb_cap"
#define SYSTEM_MNT_POINT		"/mnt/system"

#define DEFAULT_VOLUME			"phone"

#define UPGRADE_FILE_PREFIX		"upgrade"
#define UPGRADE_FILE_TYPE		"swan"
#define UPGRADE_FILE_NAME		UPGRADE_FILE_PREFIX "." UPGRADE_FILE_TYPE
#define UPGRADE_FILE_PATH		MOUNT_POINT "/" UPGRADE_FILE_NAME
#define CACHE_FILE_PATH			"/tmp/" UPGRADE_FILE_NAME

#define TEMP_FILE_NAME			"upgrade_file_path.txt"
#define TEMP_FILE_PATH			"/tmp/" TEMP_FILE_NAME
#define UPDATER_SCRIPT_PATH		"/mybin/updater-script"

#define UPGRADE_PROGRAM_NAME	"swan_upgrade"
#define UPGRADE_PROGRAM_PATH	"/tmp/" UPGRADE_PROGRAM_NAME

#define TEMP_DEVICE_PATH		"/tmp/swan_device"
#define FIRST_MINOR				0

#define MAX_CHECK_PATTERN_LEN	16
#define REBOOT_TIMEOUT			5

#define RESOURCE_IMAGE_NAME		"resource.img"
#define I200_RESOURCE_NAME		"resource-i200.img"
#define I600_RESOURCE_NAME		"resource-i600.img"
#define I700_RESOURCE_NAME		"resource-i700.img"
#define MODEM_RESOURCE_NAME		"resource-modem.img"
#define RESOURCE_IMAGE_PATH		"/tmp/" RESOURCE_IMAGE_NAME
#define BMP_PATH				"/media"
#define BACKUP_BMP_PATH			"/etc/bmps"

#define SWAN_LOGO_NAME			"logo.bmp"
#define I200_LOGO_NAME			"logo-i200.bmp"
#define I600_LOGO_NAME			"logo-i600.bmp"
#define I700_LOGO_NAME			"logo-i700.bmp"

#define SWAN_BUSYBOX_NAME		"busybox.img"
#define I200_BUSYBOX_NAME		"busybox.img"
#define I600_BUSYBOX_NAME		"busybox.img"
#define I700_BUSYBOX_NAME		"busybox-i700.img"

#define SWAN_CONSOLE_DEVICE		"/dev/tty0"
#define SWAN_KPD_NAME			"mxckpd"
#define SWAN_KPD_DEVICE			"/dev/input/event0"

#define EXTEND_PATH				"/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin:/mybin"

#define BACKLIGHT_PATH			"/sys/bus/platform/devices/pwm-backlight.0/backlight/pwm-backlight.0"
#define MAX_BRIGHTNESS_NAME		"max_brightness"
#define BRIGHTNESS_NAME			"brightness"
#define MAX_BRIGHTNESS_PATH		BACKLIGHT_PATH "/" MAX_BRIGHTNESS_NAME
#define BRIGHTNESS_PATH			BACKLIGHT_PATH "/" BRIGHTNESS_NAME

#define SWAN_VERSION_PATH		"etc/swan_version.txt"

#define SWAN_KEY_VOLUME_UP		115
#define SWAN_KEY_VOLUME_DOWN	114
#define SWAN_KEY_POWER			116
#define SWAN_KEY_SEARCH			217
#define SWAN_KEY_BACK			139
#define SWAN_KEY_MENU			28
#define SWAN_KEY_HOME			158
#define SWAN_KEY_CALL			231
#define SWAN_KEY_ENDCALL		107

#define MAX_BUFF_LEN			MB(1)

#define MAX_BUILD_PROP_LEN		MB(1)
#define BUILD_PROP_PRODUCT		"ro.build.product"
#define BUILD_PROP_NAME			"build.prop"

#define SWAN_FLAGS_MODEM_UPGRADE	(1 << 0)

extern char swan_vfat_volume[];
extern u32 swan_mkfs_mask;
extern u32 swan_machine_type;
extern struct swan_image_info swan_images[];
extern u32 swan_upgrade_flags;
extern u32 swan_package_flags;
extern enum swan_image_type swan_exclude_images[];
extern int swan_exclude_image_count;
extern int swan_need_shrink;
extern struct swan_emmc_partition_table swan_emmc_part_table;

int package(const char *pkg_name, const char *dir_name);
int unpack(const char *pkg_name, const char *dir_name);
int upgrade(const char *pkg_name, const char *dir_name);
int fupgrade_simple(int pkg_fd);
int upgrade_simple(const char *pkg_name, const char *dir_name);
int auto_upgrade(const char *pkg_name);

enum swan_board_type get_swan_board_type_by_build_prop(const char *build_prop);
enum swan_board_type get_swan_board_type_by_system_img(const char *system_img);

struct swan_image_info *get_swan_image_info_by_name(const char *filename);
struct swan_image_info *get_swan_image_info_by_type(enum swan_image_type type);

const char *get_resource_name_by_board_type(enum swan_board_type type);
const char *get_logo_name_by_board_type(enum swan_board_type type);
const char *get_busybox_name_by_board_type(enum swan_board_type type);

u32 swan_read_version(const char *system_dev);
ssize_t swan_write_version(const char *system_mnt_point, u32 version);
int swan_if_need_upgrade(const char *system_dev, u32 version_new);
int swan_config_system(const char *emmc_dev, u32 version);

