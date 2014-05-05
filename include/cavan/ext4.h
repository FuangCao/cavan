#pragma once

/*
 * File:			ext4.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2014-05-05 10:24:28
 *
 * Copyright (c) 2014 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <ext2fs/ext2fs.h>

struct cavan_ext4_fs
{
};

struct cavan_ext4_file
{
	struct ext2_inode *inode;
	char pathname[1024];
};

int cavan_ext4_init(struct cavan_ext4_fs *fs);
void cavan_ext4_deinit(struct cavan_ext4_fs *fs);
