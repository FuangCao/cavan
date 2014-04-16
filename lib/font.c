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
#include <cavan/font_10x18.h>

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

int cavan_font_load_bmp(struct cavan_font *font, const char *bmp, int lines)
{
	int fd;
	int ret;
	size_t size;
	struct bmp_file_header *file_hdr;
	struct bmp_info_header *info_hdr;
	struct bmp_header *header;

	fd = file_mmap(bmp, (void **)&header, &size, 0);
	if (fd < 0)
	{
		pr_red_info("file_mmap");
		return fd;
	}

	file_hdr = &header->file_hdr;
	info_hdr = &header->info_hdr;

	bmp_show_file_header(file_hdr);
	bmp_show_info_header(info_hdr);

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
			byte *body = font->body;

			p = ((u8 *)header) + file_hdr->offset;
			file_end = p + font->width * font->height;

			while (1)
			{
				const u8 *line_end = p + font->width;
				if (line_end > file_end)
				{
					break;
				}

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
			}
		}
		break;

	case 16:
		{
			const u16 *p, *file_end;
			byte *body = font->body;

			p = (u16 *)(((byte *)header) + file_hdr->offset);
			file_end = p + font->width * font->height;

			while (1)
			{
				const u16 *line_end = p + font->width;
				if (line_end > file_end)
				{
					break;
				}

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
			}
		}
		break;

	case 24:
		{
			const u8 *p, *file_end;
			byte *body = font->body + font->width * font->height;

			p = (u8 *)(((byte *)header) + file_hdr->offset);
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
					if (p[0] > 128 && p[1] > 128 && p[2] > 128)
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
			const u32 *p, *file_end;
			byte *body = font->body;

			p = (u32 *)(((byte *)header) + file_hdr->offset);
			file_end = p + font->width * font->height;

			while (1)
			{
				const u32 *line_end = p + font->width;
				if (line_end > file_end)
				{
					break;
				}

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
