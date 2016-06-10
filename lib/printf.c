/*
 * File:		printf.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2016-06-04 19:05:27
 *
 * Copyright (c) 2016 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/printf.h>

static int cavan_printf_get_value(const char **text)
{
	int value = 0;
	const char *p = *text;

	while (1) {
		int place = *p - '0';

		if (place < 0 || place > 9) {
			break;
		}

		value = value * 10 + place;
		p++;
	}

	*text = p;

	return value;
}

static void cavan_printf_text(struct cavan_printf_spec *spec, const char *text, int length)
{
	int width = spec->width;
	char *buff = spec->buff;
	char *buff_end = spec->buff_end;

	if (spec->flags & CAVAN_PRINTF_NEGATIVE) {
		if (buff < buff_end) {
			*buff++ = '-';
		}

		width--;
	}

	if (CAVAN_PRINTF_PREFIX_ENABLE(spec->flags) && spec->prefix) {
		const char *p = spec->prefix;

		while (*p && buff < buff_end) {
			*buff++ = *p++;
		}
	}

	if ((spec->flags & CAVAN_PRINTF_LEFT) == 0) {
		while (length < width && buff < buff_end) {
			*buff++ = spec->fill;
			width--;
		}
	}

	if (spec->flags & CAVAN_PRINTF_REVERSE) {
		const char *p = text + length - 1;

		while (p >= text && buff < buff_end) {
			*buff++ = *p--;
		}
	} else {
		const char *text_end = text + length;

		while (text < text_end && buff < buff_end) {
			*buff++ = *text++;
		}
	}

	while (length < width && buff < buff_end) {
		*buff++ = spec->fill;
		width--;
	}

	spec->buff = buff;
}

static char cavan_printf_place(struct cavan_printf_spec *spec, int place)
{
	if (place < 10) {
		return place + '0';
	} else {
		return place - 10 + spec->ten;
	}
}

static void cavan_printf_value(struct cavan_printf_spec *spec, ullong value)
{
	char data[68];
	char *p = data;
	int base = spec->base;

	if (base < 2) {
		base = 10;
	}

	if (CAVAN_PRINTF_PREFIX_ENABLE(spec->flags)) {
		switch (base) {
		case 2:
			spec->prefix = "0B";
			break;

		case 8:
			spec->prefix = "0";
			break;

		case 10:
			if (spec->flags & CAVAN_PRINTF_PREFIX_FORCE) {
				spec->prefix = "0D";
			} else {
				spec->prefix = NULL;
			}
			break;

		case 16:
			if (spec->ten == CAVAN_PRINTF_TEN_UPPER_CASE) {
				spec->prefix = "0X";
			} else {
				spec->prefix = "0x";
			}
			break;

		default:
			if (spec->flags & CAVAN_PRINTF_PREFIX_FORCE) {
				cavan_snprintf(spec->buff_prefix, sizeof(spec->buff_prefix), "%d@", base);
				spec->prefix = spec->buff_prefix;
			} else {
				spec->prefix = NULL;
			}
		}
	}

	if (value) {
		if (base == 10 && (spec->flags & CAVAN_PRINTF_SIGN)) {
			sllong svalue = value;

			if (svalue < 0) {
				value = -svalue;
				spec->flags |= CAVAN_PRINTF_NEGATIVE;
			}
		}

		while (value) {
			*p++ = cavan_printf_place(spec, value % base);
			value /= base;
		}
	} else {
		*p++ = '0';
	}

	spec->flags |= CAVAN_PRINTF_REVERSE;

	cavan_printf_text(spec, data, p - data);
}

static void cavan_printf_memory(struct cavan_printf_spec *spec, const uchar *mem)
{
	char *buff = spec->buff;
	char *buff_end = spec->buff_end - 1;
	const uchar *mem_end = mem + spec->width;

	while (mem < mem_end && buff < buff_end) {
		uchar value = *mem++;

		*buff++ = cavan_printf_place(spec, (value >> 4) & 0x0F);
		*buff++ = cavan_printf_place(spec, value & 0x0F);
	}

	spec->buff = buff;
}

int cavan_vsnprintf(char *const buff, size_t size, const char *fmt, cavan_va_list ap)
{
	char c;
	ullong value;
	const uchar *mem;
	const char *text;
	struct cavan_printf_spec spec;

	memset(&spec, 0x00, sizeof(spec));

	spec.buff = buff;
	spec.buff_end = buff + size - 1;

	while (1) {
		int type = spec.type;

		spec.type = CAVAN_PRINTF_TYPE_NONE;

		switch (type) {
		case CAVAN_PRINTF_TYPE_NONE:
			break;

		case CAVAN_PRINTF_TYPE_WIDTH:
			spec.width = cavan_va_arg(ap, int);
			if (spec.width < 0) {
				spec.width = -spec.width;
				spec.flags |= CAVAN_PRINTF_LEFT;
			}
			goto label_precision;

		case CAVAN_PRINTF_TYPE_PRECISION:
			spec.precision = cavan_va_arg(ap, int);
			if (spec.precision < 0) {
				spec.precision = 0;
			}
			goto label_qualifier;

		case CAVAN_PRINTF_TYPE_CHAR:
			c = cavan_va_arg(ap, int);
			cavan_printf_text(&spec, &c, 1);
			break;

		case CAVAN_PRINTF_TYPE_STR:
			text = cavan_va_arg(ap, const char *);
			cavan_printf_text(&spec, text, strlen(text));
			break;

		case CAVAN_PRINTF_TYPE_PERCENT:
			if (likely(spec.buff < spec.buff_end)) {
				*spec.buff++ = '%';
			} else {
				goto label_complete;
			}
			break;

		case CAVAN_PRINTF_TYPE_MEMORY:
			mem = cavan_va_arg(ap, const uchar *);
			cavan_printf_memory(&spec, mem);
			break;

		case CAVAN_PRINTF_TYPE_STR_LEN:
			switch (spec.qualifier) {
				case 'l': {
					long *ip = cavan_va_arg(ap, long *);
					*ip = spec.buff - buff;
					break;
				}

				case 'L': {
					sllong *ip = cavan_va_arg(ap, sllong *);
					*ip = spec.buff - buff;
					break;
				}

				case 'z':
				case 'Z': {
					size_t *ip = cavan_va_arg(ap, size_t *);
					*ip = spec.buff - buff;
					break;
				}

				default: {
					int *ip = cavan_va_arg(ap, int *);
					*ip = spec.buff - buff;
				}
			}
			break;

		default:
			switch (type) {
			case CAVAN_PRINTF_TYPE_PTR:
				value = (ulong) cavan_va_arg(ap, void *);
				break;

			case CAVAN_PRINTF_TYPE_ULLONG:
				value = cavan_va_arg(ap, ullong);
				break;

			case CAVAN_PRINTF_TYPE_LLONG:
				value = cavan_va_arg(ap, sllong);
				break;

			case CAVAN_PRINTF_TYPE_ULONG:
				value = cavan_va_arg(ap, ulong);
				break;

			case CAVAN_PRINTF_TYPE_LONG:
				value = cavan_va_arg(ap, slong);
				break;

			case CAVAN_PRINTF_TYPE_UBYTE:
				value = (uchar) cavan_va_arg(ap, int);
				break;

			case CAVAN_PRINTF_TYPE_BYTE:
				value = (char) cavan_va_arg(ap, int);
				break;

			case CAVAN_PRINTF_TYPE_USHORT:
				value = (ushort) cavan_va_arg(ap, int);
				break;

			case CAVAN_PRINTF_TYPE_SHORT:
				value = (short) cavan_va_arg(ap, int);
				break;

			case CAVAN_PRINTF_TYPE_UINT:
				value = cavan_va_arg(ap, uint);
				break;

			case CAVAN_PRINTF_TYPE_INT:
				value = cavan_va_arg(ap, int);
				break;

			case CAVAN_PRINTF_TYPE_SIZE:
				value = cavan_va_arg(ap, size_t);
				break;

			case CAVAN_PRINTF_TYPE_SSIZE:
				value = cavan_va_arg(ap, ssize_t);
				break;

			default:
				if (likely(spec.buff < spec.buff_end)) {
					*spec.buff++ = '*';
					goto label_find;
				} else {
					goto label_complete;
				}
			}

			cavan_printf_value(&spec, value);
		}

label_find:
		while (1) {
			switch (*fmt) {
			case 0:
				goto label_complete;

			case '%':
				fmt++;
				goto label_found;

			default:
				if (likely(spec.buff < spec.buff_end)) {
					*spec.buff++ = *fmt++;
				} else {
					goto label_complete;
				}
			}
		}

label_found:
		spec.flags = 0;
		spec.fill = ' ';
		spec.ten = CAVAN_PRINTF_TEN_UPPER_CASE;

		while (1) {
			switch (*fmt) {
			case 0:
				goto label_complete;

			case '-':
				spec.flags |= CAVAN_PRINTF_LEFT;
				break;

			case '+':
				spec.flags |= CAVAN_PRINTF_PLUS;
				break;

			case '#':
				spec.flags |= CAVAN_PRINTF_PREFIX;
				break;

			case '0':
				spec.fill = '0';
			case ' ':
				break;

			default:
				goto label_width;
			}

			fmt++;
		}

label_width:
		if (*fmt == '*') {
			spec.type = CAVAN_PRINTF_TYPE_WIDTH;
			fmt++;
			continue;
		}

		spec.width = cavan_printf_get_value(&fmt);

label_precision:
		if (*fmt == '.') {
			if (*++fmt == '*') {
				spec.type = CAVAN_PRINTF_TYPE_PRECISION;
				fmt++;
				continue;
			}

			spec.precision = cavan_printf_get_value(&fmt);
		} else {
			spec.precision = 6;
		}

label_qualifier:
		switch (*fmt) {
		case 0:
			goto label_complete;

		case 'h':
			if (*++fmt != 'h') {
				spec.qualifier = 'h';
				break;
			}
		case 'H':
			fmt++;
			spec.qualifier = 'H';
			break;

		case 'l':
			if (*++fmt != 'l') {
				spec.qualifier = 'l';
				break;
			}
		case 'L':
			fmt++;
			spec.qualifier = 'L';
			break;

		case 'z':
		case 'Z':
			fmt++;
			spec.qualifier = 'z';
			break;

		case 't':
			fmt++;
			spec.qualifier = 't';
			break;
		}

		spec.base = 10;

		switch (*fmt++) {
		case 0:
			goto label_complete;

		case 'c':
			spec.type = CAVAN_PRINTF_TYPE_CHAR;
			continue;

		case 's':
			spec.type = CAVAN_PRINTF_TYPE_STR;
			continue;

		case 'p':
			spec.ten = CAVAN_PRINTF_TEN_LOWER_CASE;
		case 'P':
			spec.fill = '0';
			spec.base = 16;
			// spec.width = sizeof(void *) * 2;
			spec.type = CAVAN_PRINTF_TYPE_PTR;
			spec.flags |= CAVAN_PRINTF_PREFIX_FORCE;
			continue;

		case 'n':
			spec.type = CAVAN_PRINTF_TYPE_STR_LEN;
			continue;

		case '%':
			spec.type = CAVAN_PRINTF_TYPE_PERCENT;
			continue;

		case 'm':
			spec.ten = CAVAN_PRINTF_TEN_LOWER_CASE;
		case 'M':
			spec.type = CAVAN_PRINTF_TYPE_MEMORY;
			continue;

		case 'b':
		case 'B':
			spec.base = 2;
			break;

		case 'o':
			spec.base = 8;
			break;

		case 'x':
			spec.ten = CAVAN_PRINTF_TEN_LOWER_CASE;
		case 'X':
			spec.base = 16;
			break;

		case 'd':
			if (*fmt == '@') {
				int base;
				const char *p = fmt + 1;

				base = cavan_printf_get_value(&p);
				if (base > 1 && base < 36) {
					spec.flags |= CAVAN_PRINTF_PREFIX_FORCE;
					spec.base = base;
					fmt = p;
				}
			}
		case 'i':
			spec.flags |= CAVAN_PRINTF_SIGN;
		case 'u':
			break;

		default:
			spec.type = CAVAN_PRINTF_TYPE_INVALID;
			continue;
		}

		if (spec.flags & CAVAN_PRINTF_SIGN) {
			switch (spec.qualifier) {
			case 'l':
				spec.type = CAVAN_PRINTF_TYPE_LONG;
				break;

			case 'L':
				spec.type = CAVAN_PRINTF_TYPE_LLONG;
				break;

			case 'z':
				spec.type = CAVAN_PRINTF_TYPE_SSIZE;
				break;

			case 'h':
				spec.type = CAVAN_PRINTF_TYPE_SHORT;
				break;

			case 'H':
				spec.type = CAVAN_PRINTF_TYPE_BYTE;
				break;

			default:
				spec.type = CAVAN_PRINTF_TYPE_INT;
			}
		} else {
			switch (spec.qualifier) {
			case 'l':
				spec.type = CAVAN_PRINTF_TYPE_ULONG;
				break;

			case 'L':
				spec.type = CAVAN_PRINTF_TYPE_ULLONG;
				break;

			case 'z':
				spec.type = CAVAN_PRINTF_TYPE_SIZE;
				break;

			case 'h':
				spec.type = CAVAN_PRINTF_TYPE_USHORT;
				break;

			case 'H':
				spec.type = CAVAN_PRINTF_TYPE_UBYTE;
				break;

			default:
				spec.type = CAVAN_PRINTF_TYPE_UINT;
			}
		}
	}

label_complete:
	*spec.buff = 0;
	return spec.buff - buff;
}

int cavan_snprintf(char *buff, size_t size, const char *fmt, ...)
{
	int length;
	cavan_va_list ap;

	cavan_va_start(ap, fmt);
	length = cavan_vsnprintf(buff, size, fmt, ap);
	cavan_va_end(ap);

	return length;
}

int cavan_fdprintf(int fd, const char *fmt, ...)
{
	int length;
	char buff[1024];
	cavan_va_list ap;

	cavan_va_start(ap, fmt);
	length = cavan_vsnprintf(buff, sizeof(buff), fmt, ap);
	cavan_va_end(ap);

	return ffile_write(fd, buff, length);
}
