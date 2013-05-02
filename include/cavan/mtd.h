#pragma once

#include <cavan.h>
#include <mtd/mtd-user.h>

// Fuang.Cao <cavan.cfa@gmail.com> 2011-12-06 18:20:47

#define FILE_PROC_MTD				"/proc/mtd"
#define MAX_MTD_NAME_LENGHT			64
#define MAX_MTD_PARTITION_COUNT		32
#define SUPPORT_MTD_WRITE_VERIFY	0

struct cavan_mtd_descriptor;
struct mtd_partition_info
{
	int index;
	char name[MAX_MTD_NAME_LENGHT];
	u32 size;
	u32 erase_size;

	struct cavan_mtd_descriptor *dev_desc;
};

struct mtd_partition_descriptor
{
	int fd;
	struct mtd_info_user user_info;
	struct mtd_partition_info *part_info;
};

struct cavan_mtd_descriptor
{
	int part_count;
	struct mtd_partition_info part_infos[MAX_MTD_PARTITION_COUNT];
	struct mtd_partition_descriptor part_descs[MAX_MTD_PARTITION_COUNT];
};

void cavan_mtd_show_parts_info(const struct mtd_partition_info *parts, size_t size);
int cavan_load_mtd_table(struct cavan_mtd_descriptor *desc, const char *mtd_table);
int cavan_mtd_init(struct cavan_mtd_descriptor *desc, const char *mtd_table);
void cavan_mtd_deinit(struct cavan_mtd_descriptor *desc);
struct mtd_partition_info *cavan_mtd_get_partition_info_by_name(struct cavan_mtd_descriptor *desc, const char *partname);
int cavan_mtd_open(struct mtd_partition_info *part, int flags);
int mtd_open_char_device(int index, int flags);
struct mtd_partition_descriptor *cavan_mtd_open_partition2(struct cavan_mtd_descriptor *desc, const char *partname, int flags);
void cavan_mtd_close_partition(struct mtd_partition_descriptor *desc);
int cavan_mtd_erase_blocks(struct mtd_partition_descriptor *desc, int start, int count);
int cavan_mtd_erase_partition(struct mtd_partition_descriptor *desc);

ssize_t cavan_mtd_write_block(struct mtd_partition_descriptor *desc, const void *buff);
int cavan_mtd_write_partition1(struct mtd_partition_descriptor *desc, int fd);
int cavan_mtd_write_partition2(struct mtd_partition_descriptor *desc, const char *filename);
int cavan_mtd_write_partition3(struct mtd_partition_info *info, const char *filename);
int cavan_mtd_write_partition4(struct cavan_mtd_descriptor *desc, const char *partname, const char *filename);
int cavan_mtd_write_partition5(const char *partname, const char *filename);

int cavan_mtd_write_image1(struct mtd_partition_descriptor *desc, int fd);
int cavan_mtd_write_image2(struct mtd_partition_descriptor *desc, const char *imagename);

struct mtd_partition_descriptor *cavan_mtd_malloc_partition_descriptor(struct cavan_mtd_descriptor *desc);
void cavan_mtd_free_partition_descriptor(struct mtd_partition_descriptor *desc);
struct mtd_partition_descriptor *cavan_mtd_open_partition(struct mtd_partition_info *info, int flags);
void cavan_mtd_show_info_user(struct mtd_info_user *info);
