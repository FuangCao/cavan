// Fuang.Cao <cavan.cfa@gmail.com> Sat May  7 00:23:47 CST 2011

#include <cavan.h>
#include <cavan/vfat.h>
#include <cavan/memory.h>
#include <cavan/text.h>
#include <cavan/math.h>

#if 0
ssize_t ffat_read_dbr(int fd, struct fat_dbr *dbr)
{
	ssize_t readlen;

	readlen = ffile_read(fd, dbr, sizeof(*dbr));
	if (readlen < 0)
	{
		pr_error_info("ffile_read");
		return readlen;
	}

	if (dbr->boot_flags != 0xAA55)
	{
		pr_error_info("DBR = 0x%04x is invalid", dbr->boot_flags);
		ERROR_RETURN(EINVAL);
	}

	return readlen;
}

const char *fat_type_to_string(enum fat_type type)
{
	switch (type)
	{
	case FAT12:
		return "FAT12";

	case FAT16:
		return "FAT16";

	case FAT32:
		return "FAT32";
	}

	return "unknown";
}

static void get_volume_label_base(char *dest, const void *src)
{
	dest = mem_copy(dest, src, 11);

	*dest = 0;
	dest--;

	while (*dest == ' ')
	{
		*dest-- = 0;
	}
}

void get_fat_volume_label(struct fat_info *info_p, char *volume)
{
	struct fat_dbr *dbr = &info_p->dbr;

	if (info_p->type == FAT32)
	{
		get_volume_label_base(volume, dbr->tail32.volume_label);
	}
	else
	{
		get_volume_label_base(volume, dbr->tail16.volume_label);
	}
}

static u32 cal_fat_table_offset(struct fat_info *info_p, u32 cluster_index)
{
	switch (info_p->type)
	{
	case FAT12:
		return cluster_index + (cluster_index >> 1);

	case FAT16:
		return cluster_index << 1;

	default:
		return cluster_index << 2;
	}

	return 0;
}

u32 get_fat_table_entry(struct fat_info *info_p, u32 cluster_index)
{
	u32 fat_entry;
	void *start_addr = (char *)info_p->fat_table + cal_fat_table_offset(info_p, cluster_index);

	if (info_p->type == FAT12)
	{
		fat_entry = *(u16 *)start_addr;
		if (cluster_index & 1)
		{
			fat_entry >>= 4;
		}
		else
		{
			fat_entry &= 0x0FFF;
		}
	}
	else
	{
		if (info_p->type == FAT32)
		{
			fat_entry = (*(u32 *)start_addr) & 0x0FFFFFFF;
		}
		else
		{
			fat_entry = *(u16 *)start_addr;
		}
	}

	return fat_entry;
}

int set_fat_table_entry(struct fat_info *info_p, u32 cluster_index, u32 fat_entry)
{
	int write_count;
	u32 fat_offset, sector_index;
	void *start_addr;
	struct fat_dbr *dbr = &info_p->dbr;

	fat_offset = cal_fat_table_offset(info_p, cluster_index);
	start_addr = (char *)info_p->fat_table + fat_offset;
	sector_index = fat_offset / dbr->bytes_per_sector;

	if (info_p->type == FAT12)
	{
		u16 tmp;

		tmp = *(u16 *)start_addr;
		if (cluster_index & 1)
		{
			tmp &= 0x000F;
			fat_entry <<= 4;
		}
		else
		{
			tmp &= 0xF000;
		}

		tmp |= fat_entry;
		*(u16 *)start_addr = tmp;

		if ((fat_offset % dbr->bytes_per_sector) + 2 > dbr->bytes_per_sector)
		{
			write_count = 2;
		}
		else
		{
			write_count = 1;
		}
	}
	else
	{
		if (info_p->type == FAT32)
		{
			*(u32 *)start_addr &= 0xF0000000;
			*(u32 *)start_addr |= fat_entry & 0x0FFFFFFF;
			write_count = 1;
		}
		else
		{
			*(u16 *)start_addr = fat_entry;
			write_count = 1;
		}
	}

	return fat_write_sectors(info_p, dbr->reserve_sector_count + sector_index, write_count, (char *)info_p->fat_table + sector_index * dbr->bytes_per_sector);
}

int entry_is_eof(struct fat_info *info_p, u32 entry_cotent)
{
	switch (info_p->type)
	{
	case FAT12:
		return entry_cotent >= 0x0FF8;

	case FAT16:
		return entry_cotent >= 0xFFF8;

	case FAT32:
		return entry_cotent >= 0x0FFFFFF8;
	}

	return 0;
}

u64 get_first_sector_of_cluster(struct fat_info *info_p, u32 cluster_index)
{
	if (cluster_index < 2)
	{
		return info_p->data_first_sector - (2 - cluster_index) * info_p->dbr.sectors_per_cluster;
	}
	else
	{
		return (cluster_index - 2) * info_p->dbr.sectors_per_cluster + info_p->data_first_sector;
	}
}

int ffat_init(int fd, struct fat_info *info_p)
{
	int ret;
	ssize_t readlen;
	struct fat_dbr *dbr = &info_p->dbr;

	info_p->fat_fd = fd;

	readlen = ffat_read_dbr(fd, dbr);
	if (readlen < 0)
	{
		pr_error_info("ffat_read_dbr");
		return readlen;
	}

	info_p->bytes_per_cluster = dbr->bytes_per_sector * dbr->sectors_per_cluster;
	info_p->descs_per_cluster = info_p->bytes_per_cluster / sizeof(struct fat_directory);

	if (dbr->fat_size16)
	{
		info_p->fat_size = dbr->fat_size16;
	}
	else
	{
		info_p->fat_size = dbr->tail32.fat_size32;
	}

	info_p->fat_table = malloc(info_p->fat_size * dbr->bytes_per_sector);
	if (info_p->fat_table == NULL)
	{
		print_error("malloc");
		ERROR_RETURN(ENOMEM);
	}

	readlen = read_fat_table(info_p);
	if (readlen < 0)
	{
		ret = readlen;
		print_error("read_fat_table");
		goto out_free_fat_table;
	}

	if (dbr->total_sector16)
	{
		info_p->total_sector_count = dbr->total_sector16;
	}
	else
	{
		info_p->total_sector_count = dbr->total_sectors32;
	}

	info_p->root_sector_count = (dbr->root_entry_count * sizeof(struct fat_directory) + dbr->bytes_per_sector - 1) / dbr->bytes_per_sector;
	info_p->data_first_sector = dbr->reserve_sector_count + dbr->fat_table_count * info_p->fat_size + info_p->root_sector_count;
	info_p->data_sector_count = info_p->total_sector_count - info_p->data_first_sector;
	info_p->data_cluster_count = info_p->data_sector_count / dbr->sectors_per_cluster;

	if (info_p->data_cluster_count < 4085)
	{
		info_p->type = FAT12;
	}
	else if (info_p->data_cluster_count < 65525)
	{
		info_p->type = FAT16;
	}
	else
	{
		info_p->type = FAT32;
	}

	if (info_p->type == FAT32)
	{
		info_p->root_first_cluster = dbr->tail32.root_first_cluster;
		info_p->root_first_sector = get_first_sector_of_cluster(info_p, info_p->root_first_cluster);
	}
	else
	{
		info_p->root_first_cluster = 0;
		info_p->root_first_sector = dbr->reserve_sector_count + dbr->fat_table_count * info_p->fat_size;
	}

	return 0;

out_free_fat_table:
	free(info_p->fat_table);

	return ret;
}

int fat_init(const char *pathname, struct fat_info *info_p)
{
	int fd;
	int ret;;

	fd = file_open_ro(pathname);
	if (fd < 0)
	{
		print_error("open device \"%s\"", pathname);
		return fd;
	}

	ret = ffat_init(fd, info_p);
	if (ret < 0)
	{
		close(fd);
		return ret;
	}

	return 0;
}

void fat_deinit(struct fat_info *info_p)
{
	close(info_p->fat_fd);

	if (info_p->fat_table)
	{
		free(info_p->fat_table);
	}
}

static void build_short_name(const char *name, char *buff)
{
	const char *name_last, *stufix_start, *stufix_end;

	stufix_end = name + 11;
	stufix_start = stufix_end - 4;
	name_last = stufix_end - 5;

	while (name_last >= name && *name_last == ' ')
	{
		name_last--;
	}

	while (name <= name_last)
	{
		*buff++ = *name++;
	}

	while (stufix_start < stufix_end && *stufix_start == ' ')
	{
		stufix_start++;
	}

	if (stufix_start < stufix_end)
	{
		*buff++ = '.';

		while (stufix_start < stufix_end)
		{
			*buff++ = *stufix_start++;
		}
	}

	*buff = 0;
}

static char *copy_name(char *dest, const char *src, size_t size)
{
	const char *src_end;

	for (src_end = src + size; src < src_end; src += 2)
	{
		*dest++ = src[0];
		if (src[1])
		{
			*dest++ = src[1];
		}
	}

	return dest;
}

static char *build_long_name_base(struct fat_long_directory *dir_p, char *buff)
{
	buff = copy_name(buff, (char *)dir_p->name1, 10);
	buff = copy_name(buff, (char *)dir_p->name2, 12);

	return copy_name(buff, (char *)dir_p->name3, 4);
}

static struct fat_long_directory *build_long_name(struct fat_long_directory *dir_p, size_t count, char *buff)
{
	struct fat_long_directory *p;

	for (p = dir_p + count - 1; p >= dir_p; p--)
	{
		buff = build_long_name_base(p, buff);
	}

	*buff = 0;

	return dir_p + count;
}

ssize_t load_directory(struct fat_info *info_p, u32 cluster_index, struct fat_soft_direcory *soft_dirs, size_t size)
{
	ssize_t readlen;
	size_t count;
	char buff[1024];
	struct fat_directory dirs[info_p->descs_per_cluster], *p, *end_p;

	count = 0;
	buff[0] = 0;

	while (1)
	{
		readlen = fat_read_clusters(info_p, cluster_index, 1, dirs);
		if (readlen < 0)
		{
			pr_error_info("fat_read_clusters");
			return readlen;
		}

		for (p = dirs, end_p = p + ARRAY_SIZE(dirs); p < end_p && p->name[0]; p++)
		{
			if (p->name[0] == 0xE5)
			{
				continue;
			}

			if (IS_LONG_NAME(p))
			{
				size_t long_count = ((struct fat_long_directory *)p)->order & (~LAST_LONG_ENTRY);

				if (p + long_count < end_p)
				{
					p = (struct fat_directory *)build_long_name((struct fat_long_directory *)p, long_count, soft_dirs[count].name);
					if (buff[0])
					{
						text_cat(soft_dirs[count].name, buff);
						buff[0] = 0;
					}
				}
				else
				{
					build_long_name((struct fat_long_directory *)p, end_p - p, buff);
					goto label_get_next_cluster;
				}
			}
			else if (p->attribute & ATTR_VOLUME_ID)
			{
				get_volume_label_base(soft_dirs[count].name, p->name);
			}
			else
			{
				build_short_name((char *)p->name, soft_dirs[count].name);
			}

			soft_dirs[count++].short_dir = *p;

			if (count >= size)
			{
				return size;
			}
		}

		if (p < end_p)
		{
			break;
		}

label_get_next_cluster:
		cluster_index = get_next_cluster_index(info_p, cluster_index);

		if (entry_is_eof(info_p, cluster_index))
		{
			break;
		}
	}

	return count;
}

ssize_t load_data(struct fat_info *info_p, u32 start_cluster, void *buff, size_t size)
{
	ssize_t totallen = 0;

	while (size)
	{
		ssize_t readlen;

		if (size >= info_p->bytes_per_cluster)
		{
			readlen = fat_read_clusters(info_p, start_cluster, 1, buff);
			if (readlen < 0)
			{
				pr_error_info("fat_read_clusters");
				return readlen;
			}
		}
		else
		{
			char tmp_buff[info_p->bytes_per_cluster];

			readlen = fat_read_clusters(info_p, start_cluster, 1, tmp_buff);
			if (readlen < 0)
			{
				pr_error_info("fat_read_clusters");
				return readlen;
			}

			mem_copy(buff, tmp_buff, size);
			readlen = size;
		}

		totallen += readlen;

		start_cluster = get_next_cluster_index(info_p, start_cluster);
		if (entry_is_eof(info_p, start_cluster))
		{
			break;
		}

		buff = (char *)buff + readlen;
		size -= readlen;
	}

	return totallen;
}

int fat16_root_find(struct fat_info *info_p, const char *filename, struct fat_directory *file_desc)
{
	ssize_t readlen;
	struct fat_directory dir_descs[info_p->dbr.root_entry_count], *desc_curr, *desc_end;
	char name_temp[1024];

	readlen = fat_read_sectors(info_p, info_p->root_first_sector, info_p->root_sector_count, dir_descs);
	if (readlen < 0)
	{
		pr_error_info("fat_read_sectors");
		return readlen;
	}

	for (desc_curr = dir_descs, desc_end = desc_curr + ARRAY_SIZE(dir_descs); desc_curr < desc_end && desc_curr->name[0]; desc_curr++)
	{
		if (desc_curr->name[0] == 0xE5)
		{
			continue;
		}

		if (IS_LONG_NAME(desc_curr))
		{
			size_t long_count = ((struct fat_long_directory *)desc_curr)->order & (~LAST_LONG_ENTRY);

			desc_curr = (struct fat_directory *)build_long_name((struct fat_long_directory *)desc_curr, long_count, name_temp);
		}
		else if (IS_VOLUME_LABEL(desc_curr))
		{
			get_volume_label_base(name_temp, desc_curr->name);
		}
		else
		{
			build_short_name((char *)desc_curr->name, name_temp);
		}

		if (text_cmp(filename, name_temp) == 0)
		{
			*file_desc = *desc_curr;
			return 0;
		}
	}

	ERROR_RETURN(ENOENT);
}

ssize_t fat16_load_root_directory(struct fat_info *info_p, struct fat_soft_direcory *soft_dirs, size_t size)
{
	ssize_t readlen;
	size_t count;
	struct fat_directory dir_descs[info_p->dbr.root_entry_count], *desc_curr, *desc_end;

	readlen = fat_read_sectors(info_p, info_p->root_first_sector, info_p->root_sector_count, dir_descs);
	if (readlen < 0)
	{
		pr_error_info("fat_read_sectors");
		return readlen;
	}

	count = 0;

	for (desc_curr = dir_descs, desc_end = desc_curr + ARRAY_SIZE(dir_descs); desc_curr < desc_end && desc_curr->name[0]; desc_curr++)
	{
		if (desc_curr->name[0] == 0xE5)
		{
			continue;
		}

		if (IS_LONG_NAME(desc_curr))
		{
			size_t long_count = ((struct fat_long_directory *)desc_curr)->order & (~LAST_LONG_ENTRY);

			desc_curr = (struct fat_directory *)build_long_name((struct fat_long_directory *)desc_curr, long_count, soft_dirs[count].name);
		}
		else if (IS_VOLUME_LABEL(desc_curr))
		{
			get_volume_label_base(soft_dirs[count].name, desc_curr->name);
		}
		else
		{
			build_short_name((char *)desc_curr->name, soft_dirs[count].name);
		}

		soft_dirs[count++].short_dir = *desc_curr;

		if (count >= size)
		{
			break;
		}
	}

	return count;
}

ssize_t load_root_directory(struct fat_info *info_p, struct fat_soft_direcory *soft_dirs, size_t size)
{
	if (info_p->type == FAT32)
	{
		return load_directory(info_p, info_p->root_first_cluster, soft_dirs, size);
	}

	return fat16_load_root_directory(info_p, soft_dirs, size);
}

int find_path(struct fat_info *info_p, const char *pathname, struct fat_directory *file_desc)
{
	char localpath[1024], *path_next, *path_curr, *path_end;
	struct fat_soft_direcory soft_dirs[1024], *dir_curr, *dir_end;
	ssize_t dir_count;

	dir_count = load_root_directory(info_p, soft_dirs, ARRAY_SIZE(soft_dirs));
	if (dir_count <= 0)
	{
		pr_error_info("load_root_directory");
		return dir_count;
	}

	path_end = text_copy(localpath, pathname);
	path_next = localpath;

	while (*path_next == '/')
	{
		path_next++;
	}

	while (1)
	{
		struct fat_directory *tmp_dir;

		path_curr = path_next;

		while (path_next < path_end && *path_next != '/')
		{
			path_next++;
		}

		*path_next = 0;

		for (dir_curr = soft_dirs, dir_end = soft_dirs + dir_count; \
			dir_curr < dir_end && text_cmp(path_curr, dir_curr->name); dir_curr++);

		if (dir_curr >= dir_end)
		{
			pr_error_info("file \"%s\" not find", localpath);
			ERROR_RETURN(ENOENT);
		}

		for (path_next++; *path_next == '/'; path_next++);

		if (path_next >= path_end)
		{
			break;
		}

		tmp_dir = &dir_curr->short_dir;
		if (!IS_DIRECTORY(tmp_dir))
		{
			ERROR_RETURN(ENOENT);
		}

		dir_count = load_directory(info_p, BUILD_START_CLUSTER(tmp_dir), soft_dirs, ARRAY_SIZE(soft_dirs));
		if (dir_count <= 0)
		{
			pr_error_info("load_directory");
			ERROR_RETURN(ENOENT);
		}
	}

	*file_desc = dir_curr->short_dir;

	return 0;
}

int find_path2_simple(struct fat_info *info_p, u32 cluster_index, const char *filename, struct fat_directory *file_desc)
{
	char name_temp[1024];

	if (cluster_index == 0)
	{
		return fat16_root_find(info_p, filename, file_desc);
	}

	name_temp[0] = 0;

	while (1)
	{
		ssize_t readlen;
		struct fat_directory dir_descs[info_p->descs_per_cluster], *desc_curr, *desc_end;
		char name_curr[1024];

		readlen = fat_read_clusters(info_p, cluster_index, 1, dir_descs);
		if (readlen < 0)
		{
			pr_error_info("fat_read_clusters");
			return readlen;
		}

		for (desc_curr = dir_descs, desc_end = dir_descs + ARRAY_SIZE(dir_descs); desc_curr < desc_end && desc_curr->name[0]; desc_curr++)
		{
			if (desc_curr->name[0] == 0xE5)
			{
				continue;
			}

			if (IS_LONG_NAME(desc_curr))
			{
				size_t long_count = ((struct fat_long_directory *)desc_curr)->order & (~LAST_LONG_ENTRY);

				if (desc_curr + long_count < desc_end)
				{
					desc_curr = (struct fat_directory *)build_long_name((struct fat_long_directory *)desc_curr, long_count, name_curr);
					if (name_temp[0])
					{
						text_cat(name_curr, name_temp);
						name_temp[0] = 0;
					}
				}
				else
				{
					build_long_name((struct fat_long_directory *)desc_curr, desc_end - desc_curr, name_temp);
					desc_curr = desc_end;
					break;
				}
			}
			else if (IS_VOLUME_LABEL(desc_curr))
			{
				get_volume_label_base(name_curr, desc_curr->name);
			}
			else
			{
				build_short_name((char *)desc_curr->name, name_curr);
			}

			if (text_cmp(filename, name_curr) == 0)
			{
				*file_desc = *desc_curr;
				return 0;
			}
		}

		if (desc_curr < desc_end)
		{
			break;
		}

		cluster_index = get_next_cluster_index(info_p, cluster_index);

		if (entry_is_eof(info_p, cluster_index))
		{
			break;
		}
	}

	ERROR_RETURN(ENOENT);
}

int find_path2(struct fat_info *info_p, const char *pathname, struct fat_directory *file_desc)
{
	u32 cluster_index;
	char localpath[1024], *path_next, *path_curr, *path_end;

	cluster_index = info_p->root_first_cluster;
	path_end = text_copy(localpath, pathname);
	path_next = localpath;

	while (*path_next == '/')
	{
		path_next++;
	}

	while (1)
	{
		int ret;

		for (path_curr = path_next; path_next < path_end && *path_next != '/'; path_next++);

		*path_next = 0;

		ret = find_path2_simple(info_p, cluster_index, path_curr, file_desc);
		if (ret < 0)
		{
			pr_error_info("find_path2_simple");
			return ret;
		}

		for (path_next++; path_next < path_end && *path_next == '/'; path_next++);

		if (path_next >= path_end)
		{
			return 0;
		}

		cluster_index = BUILD_START_CLUSTER(file_desc);
	}

	ERROR_RETURN(ENOENT);
}

const char *attribute_to_string(u8 attribute)
{
	if (attribute & ATTR_DIRECTORY)
	{
		return "directory";
	}
	else if (attribute & ATTR_VOLUME_ID)
	{
		return "volume";
	}
	else
	{
		return "file";
	}
}

int fat16_print_root(struct fat_info *info_p)
{
	ssize_t readlen;
	struct fat_directory dir_descs[info_p->dbr.root_entry_count], *desc_curr, *desc_end;
	char name_temp[1024];

	readlen = fat_read_sectors(info_p, info_p->root_first_sector, info_p->root_sector_count, dir_descs);
	if (readlen < 0)
	{
		pr_error_info("fat_read_sectors");
		return readlen;
	}

	for (desc_curr = dir_descs, desc_end = desc_curr + ARRAY_SIZE(dir_descs); desc_curr < desc_end && desc_curr->name[0]; desc_curr++)
	{
		if (desc_curr->name[0] == 0xE5)
		{
			continue;
		}

		if (IS_LONG_NAME(desc_curr))
		{
			size_t long_count = ((struct fat_long_directory *)desc_curr)->order & (~LAST_LONG_ENTRY);

			desc_curr = (struct fat_directory *)build_long_name((struct fat_long_directory *)desc_curr, long_count, name_temp);
		}
		else if (IS_VOLUME_LABEL(desc_curr))
		{
			get_volume_label_base(name_temp, desc_curr->name);
		}
		else
		{
			build_short_name((char *)desc_curr->name, name_temp);
		}

#if __WORDSIZE == 64
		println("%s[%ld]: %s", attribute_to_string(desc_curr->attribute), desc_curr - dir_descs, name_temp);
#else
		println("%s[%d]: %s", attribute_to_string(desc_curr->attribute), desc_curr - dir_descs, name_temp);
#endif
	}

	ERROR_RETURN(ENOENT);
}


int print_directory(struct fat_info *info_p, u32 cluster_index)
{
	char name_temp[1024] = "";

	if (cluster_index == 0)
	{
		return fat16_print_root(info_p);
	}

	while (1)
	{
		ssize_t readlen;
		struct fat_directory dir_descs[info_p->descs_per_cluster], *desc_curr, *desc_end;
		char name_curr[1024];

		readlen = fat_read_clusters(info_p, cluster_index, 1, dir_descs);
		if (readlen < 0)
		{
			pr_error_info("fat_read_clusters");
			return readlen;
		}

		for (desc_curr = dir_descs, desc_end = dir_descs + ARRAY_SIZE(dir_descs); desc_curr < desc_end && desc_curr->name[0]; desc_curr++)
		{
			if (desc_curr->name[0] == 0xE5)
			{
				continue;
			}

			if (IS_LONG_NAME(desc_curr))
			{
				size_t long_count = ((struct fat_long_directory *)desc_curr)->order & (~LAST_LONG_ENTRY);

				if (desc_curr + long_count < desc_end)
				{
					desc_curr = (struct fat_directory *)build_long_name((struct fat_long_directory *)desc_curr, long_count, name_curr);
					if (name_temp[0])
					{
						text_cat(name_curr, name_temp);
						name_temp[0] = 0;
					}
				}
				else
				{
					build_long_name((struct fat_long_directory *)desc_curr, desc_end - desc_curr, name_temp);
					desc_curr = desc_end;
					break;
				}
			}
			else if (IS_VOLUME_LABEL(desc_curr))
			{
				get_volume_label_base(name_curr, desc_curr->name);
			}
			else
			{
				build_short_name((char *)desc_curr->name, name_curr);
			}

#if __WORDSIZE == 64
			println("%s[%ld]: %s", attribute_to_string(desc_curr->attribute), desc_curr - dir_descs, name_curr);
#else
			println("%s[%d]: %s", attribute_to_string(desc_curr->attribute), desc_curr - dir_descs, name_curr);
#endif
		}

		if (desc_curr < desc_end)
		{
			break;
		}

		cluster_index = get_next_cluster_index(info_p, cluster_index);

		if (entry_is_eof(info_p, cluster_index))
		{
			break;
		}
	}

	return 0;
}

ssize_t load_file(struct fat_info *info_p, const char *pathname, void *buff, size_t size)
{
	int ret;
	struct fat_directory file_desc;

	ret = find_path2(info_p, pathname, &file_desc);
	if (ret < 0)
	{
		pr_error_info("file not find");
		ERROR_RETURN(ENOENT);
	}

	if (IS_NOT_FILE(&file_desc))
	{
		pr_error_info("this is't a file");
		ERROR_RETURN(EISDIR);
	}

	println("file size = %s", size2text(file_desc.file_size));

	if (size > file_desc.file_size)
	{
		if (file_desc.file_size == 0)
		{
			return 0;
		}

		size = file_desc.file_size - 1;
	}

	return load_data(info_p, BUILD_START_CLUSTER(&file_desc), buff, size);
}

int list_directory(struct fat_info *info_p, const char *pathname)
{
	u32 start_cluster;

	while (*pathname == '/')
	{
		pathname++;
	}

	if (pathname == NULL || pathname[0] == 0)
	{
		start_cluster = info_p->root_first_cluster;
	}
	else
	{
		int ret;
		struct fat_directory dir_desc;

		ret = find_path2(info_p, pathname, &dir_desc);
		if (ret < 0)
		{
			pr_error_info("directory not find");
			ERROR_RETURN(ENOENT);
		}

		if (!IS_DIRECTORY(&dir_desc))
		{
			pr_error_info("this is't a directory");
			ERROR_RETURN(EISDIR);
		}

		start_cluster = BUILD_START_CLUSTER(&dir_desc);
	}

	return print_directory(info_p, start_cluster);
}
#endif
#define CAVAN_VFAT_DEBUG	1

static u32 cavan_vfat_get_cluster_first_sector(struct cavan_vfat_fs *fs, u32 index)
{
	return ((index - 2) << fs->sectors_per_cluster_shift) + fs->data_first_sector;
}

static ssize_t cavan_vfat_read_sector(struct cavan_vfat_fs *fs, u32 index, char *buff, u32 count)
{
	pr_info("cavan_vfat_read_sector: index = %d, count = %d", index, count);
	return fs->bdev->read_block(fs->bdev, index >> fs->blocks_per_sector_shift, buff, count << fs->blocks_per_sector_shift);
}

static ssize_t cavan_vfat_read_cluster(struct cavan_vfat_fs *fs, u32 index, char *buff, u32 count)
{
	u32 sector = cavan_vfat_get_cluster_first_sector(fs, index);

	return cavan_vfat_read_sector(fs, sector, buff, count << fs->sectors_per_cluster_shift);
}

static const char *cavan_vfat_type_to_string(fat_type_t type)
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
	pr_info("fat_table_count = %d", dbr->fat_table_count);
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
	pr_info("current_time_teenth = %d", entry->current_time_teenth);
	pr_info("current_time = %d", entry->current_time);
	pr_info("current_date = %d", entry->current_date);
	pr_info("last_access_date = %d", entry->last_access_date);
	pr_info("first_cluster_HW = %d", entry->first_cluster_HW);
	pr_info("write_time = %d", entry->write_time);
	pr_info("write_date = %d", entry->write_date);
	pr_info("first_cluster_LW = %d", entry->first_cluster_LW);
	pr_info("file_size = %d", entry->file_size);
}

void cavan_vfat_dir_entry_long_dump(const struct vfat_dir_entry_long *entry)
{
	print_sep(60);
	pr_bold_info("vfat dir entry long %p", entry);

	pr_info("order = %d", entry->order);
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
	fs->root_dir_sectors = DIV_CEIL(dbr->root_entry_count * sizeof(struct vfat_dir_entry), dbr->bytes_per_sector);

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
		fs->fat_size = dbr->fat_size16;
	}
	else
	{
		fs->fat_size = dbr->dbr32.fat_size32;
	}

	fs->root_first_sector = dbr->reserve_sector_count + (dbr->fat_table_count * fs->fat_size);
	fs->data_first_sector = fs->root_first_sector + fs->root_dir_sectors;

#if CAVAN_VFAT_DEBUG
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

	return 0;
}

void cavan_vfat_deinit(struct cavan_vfat_fs *fs)
{
}

static int cavan_vfat_directory_entry_traversal(const struct vfat_dir_entry *entry, size_t count)
{
	const struct vfat_dir_entry *entry_end;

	for (entry_end = entry + count; entry < entry_end; entry++)
	{
		switch (entry->name[0])
		{
		case 0:
			return -EFAULT;

		case 0xE5:
			break;

		default:
			if (VFAT_IS_LONG_NAME(entry))
			{

			}
			else if (VFAT_IS_VOLUME_LABEL(entry))
			{
				cavan_vfat_dir_entry_long_dump((struct vfat_dir_entry_long *) entry);
			}
			else
			{
				cavan_vfat_dir_entry_dump(entry);
			}
		}
	}

	return 0;
}

static int cavan_vfat_directory_traversal(struct cavan_vfat_fs *fs, u32 cluster)
{
	int ret;
	ssize_t rdlen;

	if (cluster < 2)
	{
		u32 index;
		u32 remain;
		char buff[fs->bytes_per_sector];

		remain = fs->dbr.root_entry_count;
		index = fs->root_first_sector;

		while (1)
		{
			u32 count;

			rdlen = cavan_vfat_read_sector(fs, index, buff, 1);
			if (rdlen < 0)
			{
				pr_red_info("cavan_vfat_read_sector");
				return rdlen;
			}

			count = remain > fs->entrys_per_sector ? fs->entrys_per_sector : remain;
			ret = cavan_vfat_directory_entry_traversal((struct vfat_dir_entry *) buff, count);
			if (ret < 0)
			{
				pr_red_info("cavan_vfat_directory_entry_traversal");
				return ret;
			}

			index++;
			remain -= count;
		}
	}
	else
	{
		char buff[fs->bytes_per_cluster];

		rdlen = cavan_vfat_read_cluster(fs, cluster, buff, 1);
		if (rdlen < 0)
		{
			pr_red_info("cavan_vfat_read_cluster");
			return rdlen;
		}

		ret = cavan_vfat_directory_entry_traversal((struct vfat_dir_entry *)buff, fs->entrys_per_cluster);
		if (ret < 0)
		{
			pr_red_info("cavan_vfat_directory_entry_traversal");
			return ret;
		}
	}

	return 0;
}

static int cavan_vfat_find_file_simple(struct cavan_vfat_fs *fs, u32 cluster, const char *filename, size_t namelen)
{
	return cavan_vfat_directory_traversal(fs, cluster);
}

static int cavan_vfat_find_file(struct cavan_vfat_fs *fs, struct cavan_vfat_file *file, const char *pathname)
{
	u32 cluster = fs->root_first_cluster;

	while (1)
	{
		int ret;
		const char *p;

		while (*pathname == CAVAN_VFAT_PATH_SEP)
		{
			pathname++;
		}

#if CAVAN_VFAT_DEBUG
		println("pathname = %s", pathname);
#endif

		if (*pathname == 0)
		{
			break;
		}

		ret = cavan_vfat_find_file_simple(fs, cluster, pathname, p - pathname);
		if (ret < 0)
		{
			pr_red_info("cavan_vfat_find_file_simple");
			return ret;
		}

		for (p = pathname; *p && *p != CAVAN_VFAT_PATH_SEP; p++);

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

int cavan_vfat_list_dir(struct cavan_vfat_file *fp, void (*handler)(struct vfat_dir_entry *entry, void *data), void *data)
{
	return 0;
}

ssize_t cavan_vfat_read_file(struct cavan_vfat_file *fp, off_t offset, char *buff, size_t size)
{
	return 0;
}

ssize_t cavan_vfat_read_file3(struct cavan_vfat_file *fp, off_t offset, const char *pathname, int flags)
{
	return 0;
}
