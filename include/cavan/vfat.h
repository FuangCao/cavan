#pragma once

// Fuang.Cao <cavan.cfa@gmail.com> Sat May  7 00:23:47 CST 2011

#include <cavan.h>
#include <cavan/device.h>

#define CAVAN_VFAT_PATH_SEP					'/'

#define VFAT_ATTR_READ_ONLY			(1 << 0)
#define VFAT_ATTR_HIDDEN			(1 << 1)
#define VFAT_ATTR_SYSTEM			(1 << 2)
#define VFAT_ATTR_VOLUME_ID			(1 << 3)
#define VFAT_ATTR_DIRECTORY			(1 << 4)
#define VFAT_ATTR_AECHIVE			(1 << 5)
#define VFAT_LAST_LONG_ENTRY		0x40
#define VFAT_LONG_ENTRY_INDEX_MASK	(VFAT_LAST_LONG_ENTRY - 1)

#define VFAT_ATTR_LONG_NAME \
	(VFAT_ATTR_READ_ONLY | VFAT_ATTR_HIDDEN | VFAT_ATTR_SYSTEM | VFAT_ATTR_VOLUME_ID)

#define VFAT_ATTR_LONG_NAME_MASK \
	(VFAT_ATTR_LONG_NAME | VFAT_ATTR_DIRECTORY | VFAT_ATTR_AECHIVE)

#define VFAT_IS_LONG_NAME(desc) \
	((((desc)->attribute) & VFAT_ATTR_LONG_NAME_MASK) == VFAT_ATTR_LONG_NAME)

#define VFAT_IS_DIRECTORY(desc) \
	(((desc)->attribute) & VFAT_ATTR_DIRECTORY)

#define VFAT_IS_NOT_FILE(desc) \
	((desc)->attribute & (VFAT_ATTR_DIRECTORY | VFAT_ATTR_VOLUME_ID))

#define VFAT_IS_FILE(desc) \
	(!VFAT_IS_NOT_FILE(desc))

#define VFAT_IS_VOLUME_LABEL(desc) \
	((desc)->attribute & VFAT_ATTR_VOLUME_ID)

#define VFAT_BUILD_START_CLUSTER(entry) \
	WORDS_DWORD((entry)->first_cluster_hi, (entry)->first_cluster_lo)

typedef enum {
	FAT12,
	FAT16,
	FAT32,
} fat_type_t;

#pragma pack(1)
struct fat16_dbr
{
	u8 drive_number;
	u8 reserved1;
	u8 boot_signature;
	u32 serial_number;
	u8 volume_label[11];
	u8 fs_type[8];
};

struct fat32_dbr
{
	u32 fat_size32;
	u16 extern_flags;
	u16 fs_version;
	u32 root_first_cluster;
	u16 fs_info_sectors;
	u16 backup_boot_sectors;
	u8 reserved1[12];
	struct fat16_dbr dbr16;
	u8 reserved3[420];
};

struct fat_dbr
{
	u8 jmp_boot[3];
	u8 oem_name[8];
	u16 bytes_per_sector;
	u8 sectors_per_cluster;
	u16 reserve_sector_count;
	u8 fat_count;
	u16 root_entry_count;
	u16 total_sector16;
	u8 medium_describe;
	u16 fat_size16;
	u16 sectors_per_track;
	u16 head_count;
	u32 hidden_sectors;
	u32 total_sectors32;

	union
	{
		struct fat16_dbr dbr16;
		struct fat32_dbr dbr32;
	};

	u16 boot_flags;
};

struct fat32_fsinfo
{
	u32 lead_signal;
	u8 reserved1[480];
	u32 struct_signal;
	u32 free_count;
	u32 next_free;
	u8 reserved2[12];
	u32 trail_signal;
};

struct vfat_dir_entry
{
	u8 name[11];
	u8 attribute;
	u8 nt_reserved;
	u8 current_time_tenth;
	u16 current_time;
	u16 current_date;
	u16 last_access_date;
	u16 first_cluster_hi;
	u16 write_time;
	u16 write_date;
	u16 first_cluster_lo;
	u32 file_size;
};

struct vfat_dir_entry_long
{
	u8 order;
	u16 name1[5];
	u8 attribute;
	u8 type;
	u8 chesksum;
	u16 name2[6];
	u16 first_cluster_LW;
	u16 name3[2];
};
#pragma pack()

struct cavan_vfat_fs
{
	struct cavan_block_device *bdev;
	struct fat_dbr dbr;
	u8 *fat_table;

	fat_type_t type;
	u32 eof_flag;
	u32 fat_sectors;
	u32 total_sectors;

	u32 blocks_per_sector;
	int blocks_per_sector_shift;
	u32 bytes_per_sector;
	int bytes_per_sector_shift;

	u32 bytes_per_cluster;
	int bytes_per_cluster_shift;
	int sectors_per_cluster_shift;
	u32 blocks_per_cluster;

	u32 entrys_per_sector;
	u32 entrys_per_cluster;

	u32 data_sectors;
	u32 data_clusters;
	u32 data_first_sector;

	u32 root_dir_sectors;
	u32 root_first_sector;
	u32 root_first_cluster;

	u32 (*get_next_cluster)(const struct cavan_vfat_fs *fs, u32 cluster);
};

struct cavan_vfat_file
{
	const char *pathname;
	struct cavan_vfat_fs *fs;
	struct vfat_dir_entry entry;
};

struct cavan_vfat_scan_dir_walker
{
	char buff[256];
	char *tail;
	char *filename;
	void *context;

	int (*label_handler)(struct cavan_vfat_scan_dir_walker *walker, const char *label);
	int (*entry_handler)(struct cavan_vfat_scan_dir_walker *walker, const struct vfat_dir_entry *entry, const char *filename);
};

struct cavan_vfat_find_file_context
{
	const char *filename;
	size_t namelen;
	struct vfat_dir_entry *entry;
};

struct cavan_vfat_list_dir_context
{
	void *private_data;
	void (*handler)(const struct vfat_dir_entry *entry, const char *filename, void *data);
};

struct cavan_vfat_read_file_context
{
	void *buff;
	size_t size;
};

const char *cavan_vfat_type_to_string(fat_type_t type);
void cavan_vfat_dbr_dump(const struct fat_dbr *dbr, fat_type_t type);
void cavan_vfat_dir_entry_dump(const struct vfat_dir_entry *entry);
void cavan_vfat_dir_entry_long_dump(const struct vfat_dir_entry_long *entry);

int cavan_vfat_init(struct cavan_vfat_fs *fs, struct cavan_block_device *bdev);
void cavan_vfat_deinit(struct cavan_vfat_fs *fs);

struct cavan_vfat_file *cavan_vfat_open_file(struct cavan_vfat_fs *fs, const char *pathname);
void cavan_vfat_close_file(struct cavan_vfat_file *fp);

int cavan_vfat_list_dir(struct cavan_vfat_file *fp, void (*handler)(const struct vfat_dir_entry *entry, const char *filename, void *data), void *data);
ssize_t cavan_vfat_read_file(struct cavan_vfat_file *fp, size_t skip, char *buff, size_t size);
ssize_t cavan_vfat_read_file2(struct cavan_vfat_file *fp, size_t skip, int fd);
ssize_t cavan_vfat_read_file3(struct cavan_vfat_file *fp, size_t skip, const char *pathname, int flags);
ssize_t cavan_vfat_read_volume_label(struct cavan_vfat_fs *fs, char *buff, size_t size);
