/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Tue Nov 20 16:09:41 CST 2012
 */

#include <cavan.h>
#include <cavan/window.h>
#include <cavan/fb.h>

void cavan_window_paint(struct cavan_window *win)
{
	struct cavan_window *child;
	struct cavan_display_device *display;

	win->paint_handler(win);

	if (win->on_paint)
	{
		win->on_paint(win, win->context->private_data);
	}

	pthread_mutex_lock(&win->lock);

	for (child = win->child; child; child = child->next)
	{
		cavan_window_paint(child);
	}

	display = win->context->display;
	display->refresh(display);

	pthread_mutex_unlock(&win->lock);
}

void cavan_window_destory(struct cavan_window *win)
{
	struct cavan_window *child;

	if (win->on_destory)
	{
		win->on_destory(win, win->context->private_data);
	}

	pthread_mutex_lock(&win->lock);

	for (child = win->child; child; child = child->next)
	{
		cavan_window_destory(child);
	}

	pthread_mutex_unlock(&win->lock);

	win->destory_handler(win);
}

static void cavan_window_clicked(struct cavan_window *win, bool pressed)
{
	struct cavan_application_context *context;

	pthread_mutex_lock(&win->lock);
	win->pressed = pressed;
	context = win->context;
	pthread_mutex_unlock(&win->lock);

	if (win->on_clicked == NULL || win->on_clicked(win, context->x, context->y, pressed, context->private_data) == false)
	{
		win->click_handler(win, pressed);
	}
}

static void cavan_window_mouse_move(struct cavan_window *win, int x, int y)
{
	if (win->on_move == NULL || win->on_move(win, x, y, win->context->private_data) == false)
	{
		win->move_handler(win, x, y);
	}
}

static void cavan_window_mouse_entry(struct cavan_window *win)
{
	if (win->on_entry == NULL || win->on_entry(win, win->context->private_data) == false)
	{
		win->entry_handler(win);
	}
}

static void cavan_window_mouse_exit(struct cavan_window *win)
{
	if (win->on_exit == NULL || win->on_exit(win, win->context->private_data) == false)
	{
		win->exit_handler(win);
	}
}

static void cavan_window_key_pressed(struct cavan_window *win, const char *name, int code, int value)
{
	if (win->on_key_pressed == NULL || win->on_key_pressed(win, name, code, value, win->context->private_data) == false)
	{
		win->key_handler(win, name, code, value);
	}
}

void cavan_window_set_back_color(struct cavan_window *win, float red, float green, float blue)
{
	pthread_mutex_lock(&win->lock);
	win->back_color = cavan_display_build_color3f(win->context->display, red, green, blue);
	pthread_mutex_unlock(&win->lock);
	cavan_window_paint(win);
}

void cavan_window_set_fore_color(struct cavan_window *win, float red, float green, float blue)
{
	pthread_mutex_lock(&win->lock);
	win->fore_color = cavan_display_build_color3f(win->context->display, red, green, blue);
	pthread_mutex_unlock(&win->lock);
	cavan_window_paint(win);
}

void cavan_window_set_border_color(struct cavan_window *win, float red, float green, float blue)
{
	pthread_mutex_lock(&win->lock);
	win->border_color = cavan_display_build_color3f(win->context->display, red, green, blue);
	pthread_mutex_unlock(&win->lock);
	cavan_window_paint(win);
}

void cavan_window_set_text(struct cavan_window *win, const char *text)
{
	pthread_mutex_lock(&win->lock);
	text_ncopy(win->text, text, sizeof(win->text));
	pthread_mutex_unlock(&win->lock);
	cavan_window_paint(win);
}

void cavan_window_set_width(struct cavan_window *win, int width)
{
	pthread_mutex_lock(&win->lock);
	win->width = width;
	pthread_mutex_unlock(&win->lock);
	cavan_window_paint(win);
}

void cavan_window_set_height(struct cavan_window *win, int height)
{
	pthread_mutex_lock(&win->lock);
	win->height = height;
	pthread_mutex_unlock(&win->lock);
	cavan_window_paint(win);
}

void cavan_window_set_border_width(struct cavan_window *win, int width)
{
	pthread_mutex_lock(&win->lock);
	win->border_width = width;
	pthread_mutex_unlock(&win->lock);
	cavan_window_paint(win);
}

// ================================================================================

int cavan_window_init_handler(struct cavan_window *win, struct cavan_application_context *context)
{
	struct cavan_display_device *display;

	win->context = context;

	display = context->display;
	win->back_color = cavan_display_build_color3f(display, 0, 0, 0);
	win->fore_color = cavan_display_build_color3f(display, 1.0, 1.0, 1.0);
	win->border_color = cavan_display_build_color3f(display, 1.0, 1.0, 1.0);

	return 0;
}

void cavan_window_destory_handler(struct cavan_window *win)
{
	pthread_mutex_destroy(&win->lock);
}

void cavan_window_paint_handler(struct cavan_window *win)
{
	int x, y;
	int width, height;
	struct cavan_application_context *context;
	struct cavan_display_device *display;

	pthread_mutex_lock(&win->lock);

	context = win->context;
	display = context->display;

	x = win->abs_x + win->border_width;
	y = win->abs_y + win->border_width;
	width = win->width - win->border_width * 2;
	height = win->height - win->border_width * 2;

	display->set_color(display, win->back_color);
	display->fill_rect(display, x, y, width, height);

	display->set_color(display, win->border_color);
	display->fill_rect(display, win->abs_x, win->abs_y, win->width, win->border_width);
	display->fill_rect(display, win->abs_x, y + height, win->width, win->border_width);
	display->fill_rect(display, win->abs_x, y, win->border_width, height);
	display->fill_rect(display, x + width, y, win->border_width, height);

	pthread_mutex_unlock(&win->lock);
}

void cavan_window_key_handler(struct cavan_window *win, const char *name, int code, int value)
{
}

void cavan_window_click_handler(struct cavan_window *win, bool pressed)
{
}

void cavan_window_move_handler(struct cavan_window *win, int x, int y)
{
}

void cavan_window_entry_handler(struct cavan_window *win)
{
}

void cavan_window_exit_handler(struct cavan_window *win)
{
}

void cavan_window_get_rect_handler(struct cavan_window *win, struct cavan_display_rect *rect)
{
	rect->x = win->border_width;
	rect->y = win->border_width;
	rect->width = win->width - win->border_width * 2;
	rect->height = win->height - win->border_width * 2;
}

int cavan_window_init_base(struct cavan_window *win, int (*handler)(struct cavan_window *win, struct cavan_application_context *context))
{
	pthread_mutex_init(&win->lock, NULL);

	win->child = NULL;
	win->parent = NULL;
	win->next = NULL;

	win->on_destory = NULL;
	win->on_paint = NULL;
	win->on_clicked = NULL;
	win->on_double_clicked = NULL;
	win->on_move = NULL;
	win->on_entry = NULL;
	win->on_exit = NULL;
	win->on_key_pressed = NULL;

	win->pressed = false;
	win->init_handler = handler ? handler : cavan_window_init_handler;
	win->destory_handler = cavan_window_destory_handler;
	win->paint_handler = cavan_window_paint_handler;
	win->click_handler = cavan_window_click_handler;
	win->move_handler = cavan_window_move_handler;
	win->entry_handler = cavan_window_entry_handler;
	win->exit_handler = cavan_window_exit_handler;
	win->key_handler = cavan_window_key_handler;
	win->get_rect_handler = cavan_window_get_rect_handler;

	return 0;
}

static void cavan_window_set_abs_position_base(struct cavan_window *win, int x, int y)
{
	struct cavan_window *child;

	pthread_mutex_lock(&win->lock);
	win->abs_x = x;
	win->abs_y = y;
	pthread_mutex_unlock(&win->lock);

	for (child = win->child; child; child = child->next)
	{
		cavan_window_set_abs_position_base(child, child->x + x, child->y + y);
	}
}

void cavan_window_set_abs_position(struct cavan_window *win, int x, int y)
{
	cavan_window_set_abs_position_base(win, x, y);
	cavan_window_paint(win->parent);
}

void cavan_window_set_position(struct cavan_window *win, int x, int y)
{
	struct cavan_display_rect rect;

	pthread_mutex_lock(&win->lock);

	win->x = x;
	win->y = y;

	win->parent->get_rect_handler(win->parent, &rect);

	x += rect.x;
	y += rect.y;

	pthread_mutex_unlock(&win->lock);

	cavan_window_set_abs_position(win, x, y);
}

struct cavan_window *cavan_window_find_by_axis(struct cavan_window *head, int x, int y)
{
	struct cavan_window *win;

	for (win = head; win; win = win->next)
	{
		if (x >= win->abs_x && y >= win->abs_y && x < win->abs_x + win->width && y < win->abs_y + win->height)
		{
			struct cavan_window *child = cavan_window_find_by_axis(win->child, x, y);
			if (child)
			{
				return child;
			}
			else
			{
				return win;
			}
		}
	}

	return NULL;
}

int cavan_window_add_child(struct cavan_window *win, struct cavan_window *child)
{
	int ret;
	struct cavan_display_rect rect;

	ret = child->init_handler(child, win->context);
	if (ret  < 0)
	{
		return ret;
	}

	pthread_mutex_lock(&win->lock);

	win->get_rect_handler(win, &rect);

	if (child->x < 0)
	{
		child->x = rect.x;
	}

	if (child->width <= 0)
	{
		child->width = rect.width;
	}

	if (child->y < 0)
	{
		child->y = rect.y;
	}

	if (child->height <= 0)
	{
		child->height = rect.height;
	}

	child->parent = win;
	child->next = win->child;
	win->child = child;

	pthread_mutex_unlock(&win->lock);

	cavan_window_set_abs_position(child, child->x + win->abs_x, child->y + win->abs_y);

	return 0;
}

int cavan_window_remove_child(struct cavan_window *win, struct cavan_window *child)
{
	int ret;
	struct cavan_window *p;

	pthread_mutex_lock(&win->lock);

	if (win->child == child)
	{
		ret = 0;
		win->child = child->next;

		goto out_mutex_unlock;
	}

	for (p = win->child; p; p = p->next)
	{
		if (p->next == child)
		{
			ret = 0;
			p->next = child->next;

			goto out_mutex_unlock;
		}
	}

	ret = -ENOENT;

out_mutex_unlock:
	pthread_mutex_unlock(&win->lock);
	return ret;
}

// ================================================================================

void cavan_dialog_paint_handler(struct cavan_window *win)
{
	struct cavan_dialog *dialog = (struct cavan_dialog *)win;
	struct cavan_display_device *display;

	cavan_window_paint_handler(win);

	pthread_mutex_lock(&win->lock);

	if (dialog->title_height > 0 && win->text[0])
	{
		int x, y;

		display = win->context->display;

		display->set_color(display, win->border_color);
		display->fill_rect(display, win->abs_x + win->border_width, win->abs_y + dialog->title_height, win->width - 2 * win->border_width, win->border_width);

		x = win->abs_x + (win->width - (int)display->mesure_text(display, win->text)) / 2 + win->border_width;
		y = dialog->title_height / 2 + win->border_width;

		display->set_color(display, win->fore_color);
		display->draw_text(display, x, y, win->text);
	}

	pthread_mutex_unlock(&win->lock);
}

void cavan_dialog_click_handler(struct cavan_window *win, bool pressed)
{
	struct cavan_dialog *dialog = (struct cavan_dialog *)win;

	pthread_mutex_lock(&win->lock);

	if (pressed)
	{
		struct cavan_application_context *context = win->context;

		dialog->x_offset = context->x - win->abs_x;
		dialog->y_offset = context->y - win->abs_y;
	}
	else if (dialog->backup)
	{
		struct cavan_display_memory_rect *mem = dialog->backup;

		if (win->x != mem->x && win->y != mem->y)
		{
			pthread_mutex_unlock(&win->lock);
			cavan_window_set_abs_position(win, mem->x, mem->y);
			pthread_mutex_lock(&win->lock);
		}

		cavan_display_memory_rect_free(mem);
		dialog->backup = NULL;
	}

	pthread_mutex_unlock(&win->lock);
}

void cavan_dialog_move_handler(struct cavan_window *win, int x, int y)
{
	struct cavan_dialog *dialog = (struct cavan_dialog *)win;

	pthread_mutex_lock(&win->lock);

	if (dialog->move_able && win->pressed && dialog->backup == NULL)
	{
		struct cavan_display_device *display;
		struct cavan_display_memory_rect *mem;

		display = win->context->display;
		mem = cavan_display_memory_rect_alloc(display, win->width, win->height, 1);
		if (mem)
		{
			mem->x = win->abs_x;
			mem->y = win->abs_y;
		}

		dialog->backup = mem;
	}

	if (dialog->backup)
	{
		struct cavan_display_device *display = win->context->display;
		struct cavan_display_memory_rect *mem = dialog->backup;

		cavan_display_memory_rect_restore(display, mem);
		cavan_display_memory_rect_backup(display, mem, x - dialog->x_offset, y - dialog->y_offset);

		cavan_display_set_color3f(display, 0, 1.0, 0.5);
		display->draw_rect(display, mem->x, mem->y, mem->width, mem->height);
		display->refresh(display);
	}

	pthread_mutex_unlock(&win->lock);
}

void cavan_dialog_get_rect_handler(struct cavan_window *win, struct cavan_display_rect *rect)
{
	struct cavan_dialog *dialog = (struct cavan_dialog *)win;

	rect->x = win->border_width;
	rect->y = win->border_width + dialog->title_height;
	rect->width = win->width - win->border_width * 2;
	rect->height = win->height - dialog->title_height - win->border_width * 2;
}

int cavan_dialog_init_handler(struct cavan_window *win, struct cavan_application_context *context)
{
	int ret;
	struct cavan_dialog *dialog = (struct cavan_dialog *)win;

	ret = cavan_window_init_handler(win, context);
	if (ret < 0)
	{
		return ret;
	}

	if (win->x < 0 && win->y < 0 && win->width <= 0 && win->height <= 0)
	{
		dialog->move_able = false;
	}

	dialog->backup = NULL;
	dialog->x_offset = dialog->y_offset = 0;

	win->paint_handler = cavan_dialog_paint_handler;
	win->click_handler = cavan_dialog_click_handler;
	win->move_handler = cavan_dialog_move_handler;
	win->get_rect_handler = cavan_dialog_get_rect_handler;

	return 0;
}

// ================================================================================

void cavan_button_paint_handler(struct cavan_window *win)
{
	int x, y;
	struct cavan_display_device *display;

	cavan_window_paint_handler(win);

	pthread_mutex_lock(&win->lock);

	display = win->context->display;

	x = win->abs_x + (win->width - (int)display->mesure_text(display, win->text)) / 2;
	y = win->abs_y + win->height / 2;

	display->set_color(display, win->fore_color);
	display->draw_text(display, x, y, win->text);

	pthread_mutex_unlock(&win->lock);
}

void cavan_button_click_handler(struct cavan_window *win, bool pressed)
{
	struct cavan_display_device *display;
	struct cavan_button *button = (struct cavan_button *)win;

	pthread_mutex_lock(&win->lock);

	display = win->context->display;

	if (pressed)
	{
		button->back_color_backup = win->back_color;
		button->fore_color_backup = win->fore_color;
		button->border_color_backup = win->border_color;

		win->back_color = cavan_display_build_color3f(display, 0, 1.0, 0);
		win->fore_color = cavan_display_build_color3f(display, 0, 0, 0);
		win->border_color = cavan_display_build_color3f(display, 0, 0, 1.0);
	}
	else
	{
		win->back_color = button->back_color_backup;
		win->fore_color = button->fore_color_backup;
		win->border_color = button->border_color_backup;
	}

	pthread_mutex_unlock(&win->lock);

	cavan_window_paint(win);
}

int cavan_button_init_handler(struct cavan_window *win, struct cavan_application_context *context)
{
	int ret;
	struct cavan_display_device *display;

	ret = cavan_window_init_handler(win, context);
	if (ret < 0)
	{
		return ret;
	}

	win->paint_handler = cavan_button_paint_handler;
	win->click_handler = cavan_button_click_handler;

	display = context->display;
	win->back_color = cavan_display_build_color3f(display, 0, 0, 0);
	win->fore_color = cavan_display_build_color3f(display, 1.0, 1.0, 1.0);
	win->border_color = cavan_display_build_color3f(display, 1.0, 1.0, 1.0);

	return 0;
}

// ================================================================================

void cavan_progress_bar_paint_handler(struct cavan_window *win)
{
	struct cavan_display_rect rect;
	struct cavan_progress_bar *bar;
	struct cavan_display_device *display;
	char buff[16];
	double percent;
	int width;

	cavan_window_paint_handler(win);

	pthread_mutex_lock(&win->lock);

	POINTER_ASSIGN_CASE(bar, win);

	if (bar->total)
	{
		percent = bar->pos / bar->total;
	}
	else
	{
		percent = 1.0;
	}

	win->get_rect_handler(win, &rect);
	rect.x += win->abs_x;
	rect.y += win->abs_y;

	display = win->context->display;

	width = rect.width * percent;
	display->set_color(display, bar->complete_color);
	display->fill_rect(display, rect.x, rect.y, width, rect.height);

	sprintf(buff, "%2.2lf%%", percent * 100);
	display->set_color(display, win->fore_color);
	cavan_display_draw_text_centre(display, &rect, buff);

	pthread_mutex_unlock(&win->lock);
}

int cavan_progress_bar_init_handler(struct cavan_window *win, struct cavan_application_context *context)
{
	int ret;
	struct cavan_display_device *display;
	struct cavan_progress_bar *bar;

	ret = cavan_window_init_handler(win, context);
	if (ret < 0)
	{
		return ret;
	}

	win->paint_handler = cavan_progress_bar_paint_handler;

	POINTER_ASSIGN_CASE(bar, win);

	display = context->display;
	bar->complete_color = cavan_display_build_color3f(display, 0, 0, 1.0);

	return 0;
}

int cavan_progress_bar_init(struct cavan_progress_bar *bar, double total)
{
	int ret;

	ret = cavan_window_init_base(&bar->window, cavan_progress_bar_init_handler);
	if (ret < 0)
	{
		return ret;
	}

	bar->pos = 0;
	bar->total = total;

	return 0;
}

int cavan_progress_bar_start(struct cavan_progress_bar *bar, double total)
{
	if (total <= 0)
	{
		pr_red_info("total <= 0");
		return -EINVAL;
	}

	bar->pos = 0;

	cavan_window_paint(&bar->window);

	return 0;
}

void cavan_progress_bar_set_pos(struct cavan_progress_bar *bar, double pos)
{
	if (pos > bar->total)
	{
		bar->pos = bar->total;
	}
	else
	{
		bar->pos = pos;
	}

	cavan_window_paint(&bar->window);
}

// ================================================================================

static inline struct cavan_display_memory *cavan_application_mouse_alloc(struct cavan_application_context *context)
{
	return cavan_display_memory_alloc(context->display, CAVAN_MOUSE_SIZE, CAVAN_MOUSE_SIZE);
}

static inline int cavan_application_restore_mouse(struct cavan_application_context *context)
{
	return cavan_display_memory_restore(context->display, context->mouse_backup);
}

static void cavan_application_draw_mouse(struct cavan_application_context *context)
{
	struct cavan_display_device *display = context->display;
	struct cavan_display_memory *mem = context->mouse_backup;

	cavan_display_memory_backup(display, mem, context->x, context->y);
	cavan_display_set_color3f(display, 1.0, 0, 0);
	display->draw_rect(display, context->x, context->y, CAVAN_MOUSE_SIZE, CAVAN_MOUSE_SIZE);
	display->refresh(display);
}

static void cavan_application_move(struct cavan_application_context *context, int x, int y)
{
	if (context->win_active)
	{
		cavan_window_mouse_move(context->win_active, x, y);
	}
	else
	{
		struct cavan_window *win = cavan_window_find_by_axis(&context->win_root, x, y);

		if (win)
		{
			if (win != context->win_curr)
			{
				if (context->win_curr)
				{
					cavan_window_mouse_exit(context->win_curr);
				}

				cavan_window_mouse_entry(win);
			}

			cavan_window_mouse_move(win, x, y);
		}
		else if (context->win_curr)
		{
			cavan_window_mouse_exit(context->win_curr);
		}

		context->win_curr = win;
	}

	context->x = x;
	context->y = y;
}

static void cavan_application_click(struct cavan_application_context *context, bool pressed)
{
	if (pressed)
	{
		struct cavan_window *win = cavan_window_find_by_axis(&context->win_root, context->x, context->y);
		if (win && win->pressed == false)
		{
			cavan_window_clicked(win, true);
		}

		if (win != context->win_curr)
		{
			if (context->win_curr)
			{
				cavan_window_mouse_exit(context->win_curr);
			}

			if (win)
			{
				cavan_window_mouse_entry(win);
			}

			context->win_curr = win;
		}

		context->win_active = win;
	}
	else if (context->win_active)
	{
		if (context->win_active->pressed)
		{
			cavan_window_clicked(context->win_active, false);
		}

		context->win_active = NULL;
	}
}

// ================================================================================

static void cavan_application_key_handler(struct cavan_input_device *dev, const char *name, int code, int value, void *data)
{
	struct cavan_application_context *context = data;

	pthread_mutex_lock(&context->lock);

	if (context->win_active)
	{
		cavan_window_key_pressed(context->win_active, name, code, value);
	}
	else if (context->win_curr)
	{
		cavan_window_key_pressed(context->win_curr, name, code, value);
	}

	pthread_mutex_unlock(&context->lock);
}

static void cavan_application_mouse_wheel_handler(struct cavan_input_device *dev, int code, int value, void *data)
{
}

static void cavan_application_mouse_move_handler(struct cavan_input_device *dev, int x, int y, void *data)
{
	struct cavan_application_context *context = data;

	pthread_mutex_lock(&context->lock);

	x = x * context->mouse_speed + context->x;
	if (x < 0)
	{
		x = 0;
	}
	else if (x > context->max_x)
	{
		x = context->max_x;
	}

	y = y * context->mouse_speed + context->y;
	if (y < 0)
	{
		y = 0;
	}
	else if (y > context->max_y)
	{
		y = context->max_y;
	}

	cavan_application_restore_mouse(context);
	cavan_application_move(context, x, y);
	cavan_application_draw_mouse(context);

	pthread_mutex_unlock(&context->lock);
}

static void cavan_application_mouse_touch_handler(struct cavan_input_device *dev, int code, int value, void *data)
{
	struct cavan_application_context *context = data;

	pthread_mutex_lock(&context->lock);

	cavan_application_restore_mouse(context);
	cavan_application_click(context, value > 0);
	cavan_application_draw_mouse(context);

	pthread_mutex_unlock(&context->lock);
}

static void cavan_application_touch_handler(struct cavan_input_device *dev, struct cavan_touch_point *point, void *data)
{
	struct cavan_application_context *context;

	if (point->id != 0)
	{
		pr_std_info("point->id = %d, skiping ...", point->id);
		return;
	}

	context = data;

	pthread_mutex_lock(&context->lock);

	context->x = point->x;
	context->y = point->y;

	cavan_application_click(context, point->pressure > 0);

	pthread_mutex_unlock(&context->lock);
}

static void cavan_application_move_handler(struct cavan_input_device *dev, struct cavan_touch_point *point, void *data)
{
	struct cavan_application_context *context = data;

	pthread_mutex_lock(&context->lock);
	cavan_application_move(data, point->x, point->y);
	pthread_mutex_unlock(&context->lock);
}

static void cavan_application_gsensor_handler(struct cavan_input_device *dev, struct cavan_gsensor_event *event, void *data)
{
}

// ================================================================================

int cavan_application_init(struct cavan_application_context *context, struct cavan_display_device *display, void *data)
{
	int ret;
	struct cavan_input_service *input_service;
	struct cavan_window *win;

	if (display == NULL)
	{
		display = cavan_fb_display_create();
		if (display == NULL)
		{
			pr_red_info("cavan_fb_display_create");
			return -EFAULT;
		}
	}

	ret = cavan_display_check(display);
	if (ret < 0)
	{
		pr_red_info("cavan_display_check");
		goto out_display_destory;
	}

	context->display = display;

	win = &context->win_root;
	ret = cavan_window_init(win);
	if (ret < 0)
	{
		pr_red_info("cavan_window_init");
		goto out_display_destory;
	}

	text_copy(win->text, "ROOT");
	win->parent = win;
	win->x = win->abs_x = 0;
	win->y = win->abs_y = 0;
	win->width = display->xres;
	win->height = display->yres;
	win->border_width = 2;
	win->back_color = cavan_display_build_color3f(display, 0, 0, 0);
	win->fore_color = cavan_display_build_color3f(display, 1.0, 1.0, 1.0);
	win->border_color = cavan_display_build_color3f(display, 1.0, 1.0, 1.0);

	ret = win->init_handler(win, context);
	if (ret < 0)
	{
		pr_red_info("win->init_handler");
		goto out_display_destory;
	}

	context->private_data = data;
	context->win_curr = NULL;
	context->win_active = NULL;

	pthread_mutex_init(&context->lock, NULL);

	context->mouse_backup = cavan_application_mouse_alloc(context);
	if (context->mouse_backup == NULL)
	{
		ret = -ENOMEM;
		pr_red_info("cavan_display_memory_alloc");
		goto out_win_destroy;
	}

	context->mouse_speed = 1.0;
	context->x = context->y = 0;
	context->max_x = display->xres - 1;
	context->max_y = display->yres - 1;

	input_service = &context->input_service;
	cavan_input_service_init(input_service, NULL);
	input_service->lcd_width = display->xres;
	input_service->lcd_height = display->yres;
	input_service->key_handler = cavan_application_key_handler;
	input_service->mouse_wheel_handler = cavan_application_mouse_wheel_handler;
	input_service->mouse_move_handler = cavan_application_mouse_move_handler;
	input_service->mouse_touch_handler = cavan_application_mouse_touch_handler;
	input_service->touch_handler = cavan_application_touch_handler;
	input_service->move_handler = cavan_application_move_handler;
	input_service->gsensor_handler = cavan_application_gsensor_handler;

	ret = pipe(context->pipefd);
	if (ret < 0)
	{
		pr_error_info("pipe");
		goto out_display_memory_free;
	}

	return 0;

out_display_memory_free:
	cavan_display_memory_free(context->mouse_backup);
out_win_destroy:
	pthread_mutex_destroy(&context->lock);
	cavan_window_destory(win);
out_display_destory:
	display->destory(display);

	return ret;
}

void cavan_application_uninit(struct cavan_application_context *context)
{
	close(context->pipefd[0]);
	close(context->pipefd[1]);

	cavan_display_memory_free(context->mouse_backup);
	context->display->destory(context->display);
	cavan_window_destory(&context->win_root);

	pthread_mutex_destroy(&context->lock);
}

int cavan_application_main_loop(struct cavan_application_context *context, void (*handler)(struct cavan_application_context *context, void *data), void *data)
{
	int ret;
	int fd;
	enum cavan_application_event event;

	ret = cavan_input_service_start(&context->input_service, context);
	if (ret < 0)
	{
		pr_red_info("cavan_input_service_start");
		return ret;
	}

	cavan_window_paint(&context->win_root);

	if (handler)
	{
		handler(context, data);
	}

	fd = context->pipefd[0];

	while (1)
	{
		ret = read(fd, &event, sizeof(event));
		if (ret < (int)sizeof(event))
		{
			pr_error_info("read");
			break;
		}

		pr_bold_info("event = %d", event);

		switch (event)
		{
		case CAVAN_APP_EVENT_STOP:
		case CAVAN_APP_EVENT_EXIT:
			goto out_cavan_input_service_stop;

		default:
			pr_red_info("unknown event %d", event);
		}
	}

out_cavan_input_service_stop:
	cavan_input_service_stop(&context->input_service);
	cavan_input_service_join(&context->input_service);

	return 0;
}
