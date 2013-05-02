#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Tue Nov 20 16:09:41 CST 2012
 */

#include <cavan.h>
#include <cavan/display.h>
#include <cavan/input.h>

#define CAVAN_MOUSE_SIZE			10
#define CAVAN_APP_MESSAGE_POOL_SIZE	20
#define CAVAN_WINDOW_SEP_WIDTH		2

struct cavan_application_context;

enum cavan_application_event
{
	CAVAN_APP_EVENT_STOP,
	CAVAN_APP_EVENT_EXIT,
};

enum cavan_window_text_align
{
	CAVAN_WIN_TEXT_ALIGN_CENTER,
	CAVAN_WIN_TEXT_ALIGN_LEFT,
	CAVAN_WIN_TEXT_ALIGN_RIGHT
};

struct cavan_window
{
	int id;

	int x, y;
	int xabs, yabs;
	int width, height;
	int right, bottom;
	int border_width;

	cavan_display_color_t border_color;
	cavan_display_color_t back_color;
	cavan_display_color_t fore_color;

	bool pressed;
	bool entered;
	bool visible;
	bool active;

	pthread_mutex_t lock;
	struct cavan_application_context *context;

	struct cavan_window *parent;
	struct double_link_node node;
	struct double_link child_link;

	void (*destroy_handler)(struct cavan_window *win);
	void (*paint_handler)(struct cavan_window *win);
	void (*key_handler)(struct cavan_window *win, struct cavan_input_message_key *message);
	bool (*click_handler)(struct cavan_window *win, struct cavan_input_message_point *message);
	void (*move_handler)(struct cavan_window *win, struct cavan_input_message_point *message);
	void (*entry_handler)(struct cavan_window *win, struct cavan_input_message_point *message);
	void (*exit_handler)(struct cavan_window *win, struct cavan_input_message_point *message);
	void (*get_rect_handler)(struct cavan_window *win, struct cavan_display_rect *rect);

	void (*on_destroy)(struct cavan_window *win, void *data);
	bool (*on_paint)(struct cavan_window *win, void *data);
	bool (*on_key_pressed)(struct cavan_window *win, struct cavan_input_message_key *message, void *data);
	bool (*on_clicked)(struct cavan_window *win, struct cavan_input_message_point *message, void *data);
	bool (*on_double_clicked)(struct cavan_window *win, struct cavan_input_message_point *message, bool pressed, void *data);
	bool (*on_move)(struct cavan_window *win, struct cavan_input_message_point *message, void *data);
	bool (*on_entry)(struct cavan_window *win, struct cavan_input_message_point *message, void *data);
	bool (*on_exit)(struct cavan_window *win, struct cavan_input_message_point *message, void *data);
};

struct cavan_label
{
	struct cavan_window window;

	const char *text;
	enum cavan_window_text_align align;
};

struct cavan_button
{
	struct cavan_label label;

	cavan_display_color_t border_color_backup;
	cavan_display_color_t back_color_backup;
	cavan_display_color_t fore_color_backup;
};

struct cavan_progress_bar
{
	struct cavan_window window;

	double pos;
	double total;
	cavan_display_color_t complete_color;
};

struct cavan_dialog
{
	struct cavan_window window;
	struct cavan_label title;
	struct cavan_window body;

	bool move_able;
	int xofs, yofs;

	struct cavan_display_memory_rect *backup;
};

struct cavan_application_context
{
	void *private_data;
	struct cavan_thread thread;
	struct cavan_display_device *display;
	struct cavan_input_service input_service;

	struct cavan_display_memory *mouse_backup;
	float mouse_speed;
	int x, max_x;
	int y, max_y;

	cavan_display_color_t back_color;
	cavan_display_color_t mouse_color;
	cavan_display_color_t move_color;

	struct cavan_window *win_curr;
	struct cavan_window *win_active;
	struct double_link win_link;

	pthread_mutex_t lock;

	bool (*on_key_pressed)(struct cavan_application_context *context, struct cavan_input_message_key *message, void *data);
};

void cavan_window_add_child(struct cavan_window *win, struct cavan_window *child);
void cavan_window_remove_child(struct cavan_window *win, struct cavan_window *child);
void cavan_window_set_abs_position(struct cavan_window *win, int x, int y);
void cavan_window_set_position(struct cavan_window *win, int x, int y);
void cavan_window_set_top(struct cavan_window *win);
struct cavan_window *cavan_window_find_by_point(struct double_link *link, struct cavan_input_message_point *message);
void cavan_window_paint(struct cavan_window *win);
void cavan_window_paint_child(struct double_link *link);

int cavan_window_init(struct cavan_window *win, struct cavan_application_context *context);
void cavan_window_destroy(struct cavan_window *win);
void cavan_window_paint_handler(struct cavan_window *win);
void cavan_window_paint_handler(struct cavan_window *win);
void cavan_window_key_handler(struct cavan_window *win, struct cavan_input_message_key *message);
bool cavan_window_click_handler(struct cavan_window *win, struct cavan_input_message_point *message);
void cavan_window_move_handler(struct cavan_window *win, struct cavan_input_message_point *message);
void cavan_window_entry_handler(struct cavan_window *win, struct cavan_input_message_point *message);
void cavan_window_exit_handler(struct cavan_window *win, struct cavan_input_message_point *message);
void cavan_window_get_rect_handler(struct cavan_window *win, struct cavan_display_rect *rect);
void cavan_window_destroy_handler(struct cavan_window *win);

int cavan_dialog_init(struct cavan_dialog *dialog, struct cavan_application_context *context);
void cavan_dialog_deinit(struct cavan_dialog *dialog);
void cavan_dialog_paint_handler(struct cavan_window *win);
bool cavan_dialog_click_handler(struct cavan_window *win, struct cavan_input_message_point *message);
void cavan_dialog_move_handler(struct cavan_window *win, struct cavan_input_message_point *message);
void cavan_dialog_get_rect_handler(struct cavan_window *win, struct cavan_display_rect *rect);
void cavan_dialog_set_title_height(struct cavan_dialog *dialog, int height);

int cavan_label_init(struct cavan_label *label, struct cavan_application_context *context);
void cavan_label_deinit(struct cavan_label *label);
void cavan_label_paint_handler(struct cavan_window *win);

int cavan_button_init(struct cavan_button *button, struct cavan_application_context *context);
void cavan_button_paint_handler(struct cavan_window *win);
bool cavan_button_click_handler(struct cavan_window *win, struct cavan_input_message_point *message);

int cavan_progress_bar_init(struct cavan_progress_bar *bar, double total, struct cavan_application_context *context);
void cavan_progress_bar_deinit(struct cavan_progress_bar *bar);
int cavan_progress_bar_start(struct cavan_progress_bar *bar, double total);
void cavan_progress_bar_set_pos(struct cavan_progress_bar *bar, double pos);
void cavan_progress_bar_paint_handler(struct cavan_window *win);

bool cavan_window_clicked(struct cavan_window *win, struct cavan_input_message_point *message);

int cavan_application_init(struct cavan_application_context *context, struct cavan_display_device *display, void *data);
void cavan_application_deinit(struct cavan_application_context *context);
int cavan_application_main_loop(struct cavan_application_context *context, void (*handler)(struct cavan_application_context *context, void *data), void *data);
void cavan_application_paint(struct cavan_application_context *context);
void cavan_application_update_data(struct cavan_application_context *context);
void cavan_application_add_window(struct cavan_application_context *context, struct cavan_window *win);
void cavan_application_remove_window(struct cavan_application_context *context, struct cavan_window *win);

static inline void cavan_application_set_on_key_pressed(struct cavan_application_context *context, bool (*handler)(struct cavan_application_context *context, struct cavan_input_message_key *key, void *data))
{
	pthread_mutex_lock(&context->lock);
	context->on_key_pressed = handler;
	pthread_mutex_unlock(&context->lock);
}

static inline void cavan_window_lock(struct cavan_window *win)
{
	pthread_mutex_lock(&win->lock);
}

static inline void cavan_window_unlock(struct cavan_window *win)
{
	pthread_mutex_unlock(&win->lock);
}

static inline void cavan_window_set_back_color(struct cavan_window *win, float red, float green, float blue)
{
	pthread_mutex_lock(&win->lock);
	win->back_color = cavan_display_build_color3f(win->context->display, red, green, blue);
	pthread_mutex_unlock(&win->lock);
}

static inline void cavan_window_set_fore_color(struct cavan_window *win, float red, float green, float blue)
{
	pthread_mutex_lock(&win->lock);
	win->fore_color = cavan_display_build_color3f(win->context->display, red, green, blue);
	pthread_mutex_unlock(&win->lock);
}

static inline void cavan_window_set_border_color(struct cavan_window *win, float red, float green, float blue)
{
	pthread_mutex_lock(&win->lock);
	win->border_color = cavan_display_build_color3f(win->context->display, red, green, blue);
	pthread_mutex_unlock(&win->lock);
}

static inline void cavan_window_set_width(struct cavan_window *win, int width)
{
	pthread_mutex_lock(&win->lock);
	win->width = width;
	win->right = win->xabs + width - 1;
	pthread_mutex_unlock(&win->lock);
}

static inline void cavan_window_set_height(struct cavan_window *win, int height)
{
	pthread_mutex_lock(&win->lock);
	win->height = height;
	win->bottom = win->yabs + height - 1;
	pthread_mutex_unlock(&win->lock);
}

static inline void cavan_window_set_visible(struct cavan_window *win, bool visible)
{
	pthread_mutex_lock(&win->lock);
	win->visible = visible;
	win->entered = false;
	win->pressed = false;
	pthread_mutex_unlock(&win->lock);
}

static inline void cavan_window_set_active(struct cavan_window *win, bool active)
{
	pthread_mutex_lock(&win->lock);
	win->active = active;
	win->entered = false;
	win->pressed = false;
	pthread_mutex_unlock(&win->lock);
}

static inline void cavan_window_set_border_width(struct cavan_window *win, int width)
{
	pthread_mutex_lock(&win->lock);
	win->border_width = width;
	pthread_mutex_unlock(&win->lock);
}

static inline void cavan_window_set_on_paint(struct cavan_window *win, bool (*handler)(struct cavan_window *win, void *data))
{
	pthread_mutex_lock(&win->lock);
	win->on_paint = handler;
	pthread_mutex_unlock(&win->lock);
}

static inline void cavan_window_set_on_clicked(struct cavan_window *win, bool (*handler)(struct cavan_window *win, struct cavan_input_message_point *point, void *data))
{
	pthread_mutex_lock(&win->lock);
	win->on_clicked = handler;
	pthread_mutex_unlock(&win->lock);
}

static inline void cavan_window_set_on_key_pressed(struct cavan_window *win, bool (*handler)(struct cavan_window *win, struct cavan_input_message_key *key, void *data))
{
	pthread_mutex_lock(&win->lock);
	win->on_key_pressed = handler;
	pthread_mutex_unlock(&win->lock);
}

static inline void cavan_window_set_on_entry(struct cavan_window *win, bool (*handler)(struct cavan_window *win, struct cavan_input_message_point *point, void *data))
{
	pthread_mutex_lock(&win->lock);
	win->on_entry = handler;
	pthread_mutex_unlock(&win->lock);
}

static inline void cavan_window_set_on_exit(struct cavan_window *win, bool (*handler)(struct cavan_window *win, struct cavan_input_message_point *point, void *data))
{
	pthread_mutex_lock(&win->lock);
	win->on_exit = handler;
	pthread_mutex_unlock(&win->lock);
}

static inline void cavan_window_set_on_move(struct cavan_window *win, bool (*handler)(struct cavan_window *win, struct cavan_input_message_point *point, void *data))
{
	pthread_mutex_lock(&win->lock);
	win->on_move = handler;
	pthread_mutex_unlock(&win->lock);
}

static inline void cavan_window_get_rect(struct cavan_window *win, struct cavan_display_rect *rect)
{
	return win->get_rect_handler(win, rect);
}

static inline void cavan_window_update(struct cavan_window *win)
{
	cavan_window_lock(win);
	cavan_window_set_top(win);
	cavan_window_unlock(win);

	cavan_window_paint(win);
}

static inline void cavan_dialog_add_child(struct cavan_dialog *dialog, struct cavan_window *win)
{
	cavan_window_add_child(&dialog->body, win);
}

static inline void cavan_label_set_text(struct cavan_label *label, const char *text)
{
	cavan_window_lock(&label->window);
	label->text = text;
	cavan_window_unlock(&label->window);
}

static inline void cavan_progress_bar_add(struct cavan_progress_bar *bar, double value)
{
	cavan_progress_bar_set_pos(bar, bar->pos + value);
}

static inline void cavan_progress_bar_complete(struct cavan_progress_bar *bar)
{
	cavan_progress_bar_set_pos(bar, bar->total);
}

static inline int cavan_application_send_event(struct cavan_application_context *context, u32 event)
{
	return cavan_thread_send_event(&context->thread, event);
}

static inline int cavan_application_exit(struct cavan_application_context *context)
{
	return cavan_application_send_event(context, CAVAN_APP_EVENT_EXIT);
}
