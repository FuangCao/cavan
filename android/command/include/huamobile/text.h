/*
 * File:         text.h
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

#include <huamobile.h>

int huamobile_text_cmp(const char *left, const char *right);
int huamobile_text_lhcmp(const char *left, const char *right);
char *huamobile_text_nfind(const char *text, const char *end, char c, size_t count);
char *huamobile_text_copy(char *dest, const char *src);
char *huamobile_text_skip_space_head(const char *text, const char *line_end);
char *huamobile_text_skip_space_tail(const char *text, const char *line);
char *huamobile_text_find_line_end(const char *text, const char *file_end);
char *huamobile_text_skip_line_end(const char *text, const char *file_end);
