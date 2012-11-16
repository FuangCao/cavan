/*
 * File:         text.c
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

#include <huamobile/text.h>
#include <huamobile.h>

int huamobile_text_cmp(const char *left, const char *right)
{
	while (*left && *left == *right)
	{
		left++;
		right++;
	}

	return *left - *right;
}

int huamobile_text_lhcmp(const char *left, const char *right)
{
	while (*left)
	{
		if (*left != *right)
		{
			return *left - *right;
		}

		left++;
		right++;
	}

	return 0;
}

char *huamobile_text_nfind(const char *text, const char *end, char c, size_t count)
{
	while (text < end)
	{
		if (*text == c)
		{
			count--;

			if (count == 0)
			{
				break;
			}
		}

		text++;
	}

	return (char *)text;
}

char *huamobile_text_copy(char *dest, const char *src)
{
	while ((*dest = *src))
	{
		dest++;
		src++;
	}

	return dest;
}

char *huamobile_text_ncopy(char *dest, const char *src, size_t size)
{
	const char *src_end = src + size;

	while (src < src_end && (*dest++ = *src++));

	return dest;
}

char *huamobile_text_skip_space_head(const char *text, const char *line_end)
{
	while (text < line_end && BYTE_IS_SPACE(*text))
	{
		text++;
	}

	return (char *)text;
}

char *huamobile_text_skip_space_tail(const char *text, const char *line)
{
	while (text > line && BYTE_IS_SPACE(*text))
	{
		text--;
	}

	return (char *)text;
}

char *huamobile_text_find_line_end(const char *text, const char *file_end)
{
	while (text < file_end)
	{
		if (BYTE_IS_LF(*text))
		{
			break;
		}

		text++;
	}

	return (char *)text;
}

char *huamobile_text_skip_line_end(const char *text, const char *file_end)
{
	while (text < file_end && BYTE_IS_LF(*text))
	{
		text++;
	}

	return (char *)text;
}
