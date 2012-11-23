/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Tue Nov 20 16:09:41 CST 2012
 */

#include <cavan.h>
#include <cavan/window.h>
#include <cavan/fb.h>

void cavan_window_paint_handler(struct cavan_window *win)
{
	int x, y;
	int width, height;
	struct cavan_application_context *context;
	struct cavan_display_device *display;
	struct cavan_window *child;

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

	if (win->on_paint)
	{
		pthread_mutex_unlock(&win->lock);
		win->on_paint(win, context->private_data);
	}
	else
	{
		pthread_mutex_unlock(&win->lock);
	}

	for (child = win->child; child; child = child->next)
	{
		child->paint_handler(child);
	}

	display->refresh(display);
}

void cavan_window_destory_handler(struct cavan_window *win)
{
	pthread_mutex_destroy(&win->lock);

	for (win = win->child; win; win = win->next)
	{
		win->destory_handler(win);
	}
}

void cavan_window_click_handler(struct cavan_window *win, bool pressed)
{
	pthread_mutex_lock(&win->lock);

	win->pressed = pressed;

	if (win->on_clicked)
	{
		struct cavan_application_context *context = win->context;

		pthread_mutex_unlock(&win->lock);
		win->on_clicked(win, context->x, context->y, pressed, context->private_data);
	}
	else
	{
		pthread_mutex_unlock(&win->lock);
	}
}

void cavan_window_move_handler(struct cavan_window *win, int x, int y)
{
	pthread_mutex_lock(&win->lock);

	if (win->on_move)
	{
		pthread_mutex_unlock(&win->lock);
		win->on_move(win, x, y, win->context->private_data);
	}
	else
	{
		pthread_mutex_unlock(&win->lock);
	}
}

void cavan_window_entry_handler(struct cavan_window *win)
{
	pthread_mutex_lock(&win->lock);

	if (win->on_entry)
	{
		pthread_mutex_unlock(&win->lock);
		win->on_entry(win, win->context->private_data);
	}
	else
	{
		pthread_mutex_unlock(&win->lock);
	}
}

void cavan_window_exit_handler(struct cavan_window *win)
{
	pthread_mutex_lock(&win->lock);

	if (win->on_exit)
	{
		pthread_mutex_unlock(&win->lock);
		win->on_exit(win, win->context->private_data);
	}
	else
	{
		pthread_mutex_unlock(&win->lock);
	}
}

void cavan_window_key_handler(struct cavan_window *win, const char *name, int code, int value)
{
	pthread_mutex_lock(&win->lock);

	if (win->on_key_pressed)
	{
		pthread_mutex_unlock(&win->lock);
		win->on_key_pressed(win, name, code, value, win->context->private_data);
	}
	else
	{
		pthread_mutex_unlock(&win->lock);
	}
}

int cavan_window_init(struct cavan_window *win, struct cavan_application_context *context)
{
	pthread_mutex_init(&win->lock, NULL);

	win->child = NULL;
	win->parent = NULL;
	win->next = NULL;

	win->context = context;

	win->on_paint = NULL;
	win->on_clicked = NULL;
	win->on_move = NULL;
	win->on_entry = NULL;
	win->on_exit = NULL;
	win->on_key_pressed = NULL;

	win->pressed = false;
	win->destory_handler = cavan_window_destory_handler;
	win->paint_handler = cavan_window_paint_handler;
	win->click_handler = cavan_window_click_handler;
	win->move_handler = cavan_window_move_handler;
	win->entry_handler = cavan_window_entry_handler;
	win->exit_handler = cavan_window_exit_handler;
	win->key_handler = cavan_window_key_handler;

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
	struct cavan_window *parent;

	pthread_mutex_lock(&win->lock);
	parent = win->parent ? win->parent : win;
	pthread_mutex_unlock(&win->lock);

	cavan_window_set_abs_position_base(win, x, y);
	parent->paint_handler(parent);
}

void cavan_window_set_position(struct cavan_window *win, int x, int y)
{
	pthread_mutex_lock(&win->lock);

	win->x = x;
	win->y = y;

	if (win->parent)
	{
		struct cavan_window *parent = win->parent;

		x += parent->abs_x;
		y += parent->abs_y;
	}

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

void cavan_window_add_child(struct cavan_window *win, struct cavan_window *child)
{
	pthread_mutex_lock(&win->lock);

	if (child->x < 0)
	{
		child->x = win->border_width;
	}

	if (child->width <= 0)
	{
		child->width = win->width - child->x - win->border_width;
	}

	if (child->y < 0)
	{
		child->y = win->border_width;
	}

	if (child->height <= 0)
	{
		child->height = win->height - child->y - win->border_width;
	}

	child->parent = win;
	child->next = win->child;
	win->child = child;

	pthread_mutex_unlock(&win->lock);

	cavan_window_set_abs_position(child, child->x + win->abs_x, child->y + win->abs_y);
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
		display->refresh(display);
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

	cavan_window_click_handler(win, pressed);
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

	cavan_window_move_handler(win, x, y);
}

int cavan_dialog_init(struct cavan_dialog *dialog, struct cavan_application_context *context)
{
	int ret;
	struct cavan_display_device *display;
	struct cavan_window *win = &dialog->window;

	ret = cavan_window_init(win, context);
	if (ret < 0)
	{
		pr_red_info("cavan_window_init");
		return ret;
	}

	win->paint_handler = cavan_dialog_paint_handler;
	win->click_handler = cavan_dialog_click_handler;
	win->move_handler = cavan_dialog_move_handler;

	if (win->x < 0 && win->y < 0 && win->width <= 0 && win->height <= 0)
	{
		dialog->move_able = false;
	}

	dialog->backup = NULL;
	dialog->x_offset = dialog->y_offset = 0;

	display = context->display;
	win->back_color = cavan_display_build_color3f(display, 0, 0, 0);
	win->fore_color = cavan_display_build_color3f(display, 1.0, 1.0, 1.0);
	win->border_color = cavan_display_build_color3f(display, 1.0, 1.0, 1.0);

	return 0;
}

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
	display->refresh(display);

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

		win->back_color = cavan_display_build_color3f(display, 1.0, 1.0, 0);
		win->fore_color = cavan_display_build_color3f(display, 1.0, 0, 1.0);
		win->border_color = cavan_display_build_color3f(display, 1.0, 0, 0);
	}
	else
	{
		win->back_color = button->back_color_backup;
		win->fore_color = button->fore_color_backup;
		win->border_color = button->border_color_backup;
	}

	pthread_mutex_unlock(&win->lock);

	win->paint_handler(win);
	cavan_window_click_handler(win, pressed);
}

int cavan_button_init(struct cavan_button *button, struct cavan_application_context *context)
{
	int ret;
	struct cavan_display_device *display;
	struct cavan_window *win = &button->window;

	ret = cavan_window_init(win, context);
	if (ret < 0)
	{
		pr_red_info("cavan_window_init");
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
		context->win_active->move_handler(context->win_active, x, y);
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
					context->win_curr->exit_handler(context->win_curr);
				}

				win->entry_handler(win);
			}

			win->move_handler(win, x, y);
		}
		else if (context->win_curr)
		{
			context->win_curr->exit_handler(context->win_curr);
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
			win->click_handler(win, true);
		}

		context->win_active = win;
	}
	else if (context->win_active)
	{
		if (context->win_active->pressed)
		{
			context->win_active->click_handler(context->win_active, false);
		}

		context->win_active = NULL;
	}
}

// ================================================================================

static void cavan_application_key_handler(struct cavan_input_device *dev, const char *name, int code, int value, void *data)
{
	struct cavan_application_context *context = data;
	struct cavan_window *win;

	pthread_mutex_lock(&context->lock);

	if (context->win_active)
	{
		win = context->win_active;
	}
	else if (context->win_curr)
	{
		win = context->win_curr;
	}
	else
	{
		win = NULL;
	}

	if (win)
	{
		win->key_handler(win, name, code, value);
	}

	pthread_mutex_unlock(&context->lock);
}

static void cavan_application_mouse_wheel_handler(struct cavan_input_device *dev, int value, void *data)
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

static void cavan_application_mouse_touch_handler(struct cavan_input_device *dev, int button, bool pressed, void *data)
{
	struct cavan_application_context *context = data;

	pthread_mutex_lock(&context->lock);

	cavan_application_restore_mouse(context);
	cavan_application_click(context, pressed);
	cavan_application_draw_mouse(context);

	pthread_mutex_unlock(&context->lock);
}

static void cavan_application_touch_handler(struct cavan_input_device *dev, struct cavan_touch_point *point, void *data)
{
	struct cavan_application_context *context = data;

	pthread_mutex_lock(&context->lock);

	cavan_application_move(context, point->x, point->y);
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

int cavan_application_context_init(struct cavan_application_context *context, struct cavan_display_device *display, void *data)
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
	ret = cavan_window_init(win, context);
	if (ret < 0)
	{
		pr_red_info("cavan_window_init");
		goto out_display_destory;
	}

	text_copy(win->text, "ROOT");
	win->x = win->abs_x = 0;
	win->y = win->abs_y = 0;
	win->width = display->xres;
	win->height = display->yres;
	win->border_width = 2;
	win->back_color = cavan_display_build_color3f(display, 0, 0, 0);
	win->fore_color = cavan_display_build_color3f(display, 1.0, 1.0, 1.0);
	win->border_color = cavan_display_build_color3f(display, 1.0, 1.0, 1.0);

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
	context->max_x = display->xres - 1;
	context->max_y = display->yres - 1;

	cavan_application_move(context, 0, 0);
	cavan_application_draw_mouse(context);

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
	ret = cavan_input_service_start(input_service, context);
	if (ret < 0)
	{
		pr_red_info("cavan_input_service_start");
		goto out_display_memory_free;
	}

	return 0;

out_display_memory_free:
	cavan_display_memory_free(context->mouse_backup);
out_win_destroy:
	pthread_mutex_destroy(&context->lock);
	win->destory_handler(win);
out_display_destory:
	display->destory(display);

	return ret;
}

void cavan_application_context_uninit(struct cavan_application_context *context)
{
	cavan_input_service_stop(&context->input_service);
	cavan_display_memory_free(context->mouse_backup);
	context->display->destory(context->display);
	context->win_root.destory_handler(&context->win_root);

	pthread_mutex_destroy(&context->lock);
}
