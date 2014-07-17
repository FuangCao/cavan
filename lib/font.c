/*
 * File:			font.c
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
#include <cavan/bmp.h>
#include <cavan/file.h>
#include <cavan/font.h>
#include <cavan/display.h>
#include <cavan/font_10x18.h>
#include <cavan/font_12x22.h>
#include <cavan/font_18x32.h>
#include <cavan/font_20x36.h>

#define CAVAN_DEFAULT_FONT	CAVAN_FONT_18X32

void cavan_font_dump(struct cavan_font *font)
{
	println("name = %s", font->name);
	println("lines = %d", font->lines);
	println("width = %d", font->width);
	println("height = %d", font->height);
	println("cwidth = %d", font->cwidth);
	println("cheight = %d", font->cheight);
	println("stride = %d", font->stride);
	println("rundata_size = " PRINT_FORMAT_SIZE, font->rundata_size);
}

int cavan_font_init(struct cavan_font *font)
{
	byte *body;

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

	if (font->rundata && font->rundata_size > 0)
	{
		const byte *rundata, *rundata_end;

		for (rundata = font->rundata, rundata_end = rundata + font->rundata_size; rundata < rundata_end; rundata++)
		{
			int count = (*rundata) & 0x7F;

			mem_set(body, ((*rundata) & (1 << 7)) ? 0xFF : 0, count);
			body += count;
		}
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

	if ((int) type < 0)
	{
		type = CAVAN_DEFAULT_FONT;
	}

	switch (type)
	{
	case CAVAN_FONT_10X18:
		font = &cavan_font_10x18;
		break;

	case CAVAN_FONT_12X22:
		font = &cavan_font_12x22;
		break;

	case CAVAN_FONT_18X32:
		font = &cavan_font_18x32;
		break;

	case CAVAN_FONT_20X36:
		font = &cavan_font_20x36;
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

int cavan_font_load_bmp(struct cavan_font *font, const char *bmp, int lines)
{
	int fd;
	int ret;
	size_t size;
	struct bmp_file_header *file_hdr;
	struct bmp_info_header *info_hdr;
	struct bmp_header *header;

	fd = file_mmap(bmp, (void **) &header, &size, 0);
	if (fd < 0)
	{
		pr_red_info("file_mmap");
		return fd;
	}

	file_hdr = &header->file_hdr;
	info_hdr = &header->info_hdr;

	bmp_show_file_header(file_hdr);
	bmp_show_info_header(info_hdr);

	font->name = "BMP";
	font->lines = lines;
	font->width = info_hdr->width;
	font->height = info_hdr->height;
	font->cwidth = font->width / 96;
	font->cheight = info_hdr->height / lines;
	font->rundata = NULL;
	font->rundata_size = 0;
	font->body = NULL;

	ret = cavan_font_init(font);
	if (ret < 0)
	{
		pr_red_info("cavan_font_init");
		goto out_file_unmap;
	}

	switch (info_hdr->bit_count)
	{
	case 8:
		{
			const u8 *p, *file_end;
			byte *body = font->body + font->width * font->height;
			struct bmp_color_table_entry *colors = (struct bmp_color_table_entry *) (header + 1);

			p = ((u8 *) header) + file_hdr->offset;
			file_end = p + font->width * font->height;

			while (1)
			{
				const u8 *line_end = p + font->width;
				if (line_end > file_end)
				{
					break;
				}

				body -= font->width;

				while (p < line_end)
				{
					struct bmp_color_table_entry *color = colors + (*p);

					if (color->red > 128 && color->green > 128 && color->blue > 128)
					{
						*body = 0xFF;
					}
					else
					{
						*body = 0;
					}

					p++;
					body++;
				}

				body -= font->width;
			}
		}
		break;

	case 16:
		{
			const u16 *p, *file_end;
			byte *body = font->body + font->width * font->height;

			p = (u16 *) (((byte *) header) + file_hdr->offset);
			file_end = p + font->width * font->height;

			while (1)
			{
				const u16 *line_end = p + font->width;
				if (line_end > file_end)
				{
					break;
				}

				body -= font->width;

				while (p < line_end)
				{
					if (*p)
					{
						*body = 0xFF;
					}
					else
					{
						*body = 0;
					}

					p++;
					body++;
				}

				body -= font->width;
			}
		}
		break;

	case 24:
		{
			const u8 *p, *file_end;
			byte *body = font->body + font->width * font->height;

			p = (u8 *) (((byte *) header) + file_hdr->offset);
			file_end = p + font->width * font->height * 3;

			while (1)
			{
				const u8 *line_end = p + font->width * 3;
				if (line_end > file_end)
				{
					break;
				}

				body -= font->width;

				while (p < line_end)
				{
					int brightness;

					brightness = cavan_display_cal_brightness(p[0], p[1], p[2]);
					if (brightness > 64)
					{
						*body = 0xFF;
					}
					else
					{
						*body = 0;
					}

					p += 3;
					body++;
				}

				body -= font->width;
			}
		}
		break;

	case 32:
		{
			const u8 *p, *file_end;
			byte *body = font->body + font->width * font->height;

			p = (u8 *) (((byte *) header) + file_hdr->offset);
			file_end = p + font->width * font->height * 4;

			while (1)
			{
				const u8 *line_end = p + font->width * 4;
				if (line_end > file_end)
				{
					break;
				}

				body -= font->width;

				while (p < line_end)
				{
					if (p[0] > 128 && p[1] > 128 && p[2] > 128)
					{
						*body = 0xFF;
					}
					else
					{
						*body = 0;
					}

					p += 4;
					body++;
				}

				body -= font->width;
			}
		}
		break;

	default:
		ret = -EINVAL;
		pr_red_info("unknown bit_count = %d", info_hdr->bit_count);
		goto out_cavan_font_deinit;
	}

	file_unmap(fd, header, size);

	return 0;

out_cavan_font_deinit:
	cavan_font_deinit(font);
out_file_unmap:
	file_unmap(fd, header, size);
	return ret;
}

int cavan_font_save_bmp(struct cavan_font *font, const char *pathname, int bit_count)
{
	int fd;
	int ret;
	size_t size;
	size_t color_table_size;
	struct bmp_header *header;

	color_table_size = bmp_get_color_table_size(bit_count);
	size = sizeof(*header) + font->width * font->height * bit_count / 8;
	size += color_table_size * sizeof(struct bmp_color_table_entry);
	fd = file_mmap(pathname, (void **) &header, &size, O_RDWR | O_CREAT | O_TRUNC);
	if (fd < 0)
	{
		pr_red_info("file_mmap");
		return fd;
	}

	bmp_header_init(header, font->width, font->height, bit_count);

	switch (bit_count)
	{
	case 8:
		{
			u8 *pixel;
			const byte *body;
			struct bmp_color_table_entry *colors = (struct bmp_color_table_entry *) (header + 1);

			colors[0].red = colors[0].green = colors[0].blue = 0x00;
			colors[1].red = colors[1].green = colors[1].blue = 0xFF;
			colors[0].reserved = colors[1].reserved = 0x00;

			pixel = (u8 *) (colors + color_table_size);

			for (body = font->body + font->width * (font->height - 1); body >= font->body; body -= font->width * 2)
			{
				const byte *body_end;

				for (body_end = body + font->width; body < body_end; body++, pixel++)
				{
					*pixel = (*body) == 0 ? 0 : 1;
				}
			}
		}
		break;

	case 16:
		{
			u16 *pixel = (u16 *) (header + 1);
			const byte *body;

			for (body = font->body + font->width * (font->height - 1); body >= font->body; body -= font->width * 2)
			{
				const byte *body_end;

				for (body_end = body + font->width; body < body_end; body++, pixel++)
				{
					*pixel = (*body) == 0 ? 0x0000 : 0xFFFF;
				}
			}
		}
		break;

	case 24:
		{
			byte *pixel = (byte *) (header + 1);
			const byte *body;

			for (body = font->body + font->width * (font->height - 1); body >= font->body; body -= font->width * 2)
			{
				const byte *body_end;

				for (body_end = body + font->width; body < body_end; body++, pixel += 3)
				{
					pixel[0] = pixel[1] = pixel[2] = (*body) == 0 ? 0x00 : 0xFF;
				}
			}
		}
		break;

	case 32:
		{
			u32 *pixel = (u32 *) (header + 1);
			const byte *body;

			for (body = font->body + font->width * (font->height - 1); body >= font->body; body -= font->width * 2)
			{
				const byte *body_end;

				for (body_end = body + font->width; body < body_end; body++, pixel++)
				{
					*pixel = (*body) == 0 ? 0x00000000 : 0xFFFFFFFF;
				}
			}
		}
		break;

	default:
		pr_red_info("unknown bit_count = %d", bit_count);
		ret = -EINVAL;
		goto out_file_unmap;
	}

	ret = 0;
out_file_unmap:
	file_unmap(fd, header, size);
	return ret;
}

ssize_t cavan_font_comp(struct cavan_font *font, byte *buff, size_t size)
{
	byte *buff_bak, *buff_end;
	const byte *body, *body_end;

	if (font == NULL)
	{
		pr_red_info("font == NULL");
		return -EINVAL;
	}

	body = font->body;

#if 0
	body_end = body + font->stride;
#else
	body_end = body + font->width * font->height;
#endif

	for (buff_bak = buff, buff_end = buff + size; body < body_end && buff < buff_end; buff++)
	{
		int value;
		const byte *body_bak, *body_last;

		body_last = body + 0x7F;
		if (body_last > body_end)
		{
			body_last = body_end;
		}

		body_bak = body++;

		if (*body_bak)
		{
			value = 1;

			while (body < body_last && *body)
			{
				body++;
			}
		}
		else
		{
			value = 0;

			while (body < body_last && *body == 0)
			{
				body++;
			}
		}

		*buff = value << 7 | (body - body_bak);
	}

	return buff - buff_bak;
}
