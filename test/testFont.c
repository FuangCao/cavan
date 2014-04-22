/*
 * File:			testFont.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2014-04-16 12:54:05
 *
 * Copyright (c) 2014 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/fb.h>

#define DUMP_FONT 0
#define SAVE_FONT 0

int main(int argc, char *argv[])
{
#if 1
	int i;
#else
	int x, y;
#endif
#if DUMP_FONT
	byte *mem;
	ssize_t ssize;
#endif
#if SAVE_FONT == 0
	struct cavan_font font;
#endif
	struct cavan_display_device *display;

	display = cavan_fb_display_start();
	if (display == NULL)
	{
		pr_red_info("cavan_display_init");
		return -EFAULT;
	}

	if (argc > 1)
	{
#if SAVE_FONT
		cavan_font_save_bmp(display->font, argv[1], 16);
#else
		if (cavan_font_load_bmp(&font, argv[1], 1) == 0)
		{
			cavan_display_set_font(display, &font);

			if (argc > 2)
			{
				cavan_font_save_bmp(display->font, argv[2], 16);
			}
		}
#endif
	}

	cavan_font_dump(display->font);

#if DUMP_FONT
	ssize = display->font->width * display->font->height;
	mem = alloca(ssize);
	ssize = cavan_font_comp(display->font, mem, (size_t)ssize);
	println("ssize = %ld", ssize);
	cavan_mem_dump(mem, ssize, 15, ", ", ",\n");
#endif

	cavan_display_set_color3f(display, 1, 0, 0);

	display->draw_rect(display, 200, 200, 300, 300);

#if 1
	for (i = 32; i < 127; i++)
	{
		cavan_display_printf(display, "i = %d = %c\n", i, i);
	}

	cavan_display_flush(display);
#else
	x = y = 300;

	while (1)
	{
		display->draw_char(display, getchar(), display->pen_color);
		x += display->font->cwidth;
		cavan_display_flush(display);
	}
#endif

	msleep(200);

	cavan_display_stop(display);

	return 0;
}
