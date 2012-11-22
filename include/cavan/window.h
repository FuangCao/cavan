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

struct cavan_application_context;

struct cavan_window
{
	int id;
	char text[512];

	int x, abs_x;
	int y, abs_y;
	int width, height;
	int border_width;

	cavan_display_color_t border_color;
	cavan_display_color_t back_color;
	cavan_display_color_t fore_color;

	bool pressed;
	pthread_mutex_t lock;
	struct cavan_application_context *context;

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

	int title_height;
	int x_offset;
	int y_offset;
	struct cavan_display_memory *backup;
};

struct cavan_button
{
	struct cavan_window window;
};

struct cavan_application_context
{
	struct cavan_display_device display;
	struct cavan_input_service input_service;

	void *private_data;

	struct cavan_display_memory *mouse_backup;
	float mouse_speed;
	int x, max_x;
	int y, max_y;

	struct cavan_window *win_curr;
	struct cavan_window *win_active;
	struct cavan_window *win_head;
	size_t win_count;

	pthread_mutex_t lock;
};

void cavan_window_paint_all(struct cavan_window *win);
void cavan_window_set_position(struct cavan_window *win, int x, int y);
void cavan_window_add_child(struct cavan_window *win, struct cavan_window *child);
int cavan_window_remove_child(struct cavan_window *win, struct cavan_window *child);

int cavan_dialog_init(struct cavan_dialog *dialog, struct cavan_application_context *context);
int cavan_button_init(struct cavan_button *button, struct cavan_application_context *context);

int cavan_application_context_init(struct cavan_application_context *context, void *data);
void cavan_application_context_uninit(struct cavan_application_context *app);
void cavan_application_context_add_window(struct cavan_application_context *context, struct cavan_window *win);
int cavan_application_context_remove_window(struct cavan_application_context *context, struct cavan_window *win);
int cavan_application_paint(struct cavan_application_context *context);
int cavan_application_context_run(struct cavan_application_context *context);

static inline void cavan_application_context_update_data(struct cavan_application_context *context)
{
	context->display.update_data(&context->display);
}

static inline void cavan_window_set_back_color(struct cavan_window *win, float red, float green, float blue)
{
	win->back_color = cavan_display_build_color3f(&win->context->display, red, green, blue);
	cavan_window_paint_all(win);
}

static inline void cavan_window_set_fore_color(struct cavan_window *win, float red, float green, float blue)
{
	win->fore_color = cavan_display_build_color3f(&win->context->display, red, green, blue);
	win->paint_handler(win);
}

static inline void cavan_window_set_border_color(struct cavan_window *win, float red, float green, float blue)
{
	win->border_color = cavan_display_build_color3f(&win->context->display, red, green, blue);
	win->paint_handler(win);
}

static inline void cavan_window_set_text(struct cavan_window *win, const char *text)
{
	text_ncopy(win->text, text, sizeof(win->text));
	win->paint_handler(win);
}
