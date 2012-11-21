#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Tue Nov 20 16:09:41 CST 2012
 */

#include <cavan.h>
#include <cavan/display.h>
#include <cavan/input.h>

#define CAVAN_MOUSE_SIZE	10

struct cavan_window
{
	int x, abs_x;
	int y, abs_y;
	int width, height;
	int border_width;

	int id;
	char text[512];
	pthread_mutex_t lock;

	cavan_display_color_t border_color;
	cavan_display_color_t back_color;
	cavan_display_color_t fore_color;

	void *private_data;

	struct cavan_window *child;
	struct cavan_window *parent;
	struct cavan_window *next;

	void (*destory_handler)(struct cavan_window *win);
	void (*paint_handler)(struct cavan_window *win);
	void (*click_handler)(struct cavan_window *win, bool pressed);
	void (*move_handler)(struct cavan_window *win, int x, int y);
	void (*entry_handler)(struct cavan_window *win);
	void (*exit_handler)(struct cavan_window *win);

	bool (*on_paint)(struct cavan_window *win, void *data);
	bool (*on_click)(struct cavan_window *win, int x, int y, bool pressed, void *data);
	bool (*on_move)(struct cavan_window *win, int x, int y, void *data);
	bool (*on_entry)(struct cavan_window *win, void *data);
	bool (*on_exit)(struct cavan_window *win, void *data);
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

	void *private_data;

	struct cavan_display_memory *mouse_point_mem;
	float mouse_speed;
	int x, max_x;
	int y, max_y;

	struct cavan_window *win_curr;
	struct cavan_window *win_active;
	struct cavan_window *win_head;
	size_t win_count;

	pthread_mutex_t lock;
};

void cavan_window_set_position(struct cavan_window *win, int x, int y);
void cavan_window_add_child(struct cavan_window *win, struct cavan_window *child);
int cavan_window_remove_child(struct cavan_window *win, struct cavan_window *child);

int cavan_dislog_init(struct cavan_dialog *dialog, void *data);
int cavan_button_init(struct cavan_button *button, void *data);

int cavan_application_context_init(struct cavan_application_context *context, void *data);
void cavan_application_context_uninit(struct cavan_application_context *app);
void cavan_application_context_add_window(struct cavan_application_context *context, struct cavan_window *win);
int cavan_application_context_remove_window(struct cavan_application_context *context, struct cavan_window *win);

static inline int cavan_application_context_run(struct cavan_application_context *context)
{
	return cavan_input_service_join(&context->input_service);
}
