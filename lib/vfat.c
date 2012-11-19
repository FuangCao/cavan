// Fuang.Cao <cavan.cfa@gmail.com> Sat May  7 00:23:47 CST 2011

#include <cavan.h>
#include <cavan/vfat.h>
#include <cavan/memory.h>
#include <cavan/text.h>

ssize_t ffat_read_dbr(int fd, struct fat_dbr *dbr)
{
	ssize_t readlen;

	readlen = ffile_read(fd, dbr, sizeof(*dbr));
	if (readlen < 0)
	{
		error_msg("ffile_read");
		return readlen;
	}

	if (dbr->boot_flags != 0xAA55)
	{
		error_msg("DBR = 0x%04x is invalid", dbr->boot_flags);
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

void get_volume_label_base(char *dest, const void *src)
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

u32 cal_fat_table_offset(struct fat_info *info_p, u32 cluster_index)
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
		error_msg("ffat_read_dbr");
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

void fat_uninit(struct fat_info *info_p)
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
	const char *src_end = src + size;

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
			error_msg("fat_read_clusters");
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
				error_msg("fat_read_clusters");
				return readlen;
			}
		}
		else
		{
			char tmp_buff[info_p->bytes_per_cluster];

			readlen = fat_read_clusters(info_p, start_cluster, 1, tmp_buff);
			if (readlen < 0)
			{
				error_msg("fat_read_clusters");
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
		error_msg("fat_read_sectors");
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
		error_msg("fat_read_sectors");
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
		error_msg("load_root_directory");
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
			error_msg("file \"%s\" not find", localpath);
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
			error_msg("load_directory");
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
			error_msg("fat_read_clusters");
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
			error_msg("find_path2_simple");
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
		error_msg("fat_read_sectors");
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
			error_msg("fat_read_clusters");
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
		error_msg("file not find");
		ERROR_RETURN(ENOENT);
	}

	if (IS_NOT_FILE(&file_desc))
	{
		error_msg("this is't a file");
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
			error_msg("directory not find");
			ERROR_RETURN(ENOENT);
		}

		if (!IS_DIRECTORY(&dir_desc))
		{
			error_msg("this is't a directory");
			ERROR_RETURN(EISDIR);
		}

		start_cluster = BUILD_START_CLUSTER(&dir_desc);
	}

	return print_directory(info_p, start_cluster);
}

