#pragma once

#include <cavan.h>
#include <stdio.h>

#define PRINT_BUFFER_LEN	MB(1)
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
		text_ntrans((char *)&__tmp, sizeof(__tmp)); \
		__tmp = __tmp; \
	})

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
int prefix2base(const char *prefix, const char **prefix_ret);
u64 text2value_unsigned(const char *text, const char **text_ret, int base);
s64 text2value(const char *text, const char **text_ret, int base);

char *reverse_value2text_base2(u64 value, char *buff, size_t size);
char *reverse_value2text_base4(u64 value, char *buff, size_t size);
char *reverse_value2text_base8(u64 value, char *buff, size_t size);
char *reverse_value2text_base16(u64 value, char *buff, size_t size);
char *reverse_value2text_base32(u64 value, char *buff, size_t size);
char *reverse_value2text_all(u64 value, char *buff, size_t size, int base);
char *simple_value2text_reverse(u64 value, char *buff, size_t size, int base);
char *simple_value2text_unsigned(u64 value, char *buff, size_t size, int base);
char *simple_value2text(s64 value, char *buff, size_t size, int base);

char *value2text_base(s64 value, char *text, int size, char fill, int flags);
char *value2text(u64 value, int flags);

char *base2prefix(int base, char *prefix);
char *base2prefix_reverse(char *text, size_t size, int base);
u64 text2size_single(const char *text, const char **text_ret);
u64 text2size(const char *text, const char **text_ret);
u64 text2size_mb(const char *text);
char *size2text_base(u64 size, char *buff, size_t buff_len);
char *size2text(u64 size);

int text_match(const char *text1, const char *text2);

char *text_basename_base(char *buff, const char *path);
char *text_basename(const char *path);

char *text_dirname_base(char *buff, const char *path);
char *text_dirname(const char *path);

char *get_ntext_base(const char *src, char *dest, int start, int count);
char *get_ntext(const char *src, int start, int count);
char *text_header(const char *text, int count);
char *get_text_region_base(const char *src, char *dest, int start, int end);
char *get_text_region(const char *src, int start, int end);

void system_sync(void);

char *mem2text_base(const void *mem, char *buff, int size);
char *mem2text(const void *mem, int size);

char *to_abs_path_base(const char *rel_path, char *abs_path, size_t size);
char *to_abs_path(const char *rel_path);
char *to_abs_path_directory_base(const char *rel_path, char *abs_path, size_t size);
char *to_abs_directory(const char *rel_path);
char *to_abs_path2_base(const char *rel_path, char *abs_path, size_t size);
char *to_abs_path2(const char *rel_path);
char *prettify_pathname_base(const char *src_path, char *dest_path, size_t size);
char *prettify_pathname(const char *src_path);

char *text_path_cat(char *pathname, const char *dirname, const char *basename);

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
int text_bool_value(const char *text);

char *text_skip_space_head(const char *text, const char *line_end);
char *text_skip_space_tail(const char *text, const char *line);
char *text_find_line_end(const char *text, const char *file_end);
char *text_skip_line_end(const char *text, const char *file_end);

__printf_format_20__ char *vformat_text (char *buff, const char *fmt, va_list args);
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
int text_is_dot_name(const char *filename);
int text_isnot_dot_name(const char *filename);

size_t text_split_by_char(const char *text, char sep, char *buff, size_t size1, size_t size2);
char *text_join_by_char(char *text[], size_t size1, char sep, char *buff, size_t size2);
size_t text_split_by_text(const char *text, const char *sep, char *buff, size_t size1, size_t size2);
char *text_join_by_text(char *text[], size_t size1, const char *sep, char *buff, size_t size2);

int text_array_find(const char *text, char *buff[], int size);
char *text_find_line_end(const char *text, const char *end);

extern void mem_reverse_simple(byte *start, byte *end);
extern void mem_reverse(byte *start, byte *end);

// ============================================================

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

static inline char value2char(int index)
{
	if (index < 10)
	{
		return index + '0';
	}

	return index - 10 + 'A';
}

static inline int is_empty_character(char c)
{
	return text_has_char(" \t\n\r", c);
}

static inline char char2uppercase(char c)
{
	return IS_LOWERCASE(c) ? c - 'a' + 'A' : c;
}

static inline char char2lowercase(char c)
{
	return IS_UPPERCASE(c) ? c - 'A' + 'a' : c;
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
	mem_reverse_simple((byte *)text, (byte *)text + size - 1);
}

static inline void text_reverse3(char *text)
{
	text_reverse2(text, text_len(text));
}
