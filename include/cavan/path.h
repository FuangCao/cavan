#pragma once

/*
 * File:		path.h
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

int cavan_path_is_dot_name(const char *filename);
int cavan_path_not_dot_name(const char *filename);

char *cavan_path_to_abs_base(const char *rel_path, char *abs_path, size_t size);
char *cavan_path_to_abs(const char *rel_path);
char *cavan_path_to_abs_directory_base(const char *rel_path, char *abs_path, size_t size);
char *cavan_path_to_abs_directory(const char *rel_path);
char *cavan_path_to_abs_base2(const char *rel_path, char *abs_path, size_t size);
char *cavan_path_to_abs2(const char *rel_path);

char *cavan_path_prettify_base(const char *src_path, char *dest_path, size_t size);
char *cavan_path_prettify(const char *src_path);

char *cavan_path_copy(char *buff, size_t size, const char *pathname, boolean isdir);
char *cavan_path_join(char *buff, size_t size, const char *paths[], int count, boolean isdir);
char *cavan_path_join2(char *buff, size_t size, boolean isdir, ...);
char *cavan_path_cat(char *buff, size_t size, const char *dirname, const char *basename, boolean isdir);
