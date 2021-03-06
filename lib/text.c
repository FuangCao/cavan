#include <cavan.h>
#include <cavan/text.h>
#include <cavan/file.h>
#include <cavan/alarm.h>
#include <cavan/ctype.h>
#include <sys/wait.h>

#define USE_SYSTEM_PRINTF	1

static const char VALUE_CHAR_MAP_UPPERCASE[] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

static const char VALUE_CHAR_MAP_LOWERCASE[] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

char *text_tail(const char *text)
{
	while (*text) {
		text++;
	}

	return (char *) text;
}

size_t text_len(const char *text)
{
	return text_tail(text) - text;
}

char *text_find(const char *text, char c)
{
	while (text[0]) {
		if (text[0] == c) {
			return (char *) text;
		}

		text++;
	}

	return NULL;
}

char *text_nfind(const char *text, char c, size_t count)
{
	while (text[0]) {
		if (text[0] == c) {
			if (count > 0) {
				count--;
			} else {
				return (char *) text;
			}
		}

		text++;
	}

	return NULL;
}

char *text_nfind2(const char *text, const char *end, char c, size_t count)
{
	while (text < end) {
		if (*text == c) {
			if (count > 0) {
				count--;
			} else {
				return (char *) text;
			}
		}

		text++;
	}

	return (char *) text;
}

char *text_find_tail(const char *text, char c)
{
	const char *tmp = NULL;

	while (text[0]) {
		if (text[0] == c) {
			tmp = text;
		}

		text++;
	}

	return (char *) tmp;
}

char *text_ntrans(char *text, size_t size)
{
	char temp;
	char *last;
	char *text_bak = text;

	for (last = text + size - 1; last > text; text++, last--) {
		temp = *text;
		*text = *last;
		*last = temp;
	}

	return text_bak;
}

char *text_cat(char *dest, const char *src)
{
	while (*dest) {
		dest++;
	}

	return text_copy(dest, src);
}

char *text_cat2(char *dest, char *srcs[], size_t count)
{
	char **src_end = srcs + count;

	while (srcs < src_end) {
		const char *p = *srcs++;

		while (*p) {
			*dest++ = *p++;
		}
	}

	*dest++ = 0;

	return dest;
}

char *text_vcat3(char *dest, size_t count, va_list ap)
{
	while (count--) {
		const char *p = va_arg(ap, const char *);

		while (*p) {
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
	while (srcs) {
		const char *p = *srcs++;

		while (*p) {
			*dest++ = *p++;
		}
	}

	*dest++ = 0;

	return dest;
}

char *text_vcat5(char *dest, va_list ap)
{
	while (1) {
		const char *p = va_arg(ap, const char *);

		if (p == NULL) {
			break;
		}

		while (*p) {
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

	while (1) {
		switch (*text) {
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

	while (1) {
		switch (*text) {
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
	while ((*dest = *src)) {
		dest++;
		src++;
	}

	return dest;
}

char *text_copy_lowercase(char *dest, const char *src)
{
	while (1) {
		switch (*src) {
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
	while (1) {
		switch (*src) {
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

	for (end_src = src + size - 1; src < end_src && *src; src++, dest++) {
		*dest = *src;
	}

	*dest = 0;

	return dest;
}

u16 *text_copy16(u16 *dest, const u16 *src, size_t count)
{
	const u16 *end_src = src + count;

	while (src < end_src) {
		*dest++ = *src++;
	}

	return dest;
}

u32 *text_copy32(u32 *dest, const u32 *src, size_t count)
{
	const u32 *end_src = src + count;

	while (src < end_src) {
		*dest++ = *src++;
	}

	return dest;
}

u64 *text_copy64(u64 *dest, const u64 *src, size_t count)
{
	const u64 *end_src = src + count;

	while (src < end_src) {
		*dest++ = *src++;
	}

	return dest;
}

int text_cmp(const char *text1, const char *text2)
{
	while (*text1 && *text1 == *text2) {
		text1++;
		text2++;
	}

	return *text1 - *text2;
}

int text_ncmp(const char *text1, const char *text2, size_t size)
{
	const char *end_text1;

	if (size == 0) {
		return 0;
	}

	for (end_text1 = text1 + size - 1; *text1 && text1 < end_text1 && *text1 == *text2; text1++, text2++);

	return *text1 - *text2;
}

int text_hcmp(const char *text1, const char *text2)
{
	while (*text1 && *text1 == *text2) {
		text1++;
		text2++;
	}

	return *text1 && *text2 ? *text1 - *text2 : 0;
}

int text_lhcmp(const char *text1, const char *text2)
{
	while (*text1) {
		if (*text1 != *text2) {
			return *text1 - *text2;
		}

		text1++;
		text2++;
	}

	return 0;
}

int text_rhcmp(const char *text1, const char *text2)
{
	while (*text2) {
		if (*text1 != *text2) {
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

	while (*text1) {
		text1++;
	}

	while (*text2) {
		text2++;
	}

	while (text1 >= text_start1 && text2 >= text_start2 && *text1 == *text2) {
		text1--;
		text2--;
	}

	if (text1 < text_start1) {
		if (text2 < text_start2) {
			return 0;
		} else {
			return -1;
		}
	} else if (text2 < text_start2) {
		if (text1 < text_start1) {
			return 0;
		} else {
			return 1;
		}
	} else {
		return *text1 - *text2;
	}
}

int text_ltcmp(const char *text1, const char *text2)
{
	const char *text_start1 = text1, *text_start2 = text2;

	while (*text1) {
		text1++;
	}

	while (*text2) {
		text2++;
	}

	while (text1 >= text_start1 && text2 >= text_start2 && *text1 == *text2) {
		text1--;
		text2--;
	}

	if (text1 < text_start1) {
		return 0;
	} else if (text2 < text_start2) {
		if (text1 < text_start1) {
			return 0;
		} else {
			return 1;
		}
	} else {
		return *text1 - *text2;
	}
}

int text_rtcmp(const char *text1, const char *text2)
{
	const char *text_start1 = text1, *text_start2 = text2;

	while (*text1) {
		text1++;
	}

	while (*text2) {
		text2++;
	}

	while (text1 >= text_start1 && text2 >= text_start2 && *text1 == *text2) {
		text1--;
		text2--;
	}

	if (text1 < text_start1) {
		if (text2 < text_start2) {
			return 0;
		} else {
			return -1;
		}
	} else if (text2 < text_start2) {
		return 0;
	} else {
		return *text1 - *text2;
	}
}

void text_show(const char *text, size_t size)
{
	const char *end_text;

	if (size == 0) {
		println("Empty");
		return;
	}

	for (end_text = text + size - 1; text < end_text; text++) {
		print("0x%02x, ", *(u8 *) text);
	}

	print("0x%02x,\n", *(u8 *) text);
}

void text_show16(const u16 *text, size_t count)
{
	const u16 *end_text;

	if (count == 0) {
		println("Empty");
		return;
	}

	for (end_text = text + count - 1; text < end_text; text++) {
		print("%04x ", *text);
	}

	print("%04x\n", *text);
}

void text_show32(const u32 *text, size_t count)
{
	const u32 *end_text;

	if (count == 0) {
		println("Empty");
		return;
	}

	for (end_text = text + count - 1; text < end_text; text++) {
		print("%08x ", *text);
	}

	print("%08x\n", *text);
}

void text_show64(const u64 *text, size_t count)
{
	const u64 *end_text = text + count;

	if (count == 0) {
		println("Empty");
		return;
	}

	for (end_text = text + count - 1; text < end_text; text++) {
#if __WORDSIZE == 64
		print("%016lx ", *text);
#else
		print("%016Lx ", *text);
#endif
	}

#if __WORDSIZE == 64
	print("%016lx\n", *text);
#else
	print("%016Lx\n", *text);
#endif
}

void *text_set8(u8 *text, u8 value, size_t count)
{
	u8 *end_text = text + count;

	while (text < end_text) {
		*text++ = value;
	}

	return text;
}

void *text_set16(u16 *text, u16 value, size_t count)
{
	u16 *end_text = text + count;

	while (text < end_text) {
		*text++ = value;
	}

	return text;
}

void *text_set32(u32 *text, u32 value, size_t count)
{
	u32 *end_text = text + count;

	while (text < end_text) {
		*text++ = value;
	}

	return text;
}

void *text_set64(u64 *text, u64 value, size_t count)
{
	u64 *end_text = text + count;

	while (text < end_text) {
		*text++ = value;
	}

	return text;
}

char *text_ndup(const char *text, size_t size)
{
	char *text_new;

	text_new = malloc(size);
	if (text_new == NULL) {
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
	if (text_new == NULL) {
		return NULL;
	}

	text_copy(text_new, text);

	return text_new;
}

char *text_tok(char *text, const char *delim)
{
	char *temp;
	static char *text_bak;

	if (text) {
		text_bak = text;
	} else if (text_bak == NULL) {
		return NULL;
	}

	temp = text_bak;
	while (*text_bak) {
		if (text_find(delim, *text_bak)) {
			*text_bak++ = 0;
			return temp;
		}

		text_bak++;
	}

	return temp;
}

int char2value(char c)
{
	switch (c) {
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

char value2char_uppercase(int value)
{
	return VALUE_CHAR_MAP_UPPERCASE[value]; // [value & 0x0F];
}

char value2char_lowercase(int value)
{
	return VALUE_CHAR_MAP_LOWERCASE[value]; // [value & 0x0F];
}

int text2byte(const char text[2])
{
	return char2value(text[0]) << 4 | char2value(text[1]);
}

int prefix2base(const char *prefix, const char *prefix_end, const char **last, int base)
{
	if (last) {
		*last = prefix;
	}

	if ((prefix_end && prefix + 1 >= prefix_end) || *prefix != '0') {
		return base > 0 ? base : 10;
	}

	prefix++;

	switch (*prefix) {
	case 'b':
	case 'B':
		if (base > 11) {
			return base;
		}

		prefix++;
		base = 2;
		break;

	case 'd':
	case 'D':
		if (base > 13) {
			return base;
		}
		prefix++;
	case '.':
		base = 10;
		break;

	case 'x':
	case 'X':
		prefix++;
		base = 16;
		break;

	case 'o':
	case 'O':
		prefix++;
		base = 8;
		break;

	case '0' ... '7':
		if (base > 8) {
			return base;
		}
		base = 8;
		break;

	case 0:
		return 10;

	default:
		if (base > 0) {
			return base;
		}
		return -EINVAL;
	}

	if (last) {
		*last = prefix;
	}

	return base;
}

u64 text2value_unsigned(const char *text, const char **last, int base)
{
	u64 value;
	int tmp;

	if (text == NULL) {
		return 0;
	}

	base = prefix2base2(text, &text, base);
	if (base < 0) {
		value = 0;
		pr_red_info("prefix2base");
		goto out_return;
	}

	for (value = 0; *text; text++) {
		tmp = char2value(*text);
		if (tmp < 0 || tmp >= base) {
			break;
		}

		value = value * base + tmp;
	}

out_return:
	if (last) {
		*last = text;
	}

	return value;
}

s64 text2value(const char *text, const char **last, int base)
{
	if (text == NULL) {
		if (last) {
			*last = NULL;
		}
		return 0;
	}

	if (*text == '-') {
		return -text2value_unsigned(text + 1, last, base);
	}

	return text2value_unsigned(text, last, base);
}

ulong text2ulong(const char *text, const char *text_end, int base)
{
	ulong value;

	for (value = 0; text < text_end; text++) {
		int bit = char2value(*text);
		if (bit < 0 || bit >= base) {
			break;
		}

		value = value * base + bit;
	}

	return value;
}

long text2long(const char *text, const char *text_end, int base)
{
	if (text < text_end && *text == '-') {
		return -text2ulong(text + 1, text_end, base);
	}

	return text2ulong(text, text_end, base);
}

int text2value_array(const char *text, const char *text_end, const char **last, char sep, int values[], size_t count, int base)
{
	int i;

	if (text_end == NULL) {
		text_end = text + strlen(text);
	}

	for (i = 0; i < (int) count && text < text_end; text++) {
		const char *last;

		text = text_skip_space(text, text_end);
		values[i++] = text2value(text, &last, base);
		text = text_skip_space(last, text_end);
		if (*text != sep) {
			break;
		}
	}

	if (last) {
		*last = text;
	}

	return i;
}

double text2double_unsigned(const char *text, const char *text_end, const char **last, int base)
{
	int value;
	double result, weight;

	if (text_end == NULL) {
		text_end = text + text_len(text);
	}

	if (text >= text_end) {
		result = 0;
		goto out_return;
	}

	value = prefix2base(text, text_end, &text, base);
	if (value > 1) {
		base = value;
	}

	for (result = 0; text < text_end; text++) {
		if (*text == '.') {
			text++;
			break;
		}

		value = char2value(*text);
		if (value < 0 || value >= base) {
			goto out_return;
		}

		result = result * base + value;
	}

	for (weight = 1.0 / base; text < text_end; text++) {
		value = char2value(*text);
		if (value < 0 || value >= base) {
			break;
		}

		result += value * weight;
		weight /= base;
	}

out_return:
	if (last) {
		*last = text;
	}

	return result;
}

double text2double(const char *text, const char *text_end, const char **last, int base)
{
	if (text == NULL) {
		if (last) {
			*last = NULL;
		}

		return 0;
	}

	if (*text == '-') {
		return -text2double_unsigned(text, text_end, last, base);
	}

	return text2double_unsigned(text, text_end, last, base);
}

char *reverse_value2text_base2(u64 value, char *buff, size_t size)
{
	char *buff_end;

	for (buff_end = buff + size - 1; buff < buff_end && value; buff++, value >>= 1) {
		*buff = (value & 0x01) + '0';
	}

	*buff = 0;

	return buff;
}

char *reverse_value2text_base4(u64 value, char *buff, size_t size)
{
	char *buff_end;

	for (buff_end = buff + size - 1; buff < buff_end && value; buff++, value >>= 2) {
		*buff = (value & 0x03) + '0';
	}

	*buff = 0;

	return buff;
}

char *reverse_value2text_base8(u64 value, char *buff, size_t size)
{
	char *buff_end;

	for (buff_end = buff + size - 1; buff < buff_end && value; buff++, value >>= 3) {
		*buff = (value & 0x07) + '0';
	}

	*buff = 0;

	return buff;
}

char *reverse_value2text_base16(u64 value, char *buff, size_t size)
{
	char *buff_end;

	for (buff_end = buff + size - 1; buff < buff_end && value; buff++, value >>= 4) {
		*buff = value2char_uppercase(value & 0x0F);
	}

	*buff = 0;

	return buff;
}

char *reverse_value2text_base32(u64 value, char *buff, size_t size)
{
	char *buff_end;

	for (buff_end = buff + size - 1; buff < buff_end && value; buff++, value >>= 5) {
		*buff = value2char_uppercase(value & 0x1F);
	}

	*buff = 0;

	return buff;
}

char *reverse_value2text_all(u64 value, char *buff, size_t size, int base)
{
	char *buff_end;

	for (buff_end = buff + size - 1; buff < buff_end && value; buff++, value /= base) {
		*buff = value2char_uppercase(value % base);
	}

	*buff = 0;

	return buff;
}

char *value2text_reverse_simple(u64 value, char *buff, size_t size, int base)
{
	if (value == 0) {
		*(u16 *) buff = 0x0030;
		return buff + 1;
	}

	switch (base) {
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

char *value2text_unsigned_simple(u64 value, char *buff, size_t size, int base)
{
	char *tail;

	tail = value2text_reverse_simple(value, buff, size, base);
	mem_reverse_simple((byte *) buff, (byte *) tail - 1);

	return tail;
}

char *value2text_simple(s64 value, char *buff, size_t size, int base)
{
	if (value < 0 && base == 10) {
		*buff = '-';
		return value2text_unsigned_simple(-value, buff + 1, size - 1, 10);
	}

	return value2text_unsigned_simple(value, buff, size, base);
}

char *value2text_array(int values[], size_t count, char sep, char *buff, char *buff_end, int base)
{
	int i;

	for (i = 0; i < (int) count && buff < buff_end; i++) {
		if (i > 0) {
			*buff++ = sep;
		}

		buff = value2text_simple(values[i], buff, buff_end - buff, base);
	}

	return buff;
}

char *base2prefix(int base, char *prefix)
{
	switch (base) {
	case 2:
		*(u16 *) prefix = 0x4230;
		prefix += 2;
		break;

	case 8:
		*prefix++ = '0';
		break;

	case 10:
		*(u16 *) prefix = 0x4430;
		prefix += 2;
		break;

	case 16:
		*(u16 *) prefix = 0x7830;
		prefix += 2;
		break;
	}

	*prefix = 0;

	return prefix;
}

char *base2prefix_reverse(char *text, size_t size, int base)
{
	if (size < 2) {
		if (size) {
			*text = 0;
		}

		return text;
	}

	switch (base) {
	case 2:
		*(u16 *) text = 0x3042;
		text += 2;
		break;

	case 8:
		*text++ = '0';
		break;

	case 10:
		*(u16 *) text = 0x3044;
		text += 2;
		break;

	case 16:
		*(u16 *) text = 0x3078;
		text += 2;
		break;
	}

	*text = 0;

	return text;
}

char *value2text_base(s64 value, char *text, int length, char fill, int flags)
{
	char buff[128], *tail;
	int base;

	base = flags & 0xFF;
	if (base == 10 && value < 0 && (flags & TEXT_FLAG_SIGNED)) {
		*text++ = '-';
		tail = value2text_reverse_simple(-value, buff, sizeof(buff), 10);
	} else {
		tail = value2text_reverse_simple(value, buff, sizeof(buff), base);
	}

	if (flags & TEXT_FLAG_PREFIX) {
		text = base2prefix(base, text);
	}

	length -= (tail - buff);
	if (length > 0) {
		char *text_end;

		if (fill == 0) {
			fill = '0';
		}

		for (text_end = text + length; text < text_end; text++) {
			*text = fill;
		}
	}

	for (tail--; tail >= buff; text++, tail--) {
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

char *value2bitlist(u64 value, char *buff, size_t size, const char *sep)
{
	int i;
	int count;
	char *buff_end = buff + size - 1;

	buff = text_ncopy(buff, "[ ", buff_end - buff);

	for (i = sizeof(value) * 8 - 1, count = 0; i >= 0 && buff < buff_end; i--) {
		if ((value & (((u64) 1) << i)) == 0) {
			continue;
		}

		if (count > 0) {
			buff += snprintf(buff, buff_end - buff, "%s%d", sep, i);
		} else {
			buff += snprintf(buff, buff_end - buff, "%d", i);
		}

		count++;
	}

	buff = text_ncopy(buff, " ]", buff_end - buff);

	return buff;
}

double text2size_single(const char *text, const char **last)
{
	double size;

	size = text2double_unsigned(text, NULL, &text, 10);

	switch (*text) {
	case 't':
	case 'T':
		size *= 1LL << 40;
		break;

	case 'g':
	case 'G':
		size *= 1 << 30;
		break;

	case 'm':
	case 'M':
		size *= 1 << 20;
		break;

	case 'k':
	case 'K':
		size *= 1 << 10;
		break;

	case 'b':
	case 'B':
	case 0:
		break;

	default:
		pr_err_info("illegal character \'%c\'", *text);
	}

	if (*text) {
		text++;
	}

	if (last) {
		*last = text;
	}

	return size;
}

double text2size(const char *text, const char **last)
{
	double size;

	if (text == NULL) {
		return 0;
	}

	size = 0;

	while (*text) {
		size += text2size_single(text, &text);
	}

	if (last) {
		*last = text;
	}

	return size;
}

double text2size_mb(const char *text)
{
	if (text == NULL) {
		return 0;
	}

	if (text_is_float(text)) {
		return text2double_unsigned(text, NULL, NULL, 10);
	} else {
		return text2size(text, NULL) / (1 << 20);
	}
}

char *size2text_base(u64 size, char *buff, size_t buff_len)
{
	u64 tmp;

	if (size == 0) {
		*buff++ = '0';
		goto out_return;
	}

	tmp = (size >> 40) & 0x3FF;
	if (tmp) {
		buff = value2text_simple(tmp, buff, buff_len, 10);
		*buff++ = 'T';
	}

	tmp = (size >> 30) & 0x3FF;
	if (tmp) {
		buff = value2text_simple(tmp, buff, buff_len, 10);
		*buff++ = 'G';
	}

	tmp = (size >> 20) & 0x3FF;
	if (tmp) {
		buff = value2text_simple(tmp, buff, buff_len, 10);
		*buff++ = 'M';
	}

	tmp = (size >> 10) & 0x3FF;
	if (tmp) {
		buff = value2text_simple(tmp, buff, buff_len, 10);
		*buff++ = 'k';
	}

	tmp = size & 0x3FF;
	if (tmp) {
		buff = value2text_simple(tmp, buff, buff_len, 10);
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

double text2clock(const char *text)
{
	const char *p;
	double clock;

	clock = text2double_unsigned(text, text + strlen(text), &p, 10);

	switch (*p) {
	case 'g':
	case 'G':
		clock *= 1000000000UL;
		break;

	case 'm':
	case 'M':
		clock *= 1000000UL;
		break;

	case 'k':
	case 'K':
		clock *= 1000UL;
		break;
	}

	return clock;
}

double text2time_single(const char *text, const char **last)
{
	double time;

	time = text2double_unsigned(text, NULL, &text, 10);

	switch (*text) {
	case 'y':
	case 'Y':
		time = TIME_YEAR(time);
		break;

	case 'w':
	case 'W':
		time = TIME_WEEK(time);
		break;

	case 'd':
	case 'D':
		time = TIME_DAY(time);
		break;

	case 'h':
	case 'H':
		time = TIME_HOUR(time);
		break;

	case 'm':
	case 'M':
		time = TIME_MINUTE(time);
		break;

	case 's':
	case 'S':
	case 0:
		time = TIME_SECOND(time);
		break;

	default:
		pr_err_info("illegal character \'%c\'", *text);
	}

	if (*text) {
		text++;
	}

	if (last) {
		*last = text;
	}

	return time;
}

double text2time(const char *text, const char **last)
{
	double time;

	if (text == NULL) {
		return 0;
	}

	time = 0;

	while (*text) {
		time += text2time_single(text, &text);
	}

	if (last) {
		*last = text;
	}

	return time;
}

int text2date(const char *text, struct tm *date, ...)
{
	va_list ap;

	va_start(ap, date);

	while (1) {
		const char *format;
		struct tm date_bak;

		format = va_arg(ap, const char *);
		if (format == NULL) {
			return -EINVAL;
		}

		date_bak = *date;

		if (strptime(text, format, date)) {
			pr_bold_info("Date Format = %s", format);
			break;
		}

		*date = date_bak;
	}

	return 0;
}

#if 0
static u64 get_argument(char *args, int size)
{
	u64 value;

	switch (size) {
	case 1:
		value = *(u8 *) args;
		break;

	case 2:
		value = *(u16 *) args;
		break;

	case 4:
		value = *(u32 *) args;
		break;

	case 8:
		value = *(u64 *) args;
		break;

	default:
		value = 0;
	}

	return value;
}

char *cavan_vsnprintf(char *buff, size_t size, const char *fmt, char *args)
{
	int text_len;
	int arg_len;
	int symbol;
	char fill;

	while (1) {
		switch(fmt[0]) {
		case 0:
			goto out_return;

		case '%':
			fmt++;
			text_len = 0;
			arg_len = 4;
			symbol = 1;
			fill = ' ';
start_parse:
			switch (fmt[0]) {
			case '0':
				fill = '0';
				fmt++;
			case '1' ... '9':
				while (fmt[0] >= '0' && fmt[0] <= '9') {
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
				buff = value2text_base(get_argument(args, arg_len), buff, text_len * symbol, fill, 2 | TEXT_FLAG_PREFIX);
				break;

			case 'o':
			case 'O':
				buff = value2text_base(get_argument(args, arg_len), buff, text_len * symbol, fill, 8 | TEXT_FLAG_PREFIX);
				break;

			case 'u':
			case 'U':
				buff = value2text_base(get_argument(args, arg_len), buff, text_len * symbol, fill, 10 | TEXT_FLAG_PREFIX);
				break;

			case 'd':
			case 'D':
			case 'i':
			case 'I':
				buff = value2text_base(get_argument(args, arg_len), buff, text_len * symbol, fill, 10 | TEXT_FLAG_SIGNED);
				break;

			case 'p':
			case 'P':
				buff = value2text_base(get_argument(args, arg_len), buff, text_len * symbol, fill, 16 | TEXT_FLAG_PREFIX);
				break;

			case 'x':
			case 'X':
				buff = value2text_base(get_argument(args, arg_len), buff, text_len * symbol, fill, 16);
				break;

			case 'c':
			case 'C':
				*buff++ = *(char *) args;
				break;

			case '%':
				*buff++ = '%';
				break;

			case 's':
			case 'S':
				buff = text_copy(buff, *(char **) args);
				break;

			default:
				pr_err_info("Unknown Formate");
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

char *cavan_printf(const char *fmt, ...)
{
	char *args;
	static char buff[1024];

	args = (char *) (((int *) &fmt) + 1);
	cavan_vsnprintf(buff, sizeof(buff), fmt, args);

	return buff;
}
#endif

char *format_text(const char *fmt, ...)
{
	va_list ap;
	static char buff[1024];

	va_start(ap, fmt);
	vsnprintf(buff, sizeof(buff), fmt, ap);
	va_end(ap);

	return buff;
}

#if 0
static int symbol_match_base(char c1, char c2)
{
	return 0;
}

static int symbol_match(char c1, char c2)
{
	if (c1 == c2) {
		return 1;
	}

	return symbol_match_base(c1, c2) || symbol_match_base(c2, c1);
}
#endif

int text_match(const char *text1, const char *text2)
{
	while (text1[0] && text2[0]) {
		switch (text1[0]) {
		case '\\':
			break;
		case '*':
			break;
		default:
			if (text1[0] != text2[0]) {
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

	while (src < src_end) {
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
	if (ftemp) {
		// system_sync();
	} else {
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
	if (ftemp) {
		// system_sync();
	} else {
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
	if (fpipe == NULL) {
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

	println("excute command `%s'", command);
	ret = system(command);
	if (ret == 0) {
		// system_sync();
	} else {
		println("`%s' faild", command);
	}

	return ret;
}

int vsystem_command(const char *command, va_list ap)
{
	int ret;
	char buff[1024];

	vsprintf(buff, command, ap);
	ret = system_command_simple(buff);
	if (ret == 0) {
		return 0;
	}

	if (WEXITSTATUS(ret) != 127) {
		return -1;
	}

	vsprintf(text_copy(buff, "PATH=" DEFAULT_PATH_VALUE ":${PATH}; "), command, ap);
	ret = system_command_simple(buff);
	if (ret == 0) {
		return 0;
	}

	if (WEXITSTATUS(ret) != 127) {
		return -1;
	}

	vsprintf(text_copy(buff, "/bin/busybox "), command, ap);
	ret = system_command_simple(buff);
	if (ret == 0) {
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

char *mem2text_base(const void *mem, size_t mem_size, char *buff, int size)
{
	const uchar *p, *ep;
	char *buff_end;

	p = mem;
	ep = p + mem_size;
	buff_end = buff + size - 2;

	while (p < ep && buff < buff_end) {
		buff = value2text_base(*p++, buff, 2, 0, 16);
	}

	buff[0] = 0;

	return buff;
}

char *mem2text(const void *mem, int size)
{
	static char buff[1024];

	mem2text_base(mem, size, buff, sizeof(buff));

	return buff;
}

void system_sync(void)
{
	print("Synchronization ... ");

	sync();

	if (system("PATH=" DEFAULT_PATH_VALUE ":${PATH}; sync") == 0)
	{
		println("OK");
	} else {
		println("Failed");
	}
}

char *text_delete_char_base(const char *text_in, char *text_out, char c)
{
	char *text_bak;

	text_bak = text_out;

	while (*text_in) {
		if (*text_in == c) {
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

	while (*text_in) {
		char *text_tmp;

		text_tmp = mem_kmp_find_base(text_in, text_in + text_len(text_in), sub, sublen, step);
		if (text_tmp == NULL) {
			break;
		}

		while (text_in < text_tmp) {
			*text_out++ = *text_in++;
		}

		text_in += sublen;
	}

	while (*text_in) {
		*text_out++ = *text_in++;
	}

	*text_out = 0;

	return text_bak;
}

char *text_find_next_line(const char *text)
{
	do {
		if (*text == 0) {
			return NULL;
		}
	} while (*text++ != '\n');

	return (char *) text;
}

char *text_find_line(const char *text, int index)
{
	while (index > 0) {
		text = text_find_next_line(text);
		if (text == NULL) {
			return NULL;
		}

		index--;
	}

	return (char *) text;
}

char *text_get_line(const char *text, char *buff, int index)
{
	text = text_find_line(text, index);
	if (text == NULL) {
		return NULL;
	}

	while (*text == '\r') {
		text++;
	}

	while (*text && *text != '\n' && *text != '\r') {
		*buff++ = *text++;
	}

	*buff++ = 0;

	return buff;
}

int text_version_cmp(const char *v1, const char *v2, char sep)
{
	int result;

	while (*v1 == *v2 && *v1) {
		v1++, v2++;
	}

	result = *v1 - *v2;

	while (*v1 && *v2 && *v1 != sep && *v2 != sep) {
		v1++, v2++;
	}

	if ((*v1 == sep && *v2 == sep) || (*v1 == 0 && *v2 == 0)) {
		return result;
	}

	if (*v1 == sep || *v1 == 0) {
		return -1;
	}

	return 1;
}

int text_has_char(const char *text, char c)
{
	while (*text) {
		if (*text == c) {
			return 1;
		}

		text++;
	}

	return 0;
}

char *text_skip_char(const char *text, char c)
{
	while (*text == c) {
		text++;
	}

	return (char *) text;
}

char *text_skip_chars(const char *text, const char *chars)
{
	while (text_has_char(chars, *text)) {
		text++;
	}

	return (char *) text;
}

char *text_to_uppercase(const char *src, char *dest)
{
	while ((*dest = cavan_uppercase(*src))) {
		dest++;
		src++;
	}

	return dest;
}

char *text_to_lowercase(const char *src, char *dest)
{
	while ((*dest = cavan_lowercase(*src))) {
		dest++;
		src++;
	}

	return dest;
}

char *text_to_nuppercase(const char *src, char *dest, size_t size)
{
	const char *end_src;

	for (end_src = src + size; src < end_src && (*dest = cavan_uppercase(*src)); src++, dest++);

	return dest;
}

char *text_to_nlowercase(const char *src, char *dest, size_t size)
{
	const char *end_src;

	for (end_src = src + size; src < end_src && (*dest = cavan_lowercase(*src)); src++, dest++);

	return dest;
}

int text_cmp_nocase(const char *text1, const char *text2)
{
	while (1) {
		int diff = cavan_lowercase_simple(*text1) - cavan_lowercase_simple(*text2);
		if (diff) {
			return diff;
		}

		if (*text1 == 0) {
			return 0;
		}

		text1++;
		text2++;
	}
}

int text_ncmp_nocase(const char *text1, const char *text2, size_t size)
{
	const char *end_text1;

	if (size == 0) {
		return 0;
	}

	end_text1 = text1 + size - 1;

	while (text1 < end_text1) {
		int diff = cavan_lowercase_simple(*text1) == cavan_lowercase_simple(*text2);
		if (diff) {
			return diff;
		}

		text1++;
		text2++;
	}

	return 0;
}

bool text2bool(const char *text)
{
	unsigned int i;
	const char *true_texts[] = { "y", "yes", "true" };

	if (text == NULL) {
		return false;
	}

	if (text_is_number(text)) {
		return text2value_unsigned(text, NULL, 10) != 0;
	}

	for (i = 0; i < ARRAY_SIZE(true_texts); i++) {
		if (text_cmp_nocase(true_texts[i], text) == 0) {
			return true;
		}
	}

	return false;
}

char *mac_address_tostring_base(const char *mac, size_t maclen, char *buff)
{
	char *p;
	const char *mac_end;

	for (mac_end = mac + maclen - 1, p = buff; mac < mac_end; mac++) {
		p += sprintf(p, "%02x:", *(u8 *) mac);
	}

	return p + sprintf(p, "%02x", *(u8 *) mac);
}

char *mac_address_tostring(const char *mac, size_t maclen)
{
	static char buff[20];

	mac_address_tostring_base(mac, maclen, buff);

	return buff;
}

int text_is_number(const char *text)
{
	while (cavan_isdigit(*text)) {
		text++;
	}

	return *text == 0;
}

int text_is_float(const char *text)
{
	while (cavan_isfloat(*text)) {
		text++;
	}

	return *text == 0;
}

int text_is_uppercase(const char *text)
{
	while (cavan_isupper(*text)) {
		text++;
	}

	return *text == 0;
}

int text_is_lowercase(const char *text)
{
	while (cavan_islower(*text)) {
		text++;
	}

	return *text == 0;
}

int text_is_letter(const char *text)
{
	while (cavan_isletter(*text)) {
		text++;
	}

	return *text == 0;
}

char *text_replace_char(char *text, char c_src, char c_dest)
{
	char *text_bak;

	for (text_bak = text; *text; text++) {
		if (*text == c_src) {
			*text = c_dest;
		}
	}

	return text_bak;
}

char *text_replace_char2(const char *src, char *dest, char c_src, char c_dest)
{
	while (*src) {

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

	while (text_old < old_end) {
		p = mem_kmp_find_base(text_old, old_end, src, srclen, steps);
		if (p == NULL) {
			text_new = mem_copy(text_new, text_old, old_end - text_old);
			break;
		} else {
			text_new = mem_copy(text_new, text_old, p - text_old);
			text_new = text_copy(text_new, dest);
			text_old = p + srclen;
		}
	}

	*text_new = 0;

	return text_new;
}

int text_split_by_char(char *text, char sep, char *texts[], int size)
{
	int index;

	if (size <= 0) {
		return 0;
	}

	texts[0] = text;

	for (index = 0; *text; text++) {
		if (*text == sep) {
			*text = 0;

			if (++index < size) {
				texts[index] = text + 1;
			} else {
				return size;
			}
		}
	}

	if (texts[index] < text) {
		return index + 1;
	}

	return index;
}

int text_split_by_space(char *text, char *texts[], int size)
{
	int index;

	if (size <= 0) {
		return 0;
	}

	index = 0;
	texts[0] = text;

	while (1) {
		switch (*text) {
		case 0:
			if (texts[index] < text) {
				text_clear_space_and_lf_invert(texts[index], text - 1);
				return index + 1;
			} else {
				return index;
			}

		case ' ':
		case '\t':
		case '\f':
		case '\r':
		case '\n':
			if (texts[index] < text) {
				text_clear_space_and_lf_invert(texts[index], text - 1);
				*text = 0;

				if (++index < size) {
					texts[index] = text + 1;
				} else {
					return size;
				}
			} else {
				texts[index] = text + 1;
			}
			break;
		}

		text++;
	}
}

char *text_join_by_char(char *text[], size_t size1, char sep, char *buff, size_t size2)
{
	char *buff_end;
	char **text_end;

	for (buff_end = buff + size2, text_end = text + size1; buff < buff_end && text < text_end; text++, buff++) {
		buff = text_copy(buff, *text);
		*buff = sep;
	}

	if (size1) {
		buff--;
	}

	*buff = 0;

	return buff;
}

size_t text_split_by_text(const char *text, const char *sep, char *buff, size_t size1, size_t size2)
{
	char *buff_end;
	char *p;
	size_t sep_len;
	const char *text_end;

	sep_len = text_len(sep);
	text_end = text + text_len(text);

	for (buff_end = buff + (size1 * size2), size1 = 0; buff < buff_end && text < text_end; size1++) {
		p = mem_kmp_find(text, sep, text_end - text, sep_len);
		if (p == NULL) {
			break;
		}

		mem_copy(buff, text, p - text);
		buff += size2;
		text = p + sep_len;
	}

	if (text < text_end) {
		text_copy(buff, text);
		return size1 + 1;
	}

	return size1;
}

char *text_join_by_text(char *text[], size_t size1, const char *sep, char *buff, size_t size2)
{
	char *buff_end;
	char **text_last;

	for (buff_end = buff + size2, text_last = text + (size1 - 1); buff < buff_end; text++) {
		buff = text_copy(buff, *text);

		if (text < text_last) {
			buff = text_copy(buff, sep);
		} else {
			break;
		}
	}

	return buff;
}

int text_array_find(const char *text, char *buff[], int size)
{
	for (size--; size >= 0; size--) {
		if (text_cmp(text, buff[size]) == 0) {
			return size;
		}
	}

	return -1;
}

char *text_strip(const char *text, size_t length, char *buff, size_t size)
{
	char *buff_end = buff + size - 1;
	const char *last = text + length - 1;

	while (cavan_isspace(*text)) {
		text++;
	}

	while (last >= text && cavan_isspace(*last)) {
		last--;
	}

	while (text <= last && buff < buff_end) {
		*buff++ = *text++;
	}

	*buff = 0;

	return buff;
}

char *text_skip_space(const char *text, const char *text_end)
{
	while (text < text_end && cavan_isspace(*text)) {
		text++;
	}

	return (char *) text;
}

char *text_skip_space2(const char *text)
{
	while (cavan_isspace(*text)) {
		text++;
	}

	return (char *) text;
}

char *text_skip_space_invert(const char *text, const char *head)
{
	while (text >= head && cavan_isspace(*text)) {
		text--;
	}

	return (char *) text;
}

char *text_skip_lf(const char *text, const char *text_end)
{
	while (text < text_end && cavan_islf(*text)) {
		text++;
	}

	return (char *) text;
}

char *text_skip_lf_invert(const char *text, const char *head)
{
	while (text >= head && cavan_islf(*text)) {
		text--;
	}

	return (char *) text;
}

char *text_skip_space_and_lf(const char *text, const char *text_end)
{
	while (text < text_end && cavan_isspace_lf(*text)) {
		text++;
	}

	return (char *) text;
}

char *text_skip_space_and_lf_invert(const char *text, const char *head)
{
	while (text >= head && cavan_isspace_lf(*text)) {
		text--;
	}

	return (char *) text;
}

char *text_skip_line(const char *text, const char *text_end)
{
	while (text < text_end) {
		if (cavan_islf(*text)) {
			break;
		}

		text++;
	}

	return text_skip_lf(text, text_end);
}

char *text_skip_name(const char *text, const char *text_end)
{
	while (text < text_end && cavan_isnameable(*text)) {
		text++;
	}

	return (char *) text;
}

char *text_skip_name_invert(const char *text, const char *head)
{
	while (text >= head && cavan_isnameable(*text)) {
		text--;
	}

	return (char *) text;
}

char *text_find_space(const char *text, const char *text_end)
{
	while (text < text_end) {
		if (cavan_isspace(*text)) {
			return (char *) text;
		}

		text++;
	}

	return NULL;
}

char *text_find_space2(const char *text)
{
	while (1) {
		switch (*text) {
		case 0:
			return NULL;

		case ' ':
		case '\t':
		case '\f':
			return (char *) text;
		}

		text++;
	}
}

char *text_find_space_invert(const char *text, const char *head)
{
	while (text >= head) {
		if (cavan_isspace(*text)) {
			return (char *) text;
		}

		text--;
	}

	return NULL;
}

char *text_find_lf(const char *text, const char *text_end)
{
	while (text < text_end) {
		if (cavan_islf(*text)) {
			return (char *) text;
		}

		text++;
	}

	return NULL;
}

char *text_find_lf_invert(const char *text, const char *head)
{
	while (text >= head) {
		if (cavan_islf(*text)) {
			return (char *) text;
		}

		text--;
	}

	return NULL;
}

char *text_find_space_or_lf(const char *text, const char *text_end)
{
	while (text < text_end) {
		if (cavan_isspace_lf(*text)) {
			return (char *) text;
		}

		text++;
	}

	return NULL;
}

char *text_find_space_or_lf2(const char *text)
{
	while (1) {
		switch (*text) {
		case 0:
			return NULL;

		case ' ':
		case '\t':
		case '\f':
		case '\r':
		case '\n':
			return (char *) text;
		}

		text++;
	}
}

char *text_find_space_or_lf_invert(const char *text, const char *head)
{
	while (text >= head) {
		if (cavan_isspace_lf(*text)) {
			return (char *) text;
		}

		text--;
	}

	return NULL;
}

char *text_find_name(const char *text, const char *text_end)
{
	while (text < text_end) {
		if (cavan_isnameable(*text)) {
			return (char *) text;
		}

		text++;
	}

	return NULL;
}

char *text_find_name_invert(const char *text, const char *head)
{
	while (text >= head) {
		if (cavan_isnameable(*text)) {
			return (char *) text;
		}

		text--;
	}

	return NULL;
}

char *text_clear_space(char *text)
{
	while (cavan_isspace(*text)) {
		*text++ = 0;
	}

	return text;
}

char *text_clear_space_and_lf(char *text)
{
	while (cavan_isspace_lf(*text)) {
		*text++ = 0;
	}

	return text;
}

char *text_clear_quote(char *text)
{
	while (cavan_isquote(*text)) {
		*text++ = 0;
	}

	return text;
}

char *text_clear_space_and_quote(char *text)
{
	text = text_clear_space(text);
	return text_clear_quote(text);
}

char *text_clear_space_and_lf_and_quote(char *text)
{
	text = text_clear_space_and_lf(text);
	return text_clear_quote(text);
}

char *text_clear_space_invert(const char *head, char *text)
{
	while (text >= head && cavan_isspace(*text)) {
		*text-- = 0;
	}

	return text;
}

char *text_clear_space_and_lf_invert(const char *head, char *text)
{
	while (text >= head && cavan_isspace_lf(*text)) {
		*text-- = 0;
	}

	return text;
}

char *text_clear_quote_invert(const char *head, char *text)
{
	while (text >= head && cavan_isquote(*text)) {
		*text-- = 0;
	}

	return text;
}

char *text_clear_space_and_quote_invert(const char *head, char *text)
{
	text = text_clear_space_invert(head, text);
	return text_clear_quote_invert(head, text);
}

char *text_clear_space_and_lf_and_quote_invert(const char *head, char *text)
{
	text = text_clear_space_and_lf_invert(head, text);
	return text_clear_quote_invert(head, text);
}

const char *text_fixup_null(const char *text, const char *text_null)
{
	if (text == NULL) {
		return text_null;
	}

	return text;
}

const char *text_fixup_null_simple(const char *text)
{
	return text_fixup_null(text, "");
}

const char *text_fixup_empty(const char *text, const char *text_empty)
{
	if (text == NULL || text[0] == 0) {
		return text_empty;
	}

	return text;
}

const char *text_fixup_empty_simple(const char *text)
{
	return text_fixup_empty(text, NULL);
}

char text_get_char(const char *text, int index)
{
	if (index < 0) {
		while (*text) {
			text++;
		}
	}

	return *(text + index);
}

char text_get_escope_letter(char c)
{
	switch (c) {
	case 't':
		return '\t';

	case 'n':
		return '\n';

	case 'r':
		return '\r';

	case 'f':
		return '\f';

	case 'b':
		return '\b';

	default:
		return c;
	}
}

int text2array(char *text, u32 *array, size_t size, char sep)
{
	u32 *ap, *ap_end;

	for (ap = array, ap_end = ap + size; ap < ap_end; ap++) {
		char *p;

		for (p = text; *p != sep; p++) {
			if (*p == 0) {
				*ap = text2value_unsigned(text, NULL, 10);

				return ap - array + 1;
			}
		}

		*p = 0;
		*ap = text2value_unsigned(text, NULL, 10);
		text = p + 1;
	}

	return ap - array;
}

char *frequency_tostring(double freq, char *buff, size_t size, char **last)
{
	if (freq < 1000) {
		size = snprintf(buff, size, "%lf Hz", freq);
	} else if (freq < 1000000) {
		size = snprintf(buff, size, "%lf kHz", freq / 1000);
	} else if (freq < 1000000000) {
		size = snprintf(buff, size, "%lf MHz", freq / 1000000);
	} else {
		size = snprintf(buff, size, "%lf GHz", freq / 1000000000);
	}

	if (last) {
		*last = buff + size;
	}

	return buff;
}

u32 frequency_unit2value(const char *text, const char *text_end)
{
	u32 freq;

	if (text_end == NULL) {
		text_end = text + strlen(text);
	}

	text = text_skip_space(text, text_end);

	switch (text[0]) {
	case 'k':
	case 'K':
		freq = 1000;
		text++;
		break;

	case 'm':
	case 'M':
		freq = 1000000;
		text++;
		break;

	case 'g':
	case 'G':
		freq = 1000000000;
		text++;
		break;

	default:
		freq = 1;
	}

	if (*text == 0 || strcmp(text, "Hz") == 0) {
		return freq;
	}

	pr_red_info("Invalid frequency unit %s", text);

	return 0;
}

double text2frequency(const char *text, const char *text_end, const char **last)
{
	if (text_end == NULL) {
		text_end = text + strlen(text);
	}

	return text2double_unsigned(text, text_end, &text, 10) * frequency_unit2value(text, text_end);
}

char *time2text_msec(u64 msec, char *buff, size_t size)
{
	char *buff_end = buff + size;

	if (msec >= 86400000) {
		buff += snprintf(buff, buff_end - buff, "%dD ", (int) (msec / 86400000));
		msec %= 86400000;
	}

	buff += snprintf(buff, buff_end - buff, "%02d:", (int) (msec / 3600000));
	msec %= 3600000;
	buff += snprintf(buff, buff_end - buff, "%02d:", (int) (msec / 60000));
	msec %= 60000;
	buff += snprintf(buff, buff_end - buff, "%0.2f", ((float) msec) / 1000);

	return buff;
}

char *time2text_sec(u32 msec, char *buff, size_t size)
{
	char *buff_end = buff + size;

	if (msec >= 86400) {
		buff += snprintf(buff, buff_end - buff, "%dD ", msec / 86400);
		msec %= 86400;
	}

	buff += snprintf(buff, buff_end - buff, "%02d:", msec / 3600);
	msec %= 3600;
	buff += snprintf(buff, buff_end - buff, "%02d:", msec / 60);
	msec %= 60;
	buff += snprintf(buff, buff_end - buff, "%02d", msec);

	return buff;
}

int cavan_string_init(cavan_string_t *str, const char *text, int length)
{
	if (text == NULL) {
		if (length > 0) {
			str->text = malloc(length);
			if (str->text == NULL) {
				pr_err_info("malloc");
				return -ENOMEM;
			}

			str->text[0] = 0;
		} else {
			str->text = "";
		}

		str->length = 0;
	} else {
		str->text = (char *) text;
		str->length = length;
		length = 0;
	}

	str->allocated = length;

	return 0;
}

void cavan_string_deinit(cavan_string_t *str)
{
	if (str->allocated > 0) {
		free(str->text);
		str->allocated = 0;
		str->text = "";
		str->length = 0;
	}
}

int cavan_string_alloc(cavan_string_t *str, int size, bool force)
{
	if (force || str->allocated <= size) {
		int total = size + 1;

		if (str->allocated > 0) {
			free(str->text);
		}

		str->text = malloc(total);
		if (str->text == NULL) {
			pr_err_info("malloc");
			return -ENOMEM;
		}

		str->allocated = total;
	}

	str->text[size] = 0;
	str->length = size;

	return 0;
}

bool cavan_string_extand(cavan_string_t *str, int length)
{
	char *mem;
	int size;

	if (str->allocated > length) {
		return true;
	}

	for (size = 32; size <= length; size <<= 1);

	mem = malloc(size);
	if (mem == NULL) {
		return false;
	}

	if (str->length > 0) {
		memcpy(mem, str->text, str->length);
	}

	mem[str->length] = 0;

	if (str->allocated > 0) {
		free(str->text);
	}

	str->allocated = size;
	str->text = mem;

	return true;
}

int cavan_string_append(cavan_string_t *str, const char *text, int size)
{
	int used = str->length + size;

	if (!cavan_string_extand(str, used)) {
		return -ENOMEM;
	}

	memcpy(str->text + str->length, text, size);
	str->text[used] = 0;
	str->length = used;

	return size;
}

int cavan_string_append_line_unix(cavan_string_t *str, const char *line, int size)
{
	int ret;

	ret = cavan_string_append(str, line, size);
	if (ret < 0) {
		return ret;
	}

	return cavan_string_append_line_end_unix(str);
}

int cavan_string_append_line_dos(cavan_string_t *str, const char *line, int size)
{
	int ret;

	ret = cavan_string_append(str, line, size);
	if (ret < 0) {
		return ret;
	}

	return cavan_string_append_line_end_dos(str);
}

int cavan_string_assign(cavan_string_t *str, const char *text, int size)
{
	str->length = 0;

	return cavan_string_append(str, text, size);
}

void cavan_string_clear(cavan_string_t *str, bool depth)
{
	str->length = 0;

	if (str->allocated > 0) {
		char *text = str->text;

		if (depth) {
			str->allocated = 0;
			str->text = "";
			free(text);
		} else {
			*text = 0;
		}
	} else {
		str->text = "";
	}
}

int cavan_string_cmp(const char *text1, int len1, const char *text2, int len2)
{
	if (len1 != len2) {
		return len1 - len2;
	}

	return strncmp(text1, text2, len1);
}

int cavan_string_cmp2(const char *text1, int len1, const char *text2)
{
	const char *text1_end = text1 + len1;

	while (text1 < text1_end) {
		if (*text1 != *text2) {
			return *text1 - *text2;
		}

		text1++;
		text2++;
	}

	if (*text2 == 0) {
		return 0;
	}

	return -1;
}

int cavan_string_casecmp(const char *text1, int len1, const char *text2, int len2)
{
	if (len1 != len2) {
		return len1 - len2;
	}

	return strncasecmp(text1, text2, len1);
}

int cavan_string_casecmp2(const char *text1, int len1, const char *text2)
{
	const char *text1_end = text1 + len1;

	while (text1 < text1_end) {
		if ((*text1 | 0x20) != (*text2 | 0x20)) {
			return *text1 - *text2;
		}

		text1++;
		text2++;
	}

	if (*text2 == 0) {
		return 0;
	}

	return -1;
}


bool cavan_string_startswith(const char *text1, int len1, const char *text2, int len2)
{
	const char *text1_end = text1 + len1;
	const char *text2_end = text2 + len2;

	while (text1 < text1_end) {
		if (text2 < text2_end) {
			if (*text1 != *text2) {
				return false;
			}
		} else {
			return true;
		}

		text1++;
		text2++;
	}

	if (text2 < text2_end) {
		return false;
	}

	return true;
}

bool cavan_string_startswith2(const char *text1, int len1, const char *text2)
{
	const char *text1_end = text1 + len1;

	while (text1 < text1_end) {
		if (*text1 != *text2) {
			if (*text2 == 0) {
				return true;
			}

			return false;
		}

		text1++;
		text2++;
	}

	return (*text2 == 0);
}

char *cavan_string_trim_start(const char *text, int length)
{
	const char *text_end = text + length;

	while (text < text_end && cavan_isspace_lf(*text)) {
		text++;
	}

	return (char *) text;
}

int cavan_string_trim_end(const char *text, int length)
{
	const char *text_end = text + length;

	while (text_end > text) {
		const char *p = text_end - 1;

		if (!cavan_isspace_lf(*p)) {
			break;
		}

		text_end = p;
	}

	return text_end - text;
}

char *cavan_string_trim(const char *text, int *length)
{
	const char *text_end = text + (*length);

	while (text < text_end && cavan_isspace_lf(*text)) {
		text++;
	}

	while (text_end > text) {
		const char *p = text_end - 1;

		if (!cavan_isspace_lf(*p)) {
			break;
		}

		text_end = p;
	}

	*length = text_end - text;

	return (char *) text;
}
