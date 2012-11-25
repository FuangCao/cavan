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

struct cavan_window_rect
{
	int x, y;
	int width, height;
};

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

	int (*init_handler)(struct cavan_window *win, struct cavan_application_context *context);
	void (*destory_handler)(struct cavan_window *win);
	void (*paint_handler)(struct cavan_window *win);
	void (*key_handler)(struct cavan_window *win, const char *name, int code, int value);
	void (*click_handler)(struct cavan_window *win, bool pressed);
	void (*move_handler)(struct cavan_window *win, int x, int y);
	void (*entry_handler)(struct cavan_window *win);
	void (*exit_handler)(struct cavan_window *win);
	void (*get_rect_handler)(struct cavan_window *win, struct cavan_window_rect *rect);

	void (*on_destory)(struct cavan_window *win, void *data);
	bool (*on_paint)(struct cavan_window *win, void *data);
	bool (*on_key_pressed)(struct cavan_window *win, const char *name, int code, int value, void *data);
	bool (*on_clicked)(struct cavan_window *win, int x, int y, bool pressed, void *data);
	bool (*on_double_clicked)(struct cavan_window *win, int x, int y, bool pressed, void *data);
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
	bool move_able;
	struct cavan_display_memory_rect *backup;
};

struct cavan_button
{
	struct cavan_window window;

	cavan_display_color_t border_color_backup;
	cavan_display_color_t back_color_backup;
	cavan_display_color_t fore_color_backup;
};

struct cavan_application_context
{
	void *private_data;
	struct cavan_display_device *display;
	struct cavan_input_service input_service;

	struct cavan_display_memory *mouse_backup;
	float mouse_speed;
	int x, max_x;
	int y, max_y;

	struct cavan_window *win_curr;
	struct cavan_window *win_active;
	struct cavan_window win_root;

	pthread_mutex_t lock;
};

int cavan_window_add_child(struct cavan_window *win, struct cavan_window *child);
int cavan_window_remove_child(struct cavan_window *win, struct cavan_window *child);
void cavan_window_set_abs_position(struct cavan_window *win, int x, int y);
void cavan_window_set_position(struct cavan_window *win, int x, int y);
struct cavan_window *cavan_window_find_by_axis(struct cavan_window *head, int x, int y);
void cavan_window_paint(struct cavan_window *win);
void cavan_window_destory(struct cavan_window *win);
void cavan_window_set_back_color(struct cavan_window *win, float red, float green, float blue);
void cavan_window_set_fore_color(struct cavan_window *win, float red, float green, float blue);
void cavan_window_set_border_color(struct cavan_window *win, float red, float green, float blue);
void cavan_window_set_text(struct cavan_window *win, const char *text);
void cavan_window_set_width(struct cavan_window *win, int width);
void cavan_window_set_height(struct cavan_window *win, int height);
void cavan_window_set_border_width(struct cavan_window *win, int width);

int cavan_window_init_base(struct cavan_window *win, int (*handler)(struct cavan_window *win, struct cavan_application_context *context));
int cavan_window_init_handler(struct cavan_window *win, struct cavan_application_context *context);
void cavan_window_destory_handler(struct cavan_window *win);
void cavan_window_paint_handler(struct cavan_window *win);
void cavan_window_paint_handler(struct cavan_window *win);
void cavan_window_key_handler(struct cavan_window *win, const char *name, int code, int value);
void cavan_window_click_handler(struct cavan_window *win, bool pressed);
void cavan_window_move_handler(struct cavan_window *win, int x, int y);
void cavan_window_entry_handler(struct cavan_window *win);
void cavan_window_exit_handler(struct cavan_window *win);
void cavan_window_get_rect_handler(struct cavan_window *win, struct cavan_window_rect *rect);

int cavan_dialog_init_handler(struct cavan_window *win, struct cavan_application_context *context);
void cavan_dialog_paint_handler(struct cavan_window *win);
void cavan_dialog_click_handler(struct cavan_window *win, bool pressed);
void cavan_dialog_move_handler(struct cavan_window *win, int x, int y);
void cavan_dialog_get_rect_handler(struct cavan_window *win, struct cavan_window_rect *rect);

int cavan_button_init_handler(struct cavan_window *win, struct cavan_application_context *context);
void cavan_button_paint_handler(struct cavan_window *win);
void cavan_button_click_handler(struct cavan_window *win, bool pressed);

int cavan_application_init(struct cavan_application_context *context, struct cavan_display_device *display, void *data);
void cavan_application_uninit(struct cavan_application_context *context);
int cavan_application_main_loop(struct cavan_application_context *context);

static inline void cavan_application_update_data(struct cavan_application_context *context)
{
	cavan_window_paint(&context->win_root);
	context->display->refresh(context->display);
}

static inline void cavan_application_add_window(struct cavan_application_context *context, struct cavan_window *win)
{
	cavan_window_add_child(&context->win_root, win);
}

static inline int cavan_application_remove_window(struct cavan_application_context *context, struct cavan_window *win)
{
	return cavan_window_remove_child(&context->win_root, win);
}

static inline void cavan_window_set_on_paint(struct cavan_window *win, bool (*handler)(struct cavan_window *win, void *data))
{
	pthread_mutex_lock(&win->lock);
	win->on_paint = handler;
	pthread_mutex_unlock(&win->lock);
}

static inline void cavan_window_set_on_clicked(struct cavan_window *win, bool (*handler)(struct cavan_window *win, int x, int y, bool pressed, void *data))
{
	pthread_mutex_lock(&win->lock);
	win->on_clicked = handler;
	pthread_mutex_unlock(&win->lock);
}

static inline void cavan_window_set_on_key_pressed(struct cavan_window *win, bool (*handler)(struct cavan_window *win, const char *name, int code, int value, void *data))
{
	pthread_mutex_lock(&win->lock);
	win->on_key_pressed = handler;
	pthread_mutex_unlock(&win->lock);
}

static inline void cavan_window_set_on_entry(struct cavan_window *win, bool (*handler)(struct cavan_window *win, void *data))
{
	pthread_mutex_lock(&win->lock);
	win->on_entry = handler;
	pthread_mutex_unlock(&win->lock);
}

static inline void cavan_window_set_on_exit(struct cavan_window *win, bool (*handler)(struct cavan_window *win, void *data))
{
	pthread_mutex_lock(&win->lock);
	win->on_exit = handler;
	pthread_mutex_unlock(&win->lock);
}

static inline void cavan_window_set_on_move(struct cavan_window *win, bool (*handler)(struct cavan_window *win, int x, int y, void *data))
{
	pthread_mutex_lock(&win->lock);
	win->on_move = handler;
	pthread_mutex_unlock(&win->lock);
}

static inline int cavan_window_init(struct cavan_window *win)
{
	return cavan_window_init_base(win, cavan_window_init_handler);
}

static inline int cavan_dialog_init(struct cavan_dialog *dialog)
{
	return cavan_window_init_base(&dialog->window, cavan_dialog_init_handler);
}

static inline int cavan_button_init(struct cavan_button *button)
{
	return cavan_window_init_base(&button->window, cavan_button_init_handler);
}
