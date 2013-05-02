#pragma once

// Fuang.Cao <cavan.cfa@gmail.com> Sat May  7 00:23:47 CST 2011

#include <cavan.h>
#include <cavan/file.h>

#define ATTR_READ_ONLY	(1 << 0)
#define ATTR_HIDDEN		(1 << 1)
#define ATTR_SYSTEM		(1 << 2)
#define ATTR_VOLUME_ID	(1 << 3)
#define ATTR_DIRECTORY	(1 << 4)
#define ATTR_AECHIVE	(1 << 5)
#define LAST_LONG_ENTRY	0x40

#define ATTR_LONG_NAME \
	(ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)

#define ATTR_LONG_NAME_MASK \
	(ATTR_LONG_NAME | ATTR_DIRECTORY | ATTR_AECHIVE)

#define IS_LONG_NAME(desc) \
	((((desc)->attribute) & ATTR_LONG_NAME_MASK) == ATTR_LONG_NAME)

#define IS_DIRECTORY(desc) \
	(((desc)->attribute) & ATTR_DIRECTORY)

#define IS_NOT_FILE(desc) \
	((desc)->attribute & (ATTR_DIRECTORY | ATTR_VOLUME_ID))

#define IS_FILE(desc) \
	(!IS_NOT_FILE(desc))

#define IS_VOLUME_LABEL(desc) \
	((desc)->attribute & ATTR_VOLUME_ID)

#define BUILD_START_CLUSTER(desc) \
	WORDS_DWORD((desc)->first_cluster_HW, (desc)->first_cluster_LW)

enum fat_type
{
	FAT12,
	FAT16,
	FAT32,
};

#pragma pack(1)
struct fat16_dbr_tail
{
	u8 driver_number;
	u8 reserved1;
	u8 boot_signal;
	u8 volume_id[4];
	u8 volume_label[11];
	u8 fs_type[8];
};

struct fat32_dbr_tail
{
	u32 fat_size32;
	u16 extern_flags;
	u16 fs_version;
	u32 root_first_cluster;
	u16 fs_info_sectors;
	u16 backup_boot_sectors;
	u8 reserved1[12];
	u8 driver_number;
	u8 reserved2;
	u8 boot_signal;
	u32 volume_index;
	u8 volume_label[11];
	u8 fs_type[8];
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
	u16 sectors_per_cylinder;
	u16 header_count;
	u32 hide_sectors;
	u32 total_sectors32;

	union
	{
		struct fat16_dbr_tail tail16;
		struct fat32_dbr_tail tail32;
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

struct fat_directory
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

struct fat_long_directory
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

struct fat_soft_direcory
{
	char name[1024];
	struct fat_directory short_dir;
};

struct fat_info
{
	struct fat_dbr dbr;
	enum fat_type type;
	void *fat_table;
	int fat_fd;
	u32 fat_size;

	u32 bytes_per_cluster;
	u32 total_sector_count;
	u32 descs_per_cluster;

	u32 data_sector_count;
	u32 data_cluster_count;
	u32 data_first_sector;

	u32 root_sector_count;
	u32 root_first_cluster;
	u32 root_first_sector;
};

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

