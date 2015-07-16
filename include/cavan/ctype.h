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

#ifdef __cplusplus
extern "C" {
#endif

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
	CT_NM = BIT(10), /* nameable */
	CT_B = BIT(11), /* bracket */
	CT_BL = BIT(12), /* left bracket */
	CT_BR = BIT(13), /* right bracket */
};

extern const u16 cavan_ctype[];

static inline u16 cavan_ctype_get(u8 c)
{
	return cavan_ctype[c];
}

static inline bool cavan_isalnum(u8 c)
{
	return (cavan_ctype_get(c) & ( CT_U | CT_L | CT_D)) != 0;
}

static inline bool cavan_isalpha(u8 c)
{
	return (cavan_ctype_get(c) & (CT_U | CT_L)) != 0;
}

static inline bool cavan_iscntrl(u8 c)
{
	return (cavan_ctype_get(c) & (CT_C)) != 0;
}

static inline bool cavan_isdigit(u8 c)
{
	return (cavan_ctype_get(c) & (CT_D)) != 0;
}

static inline bool cavan_isgraph(u8 c)
{
	return (cavan_ctype_get(c) & (CT_P | CT_U | CT_L | CT_D)) != 0;
}

static inline bool cavan_islower(u8 c)
{
	return (cavan_ctype_get(c) & (CT_L)) != 0;
}

static inline bool cavan_isprint(u8 c)
{
	return (cavan_ctype_get(c) & (CT_P | CT_U | CT_L | CT_D | CT_SP)) != 0;
}

static inline bool cavan_ispunct(u8 c)
{
	return (cavan_ctype_get(c) & (CT_P)) != 0;
}

static inline bool cavan_isspace(u8 c)
{
	return (cavan_ctype_get(c) & (CT_S)) != 0;
}

static inline bool cavan_isupper(u8 c)
{
	return (cavan_ctype_get(c) & (CT_U)) != 0;
}

static inline int isodigit(char c)
{
	return (cavan_ctype_get(c) & (CT_O)) != 0;
}

static inline bool cavan_isxdigit(u8 c)
{
	return (cavan_ctype_get(c) & (CT_D | CT_X)) != 0;
}

static inline bool cavan_islf(u8 c)
{
	return (cavan_ctype_get(c) & (CT_LF)) != 0;
}

static inline bool cavan_isnameable(u8 c)
{
	return (cavan_ctype_get(c) & (CT_NM)) != 0;
}

static inline bool cavan_isbracket(u8 c)
{
	return (cavan_ctype_get(c) & (CT_B)) != 0;
}

static inline bool cavan_isbracket_left(u8 c)
{
	return (cavan_ctype_get(c) & (CT_BL)) != 0;
}

static inline bool cavan_isbracket_right(u8 c)
{
	return (cavan_ctype_get(c) & (CT_BR)) != 0;
}

static inline char cavan_tolower(char c)
{
	if (cavan_isupper(c))
	{
#if 0
		return c - 'A' + 'a';
#else
		return c | 0x20;
#endif
	}

	return c;
}

static inline char cavan_toupper(char c)
{
	if (cavan_islower(c))
	{
#if 0
		return c - 'a' + 'A';
#else
		return c & (~0x20);
#endif
	}

	return c;
}

#ifdef __cplusplus
}
#endif
