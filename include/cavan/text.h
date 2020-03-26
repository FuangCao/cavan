#pragma once

#include <cavan.h>
#include <stdio.h>

__BEGIN_DECLS;

#define PRINT_BUFFER_LEN	KB(10)
#define TEXT_FLAG_SIGNED	(1 << 9)
#define TEXT_FLAG_PREFIX	(1 << 10)

#define IS_NUMBER(c)		((c) >= '0' && (c) <= '9')
#define IS_FLOAT(c)			((c) == '.' || IS_NUMBER(c))

#define IS_LOWERCASE_AZ(c)	((c) >= 'a' && (c) <= 'z')
#define IS_UPPERCASE_AZ(c)	((c) >= 'A' && (c) <= 'Z')
#define IS_LETTER_AZ(c)		(IS_LOWERCASE_AZ(c) || IS_UPPERCASE_AZ(c))

#define IS_LOWERCASE_AF(c)	((c) >= 'a' && (c) <= 'f')
#define IS_UPPERCASE_AF(c)	((c) >= 'A' && (c) <= 'F')
#define IS_LETTER_AF(c)		(IS_LOWERCASE_AF(c) || IS_UPPERCASE_AF(c))

#define IS_LOWERCASE(c)		IS_LOWERCASE_AZ(c)
#define IS_UPPERCASE(c)		IS_UPPERCASE_AZ(c)
#define IS_LETTER(c)		IS_LETTER_AZ(c)

#define DEFAULT_PATH_VALUE	"/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin:/mybin"

#define value_trans(value) \
	({ \
		typeof_base(value) __tmp = value; \
		text_ntrans((char *) &__tmp, sizeof(__tmp)); \
		__tmp = __tmp; \
	})

#define CAVAN_TEXT_PAIR(text) \
	(text), (sizeof(text) - 1)

#define CAVAN_STRING_INITIALIZER(text) \
	{ CAVAN_TEXT_PAIR(text), 0 }

typedef struct {
	char *text;
	int length;
	int allocated;
} cavan_string_t;

char *text_tail(const char *text);
size_t text_len(const char *text);
char *text_cat(char *dest, const char *src);
char *text_cat2(char *dest, char *srcs[], size_t count);
char *text_vcat3(char *dest, size_t count, va_list ap);
char *text_cat3(char *dest, size_t count, ...);
char *text_cat4(char *dest, char *srcs[]);
char *text_vcat5(char *dest, va_list ap);
char *text_cat5(char *dest, ...);

char *text2lowercase(char *text);
char *text2uppercase(char *text);
char *text_copy_lowercase(char *dest, const char *src);
char *text_copy_uppercase(char *dest, const char *src);

char *text_copy(char *dest, const char *src);
char *text_ncopy(char *dest, const char *src, size_t count);
u16 *text_copy16(u16 *dest, const u16 *src, size_t count);
u32 *text_copy32(u32 *dest, const u32 *src, size_t count);
u64 *text_copy64(u64 *dest, const u64 *src, size_t count);

void text_show(const char *text, size_t count);
void text_show16(const u16 *text, size_t count);
void text_show32(const u32 *text, size_t count);
void text_show64(const u64 *text, size_t count);

char *text_find(const char *text, char c);
char *text_nfind(const char *text, char c, size_t count);
char *text_nfind2(const char *text, const char *end, char c, size_t count);
char *text_ntrans(char *text, size_t size);
char *text_find_tail(const char *text, char c);

int text_cmp(const char *text1, const char *text2);
int text_hcmp(const char *text1, const char *text2);
int text_lhcmp(const char *text1, const char *text2);
int text_rhcmp(const char *text1, const char *text2);
int text_tcmp(const char *text1, const char *text2);
int text_ltcmp(const char *text1, const char *text2);
int text_rtcmp(const char *text1, const char *text2);
int text_ncmp(const char *text1, const char *text2, size_t size);

void *text_set8(u8 *text, u8 value, size_t count);
void *text_set16(u16 *text, u16 value, size_t count);
void *text_set32(u32 *text, u32 value, size_t count);
void *text_set64(u64 *text, u64 value, size_t count);

char *text_ndup(const char *text, size_t size);
char *text_dup(const char *text);
char *text_tok(char *text, const char *delim);

int char2value(char c);
char value2char_uppercase(int value);
char value2char_lowercase(int value);
int text2byte(const char text[2]);
int prefix2base(const char *prefix, const char *prefix_end, const char **last, int base);
u64 text2value_unsigned(const char *text, const char **last, int base);
s64 text2value(const char *text, const char **last, int base);
ulong text2ulong(const char *text, const char *text_end, int base);
long text2long(const char *text, const char *text_end, int base);
int text2value_array(const char *text, const char *text_end, const char **last, char sep, int values[], size_t count, int base);
double text2double_unsigned(const char *text, const char *text_end, const char **last, int base);
double text2double(const char *text, const char *text_end, const char **last, int base);

char *reverse_value2text_base2(u64 value, char *buff, size_t size);
char *reverse_value2text_base4(u64 value, char *buff, size_t size);
char *reverse_value2text_base8(u64 value, char *buff, size_t size);
char *reverse_value2text_base16(u64 value, char *buff, size_t size);
char *reverse_value2text_base32(u64 value, char *buff, size_t size);
char *reverse_value2text_all(u64 value, char *buff, size_t size, int base);
char *value2text_reverse_simple(u64 value, char *buff, size_t size, int base);
char *value2text_unsigned_simple(u64 value, char *buff, size_t size, int base);
char *value2text_simple(s64 value, char *buff, size_t size, int base);
char *value2text_array(int values[], size_t count, char sep, char *buff, char *buff_end, int base);

char *value2text_base(s64 value, char *text, int length, char fill, int flags);
char *value2text(u64 value, int flags);
char *value2bitlist(u64 value, char *buff, size_t size, const char *sep);

char *base2prefix(int base, char *prefix);
char *base2prefix_reverse(char *text, size_t size, int base);
double text2size_single(const char *text, const char **last);
double text2size(const char *text, const char **last);
double text2size_mb(const char *text);
char *size2text_base(u64 size, char *buff, size_t buff_len);
char *size2text(u64 size);
double text2clock(const char *text);
double text2time_single(const char *text, const char **last);
double text2time(const char *text, const char **last);
int text2date(const char *text, struct tm *date, ...);

int text_match(const char *text1, const char *text2);

char *cavan_path_basename(char *buff, const char *path);
const char *cavan_path_basename_simple(const char *pathname);

char *cavan_path_dirname_base(char *buff, const char *path);
char *cavan_path_dirname(const char *path);

char *get_ntext_base(const char *src, char *dest, int start, int count);
char *get_ntext(const char *src, int start, int count);
char *text_header(const char *text, int count);
char *get_text_region_base(const char *src, char *dest, int start, int end);
char *get_text_region(const char *src, int start, int end);

void system_sync(void);

char *mem2text_base(const void *mem, size_t mem_size, char *buff, int size);
char *mem2text(const void *mem, int size);

char *text_delete_char_base(const char *text_in, char *text_out, char c);
char *text_delete_sub_base(const char *text_in, char *text_out, const char *sub, const size_t sublen);

char *mem_kmp_find(const char *mem, const char *sub, size_t memlen, size_t sublen);
int mem_kmp_find_all(const char *mem, const char *sub, size_t memlen, size_t sublen, char **results, size_t size);
char *text_find_next_line(const char *text);
char *text_find_line(const char *text, int index);
char *text_get_line(const char *text, char *buff, int index);
int text_version_cmp(const char *v1, const char *v2, char sep);

int text_has_char(const char *text, char c);
char *text_skip_char(const char *text, char c);
char *text_skip_chars(const char *text, const char *chars);

char *text_to_uppercase(const char *src, char *dest);
char *text_to_lowercase(const char *src, char *dest);
char *text_to_nuppercase(const char *src, char *dest, size_t size);
char *text_to_nlowercase(const char *src, char *dest, size_t size);
int text_cmp_nocase(const char *text1, const char *text2);
int text_ncmp_nocase(const char *text1, const char *text2, size_t size);
bool text2bool(const char *text);

char *text_strip(const char *text, size_t length, char *buff, size_t size);
char *text_skip_space(const char *text, const char *text_end);
char *text_skip_space2(const char *text);
char *text_skip_space_invert(const char *text, const char *line);
char *text_skip_lf(const char *text, const char *text_end);
char *text_skip_lf_invert(const char *text, const char *head);
char *text_skip_space_and_lf(const char *text, const char *text_end);
char *text_skip_space_and_lf_invert(const char *text, const char *head);
char *text_skip_line(const char *text, const char *text_end);
char *text_skip_name(const char *text, const char *text_end);
char *text_skip_name_invert(const char *text, const char *head);

char *text_find_space(const char *text, const char *text_end);
char *text_find_space2(const char *text);
char *text_find_space_invert(const char *text, const char *head);
char *text_find_lf(const char *text, const char *text_end);
char *text_find_lf_invert(const char *text, const char *head);
char *text_find_space_or_lf(const char *text, const char *text_end);
char *text_find_space_or_lf2(const char *text);
char *text_find_space_or_lf_invert(const char *text, const char *head);
char *text_find_name(const char *text, const char *text_end);
char *text_find_name_invert(const char *text, const char *head);

char *text_clear_space(char *text);
char *text_clear_space_and_lf(char *text);
char *text_clear_quote(char *text);
char *text_clear_space_and_quote(char *text);
char *text_clear_space_and_lf_and_quote(char *text);

char *text_clear_space_invert(const char *head, char *text);
char *text_clear_space_and_lf_invert(const char *head, char *text);
char *text_clear_quote_invert(const char *head, char *text);
char *text_clear_space_and_quote_invert(const char *head, char *text);
char *text_clear_space_and_lf_and_quote_invert(const char *head, char *text);

const char *text_fixup_null(const char *text, const char *text_null);
const char *text_fixup_null_simple(const char *text);
const char *text_fixup_empty(const char *text, const char *text_empty);
const char *text_fixup_empty_simple(const char *text);

#if 0
__printf_format_40__ char *cavan_vsnprintf(char *buff, size_t size, const char *fmt, char *args);
__printf_format_12__ char *cavan_printf(const char *fmt, ...);
#endif

__printf_format_12__ char *format_text(const char *fmt, ...);

int system_command_simple(const char *command);

__printf_format_10__ int vsystem_command(const char *command, va_list ap);
__printf_format_12__ int system_command(const char *command, ...);
__printf_format_23__ int system_command_retry(int count, const char *command, ...);
__printf_format_23__ int system_command_path(const char *path, const char *command, ...);

__printf_format_10__ FILE *vpipe_command(const char *command, va_list ap);
__printf_format_12__ FILE *pipe_command(const char *command, ...);
__printf_format_10__ FILE *vpipe_command_verbose(const char *command, va_list ap);
__printf_format_12__ FILE *pipe_command_verbose(const char *command, ...);
__printf_format_23__ FILE *pipe_command_path(const char *path, const char *command, ...);

__printf_format_30__ ssize_t vbuff_command(char *buff, size_t bufflen, const char *command, va_list ap);
__printf_format_12__ char *buff_command(const char *command, ...);
__printf_format_34__ ssize_t buff_command2(char *buff, size_t bufflen, const char *command, ...);
__printf_format_23__ char *buff_command_path(const char *path, const char *command, ...);
__printf_format_45__ ssize_t buff_command_path2(const char *path, char *buff, size_t bufflen, const char *command, ...);

char *mac_address_tostring_base(const char *mac, size_t maclen, char *buff);
char *mac_address_tostring(const char *mac, size_t maclen);

int text_is_number(const char *text);
int text_is_float(const char *text);
int text_is_uppercase(const char *text);
int text_is_lowercase(const char *text);
int text_is_letter(const char *text);

char *text_replace_char(char *text, char c_src, char c_dest);
char *text_replace_char2(const char *src, char *dest, char c_src, char c_dest);
char *text_replace_text_base(const char *text_old, char *text_new, const char *src, size_t srclen, const char *dest);

int text_split_by_char(char *text, char sep, char *texts[], int size);
int text_split_by_space(char *text, char *texts[], int size);
char *text_join_by_char(char *text[], size_t size1, char sep, char *buff, size_t size2);
size_t text_split_by_text(const char *text, const char *sep, char *buff, size_t size1, size_t size2);
char *text_join_by_text(char *text[], size_t size1, const char *sep, char *buff, size_t size2);

int text_array_find(const char *text, char *buff[], int size);
char text_get_char(const char *text, int index);
char text_get_escope_letter(char c);

extern void mem_reverse_simple(byte *start, byte *end);
extern void mem_reverse(byte *start, byte *end);
int text2array(char *text, u32 *array, size_t size, char sep);

char *frequency_tostring(double freq, char *buff, size_t size, char **last);
u32 frequency_unit2value(const char *text, const char *text_end);
double text2frequency(const char *text, const char *text_end, const char **last);

char *time2text_msec(u64 msec, char *buff, size_t size);
char *time2text_sec(u32 msec, char *buff, size_t size);

int cavan_string_init(cavan_string_t *str, const char *text, int length);
void cavan_string_deinit(cavan_string_t *str);
int cavan_string_alloc(cavan_string_t *str, int size, bool force);
bool cavan_string_extand(cavan_string_t *str, int length);
int cavan_string_append(cavan_string_t *str, const char *text, int size);
int cavan_string_append_line_unix(cavan_string_t *str, const char *line, int size);
int cavan_string_append_line_dos(cavan_string_t *str, const char *line, int size);
int cavan_string_assign(cavan_string_t *str, const char *text, int size);
void cavan_string_clear(cavan_string_t *str, bool depth);
int cavan_string_cmp(const char *text1, int len1, const char *text2, int len2);
int cavan_string_cmp2(const char *text1, int len1, const char *text2);
int cavan_string_casecmp(const char *text1, int len1, const char *text2, int len2);
int cavan_string_casecmp2(const char *text1, int len1, const char *text2);
bool cavan_string_startswith(const char *text1, int len1, const char *text2, int len2);
bool cavan_string_startswith2(const char *text1, int len1, const char *text2);

char *cavan_string_trim_start(const char *text, int length);
int cavan_string_trim_end(const char *text, int length);
char *cavan_string_trim(const char *text, int *length);

static inline int cavan_string_append_char(cavan_string_t *str, char c)
{
	return cavan_string_append(str, &c, 1);
}

static inline int cavan_string_append_line_end_unix(cavan_string_t *str)
{
	return cavan_string_append(str, "\n", 1);
}

static inline int cavan_string_append_line_end_dos(cavan_string_t *str)
{
	return cavan_string_append(str, "\r\n", 2);
}

static inline int cavan_string_cmp3(const cavan_string_t *str, const char *text, int length)
{
	return cavan_string_cmp(str->text, str->length, text, length);
}

static inline int cavan_string_cmp4(const cavan_string_t *str, const char *text)
{
	return cavan_string_cmp2(str->text, str->length, text);
}

static inline int cavan_string_cmp5(const cavan_string_t *str1, const cavan_string_t *str2)
{
	return cavan_string_cmp3(str1, str2->text, str2->length);
}

static inline int cavan_string_casecmp3(const cavan_string_t *str, const char *text, int length)
{
	return cavan_string_casecmp(str->text, str->length, text, length);
}

static inline int cavan_string_casecmp4(const cavan_string_t *str, const char *text)
{
	return cavan_string_casecmp2(str->text, str->length, text);
}

static inline int cavan_string_casecmp5(const cavan_string_t *str1, const cavan_string_t *str2)
{
	return cavan_string_casecmp3(str1, str2->text, str2->length);
}

static inline bool cavan_string_startswith3(const cavan_string_t *str, const char *text, int length)
{
	return cavan_string_startswith(str->text, str->length, text, length);
}

static inline bool cavan_string_startswith4(const cavan_string_t *str, const char *text)
{
	return cavan_string_startswith2(str->text, str->length, text);
}

static inline bool cavan_string_startswith5(const cavan_string_t *str1, const cavan_string_t *str2)
{
	return cavan_string_startswith3(str1, str2->text, str2->length);
}

static inline char *cavan_string_find(const char *text, int len1, const char *sub, int len2)
{
	return mem_kmp_find(text, sub, len1, len2);
}

static inline char *cavan_string_find2(const char *text, int len1, const char *sub)
{
	return mem_kmp_find(text, sub, len1, strlen(sub));
}

static inline char *cavan_string_find3(const cavan_string_t *str, const char *text, int length)
{
	return cavan_string_find(str->text, str->length, text, length);
}

static inline char *cavan_string_find4(const cavan_string_t *str, const char *text)
{
	return cavan_string_find2(str->text, str->length, text);
}

static inline char *cavan_string_find5(const cavan_string_t *str1, const cavan_string_t *str2)
{
	return cavan_string_find3(str1, str2->text, str2->length);
}

// ============================================================

static inline int prefix2base2(const char *prefix, const char **last, int base)
{
	return prefix2base(prefix, NULL, last, base);
}

static inline char *text_trans(char *text)
{
	return text_ntrans(text, text_len(text));
}

static inline char *text_kmp_find(const char *buff, const char *sub)
{
	return mem_kmp_find(buff, sub, text_len(buff), text_len(sub));
}

static inline int text_kmp_find_all(const char *buff, const char *sub, char **results, size_t size)
{
	return mem_kmp_find_all(buff, sub, text_len(buff), text_len(sub), results, size);
}

static inline char *text_delete_char(char *text, char c)
{
	return text_delete_char_base(text, text, c);
}

static inline char *text_delete_sub(char *text, const char *sub)
{
	return text_delete_sub_base(text, text, sub, text_len(sub));
}

static inline int text2value_array2(const char *text, const char **last, char sep, int values[], size_t count, int base)
{
	return text2value_array(text, NULL, last, sep, values, count, base);
}

static inline char *value2text_array2(int values[], size_t count, char sep, char *buff, size_t size, int base)
{
	return value2text_array(values, count, sep, buff, buff + size, base);
}

static inline int is_empty_character(char c)
{
	return text_has_char(" \t\n\r", c);
}

static inline char *text_replace_text(const char *text_old, char *text_new, const char *src, const char *dest)
{
	return text_replace_text_base(text_old, text_new, src, text_len(src), dest);
}

static inline int text_contain_char(const char *text, char c)
{
	return text_has_char(text, c);
}

static inline void text_reverse2(char *text, size_t size)
{
	mem_reverse_simple((byte *) text, (byte *) text + size - 1);
}

static inline void text_reverse3(char *text)
{
	text_reverse2(text, text_len(text));
}

static inline char *text_substring(const char *text, char *sub, int start, int count)
{
	get_ntext_base(text, sub, start, count);
	sub[count] = 0;

	return sub;
}

static inline ulong text2ulong2(const char *text, int length, int base)
{
	return text2ulong(text, text + length, base);
}

static inline long text2long2(const char *text, int length, int base)
{
	return text2long(text, text + length, base);
}

static inline ulong text2ulong3(const char *text, int base)
{
	return text2ulong2(text, strlen(text), base);
}

static inline long text2long3(const char *text, int base)
{
	return text2long2(text, strlen(text), base);
}

static inline const char *cavan_bool_tostring(bool value)
{
	return value ? "true" : "false";
}

__END_DECLS;
