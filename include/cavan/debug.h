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

// ============================================================

extern const char *cavan_help_message_help;
extern const char *cavan_help_message_version;
extern const char *cavan_help_message_ip;
extern const char *cavan_help_message_hostname;
extern const char *cavan_help_message_local;
extern const char *cavan_help_message_adb;
extern const char *cavan_help_message_tcp;
extern const char *cavan_help_message_udp;
extern const char *cavan_help_message_unix_tcp;
extern const char *cavan_help_message_unix_udp;
extern const char *cavan_help_message_port;
extern const char *cavan_help_message_url;
extern const char *cavan_help_message_send_file;
extern const char *cavan_help_message_recv_file;
extern const char *cavan_help_message_command;
extern const char *cavan_help_message_login;
extern const char *cavan_help_message_preserve_environment;
extern const char *cavan_help_message_shell;
extern const char *cavan_help_message_input_file;
extern const char *cavan_help_message_output_file;
extern const char *cavan_help_message_bs;
extern const char *cavan_help_message_seek;
extern const char *cavan_help_message_skip;
extern const char *cavan_help_message_super;
extern const char *cavan_help_message_daemon;
extern const char *cavan_help_message_daemon_min;
extern const char *cavan_help_message_daemon_max;
extern const char *cavan_help_message_verbose;
extern const char *cavan_help_message_logfile;

// ============================================================

char *dump_backtrace(char *buff, size_t size);
int dump_stack(void);
char *address_to_symbol(const void *addr, char *buff, size_t size);
int catch_sigsegv(void);

int cavan_get_build_time(struct tm *time);
const char *cavan_get_build_time_string(void);
