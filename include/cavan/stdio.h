#pragma once

#include <cavan.h>
#include <stdarg.h>

#if CONFIG_BUILD_FOR_ANDROID
#include <utils/Log.h>
#define printf	LOGD
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG	"Cavan"

#define DEFAULT_CONSOLE_DEVICE	"/dev/tty0"

#define FONT_DEFAULT			0	//set all attributes to their defaults
#define FONT_BOLD				1	//set bold

#define FONT_BLACK_FOREGROUND	30	//set black foreground
#define FONT_RED_FOREGROUND		31	//set red foreground
#define FONT_GREEN_FOREGROUND	32	//set green foreground
#define FONT_BROWN_FOREGROUND	33	//set brown foreground
#define FONT_BLUE_FOREGROUND	34	//set blue foreground
#define FONT_MAGENTA_FOREGROUND	35	//set magenta foreground
#define FONT_CYAN_FOREGROUND	36	//set cyan foreground
#define FONT_WHITE_FOREGROUND	37	//set white foreground
#define FONT_DEFAULT_FOREGROUND	39	//set white foreground

#define FONT_BLACK_BACKGROUND	40	//set black background
#define FONT_RED_BACKGROUND		41	//set red background
#define FONT_GREEN_BACKGROUND	42	//set green background
#define FONT_BROWN_BACKGROUND	43	//set brown background
#define FONT_BLUE_BACKGROUND	44	//set blue background
#define FONT_MAGENTA_BACKGROUND	45	//set magenta background
#define FONT_CYAN_BACKGROUND	46	//set cyan background
#define FONT_WHITE_BACKGROUND	47	//set white background
#define FONT_DEFAULT_BACKGROUND	49	//set default background color

#ifndef __THROW
#define __THROW
#endif

#ifndef __THROWNL
#define __THROWNL __THROW
#endif

#ifdef __cplusplus
#undef __THROWNL
#define __THROWNL
#endif

#ifndef _POSIX_VDISABLE
#define	_POSIX_VDISABLE	'\0'
#endif

#define printf_format(a, b)		__THROWNL __attribute__ ((__format__ (__printf__, a, b)))

#define __printf_format_10__	printf_format(1, 0)
#define __printf_format_12__	printf_format(1, 2)
#define __printf_format_20__	printf_format(2, 0)
#define __printf_format_23__	printf_format(2, 3)
#define __printf_format_30__	printf_format(3, 0)
#define __printf_format_34__	printf_format(3, 4)
#define __printf_format_40__	printf_format(4, 0)
#define __printf_format_45__	printf_format(4, 5)
#define __printf_format_50__	printf_format(5, 0)
#define __printf_format_56__	printf_format(5, 6)

#define println_black(fmt, arg ...)		print_color_text(FONT_BLACK_FOREGROUND, fmt, ##arg)
#define println_red(fmt, arg ...)		print_color_text(FONT_RED_FOREGROUND, fmt, ##arg)
#define println_green(fmt, arg ...)		print_color_text(FONT_GREEN_FOREGROUND, fmt, ##arg)
#define println_brown(fmt, arg ...)		print_color_text(FONT_BROWN_FOREGROUND, fmt, ##arg)
#define println_blue(fmt, arg ...)		print_color_text(FONT_BLUE_FOREGROUND, fmt, ##arg)
#define println_magenta(fmt, arg ...)	print_color_text(FONT_MAGENTA_FOREGROUND, fmt, ##arg)
#define println_cyan(fmt, arg ...)		print_color_text(FONT_CYAN_FOREGROUND, fmt, ##arg)
#define println_white(fmt, arg ...)		print_color_text(FONT_WHITE_FOREGROUND, fmt, ##arg)

#define CAVAN_COLOR_STAND				"\033[0m"
#define CAVAN_COLOR_BOLD				"\033[1m"
#define CAVAN_COLOR_RED					"\033[31m"
#define CAVAN_COLOR_GREEN				"\033[32m"
#define CAVAN_COLOR_BLUE				"\033[34m"
#define CAVAN_COLOR_MAGENTA				"\033[35m"

#define pr_std_info(fmt, args ...) \
	printf(fmt "\n", ##args)

#ifdef CAVAN_DEBUG
#define pr_debug_info(fmt, args ...) \
	pr_std_info(fmt, ##args)
#else
#define pr_debug_info(fmt, args ...)
#endif

#define pr_func_info(fmt, args ...) \
	pr_std_info("%s[%d]: " fmt, __FUNCTION__, __LINE__, ##args)

#define pr_color_info(color, fmt, args ...) \
	pr_std_info(color fmt CAVAN_COLOR_STAND, ##args)

#define pr_red_info(fmt, args ...) \
	pr_color_info(CAVAN_COLOR_RED, "%s[%d]: " fmt, __FUNCTION__, __LINE__, ##args)

#define pr_error_info(fmt, args ...) \
	if (errno) { \
		pr_color_info(CAVAN_COLOR_RED, "%s[%d] (" fmt "): %s", __FUNCTION__, __LINE__, ##args, strerror(errno)); \
	} else { \
		pr_red_info(fmt, ##args); \
	}

#define pr_warning_info(fmt, args ...) \
	pr_color_info(CAVAN_COLOR_MAGENTA, "%s[%d]: " fmt, __FUNCTION__, __LINE__, ##args)

#define pr_green_info(fmt, args ...) \
	pr_color_info(CAVAN_COLOR_GREEN, fmt, ##args)

#define pr_blue_info(fmt, args ...) \
	pr_color_info(CAVAN_COLOR_BLUE, fmt, ##args)

#define pr_bold_info(fmt, args ...) \
	pr_color_info(CAVAN_COLOR_BOLD, fmt, ##args)

#define pr_std_pos(fmt) \
	pr_std_info(fmt, __FILE__, __FUNCTION__, __LINE__)

#define pr_pos_info() \
	pr_std_pos("%s => %s[%d]")

#define pr_color_pos(color) \
	pr_std_pos(color "%s => %s[%d]" CAVAN_COLOR_STAND)

#define pr_red_pos() \
	pr_color_pos(CAVAN_COLOR_RED);

#define pr_green_pos() \
	pr_color_pos(CAVAN_COLOR_GREEN);

#define pr_blue_pos() \
	pr_color_pos(CAVAN_COLOR_BLUE);

#define pr_bold_pos() \
	pr_color_pos(CAVAN_COLOR_BOLD);

#define pr_date_info(name) \
	printf(CAVAN_COLOR_GREEN "Cavan %s Build Date: %s %s" CAVAN_COLOR_STAND, name, __DATE__, __TIME__);

#define show_value(val) \
	println(#val " = %d", val)

#define show_valueh_base(val, len) \
	println(#val " = 0x%0" len "x", val)

#define pr_result_info(ret) \
	((ret) < 0 ? pr_red_info("Failed") : pr_green_info("OK"))

#define show_valueh(val) \
	do { \
		switch (sizeof(val)) \
		{ \
		case 8: \
			show_value_base(val, "16"); \
			break; \
		case 4: \
			show_value_base(val, "8"); \
			break; \
		case 2: \
			show_value_base(val, "4"); \
			break; \
		default: \
			show_value_base(val, "2"); \
		} \
	} while (0)

#define print_array(a) \
	do { \
		switch (sizeof(a[0])) \
		{ \
		case 8: \
			text_show64((const u64 *)a, ARRAY_SIZE(a)); \
			break; \
		case 4: \
			text_show32((const u32 *)a, ARRAY_SIZE(a)); \
			break; \
		case 2: \
			text_show16((const u16 *)a, ARRAY_SIZE(a)); \
			break; \
		default: \
			text_show((const char *)a, sizeof(a)); \
		} \
	} while (0)

#if CONFIG_BUILD_FOR_ANDROID
#define print_text(text)					LOGD("%s", text)
#else
#define print_text(text)					print_ntext(text, strlen(text))
#endif

#define clear_screen_to_current()			print_text("\033[1J")
#define clear_whole_screen()				print_text("\033[2J")
#define clear_line_to_current()				print_text("\033[1K")
#define clear_whole_line()					print_text("\033[2K")
#define clear_console()						print_text("\033[1J\033[1;1H")
#define set_default_font()					print_text("\033[0m")
#define set_console_cursor(row, col)		print("\033[%d;%dH", row, col)
#define set_console_row(row)				print("\033[%dd", row)
#define set_console_col(col)				print("\033[%dG", col)
#define save_console_cursor()				print_text("\033[%ds")
#define restore_console_cursor()			print_text("\033[%du")

extern FILE *console_fp;

int set_tty_attr(int fd, int action, struct termios *attr);
int set_tty_mode(int fd, int mode, struct termios *attr_bak);

int has_char(long sec, long usec);
int timeout_getchar(long sec, long usec);

void print_ntext(const char *text, size_t size);
void print_buffer(const char *buff, size_t size);
void print_title(const char *title, char sep, size_t size);
void print_sep(size_t size);

void print_mem(const u8 *mem, size_t size);

int show_file(const char *dev_name, u64 start, u64 size);
int cat_file(const char *filename);

int open_console(const char *dev_path);
void close_console(void);
void fflush_console(void);

void show_menu(int x, int y, const char *menu[], int count, int select);
int get_menu_select(const char *input_dev_path, const char *menu[], int count);

int fswitch2text_mode(int tty_fd);
int switch2text_mode(const char *tty_path);
int fswitch2graph_mode(int tty_fd);
int switch2graph_mode(const char *tty_path);
void show_author_info(void);

__printf_format_23__ char *sprint(char *buff, const char *fmt, ...);
__printf_format_23__ char *sprintln(char *buff, const char *fmt, ...);
__printf_format_12__ void print(const char *fmt, ...);
__printf_format_10__ void vprint(const char *fmt, va_list ap);
__printf_format_10__ void vprintln(const char *fmt, va_list ap);
__printf_format_12__ void println(const char *fmt, ...);
__printf_format_34__ void print_to(int x, int y, const char *fmt, ...);
__printf_format_23__ void print_to_row(int row, const char *fmt, ...);
__printf_format_23__ void print_to_col(int col, const char *fmt, ...);
__printf_format_34__ void println_to(int x, int y, const char *fmt, ...);
__printf_format_23__ void println_to_row(int row, const char *fmt, ...);
__printf_format_23__ void println_to_col(int col, const char *fmt, ...);
__printf_format_20__ void vprint_color_text(int color, const char *fmt, va_list ap);
__printf_format_23__ void print_color_text(int color, const char *fmt, ...);
__printf_format_12__ void print_error_base(const char *fmt, ...);

extern char *size2text(u64 size);
bool cavan_get_choose_yesno(const char *prompt, bool def_value, int timeout_ms);

// ============================================================

static inline void print_char(char c)
{
	print_ntext(&c, 1);
}

static inline void set_console_font(int font, int foregound, int backgound)
{
	print("\033[%d;%d;%dm", font, foregound, backgound);
}

static inline void set_console_font_simple(int font)
{
	print("\033[%dm", font);
}

static inline void print_string(const char *str)
{
	print_text(str);
	print_char('\n');
}

static inline void print_string_to(int x, int y, const char *str)
{
	set_console_cursor(y, x);
	print_string(str);
}

static inline void print_string_to_row(int row, const char *str)
{
	set_console_row(row);
	print_string(str);
}

static inline void print_string_to_col(int col, const char *str)
{
	set_console_col(col);
	print_string(str);
}

static inline void print_size(u64 size)
{
	print_text(size2text(size));
	print_char('\n');
}

static inline int get_tty_attr(int fd, struct termios *attr)
{
	return tcgetattr(fd, attr);
}

static inline int restore_tty_attr(int fd, struct termios *attr)
{
	return set_tty_attr(fd, TCSADRAIN, attr);
}
