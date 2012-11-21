/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Tue Nov 20 16:09:41 CST 2012
 */

#include <cavan.h>
#include <cavan/window.h>

static void cavan_window_paint_handler(struct cavan_window *win)
{
	struct cavan_application_context *context = win->private_data;
	struct cavan_display_device *display = &context->display;

	int x, y;
	int width, height;

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

	display->set_color(display, win->fore_color);
	width = display->mesure_text(display, win->text);
	display->draw_text(display, win->abs_x + width / 2, win->abs_y, win->text);

	if (win->on_paint)
	{
		win->on_paint(win, context->private_data);
	}
}

static void cavan_window_destory_handler(struct cavan_window *win)
{
	struct cavan_window *child;

	pthread_mutex_destroy(&win->lock);

	for (child = win->child; child; child = child->next)
	{
		win->destory_handler(win);
	}
}

static void cavan_window_click_handler(struct cavan_window *win, bool pressed)
{
	struct cavan_application_context *context = win->private_data;

	if (win->on_click)
	{
		win->on_click(win, context->x, context->y, pressed, context->private_data);
	}
}

static void cavan_window_move_handler(struct cavan_window *win, int x, int y)
{
	struct cavan_application_context *context = win->private_data;

	if (win->on_move)
	{
		win->on_move(win, x, y, context->private_data);
	}
}

static void cavan_window_entry_handler(struct cavan_window *win)
{
	struct cavan_application_context *context = win->private_data;

	if (win->on_entry)
	{
		win->on_entry(win, context->private_data);
	}
}

static void cavan_window_exit_handler(struct cavan_window *win)
{
	struct cavan_application_context *context = win->private_data;

	if (win->on_exit)
	{
		win->on_exit(win, context->private_data);
	}
}

static int cavan_window_init(struct cavan_window *win, void *data)
{
	pthread_mutex_init(&win->lock, NULL);

	win->child = NULL;
	win->parent = NULL;
	win->next = NULL;

	win->private_data = data;

	win->on_paint = NULL;
	win->on_click = NULL;
	win->on_move = NULL;
	win->on_entry = NULL;
	win->on_exit = NULL;

	win->destory_handler = cavan_window_destory_handler;
	win->paint_handler = cavan_window_paint_handler;
	win->click_handler = cavan_window_click_handler;
	win->move_handler = cavan_window_move_handler;
	win->entry_handler = cavan_window_entry_handler;
	win->exit_handler = cavan_window_exit_handler;

	return 0;
}

static void cavan_window_paint(struct cavan_window *win)
{
	struct cavan_window *child;

	win->paint_handler(win);

	for (child = win->child; child; child = child->next)
	{
		cavan_window_paint(child);
	}
}

static void cavan_window_set_abs_position(struct cavan_window *win, int x, int y)
{
	struct cavan_window *child;

	win->abs_x = x;
	win->abs_y = y;

	for (child = win->child; child; child = child->next)
	{
		cavan_window_set_abs_position(child, child->x + x, child->y + y);
	}
}

void cavan_window_set_position(struct cavan_window *win, int x, int y)
{
	struct cavan_window *child;

	win->x = x;
	win->y = y;

	if (win->parent)
	{
		win->abs_x = win->parent->abs_x + x;
		win->abs_y = win->parent->abs_y + y;
	}
	else
	{
		struct cavan_application_context *context = win->private_data;
		struct cavan_display_device *display = &context->display;

		display->set_color(display, cavan_display_build_color3f(display, 0, 0, 0, 0));
		display->fill_rect(display, win->abs_x, win->abs_y, win->width, win->height);

		win->abs_x = x;
		win->abs_y = y;
	}

	for (child = win->child; child; child = child->next)
	{
		cavan_window_set_abs_position(child, child->x + win->abs_x, child->y + win->abs_y);
	}

	if (win->parent)
	{
		cavan_window_paint(win->parent);
	}
	else
	{
		cavan_window_paint(win);
	}
}

static struct cavan_window *cavan_window_find_by_axis(struct cavan_window *head, int x, int y)
{
	struct cavan_window *win;

	for (win = head; win; win = win->next)
	{
		if (x >= win->x && y >= win->y && x < win->x + win->width && y < win->y + win->height)
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

void cavan_window_add_child(struct cavan_window *win, struct cavan_window *child)
{
	pthread_mutex_lock(&win->lock);

	cavan_window_set_abs_position(child, child->x + win->abs_x, child->y + win->abs_y);
	child->parent = win;
	child->next = win->child;
	win->child = child;

	pthread_mutex_unlock(&win->lock);
}

int cavan_window_remove_child(struct cavan_window *win, struct cavan_window *child)
{
	struct cavan_window *p;

	pthread_mutex_lock(&win->lock);

	if (win->child == child)
	{
		win->child = child->next;
		goto out_found;
	}

	for (p = win->child; p; p = p->next)
	{
		if (p->next == child)
		{
			p->next = child->next;
			goto out_found;
		}
	}

	pthread_mutex_unlock(&win->lock);
	return -ENOENT;

out_found:
	pthread_mutex_unlock(&win->lock);
	return 0;
}

int cavan_dislog_init(struct cavan_dialog *dialog, void *data)
{
	int ret;
	struct cavan_window *win = &dialog->window;

	ret = cavan_window_init(win, data);
	if (ret < 0)
	{
		pr_red_info("cavan_window_init");
		return ret;
	}

	return 0;
}

static void cavan_button_paint_handler(struct cavan_window *win)
{
	struct cavan_button *button = (struct cavan_button *)win;
	struct cavan_application_context *context = win->private_data;
	struct cavan_display_device *display = &context->display;

	if (button->pressed)
	{
		win->back_color = cavan_display_build_color3f(display, 1.0, 1.0, 0, 1.0);
		win->fore_color = cavan_display_build_color3f(display, 1.0, 0, 1.0, 1.0);
		win->border_color = cavan_display_build_color3f(display, 1.0, 0, 0, 1.0);
	}
	else
	{
		win->back_color = cavan_display_build_color3f(display, 0.5, 0.5, 0.5, 1.0);
		win->fore_color = cavan_display_build_color3f(display, 0, 0, 0, 1.0);
		win->border_color = cavan_display_build_color3f(display, 1.0, 1.0, 1.0, 1.0);
	}

	cavan_window_paint_handler(win);
}

static void cavan_button_click_handler(struct cavan_window *win, bool pressed)
{
	struct cavan_button *button = (struct cavan_button *)win;

	button->pressed = pressed;
	win->paint_handler(win);

	cavan_window_click_handler(win, pressed);
}

int cavan_button_init(struct cavan_button *button, void *data)
{
	int ret;
	struct cavan_window *win = &button->window;

	ret = cavan_window_init(win, data);
	if (ret < 0)
	{
		pr_red_info("cavan_window_init");
		return ret;
	}

	win->paint_handler = cavan_button_paint_handler;
	win->click_handler = cavan_button_click_handler;

	button->pressed = false;

	return 0;

}

// ================================================================================

static inline struct cavan_display_memory *cavan_application_mouse_alloc(struct cavan_application_context *context)
{
	return cavan_display_memory_alloc(&context->display, CAVAN_MOUSE_SIZE, CAVAN_MOUSE_SIZE);
}

static inline void cavan_application_restore_mouse(struct cavan_application_context *context)
{
	struct cavan_display_device *display = &context->display;

	display->display_memory_xfer(display, context->mouse_point_mem, false);
}

static inline void cavan_application_draw_mouse(struct cavan_application_context *context)
{
	struct cavan_display_device *display = &context->display;
	struct cavan_display_memory *mem = context->mouse_point_mem;

	mem->x = context->x;
	mem->y = context->y;
	mem->width = mem->width_max;
	mem->height = mem->height_max;
	display->display_memory_xfer(display, mem, true);

	display->set_color(display, cavan_display_build_color3f(display, 1.0, 0, 0, 0));
	display->draw_rect(display, context->x, context->y, CAVAN_MOUSE_SIZE, CAVAN_MOUSE_SIZE);
}

static void cavan_application_move(struct cavan_application_context *context, int x, int y)
{
	struct cavan_window *win;

	pthread_mutex_lock(&context->lock);

	win = cavan_window_find_by_axis(context->win_head, x, y);
	if (win != context->win_curr)
	{
		if (context->win_curr)
		{
			context->win_curr->exit_handler(context->win_curr);
		}

		if (win)
		{
			win->entry_handler(win);
		}

		context->win_curr = win;
	}
	else if (win)
	{
		win->move_handler(win, x, y);
	}

	if (context->win_active && win != context->win_active)
	{
		context->win_active->click_handler(context->win_active, false);
		context->win_active = NULL;
	}

	context->x = x;
	context->y = y;

	pthread_mutex_unlock(&context->lock);
}

static void cavan_application_click(struct cavan_application_context *context, bool pressed)
{
	pthread_mutex_lock(&context->lock);

	if (pressed)
	{
		struct cavan_window *win = cavan_window_find_by_axis(context->win_head, context->x, context->y);
		if (win)
		{
			win->click_handler(win, true);
		}

		context->win_active = win;
	}
	else if (context->win_active)
	{
		context->win_active->click_handler(context->win_active, false);
		context->win_active = NULL;
	}

	pthread_mutex_unlock(&context->lock);
}

// ================================================================================

static void cavan_application_key_handler(struct cavan_input_device *dev, const char *name, int code, int value, void *data)
{
	pr_pos_info();
}

static void cavan_application_mouse_wheel_handler(struct cavan_input_device *dev, int value, void *data)
{
	pr_pos_info();
}

static void cavan_application_mouse_move_handler(struct cavan_input_device *dev, int x, int y, void *data)
{
	struct cavan_application_context *context = data;

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
}

static void cavan_application_mouse_touch_handler(struct cavan_input_device *dev, int button, bool pressed, void *data)
{
	cavan_application_restore_mouse(data);
	cavan_application_click(data, pressed);
	cavan_application_draw_mouse(data);
}

static void cavan_application_touch_handler(struct cavan_input_device *dev, struct cavan_touch_point *point, void *data)
{
	cavan_application_move(data, point->x, point->y);
	cavan_application_click(data, point->pressure);
}

static void cavan_application_move_handler(struct cavan_input_device *dev, struct cavan_touch_point *point, void *data)
{
	cavan_application_move(data, point->x, point->y);
}

static void cavan_application_gsensor_handler(struct cavan_input_device *dev, struct cavan_gsensor_event *event, void *data)
{
	pr_pos_info();
}

// ================================================================================

int cavan_application_context_init(struct cavan_application_context *context, void *data)
{
	int ret;
	struct cavan_input_service *input_service;
	struct cavan_display_device *display;

	context->private_data = data;
	context->win_curr = NULL;
	context->win_active = NULL;
	context->win_head = NULL;

	display = &context->display;
	ret = cavan_display_init(display);
	if (ret < 0)
	{
		pr_red_info("cavan_display_init");
		return ret;
	}

	context->mouse_point_mem = cavan_application_mouse_alloc(context);
	if (context->mouse_point_mem == NULL)
	{
		pr_red_info("cavan_display_memory_alloc");
		goto out_display_uninit;
	}

	context->x = context->y = 0;
	context->max_x = display->width - 1;
	context->max_y = display->height - 1;
	context->mouse_speed = 1.0;

	cavan_application_draw_mouse(context);

	input_service = &context->input_service;
	cavan_input_service_init(input_service, NULL);
	input_service->key_handler = cavan_application_key_handler;
	input_service->mouse_wheel_handler = cavan_application_mouse_wheel_handler;
	input_service->mouse_move_handler = cavan_application_mouse_move_handler;
	input_service->mouse_touch_handler = cavan_application_mouse_touch_handler;
	input_service->touch_handler = cavan_application_touch_handler;
	input_service->move_handler = cavan_application_move_handler;
	input_service->gsensor_handler = cavan_application_gsensor_handler;
	ret = cavan_input_service_start(input_service, context);
	if (ret < 0)
	{
		pr_red_info("cavan_input_service_start");
		goto out_display_memory_free;
	}

	return 0;

out_display_memory_free:
	cavan_display_memory_free(context->mouse_point_mem);
out_display_uninit:
	cavan_display_uninit(display);

	return ret;
}

void cavan_application_context_uninit(struct cavan_application_context *context)
{
	struct cavan_window *win;

	cavan_input_service_stop(&context->input_service);
	cavan_display_memory_free(context->mouse_point_mem);
	cavan_display_uninit(&context->display);

	for (win = context->win_head; win; win = win->next)
	{
		win->destory_handler(win);
	}
}

void cavan_application_context_add_window(struct cavan_application_context *context, struct cavan_window *win)
{
	pthread_mutex_lock(&context->lock);

	win->next = context->win_head;
	context->win_head = win;
	context->win_count++;

	cavan_window_set_abs_position(win, win->x, win->y);
	cavan_window_paint(win);

	pthread_mutex_unlock(&context->lock);
}

int cavan_application_context_remove_window(struct cavan_application_context *context, struct cavan_window *win)
{
	struct cavan_window *p;

	pthread_mutex_lock(&context->lock);

	if (context->win_head == win)
	{
		context->win_head = win->next;
		goto out_found;
	}

	for (p = context->win_head; p; p = p->next)
	{
		if (p->next == win)
		{
			p->next = win->next;
			goto out_found;
		}
	}

	pthread_mutex_unlock(&context->lock);
	return -ENOENT;

out_found:
	context->win_count--;
	pthread_mutex_unlock(&context->lock);
	return 0;
}
