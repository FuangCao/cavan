#pragma once

// Fuang.Cao <cavan.cfa@gmail.com> 2011-12-06 18:20:47

#define FILE_PROC_MTD	"/proc/mtd"

struct mtd_partition_descriptor
{
	int fd;
	int index;
	char name[64];
	u32 size;
	u32 erase_size;
};

struct cavan_mtd_descriptor
{
	int part_count;
	struct mtd_partition_descriptor parts[32];
};

void cavan_show_mtd_partition(const struct mtd_partition_descriptor *parts);
void cavan_show_mtd_partitions(const struct mtd_partition_descriptor *parts, size_t size);
int cavan_load_mtd_table(const char *mtd_table, struct mtd_partition_descriptor *parts, size_t size);
int cavan_mtd_init(struct cavan_mtd_descriptor *desc, const char *mtd_table);
void cavan_mtd_uninit(struct cavan_mtd_descriptor *desc);
struct mtd_partition_descriptor *cavan_mtd_find_partition_by_name(struct cavan_mtd_descriptor *desc, const char *partname);
int cavan_mtd_open_partition(struct mtd_partition_descriptor *part, int flags);
void cavan_mtd_close_partition(struct mtd_partition_descriptor *part);
int cavan_mtd_erase_blocks(struct mtd_partition_descriptor *part, int start, int count);
	
static inline int cavan_mtd_erase_partition(struct mtd_partition_descriptor *part)
{
	return cavan_mtd_erase_blocks(part, 0, part->size / part->erase_size);
}

