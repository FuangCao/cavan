#include <cavan.h>
#include <cavan/mtd.h>
#include <cavan/file.h>

// Fuang.Cao <cavan.cfa@gmail.com> 2011-12-06 18:20:47

void cavan_mtd_show_parts_info(const struct mtd_partition_info *parts, size_t size)
{
	const struct mtd_partition_info *part_end;

	for (part_end = parts + size; parts < part_end; parts++)
	{
		print("name = \"%s\", index = %02d, ", parts->name, parts->index);
		print("erase_size = %s, ", size2text(parts->erase_size));
		println("total_size = %s", size2text(parts->size));
	}
}

int cavan_load_mtd_table(struct cavan_mtd_descriptor *desc, const char *mtd_table)
{
	ssize_t readlen;
	char buff[KB(100)];
	char *p;
	int ret;
	struct mtd_partition_info *info, *info_end;

	if (mtd_table == NULL)
	{
		mtd_table = FILE_PROC_MTD;
	}

	readlen = file_readfrom(mtd_table, buff, sizeof(buff) - 1, 0, 0);
	if (readlen < 0)
	{
		error_msg("read file %s failed", mtd_table);
		return readlen;
	}

	buff[readlen] = 0;
	p = text_find_line(buff, 1);
	info = desc->part_infos;
	info_end = info + NELEM(desc->part_infos);

	while (p && info < info_end)
	{
		ret = sscanf(p, "mtd%d: %x %x \"%63[^\"]", &info->index, &info->size, &info->erase_size, info->name);
		if (ret == 4)
		{
			info->dev_desc = desc;
			info++;
		}

		p = text_find_next_line(p);
	}

	desc->part_count = info - desc->part_infos;

	return desc->part_count;
}

int cavan_mtd_init(struct cavan_mtd_descriptor *desc, const char *mtd_table)
{
	int ret;
	struct mtd_partition_descriptor *p, *p_end;

	ret = cavan_load_mtd_table(desc, mtd_table);
	if (ret < 0)
	{
		error_msg("cavan_load_mtd_table");
		return ret;
	}

	for (p = desc->part_descs, p_end = p + NELEM(desc->part_descs); p < p_end; p++)
	{
		p->fd = -1;
		p->part_info = NULL;
	}

	return 0;
}

void cavan_mtd_deinit(struct cavan_mtd_descriptor *desc)
{
}

struct mtd_partition_info *cavan_mtd_get_partition_info_by_name(struct cavan_mtd_descriptor *desc, const char *partname)
{
	struct mtd_partition_info *p, *p_end;

	for (p = desc->part_infos, p_end = p + desc->part_count; p < p_end; p++)
	{
		if (text_cmp(p->name, partname) == 0)
		{
			return p;
		}
	}

	return NULL;
}

struct mtd_partition_descriptor *cavan_mtd_malloc_partition_descriptor(struct cavan_mtd_descriptor *desc)
{
	struct mtd_partition_descriptor *p, *p_end;

	for (p = desc->part_descs, p_end = p + NELEM(desc->part_descs); p < p_end; p++)
	{
		if (p->fd < 0)
		{
			return p;
		}
	}

	return malloc(sizeof(*p));
}

void cavan_mtd_free_partition_descriptor(struct mtd_partition_descriptor *desc)
{
	struct cavan_mtd_descriptor *dev_desc = desc->part_info->dev_desc;

	if (desc >= dev_desc->part_descs && (desc - dev_desc->part_descs) < (int) NELEM(dev_desc->part_descs))
	{
		desc->fd = -1;
		desc->part_info = NULL;
	}
	else
	{
		free(desc);
	}
}

int mtd_open_char_device(int index, int flags)
{
	unsigned int i;
	int fd;
	char buff[1024];
	const char *mtd_char_dirs[] = {"/dev", "/dev/mtd"};

	for (i = 0; i < NELEM(mtd_char_dirs); i++)
	{
		sprintf(buff, "%s/mtd%d", mtd_char_dirs[i], index);
		fd = open(buff, flags);
		if (fd >= 0)
		{
			println("MTD char device is: %s", buff);
			return fd;
		}
	}

	return -ENOENT;
}

struct mtd_partition_descriptor *cavan_mtd_open_partition(struct mtd_partition_info *info, int flags)
{
	int fd;
	struct mtd_partition_descriptor *desc;

	fd = mtd_open_char_device(info->index, flags);
	if (fd < 0)
	{
		print_error("mtd_open_char_device");
		return NULL;
	}

	desc = cavan_mtd_malloc_partition_descriptor(info->dev_desc);
	if (desc == NULL)
	{
		print_error("cavan_mtd_malloc_partition_descriptor");
		goto out_close_fd;
	}

	if (ioctl(fd, MEMGETINFO, &desc->user_info) < 0)
	{
		print_error("ioctl MEMGETINFO");
		goto out_free_desc;
	}

	desc->fd = fd;
	desc->part_info = info;

	return desc;

out_free_desc:
	cavan_mtd_free_partition_descriptor(desc);
out_close_fd:
	close(fd);

	return NULL;
}

struct mtd_partition_descriptor *cavan_mtd_open_partition2(struct cavan_mtd_descriptor *desc, const char *partname, int flags)
{
	struct mtd_partition_info *info;

	info = cavan_mtd_get_partition_info_by_name(desc, partname);
	if (info == NULL)
	{
		return NULL;
	}

	return cavan_mtd_open_partition(info, flags);
}

void cavan_mtd_close_partition(struct mtd_partition_descriptor *desc)
{
	if (desc->fd < 0)
	{
		return;
	}

	close(desc->fd);

	cavan_mtd_free_partition_descriptor(desc);
}

int cavan_mtd_erase_blocks(struct mtd_partition_descriptor *desc, int start, int count)
{
	loff_t pos, bpos, pos_end;
	int bad_count;
	int fd = desc->fd;
	u32 erase_size = desc->part_info->erase_size;
	struct erase_info_user erase_info =
	{
		.length = erase_size
	};

	for (pos = start * erase_size, pos_end = pos + count * erase_size, bad_count = 0; pos < pos_end; pos += erase_size)
	{
		bpos = pos;
		if (ioctl(fd, MEMGETBADBLOCK, &bpos) > 0)
		{
#if __WORDSIZE == 64
			pr_red_info("Bad Block at 0x%08lx", bpos);
#else
			pr_red_info("Bad Block at 0x%08Lx", bpos);
#endif
			bad_count++;
			continue;
		}

		erase_info.start = pos;
		if (ioctl(fd, MEMERASE, &erase_info) < 0)
		{
#if __WORDSIZE == 64
			pr_red_info("Erase failed block at 0x%08lx", pos);
#else
			pr_red_info("Erase failed block at 0x%08Lx", pos);
#endif
			bad_count++;
		}
	}

	return count - bad_count;
}

int cavan_mtd_erase_partition(struct mtd_partition_descriptor *desc)
{
	int ret;
	const struct mtd_partition_info *info = desc->part_info;
	u32 erase_size = info->erase_size;
	u32 block_count = info->size / erase_size;
	struct erase_info_user erase_info =
	{
		.start = 0,
		.length = block_count * erase_size
	};

	ret = ioctl(desc->fd, MEMERASE, &erase_info);
	if (ret < 0)
	{
		print_error("ioctl");
		return cavan_mtd_erase_blocks(desc, 0, block_count);
	}

	return 0;
}

ssize_t cavan_mtd_write_block(struct mtd_partition_descriptor *desc, const void *buff)
{
	loff_t pos, bpos, pos_end;
	ssize_t writelen;
	int fd = desc->fd;
	struct mtd_partition_info *info = desc->part_info;
	u32 erase_size = info->erase_size;
	struct erase_info_user erase_info =
	{
		.length = erase_size
	};
#if SUPPORT_MTD_WRITE_VERIFY
	ssize_t readlen;
	char verify[erase_size];
#endif

	pos = lseek(fd, 0, SEEK_CUR);
	if (pos < 0)
	{
		print_error("lseek");
		return pos;
	}

	for (pos_end = info->size - erase_size; pos < pos_end; pos += erase_size)
	{
		bpos = pos;
		if (ioctl(fd, MEMGETBADBLOCK, &bpos) > 0)
		{
#if __WORDSIZE == 64
			pr_red_info("Bad block at 0x%08lx", bpos);
#else
			pr_red_info("Bad block at 0x%08Lx", bpos);
#endif
			continue;
		}

		erase_info.start = pos;
		if (ioctl(fd, MEMERASE, &erase_info) < 0)
		{
#if __WORDSIZE == 64
			pr_red_info("Erase failed block at 0x%08lx", pos);
#else
			pr_red_info("Erase failed block at 0x%08Lx", pos);
#endif
			continue;
		}

		pos = lseek(fd, pos, SEEK_SET);
		if (pos < 0)
		{
			print_error("lseek");
			return pos;
		}

		writelen = write(fd, buff, erase_size);
		if ((size_t) writelen != erase_size)
		{
			print_error("write");
			return writelen;
		}
#if SUPPORT_MTD_WRITE_VERIFY
		pos = lseek(fd, pos, SEEK_SET);
		if (pos < 0)
		{
			print_error("lseek");
			return pos;
		}

		readlen = read(fd, verify, erase_size);
		if (readlen != writelen)
		{
			print_error("read");
			return readlen;
		}

		if (memcmp(buff, verify, erase_size) == 0)
		{
			return readlen;
		}
#else
		return writelen;
#endif
	}

	ERROR_RETURN(ENOSPC);
}

int cavan_mtd_write_partition1(struct mtd_partition_descriptor *desc, int fd)
{
	ssize_t readlen, writelen;
	struct mtd_partition_info *info = desc->part_info;
	char buff[info->erase_size];

	while (1)
	{
		readlen = read(fd, buff, sizeof(buff));
		if (readlen < 0)
		{
			print_error("read");
			return readlen;
		}

		if (readlen == 0)
		{
			return 0;
		}

		writelen = cavan_mtd_write_block(desc, buff);
		if (writelen < 0)
		{
			error_msg("cavan_mtd_write_block");
			return writelen;
		}
	}

	return 0;
}

int cavan_mtd_write_partition2(struct mtd_partition_descriptor *desc, const char *filename)
{
	int ret;
	int fd;

	fd = open(filename, O_RDONLY);
	if (fd < 0)
	{
		print_error("open file %s", filename);
		return fd;
	}

	ret = cavan_mtd_write_partition1(desc, fd);

	close(fd);

	return ret;
}

int cavan_mtd_write_partition3(struct mtd_partition_info *info, const char *filename)
{
	int ret;
	struct mtd_partition_descriptor *desc;

	desc = cavan_mtd_open_partition(info, O_WRONLY);
	if (desc == NULL)
	{
		return -ENOENT;
	}

	ret = cavan_mtd_write_partition2(desc, filename);

	cavan_mtd_close_partition(desc);

	return ret;
}

int cavan_mtd_write_partition4(struct cavan_mtd_descriptor *desc, const char *partname, const char *filename)
{
	struct mtd_partition_info *info;

	info = cavan_mtd_get_partition_info_by_name(desc, partname);
	if (info == NULL)
	{
		return -ENOENT;
	}

	return cavan_mtd_write_partition3(info, filename);
}

int cavan_mtd_write_partition5(const char *partname, const char *filename)
{
	int ret;
	struct cavan_mtd_descriptor desc;

	ret = cavan_mtd_init(&desc, NULL);
	if (ret < 0)
	{
		error_msg("cavan_mtd_init");
		return ret;
	}

	ret = cavan_mtd_write_partition4(&desc, partname, filename);

	cavan_mtd_deinit(&desc);

	return ret;
}

void cavan_mtd_show_info_user(struct mtd_info_user *info)
{
	println("type = %d", info->type);
	println("flags = 0x%08x", info->flags);
	println("size = %s", size2text(info->size));
	println("erasesize = %s", size2text(info->erasesize));
	println("writesize = %s", size2text(info->writesize));
	println("oobsize = %s", size2text(info->oobsize));
	// println("ecctype = %d", info->ecctype);
	// println("eccsize = %s", size2text(info->eccsize));
}

int cavan_mtd_write_image1(struct mtd_partition_descriptor *desc, int fd)
{
	ssize_t readlen, writelen;
	struct mtd_partition_info *info = desc->part_info;
	char buff[info->erase_size + 16];

	cavan_mtd_show_info_user(&desc->user_info);

	while (1)
	{
		readlen = read(fd, buff, sizeof(buff));
		if (readlen < 0)
		{
			print_error("read");
			return readlen;
		}

		if (readlen == 0)
		{
			return 0;
		}

		writelen = cavan_mtd_write_block(desc, buff);
		if (writelen < 0)
		{
			error_msg("cavan_mtd_write_block");
			return writelen;
		}
	}

	return 0;
}

int cavan_mtd_write_image2(struct mtd_partition_descriptor *desc, const char *imagename)
{
	int ret;
	int fd;

	fd = open(imagename, O_RDONLY);
	if (fd < 0)
	{
		print_error("open file %s", imagename);
		return fd;
	}

	ret = cavan_mtd_write_image1(desc, fd);

	close(fd);

	return ret;
}
