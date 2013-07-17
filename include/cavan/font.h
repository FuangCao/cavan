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

struct cavan_font
{
	int width;
	int height;
	int cwidth;
	int cheight;
	int stride;
	byte *body;
};

int cavan_font_init(struct cavan_font *font);
void cavan_font_deinit(struct cavan_font *font);
