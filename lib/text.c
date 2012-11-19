#include <cavan.h>
#include <cavan/text.h>
#include <sys/wait.h>
#include <cavan/file.h>

#define MAX_PATH_LEN		KB(1)
#define USE_SYSTEM_PRINTF	1

size_t text_len(const char *text)
{
	const char *last = text;

	while (*last)
	{
		last++;
	}

	return last - text;
}

char *text_find(const char *text, char c)
{
	while (text[0])
	{
		if (text[0] == c)
		{
			return (char *)text;
		}

		text++;
	}

	return NULL;
}

char *text_nfind(const char *text, char c, size_t count)
{
	while (text[0])
	{
		if (text[0] == c)
		{
			if (count > 0)
			{
				count--;
			}
			else
			{
				return (char *)text;
			}
		}

		text++;
	}

	return NULL;
}

char *text_nfind2(const char *text, const char *end, char c, size_t count)
{
	while (text < end)
	{
		if (*text == c)
		{
			if (count > 0)
			{
				count--;
			}
			else
			{
				return (char *)text;
			}
		}

		text++;
	}

	return (char *)text;
}

char *text_find_tail(const char *text, char c)
{
	const char *tmp = NULL;

	while (text[0])
	{
		if (text[0] == c)
		{
			tmp = text;
		}

		text++;
	}

	return (char *)tmp;
}

char *text_ntrans(char *text, size_t size)
{
	char temp;
	char *last;
	char *text_bak = text;

	for (last = text + size - 1; last > text; text++, last--)
	{
		temp = *text;
		*text = *last;
		*last = temp;
	}

	return text_bak;
}

char *text_cat(char *dest, const char *src)
{
	while (*dest)
	{
		dest++;
	}

	return text_copy(dest, src);
}

char *text_cat2(char *dest, char *srcs[], size_t count)
{
	char **src_end = srcs + count;

	while (srcs < src_end)
	{
		const char *p = *srcs++;

		while (*p)
		{
			*dest++ = *p++;
		}
	}

	*dest++ = 0;

	return dest;
}

char *text_vcat3(char *dest, size_t count, va_list ap)
{
	while (count--)
	{
		const char *p = va_arg(ap, const char *);

		while (*p)
		{
			*dest++ = *p++;
		}
	}

	*dest++ = 0;

	return dest;
}

char *text_cat3(char *dest, size_t count, ...)
{
	va_list ap;

	va_start(ap, count);
	dest = text_vcat3(dest, count, ap);
	va_end(ap);

	return dest;
}

char *text_cat4(char *dest, char *srcs[])
{
	while (srcs)
	{
		const char *p = *srcs++;

		while (*p)
		{
			*dest++ = *p++;
		}
	}

	*dest++ = 0;

	return dest;
}

char *text_vcat5(char *dest, va_list ap)
{
	while (1)
	{
		const char *p = va_arg(ap, const char *);

		if (p == NULL)
		{
			break;
		}

		while (*p)
		{
			*dest++ = *p++;
		}
	}

	*dest++ = 0;

	return dest;
}

char *text_cat5(char *dest, ...)
{
	va_list ap;

	va_start(ap, dest);
	dest = text_vcat5(dest, ap);
	va_end(ap);

	return dest;
}

char *text2lowercase(char *text)
{
	char *text_bak = text;

	while (1)
	{
		switch (*text)
		{
		case 0:
			return text_bak;
		case 'A' ... 'Z':
			*text = *text - 'A' + 'a';
			break;
		}

		text++;
	}
}

char *text2uppercase(char *text)
{
	char *text_bak = text;

	while (1)
	{
		switch (*text)
		{
		case 0:
			return text_bak;
		case 'a' ... 'z':
			*text = *text - 'a' + 'Z';
			break;
		}

		text++;
	}
}

char *text_copy(char *dest, const char *src)
{
	while ((*dest = *src))
	{
		dest++;
		src++;
	}

	return dest;
}

char *text_copy_lowercase(char *dest, const char *src)
{
	while (1)
	{
		switch (*src)
		{
		case 0:
			return dest;
		case 'A' ... 'Z':
			*dest = *src - 'A' + 'a';
			break;
		default:
			*dest = *src;
		}

		dest++;
		src++;
	}
}

char *text_copy_uppercase(char *dest, const char *src)
{
	while (1)
	{
		switch (*src)
		{
		case 0:
			return dest;
		case 'a' ... 'z':
			*dest = *src - 'a' + 'A';
			break;
		default:
			*dest = *src;
		}

		dest++;
		src++;
	}
}

char *text_ncopy(char *dest, const char *src, size_t size)
{
	const char *end_src;

	for (end_src = src + size - 1; src < end_src && *src; src++, dest++)
	{
		*dest = *src;
	}

	*dest = 0;

	return dest;
}

u16 *text_copy16(u16 *dest, const u16 *src, size_t count)
{
	const u16 *end_src = src + count;

	while (src < end_src)
	{
		*dest++ = *src++;
	}

	return dest;
}

u32 *text_copy32(u32 *dest, const u32 *src, size_t count)
{
	const u32 *end_src = src + count;

	while (src < end_src)
	{
		*dest++ = *src++;
	}

	return dest;
}

u64 *text_copy64(u64 *dest, const u64 *src, size_t count)
{
	const u64 *end_src = src + count;

	while (src < end_src)
	{
		*dest++ = *src++;
	}

	return dest;
}

int text_cmp(const char *text1, const char *text2)
{
	while (*text1 && *text1 == *text2)
	{
		text1++;
		text2++;
	}

	return *text1 - *text2;
}

int text_ncmp(const char *text1, const char *text2, size_t size)
{
	const char *end_text1;

	if (size == 0)
	{
		return 0;
	}

	for (end_text1 = text1 + size - 1; *text1 && text1 < end_text1 && *text1 == *text2; text1++, text2++);

	return *text1 - *text2;
}

int text_hcmp(const char *text1, const char *text2)
{
	while (*text1 && *text1 == *text2)
	{
		text1++;
		text2++;
	}

	return *text1 && *text2 ? *text1 - *text2 : 0;
}

int text_lhcmp(const char *text1, const char *text2)
{
	while (*text1)
	{
		if (*text1 != *text2)
		{
			return *text1 - *text2;
		}

		text1++;
		text2++;
	}

	return 0;
}

int text_rhcmp(const char *text1, const char *text2)
{
	while (*text2)
	{
		if (*text1 != *text2)
		{
			return *text1 - *text2;
		}

		text1++;
		text2++;
	}

	return 0;
}

int text_tcmp(const char *text1, const char *text2)
{
	const char *text_start1 = text1, *text_start2 = text2;

	while (*text1)
	{
		text1++;
	}

	while (*text2)
	{
		text2++;
	}

	while (text1 >= text_start1 && text2 >= text_start2 && *text1 == *text2)
	{
		text1--;
		text2--;
	}

	if (text1 < text_start1)
	{
		if (text2 < text_start2)
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else if (text2 < text_start2)
	{
		if (text1 < text_start1)
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
	else
	{
		return *text1 - *text2;
	}
}

int text_ltcmp(const char *text1, const char *text2)
{
	const char *text_start1 = text1, *text_start2 = text2;

	while (*text1)
	{
		text1++;
	}

	while (*text2)
	{
		text2++;
	}

	while (text1 >= text_start1 && text2 >= text_start2 && *text1 == *text2)
	{
		text1--;
		text2--;
	}

	if (text1 < text_start1)
	{
		return 0;
	}
	else if (text2 < text_start2)
	{
		if (text1 < text_start1)
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
	else
	{
		return *text1 - *text2;
	}
}

int text_rtcmp(const char *text1, const char *text2)
{
	const char *text_start1 = text1, *text_start2 = text2;

	while (*text1)
	{
		text1++;
	}

	while (*text2)
	{
		text2++;
	}

	while (text1 >= text_start1 && text2 >= text_start2 && *text1 == *text2)
	{
		text1--;
		text2--;
	}

	if (text1 < text_start1)
	{
		if (text2 < text_start2)
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else if (text2 < text_start2)
	{
		return 0;
	}
	else
	{
		return *text1 - *text2;
	}
}

void text_show(const char *text, size_t size)
{
	const char *end_text = text + size;

	while (text < end_text)
	{
		print("%02x ", *(u8 *)text++);
	}

	print_char('\n');
}

void text_show16(const u16 *text, size_t count)
{
	const u16 *end_text = text + count;

	while (text < end_text)
	{
		print("%04x ", *text++);
	}

	print_char('\n');
}

void text_show32(const u32 *text, size_t count)
{
	const u32 *end_text = text + count;

	while (text < end_text)
	{
		print("%08x ", *text++);
	}

	print_char('\n');
}

void text_show64(const u64 *text, size_t count)
{
	const u64 *end_text = text + count;

	while (text < end_text)
	{
		print("%016Lx ", *text++);
	}

	print_char('\n');
}

void *text_set8(u8 *text, u8 value, size_t count)
{
	u8 *end_text = text + count;

	while (text < end_text)
	{
		*text++ = value;
	}

	return text;
}

void *text_set16(u16 *text, u16 value, size_t count)
{
	u16 *end_text = text + count;

	while (text < end_text)
	{
		*text++ = value;
	}

	return text;
}

void *text_set32(u32 *text, u32 value, size_t count)
{
	u32 *end_text = text + count;

	while (text < end_text)
	{
		*text++ = value;
	}

	return text;
}

void *text_set64(u64 *text, u64 value, size_t count)
{
	u64 *end_text = text + count;

	while (text < end_text)
	{
		*text++ = value;
	}

	return text;
}

char *text_ndup(const char *text, size_t size)
{
	char *text_new;

	text_new = malloc(size);
	if (text_new == NULL)
	{
		return NULL;
	}

	text_ncopy(text_new, text, size);

	return text_new;
}

char *text_dup(const char *text)
{
	char *text_new;
	size_t size;

	size = text_len(text) + 1;
	text_new = malloc(size);
	if (text_new == NULL)
	{
		return NULL;
	}

	text_copy(text_new, text);

	return text_new;
}

char *text_tok(char *text, const char *delim)
{
	char *temp;
	static char *text_bak;

	if (text)
	{
		text_bak = text;
	}
	else if (text_bak == NULL)
	{
		return NULL;
	}

	temp = text_bak;
	while (*text_bak)
	{
		if (text_find(delim, *text_bak))
		{
			*text_bak++ = 0;
			return temp;
		}

		text_bak++;
	}

	return temp;
}

int char2value(char c)
{
	switch (c)
	{
	case '0' ... '9':
		return c - '0';

	case 'a' ... 'z':
		return c - 'a' + 10;

	case 'A' ... 'Z':
		return c - 'A' + 10;

	default:
		return -EINVAL;
	}
}

int prefix2base(const char *prefix, const char **prefix_ret)
{
	int base;

	if (*prefix != '0')
	{
		return -EINVAL;
	}

	prefix++;

	switch (*prefix)
	{
	case 'b':
	case 'B':
		prefix++;
		base = 2;
		break;

	case 'd':
	case 'D':
		prefix++;
		base = 10;
		break;

	case 'x':
	case 'X':
		prefix++;
		base = 16;
		break;

	case '0' ... '7':
		base = 8;
		break;

	case 0:
		base = 0;
		break;

	default:
		base = -EINVAL;
	}

	if (prefix_ret)
	{
		*prefix_ret = prefix;
	}

	return base;
}

u64 text2value_unsigned(const char *text, const char **text_ret, int base)
{
	u64 value;
	int tmp;

	if (text == NULL || (tmp = prefix2base(text, &text)) == 0)
	{
		if (text_ret)
		{
			*text_ret = text;
		}
		return 0;
	}

	if (tmp > 0)
	{
		base = tmp;
	}
	else if (base < 1)
	{
		base = 10;
	}

	for (value = 0; *text; text++)
	{
		tmp = char2value(*text);
		if (tmp < 0 || tmp > base)
		{
			break;
		}

		value = value * base + tmp;
	}

	if (text_ret)
	{
		*text_ret = text;
	}

	return value;
}

s64 text2value(const char *text, const char **text_ret, int base)
{
	if (text == NULL)
	{
		if (text_ret)
		{
			*text_ret = NULL;
		}
		return 0;
	}

	if (*text == '-')
	{
		return -text2value_unsigned(text + 1, text_ret, base);
	}

	return text2value_unsigned(text, text_ret, base);
}

void text_reverse_simple(char *start, char *end)
{
	char tmp;

	while (start < end)
	{
		tmp = *start;
		*start = *end;
		*end = tmp;

		start++;
		end--;
	}
}

void text_reverse1(char *p1, char *p2)
{
	if (p1 < p2)
	{
		text_reverse_simple(p1, p2);
	}
	else if (p1 > p2)
	{
		text_reverse_simple(p2, p1);
	}
}

void text_reverse2(char *text, size_t size)
{
	text_reverse_simple(text, text + size - 1);
}

void text_reverse3(char *text)
{
	text_reverse2(text, text_len(text));
}

char *reverse_value2text_base2(u64 value, char *buff, size_t size)
{
	char *buff_end;

	for (buff_end = buff + size - 1; buff < buff_end && value; buff++, value >>= 1)
	{
		*buff = (value & 0x01) + '0';
	}

	*buff = 0;

	return buff;
}

char *reverse_value2text_base4(u64 value, char *buff, size_t size)
{
	char *buff_end;

	for (buff_end = buff + size - 1; buff < buff_end && value; buff++, value >>= 2)
	{
		*buff = (value & 0x03) + '0';
	}

	*buff = 0;

	return buff;
}

char *reverse_value2text_base8(u64 value, char *buff, size_t size)
{
	char *buff_end;

	for (buff_end = buff + size - 1; buff < buff_end && value; buff++, value >>= 3)
	{
		*buff = (value & 0x07) + '0';
	}

	*buff = 0;

	return buff;
}

char *reverse_value2text_base16(u64 value, char *buff, size_t size)
{
	char *buff_end;

	for (buff_end = buff + size - 1; buff < buff_end && value; buff++, value >>= 4)
	{
		*buff = value2char(value & 0x0F);
	}

	*buff = 0;

	return buff;
}

char *reverse_value2text_base32(u64 value, char *buff, size_t size)
{
	char *buff_end;

	for (buff_end = buff + size - 1; buff < buff_end && value; buff++, value >>= 5)
	{
		*buff = value2char(value & 0x1F);
	}

	*buff = 0;

	return buff;
}

char *reverse_value2text_all(u64 value, char *buff, size_t size, int base)
{
	char *buff_end;

	for (buff_end = buff + size - 1; buff < buff_end && value; buff++, value /= base)
	{
		*buff = value2char(value % base);
	}

	*buff = 0;

	return buff;
}

char *simple_value2text_reverse(u64 value, char *buff, size_t size, int base)
{
	if (value == 0)
	{
		*(u16 *)buff = 0x0030;
		return buff + 1;
	}

	switch (base)
	{
	case 2:
		return reverse_value2text_base2(value, buff, size);

	case 4:
		return reverse_value2text_base4(value, buff, size);

	case 8:
		return reverse_value2text_base8(value, buff, size);

	case 16:
		return reverse_value2text_base16(value, buff, size);

	case 32:
		return reverse_value2text_base32(value, buff, size);

	default:
		return reverse_value2text_all(value, buff, size, base);
	}
}

char *simple_value2text_unsigned(u64 value, char *buff, size_t size, int base)
{
	char *tail;

	tail = simple_value2text_reverse(value, buff, size, base);
	text_reverse_simple(buff, tail - 1);

	return tail;
}

char *simple_value2text(s64 value, char *buff, size_t size, int base)
{
	if (value < 0 && base == 10)
	{
		*buff = '-';
		return simple_value2text_unsigned(-value, buff + 1, size - 1, 10);
	}

	return simple_value2text_unsigned(value, buff, size, base);
}

char *base2prefix(int base, char *prefix)
{
	switch (base)
	{
	case 2:
		*(u16 *)prefix = 0x4230;
		prefix += 2;
		break;

	case 8:
		*prefix++ = '0';
		break;

	case 10:
		*(u16 *)prefix = 0x4430;
		prefix += 2;
		break;

	case 16:
		*(u16 *)prefix = 0x7830;
		prefix += 2;
		break;
	}

	*prefix = 0;

	return prefix;
}

char *value2text_base(s64 value, char *text, int size, char fill, int flags)
{
	char buff[128], *tail;
	int base;

	base = flags & 0xFF;
	if (base == 10 && value < 0 && (flags & TEXT_FLAG_SIGNED))
	{
		*text++ = '-';
		tail = simple_value2text_reverse(-value, buff, sizeof(buff), 10);
	}
	else
	{
		tail = simple_value2text_reverse(value, buff, sizeof(buff), base);
	}

	if (flags & TEXT_FLAG_PREFIX)
	{
		text = base2prefix(base, text);
	}

	size -= (tail - buff);
	if (size > 0)
	{
		char *text_end;

		if (fill == 0)
		{
			fill = '0';
		}

		for (text_end = text + size; text < text_end; text++)
		{
			*text = fill;
		}
	}

	for (tail--; tail >= buff; text++, tail--)
	{
		*text = *tail;
	}

	*text = 0;

	return text;
}

char *value2text(u64 value, int flags)
{
	static char buff[100];

	value2text_base(value, buff, 0, 0, flags);

	return buff;
}

u64 text2size_single(const char *text, const char **text_ret)
{
	u64 size;

	size = text2value_unsigned(text, &text, 10);

	switch (*text)
	{
	case 't':
	case 'T':
		size <<= 40;
		break;
	case 'g':
	case 'G':
		size <<= 30;
		break;
	case 'm':
	case 'M':
		size <<= 20;
		break;
	case 'k':
	case 'K':
		size <<= 10;
		break;
	case 'b':
	case 'B':
	case 0:
		break;
	default:
		error_msg("illegal character \'%c\'", *text);
	}

	if (*text)
	{
		text++;
	}

	if (text_ret)
	{
		*text_ret = text;
	}

	return size;
}

u64 text2size(const char *text, const char **text_ret)
{
	u64 size;

	if (text == NULL)
	{
		return 0;
	}

	size = 0;

	while (*text)
	{
		size += text2size_single(text, &text);
	}

	if (text_ret)
	{
		*text_ret = text;
	}

	return size;
}

u64 text2size_mb(const char *text)
{
	if (text == NULL)
	{
		return 0;
	}

	if (text_is_number(text))
	{
		return text2value_unsigned(text, NULL, 10);
	}
	else
	{
		return text2size(text, NULL) >> 20;
	}
}

char *size2text_base(u64 size, char *buff, size_t buff_len)
{
	u64 tmp;

	if (size == 0)
	{
		*buff++ = '0';
		goto out_return;
	}

	tmp = (size >> 40) & 0x3FF;
	if (tmp)
	{
		buff = simple_value2text(tmp, buff, buff_len, 10);
		*buff++ = 'T';
	}

	tmp = (size >> 30) & 0x3FF;
	if (tmp)
	{
		buff = simple_value2text(tmp, buff, buff_len, 10);
		*buff++ = 'G';
	}

	tmp = (size >> 20) & 0x3FF;
	if (tmp)
	{
		buff = simple_value2text(tmp, buff, buff_len, 10);
		*buff++ = 'M';
	}

	tmp = (size >> 10) & 0x3FF;
	if (tmp)
	{
		buff = simple_value2text(tmp, buff, buff_len, 10);
		*buff++ = 'k';
	}

	tmp = size & 0x3FF;
	if (tmp)
	{
		buff = simple_value2text(tmp, buff, buff_len, 10);
	}

out_return:
	*buff++ = 'B';
	*buff = 0;

	return buff;
}

char *size2text(u64 size)
{
	static char buff[128];

	size2text_base(size, buff, sizeof(buff));

	return buff;
}

#ifndef USE_SYSTEM_PRINTF
static u64 get_argument(char *args, int size)
{
	u64 value;

	switch (size)
	{
	case 1:
		value = *(u8 *)args;
		break;

	case 2:
		value = *(u16 *)args;
		break;

	case 4:
		value = *(u32 *)args;
		break;

	case 8:
		value = *(u64 *)args;
		break;

	default:
		value = 0;
	}

	return value;
}

char *vformat_text(char *buff, const char *fmt, va_list args)
{
	int text_len;
	int arg_len;
	int symbol;
	char fill;

	while (1)
	{
		switch(fmt[0])
		{
		case 0:
			goto out_return;

		case '%':
			fmt++;
			text_len = 0;
			arg_len = 4;
			symbol = 1;
			fill = ' ';
start_parse:
			switch (fmt[0])
			{
			case '0':
				fill = '0';
				fmt++;
			case '1' ... '9':
				while (fmt[0] >= '0' && fmt[0] <= '9')
				{
					text_len = text_len * 10 + fmt[0] - '0';
					fmt++;
				}
				goto start_parse;

			case '*':
				text_len = get_argument(args, 4);
				fmt++;
				args += 4;
				goto start_parse;

			case '-':
				symbol = -1;
				fmt++;
				goto start_parse;

			case 'l':
			case 'L':
				arg_len <<= 1;
				fmt++;
				goto start_parse;

			case 'b':
			case 'B':
				buff = value2text_base(get_argument(args, arg_len), buff, text_len * symbol, fill, 2 | FLAG_PREFIX);
				break;

			case 'o':
			case 'O':
				buff = value2text_base(get_argument(args, arg_len), buff, text_len * symbol, fill, 8 | FLAG_PREFIX);
				break;

			case 'u':
			case 'U':
				buff = value2text_base(get_argument(args, arg_len), buff, text_len * symbol, fill, 10 | FLAG_PREFIX);
				break;

			case 'd':
			case 'D':
			case 'i':
			case 'I':
				buff = value2text_base(get_argument(args, arg_len), buff, text_len * symbol, fill, 10 | FLAG_SIGNED);
				break;

			case 'p':
			case 'P':
				buff = value2text_base(get_argument(args, arg_len), buff, text_len * symbol, fill, 16 | FLAG_PREFIX);
				break;

			case 'x':
			case 'X':
				buff = value2text_base(get_argument(args, arg_len), buff, text_len * symbol, fill, 16);
				break;

			case 'c':
			case 'C':
				*buff++ = *(char *)args;
				break;

			case '%':
				*buff++ = '%';
				break;

			case 's':
			case 'S':
				buff = text_copy(buff, *(char **)args);
				break;

			default:
				error_msg("Unknown Formate");
				return NULL;
			}

			args += 4;

			break;

		default:
			*buff++ = fmt[0];
		}
		fmt++;
	}

out_return:
	buff[0] = 0;

	return buff;
}
#else
inline char *vformat_text(char *buff, const char *fmt, va_list args)
{
	return buff + vsprintf(buff, fmt, args);
}
#endif

char *format_text(const char *fmt, ...)
{
	va_list ap;
	static char buff[1024];

	va_start(ap, fmt);
	vformat_text(buff, fmt, ap);
	va_end(ap);

	return buff;
}

char *text_basename_base(char *buff, const char *path)
{
	const char *first, *last;

	last = path;

	while (last[0])
	{
		last++;
	}

	last--;

	while (last[0] == '/')
	{
		last--;
	}

	first = last;

	while (first >= path)
	{
		if (first[0] == '/')
		{
			break;
		}

		first--;
	}

	first++;

	while (first <= last)
	{
		*buff++ = *first++;
	}

	buff[0] = 0;

	return buff;
}

char *text_basename(const char *path)
{
	static char buff[MAX_PATH_LEN];

	text_basename_base(buff, path);

	return buff;
}

char *text_dirname_base(char *buff, const char *path)
{
	const char *last;

	last = path;
	while (*last)
	{
		last++;
	}

	while (--last > path && *last == '/');
	while (last > path && *last-- != '/');
	while (last > path && *last == '/')
	{
		last--;
	}

	if (path == last && *path != '/')
	{
		*buff++ = '.';
		goto out_buff_end;
	}

	while (path <= last)
	{
		*buff++ = *path++;
	}

out_buff_end:
	*buff = 0;

	return buff;
}

char *text_dirname(const char *path)
{
	static char buff[MAX_PATH_LEN];

	text_dirname_base(buff, path);

	return buff;
}

#if 0
static int symbol_match_base(char c1, char c2)
{
	return 0;
}

static int symbol_match(char c1, char c2)
{
	if (c1 == c2)
	{
		return 1;
	}

	return symbol_match_base(c1, c2) || symbol_match_base(c2, c1);
}
#endif

int text_match(const char *text1, const char *text2)
{
	while (text1[0] && text2[0])
	{
		switch (text1[0])
		{
		case '\\':
			break;
		case '*':
			break;
		default:
			if (text1[0] != text2[0])
			{
				return text1[0] - text2[0];
			}
		}

		text1++;
		text2++;
	}

	return 0;
}

char *get_ntext_base(const char *src, char *dest, int start, int count)
{
	const char *src_end;

	src += start;
	src_end = src + count;

	while (src < src_end)
	{
		*dest++ = *src++;
	}

	return dest;
}

char *get_ntext(const char *src, int start, int count)
{
	static char buff[1024];

	get_ntext_base(src, buff, start, count)[0] = 0;

	return buff;
}

char *text_header(const char *text, int count)
{
	return get_ntext(text, 0, count);
}

char *get_text_region_base(const char *src, char *dest, int start, int end)
{
	return get_ntext_base(src, dest, start, end - start + 1);
}

char *get_text_region(const char *src, int start, int end)
{
	return get_ntext(src, start, end - start + 1);
}

FILE *vpipe_command(const char *command, va_list ap)
{
	FILE *ftemp;
	char buff[1024];

	text_cat(buff + vsprintf(buff, command, ap) - 1, " 2>/dev/null");

	println("excute command \"%s\" with pipe", buff);

	ftemp = popen(buff, "r");
	if (ftemp)
	{
		system_sync();
	}
	else
	{
		println("\"%s\" faild", buff);
	}

	return ftemp;
}

FILE *pipe_command(const char *command, ...)
{
	va_list ap;
	FILE *ftemp;

	va_start(ap, command);
	ftemp = vpipe_command(command, ap);
	va_end(ap);

	return ftemp;
}

FILE *vpipe_command_verbose(const char *command, va_list ap)
{
	FILE *ftemp;
	char buff[1024];

	vsprintf(buff, command, ap);
	println("excute command \"%s\" with pipe", buff);

	ftemp = popen(buff, "r");
	if (ftemp)
	{
		system_sync();
	}
	else
	{
		println("\"%s\" faild", buff);
	}

	return ftemp;
}

FILE *pipe_command_verbose(const char *command, ...)
{
	va_list ap;
	FILE *ftemp;

	va_start(ap, command);
	ftemp = vpipe_command_verbose(command, ap);
	va_end(ap);

	return ftemp;
}

FILE *pipe_command_path(const char *path, const char *command, ...)
{
	FILE *ftemp;
	va_list ap;
	char buff[1024];

	sprintf(buff, "PATH=%s:${PATH}; %s", path, command);

	va_start(ap, command);
	ftemp = vpipe_command(buff, ap);
	va_end(ap);

	return ftemp;
}

ssize_t vbuff_command(char *buff, size_t bufflen, const char *command, va_list ap)
{
	FILE *fpipe;
	ssize_t readlen;

	fpipe = vpipe_command(command, ap);
	if (fpipe == NULL)
	{
		return -1;
	}

	readlen = fread(buff, 1, bufflen, fpipe);
	pclose(fpipe);

	return readlen > 0 ? readlen : -1;
}

char *buff_command(const char *command, ...)
{
	va_list ap;
	static char buff[1024];

	va_start(ap, command);
	vbuff_command(buff, sizeof(buff), command, ap);
	va_end(ap);

	return buff;
}

ssize_t buff_command2(char *buff, size_t bufflen, const char *command, ...)
{
	va_list ap;
	ssize_t readlen;

	va_start(ap, command);
	readlen = vbuff_command(buff, bufflen, command, ap);
	va_end(ap);

	return readlen;
}

char *buff_command_path(const char *path, const char *command, ...)
{
	va_list ap;
	static char buff[1024];
	char temp_cmd[1024];

	sprintf(temp_cmd, "PATH=%s:${PATH}; %s", path, command);

	va_start(ap, command);
	vbuff_command(buff, sizeof(buff), temp_cmd, ap);
	va_end(ap);

	return buff;
}

ssize_t buff_command_path2(const char *path, char *buff, size_t bufflen, const char *command, ...)
{
	va_list ap;
	char temp_cmd[1024];
	ssize_t readlen;

	sprintf(temp_cmd, "PATH=%s:${PATH}; %s", path, command);

	va_start(ap, command);
	readlen = vbuff_command(buff, bufflen, temp_cmd, ap);
	va_end(ap);

	return readlen;
}

int system_command_simple(const char *command)
{
	int ret;

	println("excute command \"%s\"", command);
	ret = system(command);
	if (ret == 0)
	{
		system_sync();
	}
	else
	{
		println("\"%s\" faild", command);
	}

	return ret;
}

int vsystem_command(const char *command, va_list ap)
{
	int ret;
	char buff[1024];

	vsprintf(buff, command, ap);
	ret = system_command_simple(buff);
	if (ret == 0)
	{
		return 0;
	}

	if (WEXITSTATUS(ret) != 127)
	{
		return -1;
	}

	vsprintf(text_copy(buff, "PATH=" DEFAULT_PATH_VALUE ":${PATH}; "), command, ap);
	ret = system_command_simple(buff);
	if (ret == 0)
	{
		return 0;
	}

	if (WEXITSTATUS(ret) != 127)
	{
		return -1;
	}

	vsprintf(text_copy(buff, "/bin/busybox "), command, ap);
	ret = system_command_simple(buff);
	if (ret == 0)
	{
		return 0;
	}

	return -1;
}

int system_command_retry(int count, const char *command, ...)
{
	va_list ap;

	va_start(ap, command);
	while (count-- && vsystem_command(command, ap) < 0);
	va_end(ap);

	return count;
}

int system_command(const char *command, ...)
{
	int ret;
	va_list ap;

	va_start(ap, command);
	ret = vsystem_command(command, ap);
	va_end(ap);

	return ret;
}

int system_command_path(const char *path, const char *command, ...)
{
	int ret;
	va_list ap;
	char buff[1024];

	sprintf(buff, "PATH=%s:${PATH}; %s", path, command);

	va_start(ap, command);
	ret = vsystem_command(buff, ap);
	va_end(ap);

	return ret;
}

char *mem2text_base(const void *mem, char *buff, int size)
{
	const uchar *p, *endp;

	p = mem;
	endp = p + size;

	while (p < endp)
	{
		buff = value2text_base(*p++, buff, 2, 0, 16);
	}

	buff[0] = 0;

	return buff;
}

char *mem2text(const void *mem, int size)
{
	static char buff[1024];

	mem2text_base(mem, buff, size);

	return buff;
}

void system_sync(void)
{
	print("Synchronization ... ");

	sync();

	if (system("PATH=" DEFAULT_PATH_VALUE ":${PATH}; sync") == 0)
	{
		println("OK");
	}
	else
	{
		println("Failed");
	}
}

char *to_abs_path_base(const char *rel_path, char *abs_path, size_t size)
{
	prettify_pathname_base(rel_path, abs_path, size);

	return abs_path;
}

char *to_abs_path(const char *rel_path)
{
	static char buff[1024];

	to_abs_path_base(rel_path, buff, sizeof(buff));

	return buff;
}

char *to_abs_path_directory_base(const char *rel_path, char *abs_path, size_t size)
{
	int ret;

	ret = chdir_backup(rel_path);
	if (ret < 0)
	{
		return NULL;
	}

	if (getcwd(abs_path, size) == NULL)
	{
		abs_path = NULL;
	}

	chdir_backup(NULL);

	return abs_path;
}

char *to_abs_directory(const char *rel_path)
{
	static char buff[1024];

	if (to_abs_path_directory_base(rel_path, buff, sizeof(buff)) == NULL)
	{
		buff[0] = 0;
	}

	return buff;
}

char *to_abs_path2_base(const char *rel_path, char *abs_path, size_t size)
{
	char *p;
	char dir_path[1024];

	if (file_test(rel_path, "d") == 0)
	{
		return to_abs_path_directory_base(rel_path, abs_path, size);
	}

	text_dirname_base(dir_path, rel_path);

	if (to_abs_path_directory_base(dir_path, abs_path, size) == NULL)
	{
		return NULL;
	}

	for (p = abs_path; *p; p++);

	*p++ = '/';

	text_basename_base(p, rel_path);

	return abs_path;
}

char *to_abs_path2(const char *rel_path)
{
	static char buff[1024];

	if (to_abs_path2_base(rel_path, buff, sizeof(buff)) == NULL)
	{
		buff[0] = 0;
	}

	return buff;
}

char *prettify_pathname_base(const char *src_path, char *dest_path, size_t size)
{
	char *dest_bak, *src_temp, *src_end;
	char temp_path[1024];

	src_end = text_copy(temp_path, src_path);
	src_path = temp_path;
	dest_bak = dest_path;

	if (*src_path != '/')
	{
		if (getcwd(dest_path, size) == NULL)
		{
			return NULL;
		}

		while (*dest_path)
		{
			dest_path++;
		}
	}

	while (src_path < src_end)
	{
		while (*src_path == '/')
		{
			src_path++;
		}

		src_temp = (char *)src_path;

		while (*src_temp && *src_temp != '/')
		{
			src_temp++;
		}

		*src_temp = 0;

		// println("src_path = %s", src_path);

		if (text_cmp(src_path, "..") == 0)
		{
			while (dest_path > dest_bak)
			{
				if (*--dest_path == '/')
				{
					break;
				}
			}
		}
		else if (*src_path && text_cmp(src_path, "."))
		{
			*dest_path++ = '/';

			while (src_path < src_temp)
			{
				*dest_path++ = *src_path++;
			}
		}

		src_path = src_temp + 1;
	}

	if (dest_path <= dest_bak)
	{
		*dest_bak = '/';
		dest_path = dest_bak + 1;
	}

	*dest_path = 0;

	return dest_bak;
}

char *prettify_pathname(const char *src_path)
{
	static char buff[1024];

	prettify_pathname_base(src_path, buff, sizeof(buff));

	return buff;
}

char *text_path_cat(char *pathname, const char *dirname, const char *basename)
{
	pathname = text_copy(pathname, dirname) - 1;

	while (*pathname == '/')
	{
		pathname--;
	}

	pathname[1] = '/';

	if (basename)
	{
		return text_copy(pathname + 2, text_skip_char(basename, '/'));
	}

	pathname[2] = 0;

	return pathname + 2;
}

char *text_delete_char_base(const char *text_in, char *text_out, char c)
{
	char *text_bak;

	text_bak = text_out;

	while (*text_in)
	{
		if (*text_in == c)
		{
			text_in++;
			continue;
		}

		*text_out++ = *text_in++;
	}

	*text_out = 0;

	return text_bak;
}

char *text_delete_sub_base(const char *text_in, char *text_out, const char *sub, const size_t sublen)
{
	char *text_bak;
	int step[sublen];

	text_bak = text_out;
	mem_build_kmp_array(sub, step, sublen);

	while (*text_in)
	{
		char *text_tmp;

		text_tmp = mem_kmp_find_base(text_in, text_in + text_len(text_in), sub, sublen, step);
		if (text_tmp == NULL)
		{
			break;
		}

		while (text_in < text_tmp)
		{
			*text_out++ = *text_in++;
		}

		text_in += sublen;
	}

	while (*text_in)
	{
		*text_out++ = *text_in++;
	}

	*text_out = 0;

	return text_bak;
}

char *text_find_next_line(const char *text)
{
	do {
		if (*text == 0)
		{
			return NULL;
		}
	} while (*text++ != '\n');

	return (char *)text;
}

char *text_find_line(const char *text, int index)
{
	while (index > 0)
	{
		text = text_find_next_line(text);
		if (text == NULL)
		{
			return NULL;
		}

		index--;
	}

	return (char *)text;
}

char *text_get_line(const char *text, char *buff, int index)
{
	text = text_find_line(text, index);
	if (text == NULL)
	{
		return NULL;
	}

	while (*text == '\r')
	{
		text++;
	}

	while (*text && *text != '\n' && *text != '\r')
	{
		*buff++ = *text++;
	}

	*buff++ = 0;

	return buff;
}

int text_version_cmp(const char *v1, const char *v2, char sep)
{
	int result;

	while (*v1 == *v2 && *v1)
	{
		v1++, v2++;
	}

	result = *v1 - *v2;

	while (*v1 && *v2 && *v1 != sep && *v2 != sep)
	{
		v1++, v2++;
	}

	if ((*v1 == sep && *v2 == sep) || (*v1 == 0 && *v2 == 0))
	{
		return result;
	}

	if (*v1 == sep || *v1 == 0)
	{
		return -1;
	}

	return 1;
}

int text_has_char(const char *text, char c)
{
	while (*text)
	{
		if (*text == c)
		{
			return 1;
		}

		text++;
	}

	return 0;
}

char *text_skip_char(const char *text, char c)
{
	while (*text == c)
	{
		text++;
	}

	return (char *)text;
}

char *text_skip_chars(const char *text, const char *chars)
{
	while (text_has_char(chars, *text))
	{
		text++;
	}

	return (char *)text;
}

char *text_to_uppercase(const char *src, char *dest)
{
	while ((*dest = char2uppercase(*src)))
	{
		dest++;
		src++;
	}

	return dest;
}

char *text_to_lowercase(const char *src, char *dest)
{
	while ((*dest = char2lowercase(*src)))
	{
		dest++;
		src++;
	}

	return dest;
}

char *text_to_nuppercase(const char *src, char *dest, size_t size)
{
	const char *end_src;

	for (end_src = src + size; src < end_src && (*dest = char2uppercase(*src)); src++, dest++);

	return dest;
}

char *text_to_nlowercase(const char *src, char *dest, size_t size)
{
	const char *end_src;

	for (end_src = src + size; src < end_src && (*dest = char2lowercase(*src)); src++, dest++);

	return dest;
}

int text_cmp_nocase(const char *text1, const char *text2)
{
	while (*text1 && char2lowercase(*text1) == char2lowercase(*text2))
	{
		text1++;
		text2++;
	}

	return char2lowercase(*text1) - char2lowercase(*text2);
}

int text_ncmp_nocase(const char *text1, const char *text2, size_t size)
{
	const char *end_text1;

	if (size == 0)
	{
		return 0;
	}

	for (end_text1 = text1 + size - 1; text1 < end_text1 && *text1 && char2lowercase(*text1) == char2lowercase(*text2); text1++, text2++);

	return char2lowercase(*text1) - char2lowercase(*text2);
}

int text_bool_value(const char *text)
{
	const char *true_texts[] =
	{
		"1", "y", "yes", "true",
	};
	int i;

	if (text == NULL)
	{
		return 1;
	}

	for (i = 0; i < ARRAY_SIZE(true_texts); i++)
	{
		if (text_cmp_nocase(true_texts[i], text) == 0)
		{
			return 1;
		}
	}

	return 0;
}

void *mac_address_tostring_base(const void *mac, size_t maclen, void *buff)
{
	char *p;
	const void *mac_end;

	for (mac_end = mac + maclen - 1, p = buff; mac < mac_end; mac++)
	{
		p += sprintf(p, "%02x:", *(u8 *)mac);
	}

	return p + sprintf(p, "%02x", *(u8 *)mac);
}

char *mac_address_tostring(const void *mac, size_t maclen)
{
	static char buff[20];

	mac_address_tostring_base(mac, maclen, buff);

	return buff;
}

int text_is_number(const char *text)
{
	while (IS_NUMBER(*text))
	{
		text++;
	}

	return *text == 0;
}

int text_is_float(const char *text)
{
	while (IS_FLOAT(*text))
	{
		text++;
	}

	return *text == 0;
}

int text_is_uppercase(const char *text)
{
	while (IS_UPPERCASE(*text))
	{
		text++;
	}

	return *text == 0;
}

int text_is_lowercase(const char *text)
{
	while (IS_LOWERCASE(*text))
	{
		text++;
	}

	return *text == 0;
}

int text_is_letter(const char *text)
{
	while (IS_LETTER(*text))
	{
		text++;
	}

	return *text == 0;
}

char *text_replace_char(char *text, char c_src, char c_dest)
{
	char *text_bak;

	for (text_bak = text; *text; text++)
	{
		if (*text == c_src)
		{
			*text = c_dest;
		}
	}

	return text_bak;
}

char *text_replace_char2(const char *src, char *dest, char c_src, char c_dest)
{
	while (*src)
	{

		*dest = (*src == c_src) ? c_dest : *src;

		src++;
		dest++;
	}

	*dest = 0;

	return dest;
}

char *text_replace_text_base(const char *text_old, char *text_new, const char *src, size_t srclen, const char *dest)
{
	int steps[srclen];
	const char *old_end, *p;

	mem_build_kmp_array(src, steps, srclen);
	old_end = text_old + text_len(text_old);

	while (text_old < old_end)
	{
		p = mem_kmp_find_base(text_old, old_end, src, srclen, steps);
		if (p == NULL)
		{
			text_new = mem_copy2(text_new, text_old, old_end);
			break;
		}
		else
		{
			text_new = mem_copy2(text_new, text_old, p);
			text_new = text_copy(text_new, dest);
			text_old = p + srclen;
		}
	}

	*text_new = 0;

	return text_new;
}

int text_is_dot_name(const char *filename)
{
	if (*filename != '.')
	{
		return 0;
	}

	filename++;

	if (*filename == 0)
	{
		return 1;
	}

	return filename[0] == '.' && filename[1] == 0;
}

int text_isnot_dot_name(const char *filename)
{
	if (*filename != '.')
	{
		return 1;
	}

	filename++;

	if (*filename == 0)
	{
		return 0;
	}

	return filename[0] != '.' || filename[1] != 0;
}

size_t text_split_by_char(const char *text, char sep, void *buff, size_t size1, size_t size2)
{
	void *buff_end;
	char *p;

	for (p = buff, buff_end = buff + (size1 * size2), size1 = 1; *text && buff < buff_end; text++)
	{
		if (*text == sep)
		{
			*p = 0;
			buff += size2;
			p = buff;
			size1++;
		}
		else
		{
			*p++ = *text;
		}
	}

	*p = 0;

	return size1;
}

char *text_join_by_char(char *text[], size_t size1, char sep, void *buff, size_t size2)
{
	void *buff_end;
	char **text_last;

	for (buff_end = buff + size2, text_last = text + (size1 - 1); buff < buff_end; text++)
	{
		buff = text_copy(buff, *text);

		if (text < text_last)
		{
			*(char *)buff++ = sep;
		}
		else
		{
			break;
		}
	}

	return buff;
}

size_t text_split_by_text(const char *text, const char *sep, void *buff, size_t size1, size_t size2)
{
	void *buff_end;
	char *p;
	size_t sep_len;
	const char *text_end;

	sep_len = text_len(sep);
	text_end = text + text_len(text);

	for (buff_end = buff + (size1 * size2), size1 = 0; buff < buff_end && text < text_end; size1++)
	{
		p = mem_kmp_find(text, sep, text_end - text, sep_len);
		if (p == NULL)
		{
			break;
		}

		mem_copy(buff, text, p - text);
		buff += size2;
		text = p + sep_len;
	}

	if (text < text_end)
	{
		text_copy(buff, text);
		return size1 + 1;
	}

	return size1;
}

char *text_join_by_text(char *text[], size_t size1, const char *sep, void *buff, size_t size2)
{
	void *buff_end;
	char **text_last;

	for (buff_end = buff + size2, text_last = text + (size1 - 1); buff < buff_end; text++)
	{
		buff = text_copy(buff, *text);

		if (text < text_last)
		{
			buff = text_copy(buff, sep);
		}
		else
		{
			break;
		}
	}

	return buff;
}

int text_array_find(const char *text, char *buff[], int size)
{
	for (size--; size >= 0; size--)
	{
		if (text_cmp(text, buff[size]) == 0)
		{
			return size;
		}
	}

	return -1;
}

char *text_skip_space_head(const char *text, const char *line_end)
{
	while (text < line_end && BYTE_IS_SPACE(*text))
	{
		text++;
	}

	return (char *)text;
}

char *text_skip_space_tail(const char *text, const char *line)
{
	while (text > line && BYTE_IS_SPACE(*text))
	{
		text--;
	}

	return (char *)text;
}

char *text_find_line_end(const char *text, const char *file_end)
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

char *text_skip_line_end(const char *text, const char *file_end)
{
	while (text < file_end && BYTE_IS_LF(*text))
	{
		text++;
	}

	return (char *)text;
}
