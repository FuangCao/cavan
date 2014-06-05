// Fuang.Cao <cavan.cfa@gmail.com> Sat May  7 00:23:47 CST 2011

#include <cavan.h>
#include <cavan/vfat.h>
#include <cavan/memory.h>
#include <cavan/text.h>
#include <cavan/math.h>

#define CAVAN_VFAT_DEBUG	1

static inline u32 cavan_vfat_get_cluster_first_sector(struct cavan_vfat_fs *fs, u32 index)
{
	return ((index - 2) << fs->sectors_per_cluster_shift) + fs->data_first_sector;
}

static inline ssize_t cavan_vfat_read_sector(struct cavan_vfat_fs *fs, u32 index, void *buff, u32 count)
{
	return fs->bdev->read_block(fs->bdev, index >> fs->blocks_per_sector_shift, buff, count << fs->blocks_per_sector_shift);
}

static inline ssize_t cavan_vfat_read_cluster(struct cavan_vfat_fs *fs, u32 index, void *buff, u32 count)
{
	u32 sector = cavan_vfat_get_cluster_first_sector(fs, index);

	return cavan_vfat_read_sector(fs, sector, buff, count << fs->sectors_per_cluster_shift);
}

static u32 cavan_vfat_get_next_cluster(struct cavan_vfat_fs *fs, u32 cluster)
{
	switch (fs->type)
	{
	case FAT12:
		{
			u16 value = *(u16 *) (fs->fat_table + cluster + (cluster >> 1));
			if (cluster & 1)
			{
				return value >> 4;
			}
			else
			{
				return value & 0x0FFF;
			}
		}

	case FAT16:
		return ((u16 *) fs->fat_table)[cluster];

	case FAT32:
		return ((u32 *) fs->fat_table)[cluster] & 0x0FFFFFFF;

	default:
		return fs->eof_flag;
	}
}

const char *cavan_vfat_type_to_string(fat_type_t type)
{
	switch (type)
	{
	case FAT12:
		return "FAT12";

	case FAT16:
		return "FAT16";

	case FAT32:
		return "FAT32";

	default:
		return "Unknown";
	}
}

static void cavan_vfat16_dbr_dump(const struct fat16_dbr *dbr)
{
	pr_info("drive_number = 0x%02x", dbr->drive_number);
	pr_info("boot_signature = 0x%02x", dbr->boot_signature);
	pr_info("serial_number = 0x%08x", dbr->serial_number);
	pr_info("volume_label = %s", text_header((char *) dbr->volume_label, sizeof(dbr->volume_label)));
	pr_info("fs_type = %s", text_header((char *) dbr->fs_type, sizeof(dbr->fs_type)));
}

static void cavan_vfat32_dbr_dump(const struct fat32_dbr *dbr)
{
	pr_info("fat_size32 = %d", dbr->fat_size32);
	pr_info("extern_flags = %d", dbr->extern_flags);
	pr_info("fs_version = %d", dbr->fs_version);
	pr_info("root_first_cluster = %d", dbr->root_first_cluster);
	pr_info("fs_info_sectors = %d", dbr->fs_info_sectors);
	pr_info("backup_boot_sectors = %d", dbr->backup_boot_sectors);
	cavan_vfat16_dbr_dump(&dbr->dbr16);
}

void cavan_vfat_dbr_dump(const struct fat_dbr *dbr, fat_type_t type)
{
	print_sep(60);
	pr_bold_info("fat dbr %p", dbr);

	// pr_info("jmp_boot[3] = %d", dbr->jmp_boot[3]);
	pr_info("oem_name = %s", dbr->oem_name);
	pr_info("bytes_per_sector = %d", dbr->bytes_per_sector);
	pr_info("sectors_per_cluster = %d", dbr->sectors_per_cluster);
	pr_info("reserve_sector_count = %d", dbr->reserve_sector_count);
	pr_info("fat_count = %d", dbr->fat_count);
	pr_info("root_entry_count = %d", dbr->root_entry_count);
	pr_info("total_sector16 = %d", dbr->total_sector16);
	pr_info("medium_describe = 0x%02x", dbr->medium_describe);
	pr_info("fat_size16 = %d", dbr->fat_size16);
	pr_info("sectors_per_track = %d", dbr->sectors_per_track);
	pr_info("head_count = %d", dbr->head_count);
	pr_info("hidden_sectors = %d", dbr->hidden_sectors);
	pr_info("total_sectors32 = %d", dbr->total_sectors32);

	if (type == FAT32)
	{
		cavan_vfat32_dbr_dump(&dbr->dbr32);
	}
	else
	{
		cavan_vfat16_dbr_dump(&dbr->dbr16);
	}
}

void cavan_vfat_dir_entry_dump(const struct vfat_dir_entry *entry)
{
	print_sep(60);
	pr_bold_info("vfat dir entry %p", entry);

	pr_info("name[11] = %s", text_header((char *) entry->name, sizeof(entry->name)));
	pr_info("attribute = %d", entry->attribute);
	pr_info("nt_reserved = %d", entry->nt_reserved);
	pr_info("current_time_tenth = %d", entry->current_time_tenth);
	pr_info("current_time = %d", entry->current_time);
	pr_info("current_date = %d", entry->current_date);
	pr_info("last_access_date = %d", entry->last_access_date);
	pr_info("first_cluster_HW = %d", entry->first_cluster_hi);
	pr_info("write_time = %d", entry->write_time);
	pr_info("write_date = %d", entry->write_date);
	pr_info("first_cluster_LW = %d", entry->first_cluster_lo);
	pr_info("file_size = %d", entry->file_size);
}

void cavan_vfat_dir_entry_long_dump(const struct vfat_dir_entry_long *entry)
{
	print_sep(60);
	pr_bold_info("vfat dir entry long %p", entry);

	pr_info("order = 0x%02x", entry->order);
	pr_info("name1[10] = %s", text_header((char *) entry->name1, sizeof(entry->name1)));
	pr_info("attribute = %d", entry->attribute);
	pr_info("type = %d", entry->type);
	pr_info("chesksum = %d", entry->chesksum);
	pr_info("name2[12] = %s", text_header((char *) entry->name2, sizeof(entry->name2)));
	pr_info("first_cluster_LW = %d", entry->first_cluster_LW);
	pr_info("name3[4] = %s", text_header((char *) entry->name3, sizeof(entry->name3)));
}

static int cavan_vfat_read_dbr(struct cavan_block_device *bdev, struct fat_dbr *dbr)
{
	ssize_t rdlen;

	rdlen = bdev->read_byte(bdev, 0, 0, dbr, sizeof(*dbr));
	if (rdlen < 0)
	{
		pr_red_info("bdev->read_byte");
		return rdlen;
	}

	if (dbr->boot_flags != 0xAA55)
	{
		pr_red_info("dbr->boot_flags = 0x%04x", dbr->boot_flags);
		return -EINVAL;
	}

	return 0;
}

int cavan_vfat_init(struct cavan_vfat_fs *fs, struct cavan_block_device *bdev)
{
	int ret;
	size_t fat_sectors;
	struct fat_dbr *dbr;

	fs->bdev = bdev;

	dbr = &fs->dbr;
	ret = cavan_vfat_read_dbr(bdev, dbr);
	if (ret < 0)
	{
		pr_red_info("cavan_vfat_read_dbr");
		return ret;
	}

	fs->bytes_per_sector = dbr->bytes_per_sector;
	fs->bytes_per_sector_shift = math_get_value_shift(fs->bytes_per_sector);

	fs->blocks_per_sector = dbr->bytes_per_sector >> bdev->block_shift;
	fs->blocks_per_sector_shift = fs->bytes_per_sector_shift - bdev->block_shift;

	fs->sectors_per_cluster_shift = math_get_value_shift(dbr->sectors_per_cluster);
	fs->bytes_per_cluster_shift = fs->sectors_per_cluster_shift + fs->bytes_per_sector_shift;
	fs->bytes_per_cluster = dbr->sectors_per_cluster * dbr->bytes_per_sector;
	fs->root_dir_sectors = RIGHT_SHIFT_CEIL(dbr->root_entry_count * sizeof(struct vfat_dir_entry), fs->bytes_per_sector_shift);

	fs->entrys_per_sector = fs->bytes_per_sector / sizeof(struct vfat_dir_entry);
	fs->entrys_per_cluster = fs->entrys_per_sector << fs->sectors_per_cluster_shift;

#if CAVAN_VFAT_DEBUG
	pr_info("bytes_per_sector = %d", fs->bytes_per_sector);
	pr_info("bytes_per_sector_shift = %d", fs->bytes_per_sector_shift);
	pr_info("blocks_per_sector_shift = %d", fs->blocks_per_sector_shift);
	pr_info("sectors_per_cluster_shift = %d", fs->sectors_per_cluster_shift);
	pr_info("bytes_per_cluster_shift = %d", fs->bytes_per_cluster_shift);
	pr_info("blocks_per_sector = %d", fs->blocks_per_sector);
	pr_info("bytes_per_cluster = %d", fs->bytes_per_cluster);
	pr_info("root_dir_sectors = %d", fs->root_dir_sectors);
	pr_info("entrys_per_sector = %d", fs->entrys_per_sector);
	pr_info("entrys_per_cluster = %d", fs->entrys_per_cluster);
#endif

	if (dbr->fat_size16)
	{
		fs->fat_sectors = dbr->fat_size16;
	}
	else
	{
		fs->fat_sectors = dbr->dbr32.fat_size32;
	}

	fat_sectors = fs->fat_sectors * dbr->fat_count;
	fs->root_first_sector = dbr->reserve_sector_count + fat_sectors;
	fs->data_first_sector = fs->root_first_sector + fs->root_dir_sectors;

#if CAVAN_VFAT_DEBUG
	pr_info("fat_sectors = %d", fs->fat_sectors);
	pr_info("root_first_sector = %d", fs->root_first_sector);
	pr_info("data_first_sector = %d", fs->data_first_sector);
#endif

	if (dbr->total_sector16)
	{
		fs->total_sectors = dbr->total_sector16;
	}
	else
	{
		fs->total_sectors = dbr->total_sectors32;
	}

	fs->data_sectors = fs->total_sectors - fs->data_first_sector;
	fs->data_clusters = fs->data_sectors / dbr->sectors_per_cluster;

#if CAVAN_VFAT_DEBUG
	pr_info("data_sectors = %d", fs->data_sectors);
	pr_info("data_clusters = %d", fs->data_clusters);
#endif

	if (fs->data_clusters < 4085)
	{
		fs->type = FAT12;
		fs->eof_flag = 0x0FF8;
	}
	else if (fs->data_clusters < 65525)
	{
		fs->type = FAT16;
		fs->eof_flag = 0xFFF8;
	}
	else
	{
		fs->type = FAT32;
		fs->eof_flag = 0x0FFFFFF8;
	}

	if (fs->type == FAT32)
	{
		fs->root_first_cluster = dbr->dbr32.root_first_cluster;
	}
	else
	{
		fs->root_first_cluster = 0;
	}

#if CAVAN_VFAT_DEBUG
	pr_info("type = %s", cavan_vfat_type_to_string(fs->type));
	pr_info("eof_flag = 0x%08x", fs->eof_flag);
	pr_info("root_first_cluster = %d", fs->root_first_cluster);
	cavan_vfat_dbr_dump(dbr, fs->type);
#endif

	fs->fat_table = malloc(fat_sectors << fs->bytes_per_sector_shift);
	if (fs->fat_table == NULL)
	{
		pr_error_info("malloc");
		return -ENOMEM;
	}

	ret = cavan_vfat_read_sector(fs, dbr->reserve_sector_count, fs->fat_table, fat_sectors);
	if (ret < 0)
	{
		pr_red_info("cavan_vfat_read_sector");
		goto out_free_fat_table;
	}

	return 0;

out_free_fat_table:
	free(fs->fat_table);
	return ret;
}

void cavan_vfat_deinit(struct cavan_vfat_fs *fs)
{
	free(fs->fat_table);
}

static char *cavan_vfat_build_volume_label(const u8 name[11], char *buff, size_t size)
{
	const u8 *p;

	for (p = name + 11 - 1; p >= name; p--)
	{
		if (*p != 0x20)
		{
			size_t count;

			count = p - name + 1;
			if (count > size)
			{
				count = size;
			}

			buff = mem_copy(buff, name, count);
			break;
		}
	}

	*buff = 0;

	return buff;
}

static char *cavan_vfat_build_short_name(const u8 name[11], char *buff, size_t size)
{
	int i;
	char *buff_end = buff + size;
	static const int size_array[] = {8, 3};

	for (i = 0; i < 2; i++)
	{
		const u8 *start, *p;

		for (start = name + 8 * i, p = start + size_array[i] - 1; p >= start; p--)
		{
			if (*p != 0x20)
			{
				int count;

				if (i > 0 && buff < buff_end)
				{
					*buff++ = '.';
				}

				count = p - start + 1;
				if (buff + count >= buff_end)
				{
					count = buff_end - buff;
				}

				buff = mem_copy(buff, start, count);
				break;
			}
		}
	}

	*buff = 0;

	return buff;
}

static char *cavan_vfat_copy_name(char *dest, char *head, const u16 *src, size_t count)
{
	const u16 *last;

	for (last = src + count - 1; last >= src && dest >= head; dest--, last--)
	{
#if 1
		u16 value = *last;

		if (value & 0xFF00)
		{
			*dest = value >> 8;
			if (--dest < head)
			{
				break;
			}
		}

		*dest = value;
#else
		*dest = *last;
#endif
	}

	return dest;
}

static char *cavan_vfat_build_long_name(const struct vfat_dir_entry_long *entry, char *buff, char *head)
{
	buff = cavan_vfat_copy_name(buff, head, entry->name3, NELEM(entry->name3));
	buff = cavan_vfat_copy_name(buff, head, entry->name2, NELEM(entry->name2));
	buff = cavan_vfat_copy_name(buff, head, entry->name1, NELEM(entry->name1));

	return buff + 1;
}

static int cavan_vfat_scan_dir_label_handler_dummy(struct cavan_vfat_scan_dir_walker *walker, const char *label)
{
#if CAVAN_VFAT_DEBUG
	pr_info("volume label = %s", label);
#endif

	return WALKER_ACTION_CONTINUE;
}

static int cavan_vfat_scan_dir_entry_handler_dummy(struct cavan_vfat_scan_dir_walker *walker, const struct vfat_dir_entry *entry, const char *filename)
{
#if CAVAN_VFAT_DEBUG
	pr_info("%s%s", filename, VFAT_IS_DIRECTORY(entry) ? "/" : "");
#endif

	return WALKER_ACTION_CONTINUE;
}

static void cavan_vfat_scan_dir_walker_init(struct cavan_vfat_scan_dir_walker *walker, void *context)
{
	walker->context = context;
	walker->filename = NULL;
	walker->tail = walker->buff + sizeof(walker->buff) - 1;
	walker->tail[0] = 0;

	walker->entry_handler = cavan_vfat_scan_dir_entry_handler_dummy;
	walker->label_handler = cavan_vfat_scan_dir_label_handler_dummy;
}

static int cavan_vfat_scan_dir_entrys(const struct vfat_dir_entry *entry, size_t count, struct cavan_vfat_scan_dir_walker *walker)
{
	const struct vfat_dir_entry *entry_end;

	for (entry_end = entry + count; entry < entry_end; entry++)
	{
		switch (entry->name[0])
		{
		case 0:
			return WALKER_ACTION_EOF;

		case 0xE5:
			break;

		default:
			if (VFAT_IS_LONG_NAME(entry))
			{
				const struct vfat_dir_entry_long *entry_long = (struct vfat_dir_entry_long *) entry;

				if (entry_long->order & VFAT_LAST_LONG_ENTRY)
				{
					walker->filename = walker->tail;
				}
				else if (walker->filename == NULL)
				{
					return -EFAULT;
				}

				walker->filename = cavan_vfat_build_long_name(entry_long, walker->filename - 1, walker->buff);
			}
			else if (VFAT_IS_VOLUME_LABEL(entry))
			{
				int ret;

				cavan_vfat_build_volume_label(entry->name, walker->buff, sizeof(walker->buff) - 1);

				ret = walker->label_handler(walker, walker->buff);
				if (ret != WALKER_ACTION_CONTINUE)
				{
					return ret;
				}
			}
			else
			{
				int ret;
				char *filename;

				if (walker->filename)
				{
					filename = walker->filename;
					walker->filename = NULL;
				}
				else
				{
					cavan_vfat_build_short_name(entry->name, walker->buff, sizeof(walker->buff) - 1);
					filename = walker->buff;
					if (filename[0] == 0x05)
					{
						filename[0] = 0xE5;
					}
				}

				ret = walker->entry_handler(walker, entry, filename);
				if (ret != WALKER_ACTION_CONTINUE)
				{
					return ret;
				}
			}
		}
	}

	return WALKER_ACTION_CONTINUE;
}

static int cavan_vfat_scan_dir(struct cavan_vfat_fs *fs, u32 cluster, struct cavan_vfat_scan_dir_walker *walker)
{
	int ret;
	ssize_t rdlen;

#if CAVAN_VFAT_DEBUG
	println("cluster = 0x%08x", cluster);
#endif

	if (cluster < 2)
	{
		u32 index;
		u32 remain;
		char buff[fs->bytes_per_sector];

		if (fs->type == FAT32)
		{
			cluster = fs->root_first_cluster;
			goto label_scan_general;
		}

		remain = fs->dbr.root_entry_count;
		index = fs->root_first_sector;

		while (remain)
		{
			u32 count;

			rdlen = cavan_vfat_read_sector(fs, index, buff, 1);
			if (rdlen < 0)
			{
				pr_red_info("cavan_vfat_read_sector");
				return rdlen;
			}

			count = remain > fs->entrys_per_sector ? fs->entrys_per_sector : remain;
			ret = cavan_vfat_scan_dir_entrys((struct vfat_dir_entry *) buff, count, walker);
			if (ret != WALKER_ACTION_CONTINUE)
			{
				return ret;
			}

			remain -= count;
			index++;
		}
	}
	else
	{
label_scan_general:
		while (cluster < fs->eof_flag)
		{
			char buff[fs->bytes_per_cluster];

			rdlen = cavan_vfat_read_cluster(fs, cluster, buff, 1);
			if (rdlen < 0)
			{
				pr_red_info("cavan_vfat_read_cluster");
				return rdlen;
			}

			ret = cavan_vfat_scan_dir_entrys((struct vfat_dir_entry *) buff, fs->entrys_per_cluster, walker);
			if (ret != WALKER_ACTION_CONTINUE)
			{
				return ret;
			}

			cluster = cavan_vfat_get_next_cluster(fs, cluster);
		}
	}

	return WALKER_ACTION_EOF;
}

static int cavan_vfat_find_file_handler(struct cavan_vfat_scan_dir_walker *walker, const struct vfat_dir_entry *entry, const char *filename)
{
	struct cavan_vfat_find_file_context *context = walker->context;

	if (text_lhcmp(filename, context->filename) || text_len(filename) != context->namelen)
	{
		return WALKER_ACTION_CONTINUE;
	}

	mem_copy(context->entry, entry, sizeof(*entry));

	return WALKER_ACTION_COMPLETE;
}

static int cavan_vfat_find_file(struct cavan_vfat_fs *fs, struct cavan_vfat_file *file, const char *pathname)
{
	struct cavan_vfat_scan_dir_walker walker;
	struct vfat_dir_entry *entry = &file->entry;
	struct cavan_vfat_find_file_context context;

	context.entry = entry;
	entry->attribute |= VFAT_ATTR_DIRECTORY;
	entry->first_cluster_hi = fs->root_first_cluster >> 16;
	entry->first_cluster_lo = fs->root_first_cluster & 0xFFFF;

	cavan_vfat_scan_dir_walker_init(&walker, &context);
	walker.entry_handler = cavan_vfat_find_file_handler;

	while (1)
	{
		int ret;
		u32 cluster;
		const char *p;

		while (*pathname == CAVAN_VFAT_PATH_SEP)
		{
			pathname++;
		}

		if (*pathname == 0)
		{
			break;
		}

		if (VFAT_IS_DIRECTORY(entry) == false)
		{
			return -EFAULT;
		}

		for (p = pathname; *p && *p != CAVAN_VFAT_PATH_SEP; p++);

		context.filename = pathname;
		context.namelen = p - pathname;
		cluster = VFAT_BUILD_START_CLUSTER(entry);
		ret = cavan_vfat_scan_dir(fs, cluster, &walker);
		if (ret != WALKER_ACTION_COMPLETE)
		{
			if (cluster > fs->root_first_cluster || context.namelen > 2 || pathname[0] != '.' || (context.namelen > 1 && pathname[1] != '.'))
			{
				return ret < 0 ? ret : -EFAULT;
			}
		}

		pathname = p;
	}

	return 0;
}

struct cavan_vfat_file *cavan_vfat_open_file(struct cavan_vfat_fs *fs, const char *pathname)
{
	int ret;
	struct cavan_vfat_file *file;

	file = malloc(sizeof(*file));
	if (file == NULL)
	{
		pr_error_info("malloc");
		return NULL;
	}

	file->fs = fs;
	file->pathname = pathname;

	ret = cavan_vfat_find_file(fs, file, pathname);
	if (ret < 0)
	{
		pr_red_info("cavan_vfat_find_file");
		goto out_free_file;
	}

	return file;

out_free_file:
	free(file);
	return NULL;
}

void cavan_vfat_close_file(struct cavan_vfat_file *fp)
{
	free(fp);
}

static int cavan_vfat_list_dir_handler(struct cavan_vfat_scan_dir_walker *walker, const struct vfat_dir_entry *entry, const char *filename)
{
	struct cavan_vfat_list_dir_context *context = walker->context;

	context->handler(entry, filename, context->private_data);

	return WALKER_ACTION_CONTINUE;
}

int cavan_vfat_list_dir(struct cavan_vfat_file *fp, void (*handler)(const struct vfat_dir_entry *entry, const char *filename, void *data), void *data)
{
	struct cavan_vfat_scan_dir_walker walker;
	struct cavan_vfat_list_dir_context context =
	{
		.private_data = data,
		.handler = handler
	};

	if (VFAT_IS_DIRECTORY(&fp->entry) == 0)
	{
		return -ENOTDIR;
	}

	cavan_vfat_scan_dir_walker_init(&walker, &context);
	walker.entry_handler = cavan_vfat_list_dir_handler;

	return cavan_vfat_scan_dir(fp->fs, VFAT_BUILD_START_CLUSTER(&fp->entry), &walker);
}

static ssize_t cavan_vfat_read_file_base(struct cavan_vfat_file *fp, size_t skip, size_t size, int (*handler)(const char *buff, size_t size, void *date), void *data)
{
	bool complete;
	size_t remain;
	struct cavan_vfat_fs *fs = fp->fs;
	size_t file_remain = fp->entry.file_size;
	u32 cluster = VFAT_BUILD_START_CLUSTER(&fp->entry);

	if (size == 0)
	{
		size = file_remain;
	}

	remain = size;
	complete = false;

	while (complete == false && cluster < fs->eof_flag)
	{
		int ret;
		char *p;
		ssize_t rdlen;
		char buff[fs->bytes_per_cluster];

		rdlen = cavan_vfat_read_cluster(fs, cluster, buff, 1);
		if (rdlen < 0)
		{
			pr_red_info("cavan_vfat_read_cluster");
			return rdlen;
		}

		if (file_remain > sizeof(buff))
		{
			rdlen = sizeof(buff);
			file_remain -= rdlen;
		}
		else
		{
			rdlen = file_remain;
			complete = true;
		}

		p = buff;

		if (skip)
		{
			if ((size_t) rdlen <= skip)
			{
				skip -= rdlen;
				continue;
			}

			rdlen -= skip;
			p += skip;
			skip = 0;
		}

		if ((size_t) rdlen > remain)
		{
			rdlen = remain;
			complete = true;
		}
		else
		{
			remain -= rdlen;
		}

		ret = handler(p, rdlen, data);
		if (ret < 0)
		{
			pr_red_info("handler");
			return ret;
		}

		cluster = cavan_vfat_get_next_cluster(fs, cluster);
	}

	return size - remain;
}

static int cavan_vfat_read_file_to_buff_handler(const char *buff, size_t size, void *data)
{
	struct cavan_vfat_read_file_context *context = data;

	mem_copy(ADDR_ADD(context->buff, context->size), buff, size);
	context->size += size;

	return 0;
}

ssize_t cavan_vfat_read_file(struct cavan_vfat_file *fp, size_t skip, char *buff, size_t size)
{
	struct cavan_vfat_read_file_context context =
	{
		.buff = buff,
		.size = 0
	};

	return cavan_vfat_read_file_base(fp, skip, size, cavan_vfat_read_file_to_buff_handler, &context);
}

static int cavan_vfat_read_file_to_file_handler(const char *buff, size_t size, void *data)
{
	return ffile_write(*(int *) data, buff, size);
}

ssize_t cavan_vfat_read_file2(struct cavan_vfat_file *fp, size_t skip, int fd)
{
	return cavan_vfat_read_file_base(fp, skip, 0, cavan_vfat_read_file_to_file_handler, &fd);
}

ssize_t cavan_vfat_read_file3(struct cavan_vfat_file *fp, size_t skip, const char *pathname, int flags)
{
	int fd;
	ssize_t rdlen;

	fd = open(pathname, O_WRONLY | O_CREAT | flags, 0777);
	if (fd < 0)
	{
		pr_error_info("open file %s", pathname);
		return fd;
	}

	rdlen = cavan_vfat_read_file2(fp, skip, fd);
	close(fd);

	return rdlen;

	return 0;
}

static int cavan_vfat_read_volume_label_handler(struct cavan_vfat_scan_dir_walker *walker, const char *label)
{
	struct cavan_vfat_read_file_context *context = walker->context;

	context->size = ADDR_SUB2(text_ncopy(context->buff, label, context->size), context->buff);

	return WALKER_ACTION_COMPLETE;
}

ssize_t cavan_vfat_read_volume_label(struct cavan_vfat_fs *fs, char *buff, size_t size)
{
	int ret;
	const u8 *name;
	struct cavan_vfat_scan_dir_walker walker;
	struct cavan_vfat_read_file_context context =
	{
		.buff = buff,
		.size = size
	};

	cavan_vfat_scan_dir_walker_init(&walker, &context);
	walker.label_handler = cavan_vfat_read_volume_label_handler;

	ret = cavan_vfat_scan_dir(fs, fs->root_first_cluster, &walker);
	if (ret == WALKER_ACTION_COMPLETE)
	{
		return context.size;
	}

	if (fs->type == FAT32)
	{
		name = fs->dbr.dbr32.dbr16.volume_label;
	}
	else
	{
		name = fs->dbr.dbr16.volume_label;
	}

	return cavan_vfat_build_volume_label(name, buff, size) - buff;
}
