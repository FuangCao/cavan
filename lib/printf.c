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

static char *cavan_printf_text(char *buff, char *buff_end, const char *text, int length, struct cavan_printf_spec *spec, bool reverse)
{
	int width = spec->width;

	if ((spec->flags & CAVAN_PRINTF_PREFIX) && spec->prefix) {
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

	if (reverse) {
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

	return buff;
}

static char cavan_printf_place(int place, struct cavan_printf_spec *spec)
{
	if (place < 10) {
		return place + '0';
	} else {
		return place - 10 + spec->first_letter;
	}
}

static char *cavan_printf_value(char *buff, char *buff_end, struct cavan_printf_spec *spec)
{
	char data[64];
	char *p = data;
	ulonglong value = spec->value;

	if (value) {
		while (value) {
			*p++ = cavan_printf_place(value % spec->base, spec);
			value /= spec->base;
		}
	} else {
		*p++ = '0';
	}

	return cavan_printf_text(buff, buff_end, data, p - data, spec, true);
}

static char *cavan_printf_memory(char *buff, char *buff_end, struct cavan_printf_spec *spec)
{
	const uchar *mem = spec->mem;
	const uchar *mem_end = mem + spec->width;

	buff_end--;

	while (mem < mem_end && buff < buff_end) {
		uchar value = *mem++;

		*buff++ = cavan_printf_place((value >> 4) & 0x0F, spec);
		*buff++ = cavan_printf_place(value & 0x0F, spec);
	}

	return buff;
}

int cavan_vsnprintf(char *buff, size_t size, const char *fmt, cavan_va_list ap)
{
	char *data, *data_end;
	struct cavan_printf_spec spec;

	data = buff;
	data_end = data + size - 1;
	memset(&spec, 0x00, sizeof(spec));

	while (data < data_end) {
		int type = spec.type;

		spec.type = CAVAN_PRINTF_TYPE_NONE;

		switch (type) {
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
			spec.letter = cavan_va_arg(ap, int);
			data = cavan_printf_text(data, data_end, &spec.letter, 1, &spec, false);
			break;

		case CAVAN_PRINTF_TYPE_STR:
			spec.text = cavan_va_arg(ap, const char *);
			data = cavan_printf_text(data, data_end, spec.text, strlen(spec.text), &spec, false);
			break;

		case CAVAN_PRINTF_TYPE_PERCENT:
			*data++ = '%';
			break;

		case CAVAN_PRINTF_TYPE_MEMORY:
			spec.mem = cavan_va_arg(ap, const uchar *);
			data = cavan_printf_memory(data, data_end, &spec);
			break;

		case CAVAN_PRINTF_TYPE_STR_LEN:
			switch (spec.qualifier) {
				case 'l': {
					long *ip = cavan_va_arg(ap, long *);
					*ip = data - buff;
					break;
				}

				case 'L': {
					long long *ip = cavan_va_arg(ap, long long *);
					*ip = data - buff;
					break;
				}

				case 'z':
				case 'Z': {
					size_t *ip = cavan_va_arg(ap, size_t *);
					*ip = data - buff;
					break;
				}

				default: {
					int *ip = cavan_va_arg(ap, int *);
					*ip = data - buff;
				}
			}
			break;

		default:
			switch (type) {
			case CAVAN_PRINTF_TYPE_PTR:
				spec.value = (ulonglong) cavan_va_arg(ap, void *);
				break;

			case CAVAN_PRINTF_TYPE_LONG_LONG:
				spec.value = cavan_va_arg(ap, slonglong);
				break;

			case CAVAN_PRINTF_TYPE_ULONG:
				spec.value = cavan_va_arg(ap, ulong);
				break;

			case CAVAN_PRINTF_TYPE_LONG:
				spec.value = cavan_va_arg(ap, slong);
				break;

			case CAVAN_PRINTF_TYPE_UBYTE:
				spec.value = (uchar) cavan_va_arg(ap, int);
				break;

			case CAVAN_PRINTF_TYPE_BYTE:
				spec.value = (char) cavan_va_arg(ap, int);
				break;

			case CAVAN_PRINTF_TYPE_USHORT:
				spec.value = (ushort) cavan_va_arg(ap, int);
				break;

			case CAVAN_PRINTF_TYPE_SHORT:
				spec.value = (short) cavan_va_arg(ap, int);
				break;

			case CAVAN_PRINTF_TYPE_UINT:
				spec.value = cavan_va_arg(ap, uint);
				break;

			case CAVAN_PRINTF_TYPE_INT:
				spec.value = cavan_va_arg(ap, int);
				break;

			case CAVAN_PRINTF_TYPE_SIZE:
				spec.value = cavan_va_arg(ap, size_t);
				break;

			case CAVAN_PRINTF_TYPE_SSIZE:
				spec.value = cavan_va_arg(ap, ssize_t);
				break;

			default:
				goto label_start;
			}

			data = cavan_printf_value(data, data_end, &spec);
		}

label_start:
		switch (*fmt) {
		case 0:
			goto out_complete;

		case '%':
			fmt++;
			break;

		default:
			if (data >= data_end) {
				goto out_complete;
			}
			*data++ = *fmt++;
			continue;
		}

		spec.flags = 0;
		spec.fill = ' ';
		spec.prefix = NULL;
		spec.first_letter = 'A';

		while (1) {
			switch (*fmt) {
			case 0:
				goto out_complete;

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
			goto out_complete;

		case 'h':
			if (*++fmt != 'h') {
				spec.qualifier = 'h';
				break;
			} else {
				fmt++;
			}
		case 'H':
			spec.qualifier = 'H';
			break;

		case 'l':
			if (*++fmt != 'l') {
				spec.qualifier = 'l';
				break;
			} else {
				fmt++;
			}
		case 'L':
			spec.qualifier = 'L';
			break;

		case 'z':
		case 'Z':
			spec.qualifier = 'z';
			fmt++;
			break;

		case 't':
			spec.qualifier = 't';
			fmt++;
			break;
		}

		spec.base = 10;

		switch (*fmt++) {
		case 'c':
			spec.type = CAVAN_PRINTF_TYPE_CHAR;
			continue;

		case 's':
			spec.type = CAVAN_PRINTF_TYPE_STR;
			continue;

		case 'p':
			spec.first_letter = 'a';
		case 'P':
			spec.fill = '0';
			spec.base = 16;
			spec.prefix = "0x";
			spec.width = sizeof(void *) * 2;
			spec.flags |= CAVAN_PRINTF_PREFIX;
			spec.type = CAVAN_PRINTF_TYPE_PTR;
			continue;

		case 'n':
			spec.type = CAVAN_PRINTF_TYPE_STR_LEN;
			continue;

		case '%':
			spec.type = CAVAN_PRINTF_TYPE_PERCENT;
			continue;

		case 'm':
			spec.first_letter = 'a';
		case 'M':
			spec.type = CAVAN_PRINTF_TYPE_MEMORY;
			continue;

		case 'b':
			spec.base = 2;
			spec.prefix = "0b";
			break;

		case 'B':
			spec.base = 2;
			spec.prefix = "0B";
			break;

		case 'o':
			spec.base = 8;
			spec.prefix = "0";
			break;

		case 'x':
			spec.first_letter = 'a';
		case 'X':
			spec.base = 16;
			spec.prefix = "0x";
			break;

		case 'd':
		case 'i':
			spec.flags |= CAVAN_PRINTF_SIGN;
		case 'u':
			break;

		default:
			spec.type = CAVAN_PRINTF_TYPE_INVALID;
			continue;
		}

		switch (spec.qualifier) {
		case 'l':
			if (spec.flags & CAVAN_PRINTF_SIGN) {
				spec.type = CAVAN_PRINTF_TYPE_LONG;
			} else {
				spec.type = CAVAN_PRINTF_TYPE_ULONG;
			}
			break;

		case 'L':
			spec.type = CAVAN_PRINTF_TYPE_LONG_LONG;
			break;

		case 'z':
			if (spec.flags & CAVAN_PRINTF_SIGN) {
				spec.type = CAVAN_PRINTF_TYPE_SSIZE;
			} else {
				spec.type = CAVAN_PRINTF_TYPE_SIZE;
			}
			break;

		case 'h':
			if (spec.flags & CAVAN_PRINTF_SIGN) {
				spec.type = CAVAN_PRINTF_TYPE_SHORT;
			} else {
				spec.type = CAVAN_PRINTF_TYPE_USHORT;
			}
			break;

		case 'H':
			if (spec.flags & CAVAN_PRINTF_SIGN) {
				spec.type = CAVAN_PRINTF_TYPE_BYTE;
			} else {
				spec.type = CAVAN_PRINTF_TYPE_UBYTE;
			}
			break;

		default:
			if (spec.flags & CAVAN_PRINTF_SIGN) {
				spec.type = CAVAN_PRINTF_TYPE_INT;
			} else {
				spec.type = CAVAN_PRINTF_TYPE_UINT;
			}
		}
	}

out_complete:
	*data = 0;
	return data - buff;
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
