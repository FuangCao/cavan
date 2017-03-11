/*
 * File:		path.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2017-03-11 18:49:01
 *
 * Copyright (c) 2017 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/path.h>

const char *cavan_tmp_path;

const char *cavan_path_get_tmp_directory_force(void)
{
	int i;
	const char *path_envs[] = { "TMP_PATH", "CACHE_PATH", "HOME" };
	const char *paths[] = { "/tmp", "/data/local/tmp", "/dev", "/data", "/cache" };

	for (i = 0; i < NELEM(paths); i++) {
		if (file_access_w(paths[i])) {
			return paths[i];
		}
	}

	for (i = 0; i < NELEM(path_envs); i++) {
		const char *path = getenv(path_envs[i]);
		if (path && file_access_w(path)) {
			return path;
		}
	}

	return paths[0];
}

const char *cavan_path_get_tmp_directory(void)
{
	if (cavan_tmp_path) {
		return cavan_tmp_path;
	}

	cavan_tmp_path = cavan_path_get_tmp_directory_force();

	return cavan_tmp_path;
}

const char *cavan_path_build_tmp_path(const char *filename, char *buff, size_t size)
{
	const char *dirname = cavan_path_get_tmp_directory();
	char *p = cavan_path_cat(buff, size, dirname, filename, false) - 1;

	while (p >= buff) {
		if (*p == '/') {
			int ret;

			*p = 0;

			ret = mkdir_hierarchy_length(buff, p - buff, 0777);
			if (ret < 0) {
				pd_err_info("mkdir_hierarchy_length: %d", ret);
				return NULL;
			}

			*p = '/';
			break;
		}

		p--;
	}

	return buff;
}

int cavan_path_is_dot_name(const char *filename)
{
	if (*filename != '.') {
		return 0;
	}

	filename++;

	if (*filename == 0) {
		return 1;
	}

	return filename[0] == '.' && filename[1] == 0;
}

int cavan_path_not_dot_name(const char *filename)
{
	if (*filename != '.') {
		return 1;
	}

	filename++;

	if (*filename == 0) {
		return 0;
	}

	return filename[0] != '.' || filename[1] != 0;
}

char *cavan_path_to_abs_base(const char *rel_path, char *abs_path, size_t size)
{
	cavan_path_prettify_base(rel_path, abs_path, size);

	return abs_path;
}

char *cavan_path_to_abs(const char *rel_path)
{
	static char buff[1024];

	cavan_path_to_abs_base(rel_path, buff, sizeof(buff));

	return buff;
}

char *cavan_path_to_abs_directory_base(const char *rel_path, char *abs_path, size_t size)
{
	int ret;

	ret = chdir_backup(rel_path);
	if (ret < 0) {
		return NULL;
	}

	if (getcwd(abs_path, size) == NULL) {
		abs_path = NULL;
	}

	chdir_backup(NULL);

	return abs_path;
}

char *cavan_path_to_abs_directory(const char *rel_path)
{
	static char buff[1024];

	if (cavan_path_to_abs_directory_base(rel_path, buff, sizeof(buff)) == NULL) {
		buff[0] = 0;
	}

	return buff;
}

char *cavan_path_to_abs_base2(const char *rel_path, char *abs_path, size_t size)
{
	char *p;
	char dir_path[1024];

	if (file_test(rel_path, "d") == 0) {
		return cavan_path_to_abs_directory_base(rel_path, abs_path, size);
	}

	cavan_path_dirname_base(dir_path, rel_path);

	if (cavan_path_to_abs_directory_base(dir_path, abs_path, size) == NULL) {
		return NULL;
	}

	for (p = abs_path; *p; p++);

	*p++ = '/';

	cavan_path_basename(p, rel_path);

	return abs_path;
}

char *cavan_path_to_abs2(const char *rel_path)
{
	static char buff[1024];

	if (cavan_path_to_abs_base2(rel_path, buff, sizeof(buff)) == NULL) {
		buff[0] = 0;
	}

	return buff;
}

const char *cavan_path_basename_simple(const char *pathname)
{
	const char *basename;

	for (basename = pathname; *pathname; pathname++) {
		if (*pathname == '/') {
			basename = pathname + 1;
		}
	}

	return basename;
}

char *cavan_path_basename(char *buff, const char *path)
{
	const char *first, *last;

	last = path;

	while (last[0]) {
		last++;
	}

	last--;

	while (last[0] == '/') {
		last--;
	}

	first = last;

	while (first >= path) {
		if (first[0] == '/') {
			break;
		}

		first--;
	}

	first++;

	while (first <= last) {
		*buff++ = *first++;
	}

	buff[0] = 0;

	return buff;
}

char *cavan_path_dirname_base(char *buff, const char *path)
{
	const char *last;

	last = path;
	while (*last) {
		last++;
	}

	while (--last > path && *last == '/');
	while (last > path && *last-- != '/');
	while (last > path && *last == '/') {
		last--;
	}

	if (path == last && *path != '/') {
		*buff++ = '.';
		goto out_buff_end;
	}

	while (path <= last) {
		*buff++ = *path++;
	}

out_buff_end:
	*buff = 0;

	return buff;
}

char *cavan_path_dirname(const char *path)
{
	static char buff[MAX_PATH_LEN];

	cavan_path_dirname_base(buff, path);

	return buff;
}

char *cavan_path_prettify_base(const char *src_path, char *dest_path, size_t size)
{
	char *dest_bak, *src_temp, *src_end;
	char temp_path[1024];

	src_end = text_copy(temp_path, src_path);
	src_path = temp_path;
	dest_bak = dest_path;

	if (*src_path != '/') {
		if (getcwd(dest_path, size) == NULL) {
			return NULL;
		}

		while (*dest_path) {
			dest_path++;
		}
	}

	while (src_path < src_end) {
		while (*src_path == '/') {
			src_path++;
		}

		src_temp = (char *) src_path;

		while (*src_temp && *src_temp != '/') {
			src_temp++;
		}

		*src_temp = 0;

		// println("src_path = %s", src_path);

		if (text_cmp(src_path, "..") == 0) {
			while (dest_path > dest_bak) {
				if (*--dest_path == '/') {
					break;
				}
			}
		} else if (*src_path && text_cmp(src_path, ".")) {
			*dest_path++ = '/';

			while (src_path < src_temp) {
				*dest_path++ = *src_path++;
			}
		}

		src_path = src_temp + 1;
	}

	if (dest_path <= dest_bak) {
		*dest_bak = '/';
		dest_path = dest_bak + 1;
	}

	*dest_path = 0;

	return dest_bak;
}

char *cavan_path_prettify(const char *src_path)
{
	static char buff[1024];

	cavan_path_prettify_base(src_path, buff, sizeof(buff));

	return buff;
}

char *cavan_path_copy(char *buff, size_t size, const char *pathname, boolean isdir)
{
	char *buff_end = buff + size;

	while (buff < buff_end) {
		switch (*pathname) {
		case 0:
			if (isdir) {
				*buff++ = '/';
			}

			*buff = 0;

			return buff;

		case '/':
			pathname = text_skip_char(pathname, '/');
			if (*pathname == 0) {
				if (isdir) {
					*buff++ = '/';

					if (buff < buff_end) {
						*buff = 0;
					}
				} else {
					*buff = 0;
				}

				return buff;
			}

			*buff++ = '/';
			break;

		default:
			*buff++ = *pathname++;
		}
	}

	return buff;
}

char *cavan_path_join(char *buff, size_t size, const char *paths[], int count, boolean isdir)
{
	if (count > 0) {
		char *buff_end = buff + size;
		const char *pathname = paths[0];

		if (count > 1) {
			int i = 1;

			buff = cavan_path_copy(buff, size, pathname, true);

			while (buff < buff_end) {
				pathname = text_skip_char(paths[i], '/');

				if (++i < count) {
					buff = cavan_path_copy(buff, buff_end - buff, pathname, true);
				} else {
					break;
				}
			}
		}

		buff = cavan_path_copy(buff, buff_end - buff, pathname, isdir);
	} else {
		buff = text_ncopy(buff, "/", size);
	}

	return buff;
}

char *cavan_path_join2(char *buff, size_t size, boolean isdir, ...)
{
	va_list ap;
	int count;
	const char *paths[20];

	va_start(ap, isdir);

	for (count = 0; count < NELEM(paths); count++) {
		const char *pathname = va_arg(ap, const char *);
		if (pathname == NULL) {
			break;
		}

		paths[count] = pathname;
	}

	va_end(ap);

	return cavan_path_join(buff, size, paths, count, isdir);
}

char *cavan_path_cat(char *buff, size_t size, const char *dirname, const char *basename, boolean isdir)
{
	if (dirname == NULL) {
		dirname = "/";
	}

	if (basename) {
		const char *paths[] = { dirname, basename };
		return cavan_path_join(buff, size, paths, NELEM(paths), isdir);
	}

	return cavan_path_copy(buff, size, dirname, true);
}
