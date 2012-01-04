#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Tue Jan  3 15:37:38 CST 2012
 */

#include <stdio.h>
#include <string.h>
#include <ncurses.h>

#define ARRAY_SIZE(a) \
	(sizeof(a) / sizeof((a)[0]))

#define NELEM		ARRAY_SIZE
#define KEY_ESC		27
#define KEY_TAB		9

enum kconfig_color
{
	KCONFIG_COLOR_UNKNOWN,
	KCONFIG_COLOR_WHITE_WHITE,
	KCONFIG_COLOR_WHITE_BLACK,
	KCONFIG_COLOR_WHITE_RED,
	KCONFIG_COLOR_WHITE_YELLOW,
	KCONFIG_COLOR_WHITE_BLUE,
	KCONFIG_COLOR_BLUE_WHITE,
	KCONFIG_COLOR_BLUE_BLACK,
	KCONFIG_COLOR_BLUE_YELLOW,
	KCONFIG_COLOR_BLACK_BLACK,
};

struct ncurses_menu_item
{
	char select;
	const char *text;
};

// void ncurses_fill_line(WINDOW *win, int x, int y, int width, int attr, chtype ch);
// void ncurses_fill_column(WINDOW *win, int x, int y, int height, int attr, chtype ch);
// void ncurses_fill_rectangle(WINDOW *win, int x, int y, int width, int height, int attr, chtype ch);
// void ncurses_draw_border(WINDOW *win, const char *title, int x, int y, int width, int height, int attr1, int attr2, int attr3);
// void ncurses_draw_separate(WINDOW *win, int y, int width, int attr1, int attr2);
// int ncurses_print_text(WINDOW *win, int x, int y, int width, const char *text, int attr);
// WINDOW *ncurses_new_window(int x, int y, int width, int height, const char *title, const char *prompt);
// void ncurses_draw_button(WINDOW *win, int x, int y, const char *text, int attr1, int attr2);
// void ncurses_draw_buttons(WINDOW *win, int y, int width, const char *texts[], size_t count, int selected);
// int ncurses_show_message_box(int width, int height, const char *prompt, const char *buttons[], size_t count);
void ncurses_init(void);
int ncurses_show_yes_no_dialog(int width, int height, const char *prompt);
int ncurses_show_menu_box(const char *title, const char *prompt, struct ncurses_menu_item items[], size_t count);
