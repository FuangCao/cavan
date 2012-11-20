#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Tue Nov 20 16:09:41 CST 2012
 */

#include <cavan.h>
#include <cavan/display.h>
#include <cavan/input.h>

struct cavan_application_context;

struct cavan_window
{
	int x, abs_x;
	int y, abs_y;
	int width, height;
	int border_width;

	char text[512];
	pthread_mutex_t lock;

	struct cavan_display_color3f border_color;
	struct cavan_display_color3f back_color;
	struct cavan_display_color3f fore_color;

	struct cavan_window *child;
	struct cavan_window *parent;
	struct cavan_window *next;

	int (*paint)(struct cavan_window *win, struct cavan_application_context *context);
	bool (*paint_handler)(struct cavan_window *win, struct cavan_application_context *context);
	bool (*click_handler)(struct cavan_window *win, bool pressed);
	bool (*move_handler)(struct cavan_window *win, struct cavan_touch_point *point);
	bool (*entry_handler)(struct cavan_window *win, struct cavan_touch_point *point);
	bool (*exit_handler)(struct cavan_window *win, struct cavan_touch_point *point);
};

struct cavan_dialog
{
	struct cavan_window window;
};

struct cavan_button
{
	struct cavan_window window;
	bool pressed;
};

struct cavan_application_context
{
	struct cavan_display_device display;
	struct cavan_input_service input_service;

	struct cavan_window *win_head;
	size_t win_count;

	pthread_mutex_t lock;
};

void cavan_window_add_child(struct cavan_window *win, struct cavan_window *child);
int cavan_window_remove_child(struct cavan_window *win, struct cavan_window *child);
