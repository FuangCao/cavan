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

#ifdef CONFIG_ANDROID_NDK
#include <cavan/android.h>

#ifdef CONFIG_ANDROID_NDK
#include <android-ndk/make_ext4fs.h>
#include <android-ndk/fs_mgr.h>
#else
#if 0
#include <make_ext4fs.h>
#else
#include <ext4_utils/make_ext4fs.h>
#endif
#include <fs_mgr.h>
#endif

__BEGIN_DECLS

extern struct fstab *android_fstab;

char *fs_get_fstab_pathname(char *buff, size_t size);
struct fstab *fs_load_fstab(const char *fstab);
Volume *fs_find_volume(const char *volume, const char *fstab);
bool fs_volume_umount(Volume *volume, bool force);
bool fs_volume_umount2(const char *volume, const char *fstab, bool force);
bool fs_volume_format(Volume *volume, const char *fstab, bool force);
bool fs_volume_format2(const char *volume, const char *fs_type, const char *fstab, bool force);

__END_DECLS
#endif
