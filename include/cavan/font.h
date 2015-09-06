#pragma once

/*
 * File:		font.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2013-07-17 10:54:57
 *
 * Copyright (c) 2013 Fuang.Cao <cavan.cfa@gmail.com>
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

typedef enum cavan_font_type {
	CAVAN_FONT_10X18,
	CAVAN_FONT_12X22,
	CAVAN_FONT_18X32,
	CAVAN_FONT_20X36,
	CAVAN_FONT_COUNT
} cavan_font_type_t;

struct cavan_font
{
	const char *name;
	int lines;
	int width;
	int height;
	int cwidth;
	int cheight;
	int stride;
	byte *body;
	size_t rundata_size;
	const byte *rundata;
};

void cavan_font_dump(struct cavan_font *font);
int cavan_font_init(struct cavan_font *font);
void cavan_font_deinit(struct cavan_font *font);
struct cavan_font *cavan_font_get(int type);
void cavan_font_put(struct cavan_font *font);
int cavan_font_load_bmp(struct cavan_font *font, const char *bmp, int lines);
int cavan_font_save_bmp(struct cavan_font *font, const char *pathname, int bit_count);
ssize_t cavan_font_comp(struct cavan_font *font, byte *buff, size_t size);
