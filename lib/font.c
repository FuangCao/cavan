/*
 * File:		font.c
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
#include <cavan/font.h>
#include <cavan/font_10x18.h>

int cavan_font_init(struct cavan_font *font)
{
	byte *body;
	const byte *rundata, *rundata_end;

	if (font->body)
	{
		return 0;
	}

	body = malloc(font->width * font->height);
	if (body == NULL)
	{
		pr_error_info("malloc");
		return -ENOMEM;
	}

	font->body = body;
	font->stride = font->width * font->cheight;

	for (rundata = font->rundata, rundata_end = rundata + font->rundata_size; rundata < rundata_end; rundata++)
	{
		int count = (*rundata) & 0x7F;

		mem_set(body, ((*rundata) & (1 << 7)) ? 0xFF : 0, count);
		body += count;
	}

	return 0;
}

void cavan_font_deinit(struct cavan_font *font)
{
	if (font->body)
	{
		free(font->body);
		font->body = NULL;
	}
}

struct cavan_font *cavan_font_get(cavan_font_type_t type)
{
	int ret;
	struct cavan_font *font;

	switch (type)
	{
	case CAVAN_FONT_10X18:
		font = &cavan_font_10x18;
		break;

	default:
		pr_red_info("unknown font type %d", type);
		return NULL;
	}

	ret = cavan_font_init(font);
	if (ret < 0)
	{
		pr_red_info("cavan_font_init");
		return NULL;
	}

	return font;
}

void cavan_font_put(struct cavan_font *font)
{
	cavan_font_deinit(font);
}
