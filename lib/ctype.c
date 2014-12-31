/*
 * File:		ctype.c
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
#include <cavan/ctype.h>

const u16 cavan_ctype[] =
{
	CT_C,
	CT_C,
	CT_C,
	CT_C,
	CT_C,
	CT_C,
	CT_C,
	CT_C,
	CT_C, /* \b */
	CT_C | CT_S, /* \t */ /* 9 */
	CT_C | CT_S | CT_LF, /* \n */
	CT_C | CT_S,
	CT_C | CT_S, /* \f */
	CT_C | CT_S | CT_LF, /* \r */
	CT_C,
	CT_C,
	CT_C,
	CT_C,
	CT_C,
	CT_C, /* 19 */
	CT_C,
	CT_C,
	CT_C,
	CT_C,
	CT_C,
	CT_C,
	CT_C,
	CT_C,
	CT_C,
	CT_C, /* 29 */
	CT_C,
	CT_C,
	CT_S | CT_SP, /* space */
	CT_P, /* ! */
	CT_P, /* " */
	CT_P, /* # */
	CT_P, /* $ */
	CT_P, /* % */
	CT_P, /* & */
	CT_P, /* ' */ /* 39 */
	CT_P, /* ( */
	CT_P, /* ) */
	CT_P, /* * */
	CT_P, /* + */
	CT_P, /* , */
	CT_P, /* - */
	CT_P, /* . */
	CT_P, /* / */
	CT_D | CT_NM | CT_X | CT_O, /* 0 */
	CT_D | CT_NM | CT_X | CT_O, /* 1 */ /* 49 */
	CT_D | CT_NM | CT_X | CT_O, /* 2 */
	CT_D | CT_NM | CT_X | CT_O, /* 3 */
	CT_D | CT_NM | CT_X | CT_O, /* 4 */
	CT_D | CT_NM | CT_X | CT_O, /* 5 */
	CT_D | CT_NM | CT_X | CT_O, /* 6 */
	CT_D | CT_NM | CT_X | CT_O, /* 7 */
	CT_D | CT_NM | CT_X, /* 8 */
	CT_D | CT_NM | CT_X, /* 9 */
	CT_P, /* : */
	CT_P, /* ; */ /* 59 */
	CT_P, /* < */
	CT_P, /* = */
	CT_P, /* > */
	CT_P, /* ? */
	CT_P, /* @ */
	CT_U | CT_NM | CT_X, /* A */
	CT_U | CT_NM | CT_X, /* B */
	CT_U | CT_NM | CT_X, /* C */
	CT_U | CT_NM | CT_X, /* D */
	CT_U | CT_NM | CT_X, /* E */ /* 69 */
	CT_U | CT_NM | CT_X, /* F */
	CT_U | CT_NM, /* G */
	CT_U | CT_NM, /* H */
	CT_U | CT_NM, /* I */
	CT_U | CT_NM, /* J */
	CT_U | CT_NM, /* K */
	CT_U | CT_NM, /* L */
	CT_U | CT_NM, /* M */
	CT_U | CT_NM, /* N */
	CT_U | CT_NM, /* O */ /* 79 */
	CT_U | CT_NM, /* P */
	CT_U | CT_NM, /* Q */
	CT_U | CT_NM, /* R */
	CT_U | CT_NM, /* S */
	CT_U | CT_NM, /* T */
	CT_U | CT_NM, /* U */
	CT_U | CT_NM, /* V */
	CT_U | CT_NM, /* W */
	CT_U | CT_NM, /* X */
	CT_U | CT_NM, /* Y */ /* 89 */
	CT_U | CT_NM, /* Z */
	CT_P, /* [ */
	CT_P, /* \ */
	CT_P, /* ] */
	CT_P, /* ^ */
	CT_P | CT_NM, /* _ */
	CT_P, /* ` */
	CT_L | CT_NM | CT_X, /* a */
	CT_L | CT_NM | CT_X, /* b */
	CT_L | CT_NM | CT_X, /* c */ /* 99 */
	CT_L | CT_NM | CT_X, /* d */
	CT_L | CT_NM | CT_X, /* e */
	CT_L | CT_NM | CT_X, /* f */
	CT_L | CT_NM, /* g */
	CT_L | CT_NM, /* h */
	CT_L | CT_NM, /* i */
	CT_L | CT_NM, /* j */
	CT_L | CT_NM, /* k */
	CT_L | CT_NM, /* l */
	CT_L | CT_NM, /* m */ /* 109 */
	CT_L | CT_NM, /* n */
	CT_L | CT_NM, /* o */
	CT_L | CT_NM, /* p */
	CT_L | CT_NM, /* q */
	CT_L | CT_NM, /* r */
	CT_L | CT_NM, /* s */
	CT_L | CT_NM, /* t */
	CT_L | CT_NM, /* u */
	CT_L | CT_NM, /* v */
	CT_L | CT_NM, /* w */ /* 119 */
	CT_L | CT_NM, /* x */
	CT_L | CT_NM, /* y */
	CT_L | CT_NM, /* z */
	CT_P, /* { */
	CT_P, /* | */
	CT_P, /* } */
	CT_P, /* ~ */
	CT_C,
	CT_NONE,
	CT_NONE, /* 129 */
	CT_NONE,
	CT_NONE,
	CT_NONE,
	CT_NONE,
	CT_NONE,
	CT_NONE,
	CT_NONE,
	CT_NONE,
	CT_NONE,
	CT_NONE, /* 139 */
	CT_NONE,
	CT_NONE,
	CT_NONE,
	CT_NONE,
	CT_NONE,
	CT_NONE,
	CT_NONE,
	CT_NONE,
	CT_NONE,
	CT_NONE, /* 149 */
	CT_NONE,
	CT_NONE,
	CT_NONE,
	CT_NONE,
	CT_NONE,
	CT_NONE,
	CT_NONE,
	CT_NONE,
	CT_NONE,
	CT_NONE, /* 159 */
	CT_S | CT_SP,
	CT_P,
	CT_P,
	CT_P,
	CT_P,
	CT_P,
	CT_P,
	CT_P,
	CT_P,
	CT_P, /* 169 */
	CT_P,
	CT_P,
	CT_P,
	CT_P,
	CT_P,
	CT_P,
	CT_P,
	CT_P,
	CT_P,
	CT_P, /* 179 */
	CT_P,
	CT_P,
	CT_P,
	CT_P,
	CT_P,
	CT_P,
	CT_P,
	CT_P,
	CT_P,
	CT_P, /* 189 */
	CT_P,
	CT_P,
	CT_U,
	CT_U,
	CT_U,
	CT_U,
	CT_U,
	CT_U,
	CT_U,
	CT_U, /* 199 */
	CT_U,
	CT_U,
	CT_U,
	CT_U,
	CT_U,
	CT_U,
	CT_U,
	CT_U,
	CT_U,
	CT_U, /* 209 */
	CT_U,
	CT_U,
	CT_U,
	CT_U,
	CT_U,
	CT_P,
	CT_U,
	CT_U,
	CT_U,
	CT_U, /* 219 */
	CT_U,
	CT_U,
	CT_U,
	CT_L,
	CT_L,
	CT_L,
	CT_L,
	CT_L,
	CT_L,
	CT_L, /* 229 */
	CT_L,
	CT_L,
	CT_L,
	CT_L,
	CT_L,
	CT_L,
	CT_L,
	CT_L,
	CT_L,
	CT_L, /* 239 */
	CT_L,
	CT_L,
	CT_L,
	CT_L,
	CT_L,
	CT_L,
	CT_L,
	CT_P,
	CT_L,
	CT_L, /* 249 */
	CT_L,
	CT_L,
	CT_L,
	CT_L,
	CT_L,
	CT_L
};
