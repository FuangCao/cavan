#pragma once

#include <cavan.h>

#define PREFIX

#define stand_msg(fmt, arg ...) \
	println(PREFIX fmt, ##arg)

#define position_msg() \
	stand_msg("%s => %s[%d]", __FILE__, __FUNCTION__, __LINE__)

#define error_msg(fmt, arg ...) \
	do { \
		set_console_font_simple(FONT_RED_FOREGROUND); \
		position_msg(); \
		stand_msg("Error: " fmt, ##arg); \
		set_default_font(); \
	} while (0)

#define print_error(fmt, arg ...) \
	do { \
		set_console_font_simple(FONT_RED_FOREGROUND); \
		position_msg(); \
		print_error_base(fmt, ##arg); \
		set_default_font(); \
	} while (0)

#define right_msg(fmt, arg ...) \
	do { \
		set_console_font_simple(FONT_GREEN_FOREGROUND); \
		position_msg(); \
		stand_msg("Right: " fmt, ##arg); \
		set_default_font(); \
	} while (0)

#define warning_msg(fmt, arg ...) \
	do { \
		set_console_font_simple(FONT_MAGENTA_FOREGROUND); \
		position_msg(); \
		stand_msg("Warnining: " fmt, ##arg); \
		set_default_font(); \
	} while (0)

#define ERROR_RETURN(en) \
	do { \
		errno = en; \
		return -errno; \
	} while (0)

#define RETRY(func, ret, condition, count) \
	do { \
		int i = count; \
		do { \
			ret = func; \
			i--; \
		} while (i && (condition)); \
	} while (0)

char *dump_backtrace(char *buff, size_t size);
int dump_stack(void);
char *address_to_symbol(const void *addr, char *buff, size_t size);
int catch_sigsegv(void);

int cavan_get_build_time(struct tm *time);
const char *cavan_get_build_time_string(void);
