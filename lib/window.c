/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Tue Nov 20 16:09:41 CST 2012
 */

#include <cavan.h>
#include <cavan/window.h>

int cavan_window_init(struct cavan_window *win, int (*paint)(struct cavan_window *, struct cavan_application_context *))
{
	pthread_mutex_init(&win->lock, NULL);

	win->child = NULL;
	win->parent = NULL;
	win->next = NULL;

	win->paint = paint;
	win->paint_handler = NULL;
	win->click_handler = NULL;
	win->move_handler = NULL;
	win->entry_handler = NULL;
	win->exit_handler = NULL;

	cavan_display_build_color3f(&win->back_color, 0.5, 0.5, 0.5);
	cavan_display_build_color3f(&win->border_color, 1.0, 1.0, 1.0);
	cavan_display_build_color3f(&win->fore_color, 0, 0, 0);

	return 0;
}

void cavan_window_destory(struct cavan_window *win)
{
	struct cavan_window *child;

	pthread_mutex_destroy(&win->lock);

	for (child = win->child; child; child = child->next)
	{
		cavan_window_destory(child);
	}
}

void cavan_window_set_abs_position(struct cavan_window *win, int x, int y)
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
		win->abs_x = x;
		win->abs_y = y;
	}

	for (child = win->child; child; child = child->next)
	{
		cavan_window_set_abs_position(child, child->x + win->abs_x, child->y + win->abs_y);
	}
}

void cavan_window_paint(struct cavan_window *win, struct cavan_application_context *context)
{
	struct cavan_window *child;

	win->paint(win, context);

	for (child = win->child; child; child = child->next)
	{
		cavan_window_paint(child, context);
	}
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

int cavan_window_paint_handler(struct cavan_window *win, struct cavan_application_context *context)
{
	struct cavan_display_device *display = &context->display;

	int x, y;
	int width, height;

	x = win->abs_x + win->border_width;
	y = win->abs_y + win->border_width;
	width = win->width - win->border_width * 2;
	height = win->height - win->border_width * 2;

	display->set_color(display, &win->back_color);
	display->fill_rect(display, x, y, width, height);

	display->set_color(display, &win->border_color);
	display->fill_rect(display, win->abs_x, win->abs_y, win->width, win->border_width);
	display->fill_rect(display, win->abs_x, y + height, win->width, win->border_width);
	display->fill_rect(display, win->abs_x, y, win->border_width, height);
	display->fill_rect(display, x + width, y, win->border_width, height);

	display->set_color(display, &win->fore_color);
	// TODO: Show Text

	if (win->paint_handler)
	{
		win->paint_handler(win, context);
	}

	return 0;
}

int cavan_dialog_paint_handler(struct cavan_window *win, struct cavan_application_context *context)
{
	return cavan_window_paint_handler(win, context);
}

int cavan_button_paint_handler(struct cavan_window *win, struct cavan_application_context *context)
{
	return cavan_window_paint_handler(win, context);
}

// ================================================================================

static void cavan_application_key_handler(struct cavan_input_device *dev, const char *name, int code, int value, void *data)
{
	pr_pos_info();
}

static void cavan_application_wheel_handler(struct cavan_input_device *dev, int value, void *data)
{
	pr_pos_info();
}

static void cavan_application_touch_handler(struct cavan_input_device *dev, struct cavan_touch_point *point, void *data)
{
	pr_pos_info();
}

static void cavan_application_right_touch_handler(struct cavan_input_device *dev, struct cavan_touch_point *point, void *data)
{
	pr_pos_info();
}

static void cavan_application_move_handler(struct cavan_input_device *dev, struct cavan_touch_point *point, void *data)
{
	pr_pos_info();
}

static void cavan_application_release_handler(struct cavan_input_device *dev, struct cavan_touch_point *point, void *data)
{
	pr_pos_info();
}

static void cavan_application_right_release_handler(struct cavan_input_device *dev, struct cavan_touch_point *point, void *data)
{
	pr_pos_info();
}

static void cavan_application_gsensor_handler(struct cavan_input_device *dev, struct cavan_gsensor_event *event, void *data)
{
	pr_pos_info();
}

int cavan_application_context_init(struct cavan_application_context *context, void *data)
{
	int ret;
	struct cavan_input_service *input_service;

	ret = cavan_display_init(&context->display);
	if (ret < 0)
	{
		pr_red_info("cavan_display_init");
		return ret;
	}

	input_service = &context->input_service;
	cavan_input_service_init(input_service, NULL);
	input_service->key_handler = cavan_application_key_handler;
	input_service->wheel_handler = cavan_application_wheel_handler;
	input_service->touch_handler = cavan_application_touch_handler;
	input_service->right_touch_handler = cavan_application_right_touch_handler;
	input_service->move_handler = cavan_application_move_handler;
	input_service->release_handler = cavan_application_release_handler;
	input_service->right_release_handler = cavan_application_right_release_handler;
	input_service->gsensor_handler = cavan_application_gsensor_handler;
	ret = cavan_input_service_start(input_service, context);
	if (ret < 0)
	{
		pr_red_info("cavan_input_service_start");
		goto out_display_uninit;
	}

	return 0;

out_display_uninit:
	cavan_display_uninit(&context->display);

	return ret;
}

void cavan_application_context_uninit(struct cavan_application_context *app)
{
	cavan_input_service_stop(&app->input_service);
	cavan_display_uninit(&app->display);
}

void cavan_application_context_add_window(struct cavan_application_context *context, struct cavan_window *win)
{
	pthread_mutex_lock(&context->lock);

	win->next = context->win_head;
	context->win_head = win;
	context->win_count++;

	cavan_window_set_abs_position(win, win->x, win->y);

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
