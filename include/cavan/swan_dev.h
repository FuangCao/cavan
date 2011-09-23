#pragma once

#include <cavan/device.h>

#define BOOT_SIZE				36
#define SYSTEM_SIZE				256
#define RECOVERY_SIZE			32
#define USERDATA_SIZE			512
#define CACHE_SIZE				128

#define LINUX_PARTITION_TYPE	"ext3"
#define EMMC_VFAT_DEFAULT_LABEL	""

#define CPUINFO_LINE_MACHINE_NAME	"Hardware"

enum swan_board_type
{
	SWAN_BOARD_UNKNOWN,
	SWAN_BOARD_I200,
	SWAN_BOARD_I600,
};

struct swan_machine
{
	enum swan_board_type type;
	char name[1024];
};

int swan_sfdisk(struct partition_desc *desc);
int swan_mkfs(struct partition_desc *dev_desc, struct partition_desc *part_descs, int count);
int swan_copy(const char *cache_file);
int destroy_environment(const char *dev_path);
int swan_check(const char *dev_path, const char *list_partition);
int swan_find_upgrade_file_from(const char *dev_path, const char *tmp_file);
int swan_find_upgrade_file(const char *tmp_file);

int fset_brightness_shadow_increase(int fd, int current, int target, int step, u32 msec);
int fset_brightness_shadow_decrease(int fd, int current, int target, int step, u32 msec);
int set_brightness_shadow(int brightness, int step, u32 msec);
int display_config(int text_mode);

int swan_board_name_to_type(const char *name);
const char *swan_board_type_tostring(enum swan_board_type type);
enum swan_board_type get_swan_board_type(void);
int swan_board_type_check(int pkg_type);

