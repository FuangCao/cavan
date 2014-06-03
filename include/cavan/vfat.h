#pragma once

// Fuang.Cao <cavan.cfa@gmail.com> Sat May  7 00:23:47 CST 2011

#include <cavan.h>
#include <cavan/device.h>

#define CAVAN_VFAT_PATH_SEP					'/'

#define VFAT_ATTR_READ_ONLY		(1 << 0)
#define VFAT_ATTR_HIDDEN		(1 << 1)
#define VFAT_ATTR_SYSTEM		(1 << 2)
#define VFAT_ATTR_VOLUME_ID		(1 << 3)
#define VFAT_ATTR_DIRECTORY		(1 << 4)
#define VFAT_ATTR_AECHIVE		(1 << 5)
#define VFAT_LAST_LONG_ENTRY	0x40

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

#define VFAT_BUILD_START_CLUSTER(desc) \
	WORDS_DWORD((desc)->first_cluster_HW, (desc)->first_cluster_LW)

typedef enum
{
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
	u8 fat_table_count;
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
	u8 current_time_teenth;
	u16 current_time;
	u16 current_date;
	u16 last_access_date;
	u16 first_cluster_HW;
	u16 write_time;
	u16 write_date;
	u16 first_cluster_LW;
	u32 file_size;
};

struct vfat_dir_entry_long
{
	u8 order;
	u8 name1[10];
	u8 attribute;
	u8 type;
	u8 chesksum;
	u8 name2[12];
	u16 first_cluster_LW;
	u8 name3[4];
};
#pragma pack()

struct cavan_vfat_fs
{
	struct cavan_block_device *bdev;
	struct fat_dbr dbr;
	const u8 *fat_table;

	fat_type_t type;
	u32 eof_flag;
	u32 fat_size;
	u32 total_sectors;

	u32 blocks_per_sector;
	int blocks_per_sector_shift;

	u32 bytes_per_sector;
	int bytes_per_sector_shift;

	u32 bytes_per_cluster;
	int bytes_per_cluster_shift;
	int sectors_per_cluster_shift;

	u32 data_sectors;
	u32 data_clusters;
	u32 data_first_sector;

	u32 root_dir_sectors;
	u32 root_first_sector;
	u32 root_first_cluster;
};

struct cavan_vfat_file
{
	const char *pathname;
	struct cavan_vfat_fs *fs;
	struct vfat_dir_entry entry;
};

#if 0

ssize_t ffat_read_dbr(int fd, struct fat_dbr *dbr);
const char *fat_type_to_string(enum fat_type type);

u32 get_fat_table_entry(struct fat_info *info_p, u32 cluster_index);
int set_fat_table_entry(struct fat_info *info_p, u32 cluster_index, u32 fat_entry);
int entry_is_eof(struct fat_info *info_p, u32 entry_cotent);

u64 get_first_sector_of_cluster(struct fat_info *info_p, u32 cluster_index);

int ffat_init(int fd, struct fat_info *info_p);
int fat_init(const char *pathname, struct fat_info *info_p);
void fat_deinit(struct fat_info *info_p);

void get_fat_volume_label(struct fat_info *info_p, char *volume);
struct fat_directory *get_file_name(struct fat_directory *dir_p, char *buff);
const char *attribute_to_string(u8 attribute);
ssize_t load_directory(struct fat_info *info_p, u32 cluster_index, struct fat_soft_direcory *soft_dir, size_t size);
int find_path(struct fat_info *info_p, const char *pathname, struct fat_directory *file_desc);
int fat16_root_find(struct fat_info *info_p, const char *filename, struct fat_directory *file_desc);
int find_path2_simple(struct fat_info *info_p, u32 cluster_index, const char *filename, struct fat_directory *file_desc);
int find_path2(struct fat_info *info_p, const char *pathname, struct fat_directory *file_desc);

ssize_t load_data(struct fat_info *info_p, u32 start_cluster, void *buff, size_t size);
ssize_t load_file(struct fat_info *info_p, const char *filename, void *buff, size_t size);
ssize_t load_root_directory(struct fat_info *info_p, struct fat_soft_direcory *soft_dirs, size_t size);
ssize_t fat16_load_root_directory(struct fat_info *info_p, struct fat_soft_direcory *soft_dirs, size_t size);
int fat16_print_root(struct fat_info *info_p);

int print_directory(struct fat_info *info_p, u32 cluster_index);
int list_directory(struct fat_info *info_p, const char *pathname);

static inline int cluster_is_eof(struct fat_info *info_p, u32 cluster_index)
{
	u32 entry_cotent = get_fat_table_entry(info_p, cluster_index);

	return entry_is_eof(info_p, entry_cotent);
}

static inline u32 get_next_cluster_index(struct fat_info * info_p, u32 cluster_index)
{
	return get_fat_table_entry(info_p, cluster_index);
}

static inline ssize_t fat_read_sectors(struct fat_info *info_p, u64 sector_index, size_t count, void *buff)
{
	u64 bytes_per_sector = info_p->dbr.bytes_per_sector;

	return ffile_readfrom(info_p->fat_fd, buff, bytes_per_sector * count, bytes_per_sector * sector_index);
}

static inline ssize_t fat_write_sectors(struct fat_info *info_p, u64 sector_index, size_t count, const void *buff)
{
	u64 bytes_per_sector = info_p->dbr.bytes_per_sector;

	return ffile_writeto(info_p->fat_fd, buff, bytes_per_sector * count, bytes_per_sector * sector_index);
}

static inline ssize_t fat_read_clusters(struct fat_info *info_p, u32 cluster_index, size_t count, void *buff)
{
	u64 first_sector = get_first_sector_of_cluster(info_p, cluster_index);

	return fat_read_sectors(info_p, first_sector, info_p->dbr.sectors_per_cluster * count, buff);
}

static inline ssize_t fat_write_clusters(struct fat_info *info_p, u32 cluster_index, size_t count, const void *buff)
{
	u64 first_sector = get_first_sector_of_cluster(info_p, cluster_index);

	return fat_write_sectors(info_p, first_sector, info_p->dbr.sectors_per_cluster * count, buff);
}

static inline ssize_t read_fat_table(struct fat_info *info_p)
{
	return fat_read_sectors(info_p, info_p->dbr.reserve_sector_count, info_p->fat_size, info_p->fat_table);
}
#endif
void cavan_vfat_dbr_dump(const struct fat_dbr *dbr, fat_type_t type);
void cavan_vfat_dir_entry_dump(const struct vfat_dir_entry *entry);
void cavan_vfat_dir_entry_long_dump(const struct vfat_dir_entry_long *entry);

int cavan_vfat_init(struct cavan_vfat_fs *fs, struct cavan_block_device *bdev);
void cavan_vfat_deinit(struct cavan_vfat_fs *fs);

struct cavan_vfat_file *cavan_vfat_open_file(struct cavan_vfat_fs *fs, const char *pathname);
void cavan_vfat_close_file(struct cavan_vfat_file *fp);

int cavan_vfat_list_dir(struct cavan_vfat_file *fp, void (*handler)(struct vfat_dir_entry *entry, void *data), void *data);
ssize_t cavan_vfat_read_file(struct cavan_vfat_file *fp, off_t offset, char *buff, size_t size);
ssize_t cavan_vfat_read_file3(struct cavan_vfat_file *fp, off_t offset, const char *pathname, int flags);
