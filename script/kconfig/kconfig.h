#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Tue Jan  3 15:37:38 CST 2012
 */

#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include "list.h"

#define ARRAY_SIZE(a) \
	(sizeof(a) / sizeof((a)[0]))

#define STRUCT_MEMBER_OFFSET(type, member) \
	((void *) &((type *) 0)->member)

#define GET_STRUCT_POINTER(type, member, addr) \
	((type *) ((void *) (addr) - STRUCT_MEMBER_OFFSET(type, member)))

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

enum kconfig_state
{
	KCONFIG_STATE_UNKNOWN,
	KCONFIG_STATE_DESELED,
	KCONFIG_STATE_SELECTED,
};

struct kconfig_menu_item {
	int x, y;
	int width;
	int level;
	int state;
	char text[1024];
	struct cavan_list_node list_node;
	struct kconfig_menu_item *child;
	struct kconfig_menu_item *parent;
};

struct kconfig_menu_descriptor {
	int x, y;
	int width, height;
	char title[1024];
	char prompt[1024];
	struct cavan_list_node *head;
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
int ncurses_show_menu_box(struct kconfig_menu_descriptor *desc);
void kconfig_menu_init(struct kconfig_menu_descriptor *desc);
void kconfig_menu_item_init(struct kconfig_menu_item *item, const char *text);
struct kconfig_menu_item *kconfig_menu_new_item(const char *text);
void kconfig_menu_add_item(struct kconfig_menu_descriptor *desc, struct kconfig_menu_item *item);
void kconfig_menu_add_child(struct kconfig_menu_item *item, struct kconfig_menu_item *child);
void ncurses_draw_menu_base(WINDOW *win, struct kconfig_menu_descriptor *desc, int y, int level, struct cavan_list_node *head);
void ncurses_draw_menu(WINDOW *win, struct kconfig_menu_descriptor *desc);
struct kconfig_menu_item *list_node_to_menu_item(struct cavan_list_node *node);
