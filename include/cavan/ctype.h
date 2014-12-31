#pragma once

/*
 * File:		ctype.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2014-12-31 15:14:56
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

enum
{
	CT_NONE = 0,
	CT_U = BIT(0), /* upper */
	CT_L = BIT(1), /* lower */
	CT_D = BIT(2), /* digit */
	CT_C = BIT(3), /* cntrl */
	CT_P = BIT(4), /* punct */
	CT_S = BIT(5), /* white space (space/lf/tab) */
	CT_X = BIT(6), /* hex digit */
	CT_SP = BIT(7), /* hard space (0x20) */
	CT_O = BIT(8), /* octal digit */
	CT_LF = BIT(9), /* \n \r digit */
};

extern const u16 cavan_ctype[];

static inline u16 cavan_ctype_get(int c)
{
	return cavan_ctype[c];
}

static inline bool cavan_isalnum(int c)
{
	return (cavan_ctype_get(c) & ( CT_U | CT_L | CT_D)) != 0;
}

static inline bool cavan_isalpha(int c)
{
	return (cavan_ctype_get(c) & (CT_U | CT_L)) != 0;
}

static inline bool cavan_iscntrl(int c)
{
	return (cavan_ctype_get(c) & (CT_C)) != 0;
}

static inline bool cavan_isdigit(int c)
{
	return (cavan_ctype_get(c) & (CT_D)) != 0;
}

static inline bool cavan_isgraph(int c)
{
	return (cavan_ctype_get(c) & (CT_P | CT_U | CT_L | CT_D)) != 0;
}

static inline bool cavan_islower(int c)
{
	return (cavan_ctype_get(c) & (CT_L)) != 0;
}

static inline bool cavan_isprint(int c)
{
	return (cavan_ctype_get(c) & (CT_P | CT_U | CT_L | CT_D | CT_SP)) != 0;
}

static inline bool cavan_ispunct(int c)
{
	return (cavan_ctype_get(c) & (CT_P)) != 0;
}

static inline bool cavan_isspace(int c)
{
	return (cavan_ctype_get(c) & (CT_S)) != 0;
}

static inline bool cavan_isupper(int c)
{
	return (cavan_ctype_get(c) & (CT_U)) != 0;
}

static inline int isodigit(char c)
{
	return (cavan_ctype_get(c) & (CT_O)) != 0;
}

static inline bool cavan_isxdigit(int c)
{
	return (cavan_ctype_get(c) & (CT_D | CT_X)) != 0;
}

static inline bool cavan_islf(int c)
{
	return (cavan_ctype_get(c) & (CT_LF)) != 0;
}

static inline char cavan_tolower(char c)
{
	if (cavan_isupper(c))
	{
		return c - 'A' + 'a';
	}

	return c;
}

static inline char cavan_toupper(char c)
{
	if (cavan_islower(c))
	{
		return c - 'a' + 'A';
	}

	return c;
}

static inline char cavan_tolower_fast(char c)
{
	return c | 0x20;
}
