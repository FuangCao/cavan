#pragma once

#include <cavan.h>
#include <cavan/device.h>

#define BOOT_SIZE				36
#define SYSTEM_SIZE				256
#define SYSTEM_MIN_SIZE			128
#define RECOVERY_SIZE			32
#define RECOVERY_MIN_SIZE		16
#define USERDATA_SIZE			512
#define USERDATA_MIN_SIZE		128
#define CACHE_SIZE				128
#define CACHE_MIN_SIZE			64

#define LINUX_PARTITION_TYPE	"ext3"
#define EMMC_VFAT_DEFAULT_LABEL	""

#define CPUINFO_LINE_MACHINE_NAME	"Hardware"

enum swan_board_type
{
	SWAN_BOARD_UNKNOWN,
	SWAN_BOARD_I200,
	SWAN_BOARD_I600,
	SWAN_BOARD_I700,
};

struct swan_machine
{
	enum swan_board_type type;
	char name[1024];
};

struct swan_emmc_partition_table
{
	u32 system_size;
	u32 recovery_size;
	u32 userdata_size;
	u32 cache_size;
	u32 vendor_size;
};

enum swan_image_type;

void show_swan_emmc_partation_table(struct swan_emmc_partition_table *part_table);
ssize_t get_partition_size_by_type(enum swan_image_type type, struct swan_emmc_partition_table *part_table);
void get_default_emmc_partition_table(struct swan_emmc_partition_table *part_table);
int fix_emmc_partition_table(struct swan_emmc_partition_table *part_table);
int swan_sfdisk(struct partition_desc *dev_desc, struct swan_emmc_partition_table *part_table);
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
int swan_check_emmc_device(void);
int swan_check_uname(void);

