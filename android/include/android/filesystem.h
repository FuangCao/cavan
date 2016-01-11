#pragma once

/*
 * File:			file_system.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2016-01-11 11:55:09
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

#include <cavan/android.h>

#include <make_ext4fs.h>
#include <fs_mgr.h>

__BEGIN_DECLS

extern struct fstab *android_fstab;

char *fs_get_fstab_pathname(char *buff, size_t size);
struct fstab *fs_load_fstab(void);
Volume *fs_find_volume(const char *volume);
bool fs_volume_umount(Volume *volume, bool force);
bool fs_volume_umount2(const char *volume, bool force);
bool fs_volume_format(Volume *volume, const char *fs_type, bool force);
bool fs_volume_format2(const char *volume, const char *fs_type, bool force);

__END_DECLS
