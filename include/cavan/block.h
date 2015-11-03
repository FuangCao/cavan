#pragma once

/*
 * File:		block.h
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

struct cavan_part {
	const char *name;
	const char *pathname;
};

struct cavan_part_table {
	const char *name;
	size_t part_count;
	struct cavan_part *parts;
};

void cavan_part_table_dump(const struct cavan_part_table *table);
struct cavan_part_table *cavan_block_get_part_table(const char *name);
struct cavan_part_table *cavan_block_get_part_table2(void);
const struct cavan_part *cavan_block_get_part(const struct cavan_part_table *table, const char *name);
int cavan_block_get_part_pathname(const struct cavan_part_table *table, const char *name, char *buff, size_t size);
