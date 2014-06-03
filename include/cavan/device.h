#pragma once

#include <cavan.h>
#include <sys/mount.h>
#include <linux/hdreg.h>
#include <linux/loop.h>
#include <sys/vfs.h>

#define TEMP_MOUNT_POINT	"/tmp/tmp_mount_point"
#define TEMP_DEVICE_NODE	"/tmp/tmp_deivce_node"
#define MKFS_FLAG_TEST		(1 << 0)
#define FSTYPE_NAME_LEN		16

#ifndef MNT_FORCE
#define MNT_FORCE			(1 << 0)
#endif

#ifndef MNT_DETACH
#define MNT_DETACH			(1 << 1)
#endif

#ifndef MNT_EXPIRE
#define MNT_EXPIRE			(1 << 2)
#endif

#ifndef BLKSSZGET
#define BLKSSZGET			_IO(0x12,104) /* Get block device sector size.  */
#define BLKGETSIZE64		_IOR(0x12,114,size_t) /* return device size.  */
#endif

typedef struct loop_info64	cavan_loop_info_t;

#pragma pack(1)
struct disk_address
{
	u8	header_number;
	u8	sector_number		:6;
	u8	cylinder_number_hsb	:2;
	u8	cylinder_number_lsb;
};

struct disk_partition_table
{
	u8 active_mark;
	struct disk_address start_addr;
	u8 file_system_mark;
	struct disk_address end_addr;
	u32 abs_start_sector_number;
	u32 total_sector_count;
};

struct master_boot_sector
{
	u8 master_boot_record[218];
	u8 idlesse_space[228];
	struct disk_partition_table disk_part_tables[4];
	u16 magic_number;
};
#pragma pack()

enum filesystem_type
{
	FS_START, FS_EXT2, FS_EXT3, FS_EXT4, FS_VFAT, FS_NTFS, FS_FUSE, FS_SYS, FS_PROC, FS_TMP, FS_END
};

struct filesystem_desc
{
	enum filesystem_type type;
	char name[64];
	char mkfs_cmd[64];
	char label_cmd[64];
	char mkfs_label_opt;
};

struct partition_desc
{
	int major;
	int minor;
	u32 flags;
	char path[128];
	char label[16];
	enum filesystem_type type;
};

struct mount_table
{
	char source[128];
	char target[128];
	char fstype[64];
	char option[256];
};

struct mtd_descriptor
{
	int fd;
	size_t erase_size;
};

struct cavan_block_device
{
	void *context;
	int block_shift;
	u16 block_size;
	u32 block_mask;

	ssize_t (*read_block)(struct cavan_block_device *bdev, u64 index, void *buff, size_t count);
	ssize_t (*write_block)(struct cavan_block_device *bdev, u64 index, const void *buff, size_t count);
	ssize_t (*read_byte)(struct cavan_block_device *bdev, u64 index, u32 offset, void *buff, size_t size);
	ssize_t (*write_byte)(struct cavan_block_device *bdev, u64 index, u32 offset, const void *buff, size_t size);
};

// ================================================================================

void visual_usleep(u32 usec);
void visual_msleep(u32 msec);
void visual_ssleep(u32 sec);

int fget_device_size(int dev_fd, u64 *size);
int get_device_size(const char *dev_path, u64 *size);

int system_mount(const char *mnt_dev, const char *mnt_point, const void *data);
int mount_update_mtab(const char *source, const char *target, const char *fstype, unsigned long flags, const void *data);
int libc_mount_auto(const char *source, const char *target, unsigned long flags, const void *data);
int libc_mount(const char *source, const char *target, const char *fstype, unsigned long flags, const void *data);
int libc_mount_to(const char *source, const char *target, const char *fstype, unsigned long flags, const void *data);
int mount_system_devices(void);
int system_init(const char *path, char *argv[]);

int mount_to(const char *mnt_dev, const char *mnt_point, const char *fstype, const void *data);
int mount_main(const char *mnt_dev, const char *mnt_point, const char *fstype, const void *data);

int umount_directory_wait(const char *mnt_point);
int umount_directory2(const char *mnt_point, int flags);
int umount_by_key_text(const char *key_text, int flags);
int umount_device(const char *dev_path, int flags);
int umount_abs_path(const char *abs_path, int flags);
int umount_main(const char *pathname, int flags);
int umount_partition(const char *dev_path, int flags);
int umount_all1(char *devs[], size_t count, int flags);
int umount_all2(char *devs[], int flags);
int vumount_all3(int flags, va_list ap);
int umount_all3(int flags, ...);

int can_mount(const char *mnt_dev, const char *fstype);
int cavan_mkdir(const char *dir_path);

int partition_is_mounted(const char *source, const char *target);
int device_is_mounted_base(const char *dev_abs_path);
int device_is_mounted(const char *dev_path);

int simple_ioctl(const char *dev_path, int request);

int reread_part_table(const char *dev_path);
int freread_part_table_retry(int dev_fd, int count);
int reread_part_table_retry(const char *dev_path, int count);

void show_disk_address(struct disk_address *addr);
void show_partition_table(struct disk_partition_table *ptable_p);
void show_partition_tables(struct disk_partition_table *ptables_p);
void show_master_boot_sector(struct master_boot_sector *mbs_p);
int fread_master_boot_sector(int dev_fd, struct master_boot_sector *mbs_p);
int read_master_boot_sector(const char *dev_path, struct master_boot_sector *mbs_p);
int fwrite_master_boot_sector(int dev_fd, struct master_boot_sector *mbs_p);
int write_master_boot_sector(const char *dev_path, struct master_boot_sector *mbs_p);
ssize_t fget_first_partition_offset(int fd, off_t *offset);
ssize_t get_first_partition_offset(const char *dev_path, off_t *offset);

int ioctl_get32(const char *dev_path, int request, u32 *val_p);
int ioctl_set32(const char *dev_path, int request, u32 val);

void show_geometry(struct hd_geometry *geo_p);
int get_device_geometry(const char *dev_path, struct hd_geometry *geo_p);
void sector_index2disk_address(struct hd_geometry *geo_p, u32 sec_index, struct disk_address *disk_addr);
u32 disk_address2sector_index(struct hd_geometry *geo_p, struct disk_address *disk_addr);
u32 sector_cylinder_halignment(u32 sec_index, struct hd_geometry *geo_p);
u32 sector_cylinder_lalignment(u32 sec_index, struct hd_geometry *geo_p);
u32 sector_cylinder_alignment_auto(u32 sec_index, struct hd_geometry *geo_p);
void set_part_address(struct disk_partition_table *part_table_p, struct hd_geometry *geo_p, u32 sec_index, u32 sec_count);

struct filesystem_desc *get_fsdesc_by_name(const char *name);
struct filesystem_desc *get_fsdesc_by_type(enum filesystem_type type);

int partition_mount_to(struct partition_desc *part_desc, const char *mnt_point, const void *data);
int partition_test_mount(struct partition_desc *part_desc);

int partition_test(struct partition_desc *part_desc);
int partition_change_label(struct partition_desc *part_desc);
int partition_change_label_auto(struct partition_desc *part_desc);
int partition_mkfs(struct partition_desc *part_desc);
void show_partition_desc(struct partition_desc *part_desc);
ssize_t partition_read_label_base(const char *dev_path, const char *read_cmd, char *buff, size_t buff_len);
ssize_t partition_read_label(struct partition_desc *part_desc);
ssize_t partition_read_label_auto(const char *dev_path, char *buff, size_t size);

void show_mount_table(struct mount_table *mtab);

ssize_t parse_mount_table(int fd, struct mount_table *mtab, size_t mtab_size);
ssize_t read_mount_table(struct mount_table *mtab, size_t size);
void print_mount_table_base(struct mount_table *mtab, size_t size);
int print_mount_table(void);
const char *find_mount_table_item(const char *mounts, const char *item);
const char *find_mount_table_base(const char *mounts, struct mount_table *mtab, const char *text);
int find_mount_table(struct mount_table *mtab, const char *text);
char *get_mount_source_base(const char *target, char *buff, size_t size);
char *get_mount_source(const char *target);
char *get_mount_target_base(const char *source, char *buff, size_t size);
char *get_mount_target(const char *source);

int loop_get_fd(const char *filename, char *loop_path, u64 offset);

ssize_t parse_filesystems(int fd, char (*fstypes)[FSTYPE_NAME_LEN], size_t fstype_size);
ssize_t read_filesystems(char (*fstypes)[FSTYPE_NAME_LEN], size_t fstype_size);

ssize_t device_read_blocks(int fd, void *buff, size_t block_size, off_t start, size_t count);
ssize_t device_write_blocks(int fd, void *buff, size_t block_size, off_t start, size_t count);

int device_is_mmc(const char *dev_path);
char *get_device_real_path(char *dest, const char *src);

void show_statfs(struct statfs *stfs);
int get_device_statfs(const char *devpath, const char *fstype, struct statfs *stfs);

int cavan_block_device_init(struct cavan_block_device *bdev, void *context);
void cavan_block_device_deinit(struct cavan_block_device *bdev);

static inline int parse_mount_table_simple(const char *buff, struct mount_table *mtab)
{
	return sscanf(buff, "%s %s %s %s", mtab->source, mtab->target, mtab->fstype, mtab->option) == 4 ? 0 : -EINVAL;
}

static inline int loop_clr_fd(const char *loop_path)
{
	return simple_ioctl(loop_path, LOOP_CLR_FD);
}

static inline u32 calculate_partition_size(u32 start_sec, u32 sec_count, struct hd_geometry *geo_p)
{
	return (sec_count - (start_sec % (geo_p->heads * geo_p->sectors)));
}

static inline int remknod(const char *path, mode_t mode, dev_t dev)
{
	remove(path);

	return mknod(path, S_IFBLK | mode, dev);
}

static inline int remkfifo(const char *path, mode_t mode)
{
	remove(path);

	return mkfifo(path, S_IFIFO | mode);
}

static inline int fioctl_get32(int fd, int request, u32 *val_p)
{
	return ioctl(fd, request, val_p);
}

static inline int fioctl_set32(int fd, int request, u32 val)
{
	return ioctl(fd, request, val);
}

static inline int get_sector_size(int dev_fd, u32 *val_p)
{
	return fioctl_get32(dev_fd, BLKSSZGET, val_p);
}

static inline int fget_device_geometry(int dev_fd, struct hd_geometry *geo_p)
{
	return ioctl(dev_fd, HDIO_GETGEO, geo_p);
}

static inline int fsimple_ioctl(int fd, int request)
{
	return ioctl(fd, request);
}

static inline int freread_part_table(int dev_fd)
{
	return fsimple_ioctl(dev_fd, BLKRRPART);
}

static inline char *get_partition_mount_point_base(const char *devpath, char *buff, size_t size)
{
	return get_mount_target_base(devpath, buff, size);
}

static inline char *get_partition_mount_point(const char *devpath)
{
	return get_mount_target(devpath);
}

