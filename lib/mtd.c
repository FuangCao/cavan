#include <cavan.h>
#include <cavan/mtd.h>
#include <cavan/file.h>
#include <mtd/mtd-user.h>

// Fuang.Cao <cavan.cfa@gmail.com> 2011-12-06 18:20:47

void cavan_show_mtd_partition(const struct mtd_partition_descriptor *part)
{
	print_sep(60);
	println("index = %d, name = %s", part->index, part->name);
	println("size = %s", size2text(part->size));
	println("erase size = %s", size2text(part->erase_size));
	print_sep(60);
}

void cavan_show_mtd_partitions(const struct mtd_partition_descriptor *parts, size_t size)
{
	const struct mtd_partition_descriptor *part_end;

	print_sep(60);

	for (part_end = parts + size; parts < part_end; parts++)
	{
		println("index = %d, name = %s", parts->index, parts->name);
		println("size = %s", size2text(parts->size));
		println("erase size = %s", size2text(parts->erase_size));
		print_sep(60);
	}
}

int cavan_load_mtd_table(const char *mtd_table, struct mtd_partition_descriptor *parts, size_t size)
{
	ssize_t readlen;
	char buff[KB(100)];
	char *p;
	int ret;
	struct mtd_partition_descriptor *part_bak, *part_end;

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
	p = text_find_line(buff, 2);
	part_bak = parts;
	part_end = parts + size;

	while (p && parts < part_end)
	{
		ret = sscanf(p, "mtd%d: %x %x \"%63[^\"]", &parts->index, &parts->size, &parts->erase_size, parts->name);
		if (ret == 4)
		{
			parts->fd = -1;
			parts++;
		}

		p = text_find_next_line(p);
	}

	return parts - part_bak;
}

int cavan_mtd_init(struct cavan_mtd_descriptor *desc, const char *mtd_table)
{
	int ret;

	ret = cavan_load_mtd_table(mtd_table, desc->parts, ARRAY_SIZE(desc->parts));
	if (ret < 0)
	{
		error_msg("cavan_load_mtd_table");
		return ret;
	}

	desc->part_count = ret;

	cavan_show_mtd_partitions(desc->parts, desc->part_count);

	return 0;
}

void cavan_mtd_uninit(struct cavan_mtd_descriptor *desc)
{
	struct mtd_partition_descriptor *p, *p_end;

	for (p = desc->parts, p_end = p + desc->part_count; p < p_end; p++)
	{
		if (p->fd >= 0)
		{
			close(p->fd);
		}
	}
}

struct mtd_partition_descriptor *cavan_mtd_find_partition_by_name(struct cavan_mtd_descriptor *desc, const char *partname)
{
	struct mtd_partition_descriptor *p, *p_end;

	for (p = desc->parts, p_end = p + desc->part_count; p < p_end; p++)
	{
		if (text_cmp(p->name, partname) == 0)
		{
			return p;
		}
	}

	return NULL;
}

int cavan_mtd_open_partition(struct mtd_partition_descriptor *part, int flags)
{
	if (part->fd < 0)
	{
		char buff[1024];

		sprintf(buff, "/dev/mtd/mtd%d", part->index);
		part->fd = open(buff, flags);
	}

	return part->fd;
}

void cavan_mtd_close_partition(struct mtd_partition_descriptor *part)
{
	if (part->fd >= 0)
	{
		close(part->fd);
		part->fd = -1;
	}
}

int cavan_mtd_erase_blocks(struct mtd_partition_descriptor *part, int start, int count)
{
	int ret;
	int fd;
	u32 erase_size = part->erase_size;
	struct erase_info_user erase_info =
	{
		.start = start * erase_size,
		.length = erase_size
	};

	fd = cavan_mtd_open_partition(part, O_WRONLY);
	if (fd < 0)
	{
		error_msg("cavan_mtd_open_partition");
		return fd;
	}

	while (count)
	{
		ret = ioctl(fd, MEMERASE, &erase_info);
		if (ret < 0)
		{
			print_error("ioctl MEMERASE");
			return ret;
		}

		erase_info.start += erase_size;
		count--;
	}

	return 0;
}
