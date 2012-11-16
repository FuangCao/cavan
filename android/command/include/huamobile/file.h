/*
 * File:         file.h
 * Based on:
 * Author:       Fuang Cao <cavan.cfa@gmail.com>
 *
 * Created:	  2012-11-14
 * Description:  HUAMOBILE LIBRARY
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

#pragma once

#include <fcntl.h>
#include <dirent.h>
#include <poll.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <huamobile.h>

int huamobile_file_mmap(const char *pathname, void **addr, size_t *size, int flags);
void huamobile_file_unmap(int fd, void *map, size_t size);
void *huamobule_file_read_all(const char *pathname, size_t *size);
