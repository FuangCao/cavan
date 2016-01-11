/*
 * File:			filesystem.c
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

#include <cavan/device.h>
#include <cavan/command.h>
#include <android/filesystem.h>

struct fstab *android_fstab;

char *fs_get_fstab_pathname(char *buff, size_t size)
{
	int ret;
	DIR *dp;
	struct dirent *dt;
	char hardware[1024];

	ret = android_getprop("ro.hardware", hardware, sizeof(hardware));
	if (ret > 0) {
		println("hardware = %s", hardware);

		snprintf(buff, size, "/fstab.%s", hardware);
		if (file_access_e(buff)) {
			return buff;
		}
	}

	dp = opendir("/");
	if (dp == NULL) {
		return  NULL;
	}

	while ((dt = cavan_readdir_skip_dot(dp))) {
		if (text_lhcmp("fstab.", dt->d_name) == 0) {
			snprintf(buff, size, "/%s", dt->d_name);
			if (file_access_e(buff)) {
				goto out_closedir;
			}
		}
	}

	buff = NULL;

out_closedir:
	closedir(dp);
	return buff;
}

struct fstab *fs_load_fstab(const char *fstab)
{
	if (android_fstab == NULL) {
		char pathname[1024];

		if (fstab == NULL) {
			if (fs_get_fstab_pathname(pathname, sizeof(pathname)) == NULL) {
				return NULL;
			}

			fstab = pathname;
		}

		println("fstab = %s", fstab);

		android_fstab = fs_mgr_read_fstab(pathname);
	}

	return android_fstab;
}

Volume *fs_find_volume(const char *volume, const char *pathname)
{
	struct fstab *fstab;

	if (volume == NULL) {
		return NULL;
	}

	fstab = fs_load_fstab(pathname);
	if (fstab == NULL) {
		return NULL;
	}

	return fs_mgr_get_entry_for_mount_point(android_fstab, volume);
}

bool fs_volume_umount(Volume *volume, bool force)
{
	char pathname[1024];
	int flags = force ? MNT_DETACH : 0;

	while (umount2(volume->mount_point, flags) == 0);

	if (device_is_mounted(volume->blk_device)) {
		return false;
	}

	return true;
}

bool fs_volume_umount2(const char *volume, const char *fstab, bool force)
{
	Volume *v = fs_find_volume(volume, fstab);

	if (v == NULL) {
		pr_red_info("can't find volume: %s", volume);
		return false;
	}

	return fs_volume_umount(v, force);
}

bool fs_volume_format(Volume *volume, const char *fs_type, bool force)
{
	int ret;

	if (fs_type == NULL) {
		fs_type = volume->fs_type;
	}

	println("device = %s", volume->blk_device);
	println("fs_type = %s, length = %lld", fs_type, volume->length);

	if (!fs_volume_umount(volume, force)) {
		pr_red_info("fs_volume_umount");
		return false;
	}

	if (strcmp(fs_type, "ext4") == 0) {
		ret = make_ext4fs(volume->blk_device, volume->length, volume->mount_point, NULL);
		if (ret < 0) {
			pr_red_info("make_ext4fs: %d", ret);
			return false;
		}
	} else {
		pr_red_info("invalid file system: %s", volume->fs_type);
		return false;
	}

	return true;
}

bool fs_volume_format2(const char *volume, const char *fs_type, const char *fstab, bool force)
{
	Volume *v;

	println("volume = %s", volume);

	v = fs_find_volume(volume, fstab);
	if (v == NULL) {
		pr_red_info("can't find volume: %s", volume);
		return false;
	}

	return fs_volume_format(v, fs_type, force);
}
