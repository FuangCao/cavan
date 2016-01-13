/*
 * File:		block.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-10-14 16:07:09
 *
 * Copyright (c) 2015 Fuang.Cao <cavan.cfa@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <cavan.h>
#include <cavan/block.h>
#include <cavan/android.h>

static struct cavan_part cavan_part_imx6ms600[] = {
	{ "boot", "/dev/block/mmcblk3p1" },
	{ "recovery", "/dev/block/mmcblk3p2" },
	{ "data", "/dev/block/mmcblk3p4" },
	{ "system", "/dev/block/mmcblk3p5" },
	{ "cache", "/dev/block/mmcblk3p6" },
	{ "device", "/dev/block/mmcblk3p7" },
	{ "misc", "/dev/block/mmcblk3p8" },
};

static struct cavan_part_table cavan_part_tables[] = {
	{
		.name = "imx6ms600",
		.part_count = NELEM(cavan_part_imx6ms600),
		.parts = cavan_part_imx6ms600
	}
};

void cavan_part_table_dump(const struct cavan_part_table *table)
{
	const struct cavan_part *p, *p_end;

	println("table_name = %s", table->name);

	for (p = table->parts, p_end = p + table->part_count; p < p_end; p++) {
		println("partition: %s -> %s", p->name, p->pathname);
	}
}

struct cavan_part_table *cavan_block_get_part_table(const char *name)
{
	struct cavan_part_table *p, *p_end;

	pd_func_info("table_name = %s", name);

	for (p = cavan_part_tables, p_end = p + NELEM(cavan_part_tables); p < p_end; p++) {
		if (strcmp(name, p->name) == 0) {
			return p;
		}
	}

	return NULL;
}

struct cavan_part_table *cavan_block_get_part_table2(void)
{
	int ret;
	char name[64];

	ret = android_getprop("ro.product.name", name, sizeof(name));
	if (ret < 0) {
		return NULL;
	}

	return cavan_block_get_part_table(name);
}

const struct cavan_part *cavan_block_get_part(const struct cavan_part_table *table, const char *name)
{
	const struct cavan_part *p, *p_end;

	for (p = table->parts, p_end = p + table->part_count; p < p_end; p++) {
		if (strcmp(p->name, name) == 0) {
			return p;
		}
	}

	return NULL;
}

int cavan_block_get_part_pathname(const struct cavan_part_table *table, const char *name, char *buff, size_t size)
{
	const struct cavan_part *part = cavan_block_get_part(table, name);
	if (part == NULL) {
		return -EFAULT;
	}

	strncpy(buff, part->pathname, size);

	return 0;
}
