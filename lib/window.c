/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Tue Nov 20 16:09:41 CST 2012
 */

#include <cavan.h>
#include <cavan/window.h>
#include <cavan/fb.h>

static void cavan_child_window_paint_handler(struct double_link *link, struct double_link_node *node, void *data);
static void cavan_child_window_destory_handler(struct double_link *link, struct double_link_node *node, void *data);
static void cavan_child_window_set_abs_position_handler(struct double_link *link, struct double_link_node *node, void *data);
static bool cavan_window_find_by_point_matcher(struct double_link *link, struct double_link_node *node, void *data);

void cavan_window_paint_child(struct double_link *link)
{
	double_link_traversal2(link, NULL, cavan_child_window_paint_handler);
}

static void cavan_window_paint_base(struct cavan_window *win)
{
	pthread_mutex_lock(&win->lock);

	win->paint_handler(win);

	if (win->on_paint)
	{
		pthread_mutex_unlock(&win->lock);
		win->on_paint(win, win->context->private_data);
		pthread_mutex_lock(&win->lock);
	}

	cavan_window_paint_child(&win->child_link);

	pthread_mutex_unlock(&win->lock);
}

static void cavan_child_window_paint_handler(struct double_link *link, struct double_link_node *node, void *data)
{
	struct cavan_window *win = double_link_get_container(link, node);

	cavan_window_paint_base(win);
}

void cavan_window_paint(struct cavan_window *win)
{
	cavan_window_paint_base(win);
	cavan_display_refresh(win->context->display);
}

void cavan_window_destory(struct cavan_window *win)
{
	pthread_mutex_lock(&win->lock);

	if (win->on_destory)
	{
		pthread_mutex_unlock(&win->lock);
		win->on_destory(win, win->context->private_data);
		pthread_mutex_lock(&win->lock);
	}

	double_link_traversal(&win->child_link, NULL, cavan_child_window_destory_handler);

	pthread_mutex_unlock(&win->lock);

	win->destory_handler(win);
}

static void cavan_child_window_destory_handler(struct double_link *link, struct double_link_node *node, void *data)
{
	struct cavan_window *win = double_link_get_container(link, node);

	cavan_window_destory(win);
}

bool cavan_window_clicked(struct cavan_window *win, cavan_touch_point_t *point)
{
	bool pressed;

	pthread_mutex_lock(&win->lock);

	pressed = point->pressure > 0;
	if (win->pressed == pressed)
	{
		pthread_mutex_unlock(&win->lock);
		return false;
	}

	win->pressed = pressed;

	if (win->on_clicked)
	{
		pthread_mutex_unlock(&win->lock);

		if (win->on_clicked(win, point, win->context->private_data))
		{
			return true;
		}

		pthread_mutex_lock(&win->lock);
	}

	win->click_handler(win, point);

	pthread_mutex_unlock(&win->lock);

	cavan_window_paint(win);

	return false;
}

static bool cavan_window_mouse_move(struct cavan_window *win, cavan_touch_point_t *point)
{
	pthread_mutex_lock(&win->lock);

	if (win->on_move)
	{
		pthread_mutex_unlock(&win->lock);

		if (win->on_move(win, point, win->context->private_data))
		{
			return true;
		}

		pthread_mutex_lock(&win->lock);
	}

	win->move_handler(win, point);

	pthread_mutex_unlock(&win->lock);

	return false;
}

static bool cavan_window_mouse_entry(struct cavan_window *win, cavan_touch_point_t *point)
{
	pthread_mutex_lock(&win->lock);

	if (win->on_entry)
	{
		pthread_mutex_unlock(&win->lock);

		if (win->on_entry(win, point, win->context->private_data))
		{
			return true;
		}

		pthread_mutex_lock(&win->lock);
	}

	win->entry_handler(win, point);

	pthread_mutex_unlock(&win->lock);

	return false;
}

static bool cavan_window_mouse_exit(struct cavan_window *win, cavan_touch_point_t *point)
{

	pthread_mutex_lock(&win->lock);

	if (win->on_exit)
	{
		pthread_mutex_unlock(&win->lock);

		if (win->on_exit(win, point, win->context->private_data))
		{
			return true;
		}

		pthread_mutex_lock(&win->lock);
	}

	win->exit_handler(win, point);

	pthread_mutex_unlock(&win->lock);

	return false;
}

static bool cavan_window_key_pressed(struct cavan_window *win, const char *name, int code, int value)
{
	pthread_mutex_lock(&win->lock);

	if (win->on_key_pressed)
	{
		pthread_mutex_unlock(&win->lock);

		if (win->on_key_pressed(win, name, code, value, win->context->private_data))
		{
			return true;
		}

		pthread_mutex_lock(&win->lock);
	}

	win->key_handler(win, name, code, value);

	pthread_mutex_unlock(&win->lock);

	return false;
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
	double_link_deinit(&win->child_link);
	pthread_mutex_destroy(&win->lock);
}

void cavan_window_paint_handler(struct cavan_window *win)
{
	int x, y;
	int width, height;
	struct cavan_application_context *context;
	struct cavan_display_device *display;

	context = win->context;
	display = context->display;

	x = win->abs_x + win->border_width;
	y = win->abs_y + win->border_width;
	width = win->width - win->border_width * 2;
	height = win->height - win->border_width * 2;

	pthread_mutex_lock(&display->lock);

	display->set_color(display, win->back_color);
	display->fill_rect(display, x, y, width, height);

	display->set_color(display, win->border_color);
	display->fill_rect(display, win->abs_x, win->abs_y, win->width, win->border_width);
	display->fill_rect(display, win->abs_x, y + height, win->width, win->border_width);
	display->fill_rect(display, win->abs_x, y, win->border_width, height);
	display->fill_rect(display, x + width, y, win->border_width, height);

	pthread_mutex_unlock(&display->lock);
}

void cavan_window_key_handler(struct cavan_window *win, const char *name, int code, int value)
{
}

void cavan_window_click_handler(struct cavan_window *win, cavan_touch_point_t *point)
{
}

void cavan_window_move_handler(struct cavan_window *win, cavan_touch_point_t *point)
{
}

void cavan_window_entry_handler(struct cavan_window *win, cavan_touch_point_t *point)
{
}

void cavan_window_exit_handler(struct cavan_window *win, cavan_touch_point_t *point)
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
	int ret;

	ret = pthread_mutex_init(&win->lock, NULL);
	if (ret < 0)
	{
		pr_error_info("pthread_mutex_init");
		return ret;
	}

	ret = double_link_init(&win->child_link, MEMBER_OFFSET(struct cavan_window, node));
	if (ret < 0)
	{
		pr_red_info("double_link_init");
		goto out_pthread_mutex_destroy;
	}

	double_link_node_init(&win->node);

	win->parent = NULL;

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

out_pthread_mutex_destroy:
	pthread_mutex_destroy(&win->lock);
	return ret;
}

static void cavan_window_set_abs_position_base(struct cavan_window *win, int x, int y)
{
	struct cavan_display_rect rect;

	pthread_mutex_lock(&win->lock);

	win->abs_x = x;
	win->abs_y = y;

	win->get_rect_handler(win, &rect);

	rect.x += x;
	rect.y += y;

	double_link_traversal(&win->child_link, &rect, cavan_child_window_set_abs_position_handler);

	pthread_mutex_unlock(&win->lock);
}

static void cavan_child_window_set_abs_position_handler(struct double_link *link, struct double_link_node *node, void *data)
{
	struct cavan_window *win = double_link_get_container(link, node);
	struct cavan_display_rect *rect = data;

	cavan_window_set_abs_position_base(win, win->x + rect->x, win->y + rect->y);
}

void cavan_window_set_abs_position(struct cavan_window *win, int x, int y)
{
	struct cavan_window *parent = win->parent;
	struct cavan_application_context *context = win->context;

	if (parent == NULL)
	{
		struct cavan_display_device *display = context->display;

		pthread_mutex_lock(&display->lock);
		display->set_color(display, context->back_color);
		pthread_mutex_lock(&win->lock);
		display->fill_rect(display, win->abs_x, win->abs_y, win->width, win->height);
		pthread_mutex_unlock(&win->lock);
		pthread_mutex_unlock(&display->lock);
	}

	cavan_window_set_abs_position_base(win, x, y);

	if (parent)
	{
		cavan_window_paint(parent);
	}
	else
	{
		cavan_window_paint_child(&context->win_link);
		cavan_display_refresh(context->display);
	}
}

void cavan_window_set_position(struct cavan_window *win, int x, int y)
{
	struct cavan_window *parent = win->parent;

	pthread_mutex_lock(&win->lock);

	win->x = x;
	win->y = y;

	if (parent)
	{
		struct cavan_display_rect rect;

		parent->get_rect_handler(win->parent, &rect);

		x += rect.x;
		y += rect.y;
	}

	pthread_mutex_unlock(&win->lock);

	cavan_window_set_abs_position(win, x, y);
}

struct cavan_window *cavan_window_find_by_point(struct double_link *link, cavan_touch_point_t *point)
{
	struct double_link_node *node;
	struct cavan_window *win, *child;

	node = double_link_find(link, point, cavan_window_find_by_point_matcher);
	if (node == NULL)
	{
		return NULL;
	}

	win = double_link_get_container(link, node);
	child = cavan_window_find_by_point(&win->child_link, point);

	return child ? child : win;
}

static bool cavan_window_find_by_point_matcher(struct double_link *link, struct double_link_node *node, void *data)
{
	cavan_touch_point_t *point = data;
	struct cavan_window *win = double_link_get_container(link, node);

	if (point->x < win->abs_x || point->x >= win->abs_x + win->width)
	{
		return false;
	}

	if (point->y < win->abs_y || point->y >= win->abs_y + win->height)
	{
		return false;
	}

	return true;
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

	child->x += rect.x;
	if (child->x < 0)
	{
		child->x = 0;
	}

	child->y += rect.y;
	if (child->y < 0)
	{
		child->y = 0;
	}

	if (child->width < 0)
	{
		child->width = rect.width;
	}

	if (child->height < 0)
	{
		child->height = rect.height;
	}

	child->parent = win;
	double_link_append(&win->child_link, &child->node);

	pthread_mutex_unlock(&win->lock);

	cavan_window_set_abs_position_base(child, child->x + win->abs_x, child->y + win->abs_y);

	return 0;
}

void cavan_window_remove_child(struct cavan_window *win, struct cavan_window *child)
{
	pthread_mutex_lock(&win->lock);
	double_link_remove(&win->child_link, &child->node);
	pthread_mutex_unlock(&win->lock);
}

// ================================================================================

void cavan_dialog_paint_handler(struct cavan_window *win)
{
	struct cavan_dialog *dialog = (struct cavan_dialog *)win;
	struct cavan_display_device *display;

	cavan_window_paint_handler(win);

	if (dialog->title_height > 0 && win->text[0])
	{
		int x, y;

		display = win->context->display;

		pthread_mutex_lock(&display->lock);

		display->set_color(display, win->border_color);
		display->fill_rect(display, win->abs_x + win->border_width, win->abs_y + dialog->title_height + win->border_width, win->width - 2 * win->border_width, win->border_width);

		x = win->abs_x + (win->width - (int)display->mesure_text(display, win->text)) / 2 + win->border_width;
		y = dialog->title_height / 2 + win->border_width;

		display->set_color(display, win->fore_color);
		display->draw_text(display, x, y, win->text);

		pthread_mutex_unlock(&display->lock);
	}
}

void cavan_dialog_click_handler(struct cavan_window *win, cavan_touch_point_t *point)
{
	struct cavan_dialog *dialog = (struct cavan_dialog *)win;

	if (win->pressed)
	{
		dialog->x_offset = point->x - win->abs_x;
		dialog->y_offset = point->y - win->abs_y;
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
}

void cavan_dialog_move_handler(struct cavan_window *win, cavan_touch_point_t *point)
{
	struct cavan_dialog *dialog = (struct cavan_dialog *)win;

	if (dialog->move_able && win->pressed && dialog->backup == NULL)
	{
		struct cavan_display_device *display;
		struct cavan_display_memory_rect *mem;

		display = win->context->display;
		pthread_mutex_lock(&display->lock);
		mem = cavan_display_memory_rect_alloc(display, win->width, win->height, 1);
		pthread_mutex_unlock(&display->lock);
		if (mem)
		{
			mem->x = win->abs_x;
			mem->y = win->abs_y;
		}

		dialog->backup = mem;
	}

	if (dialog->backup)
	{
		struct cavan_application_context *context = win->context;
		struct cavan_display_device *display = context->display;
		struct cavan_display_memory_rect *mem = dialog->backup;

		pthread_mutex_lock(&display->lock);

		cavan_display_memory_rect_restore(display, mem);
		cavan_display_memory_rect_backup(display, mem, point->x - dialog->x_offset, point->y - dialog->y_offset);

		display->set_color(display, context->move_color);
		display->draw_rect(display, mem->x, mem->y, mem->width, mem->height);
		display->refresh(display);

		pthread_mutex_unlock(&display->lock);
	}
}

void cavan_dialog_get_rect_handler(struct cavan_window *win, struct cavan_display_rect *rect)
{
	struct cavan_dialog *dialog = (struct cavan_dialog *)win;

	rect->x = win->border_width;
	rect->y = dialog->title_height + win->border_width * 2;
	rect->width = win->width - win->border_width * 2;
	rect->height = win->height - dialog->title_height - win->border_width * 3;
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

void cavan_label_paint_handler(struct cavan_window *win)
{
	int x, y;
	int width, height;
	struct cavan_application_context *context = win->context;
	struct cavan_display_device *display = context->display;

	x = win->abs_x + win->border_width;
	y = win->abs_y + win->border_width;
	width = win->width - win->border_width * 2;
	height = win->height - win->border_width * 2;

	pthread_mutex_lock(&display->lock);

	display->set_color(display, win->parent ? win->parent->back_color : context->back_color);
	display->fill_rect(display, x, y, width, height);

	display->set_color(display, win->fore_color);
	display->draw_text(display, win->abs_x, win->abs_y + win->height / 2, win->text);

	pthread_mutex_unlock(&display->lock);
}

int cavan_label_init_handler(struct cavan_window *win, struct cavan_application_context *context)
{
	cavan_window_init_handler(win, context);

	win->paint_handler = cavan_label_paint_handler;

	return 0;
}

// ================================================================================

void cavan_button_paint_handler(struct cavan_window *win)
{
	int x, y;
	struct cavan_display_device *display;

	cavan_window_paint_handler(win);

	display = win->context->display;

	pthread_mutex_lock(&display->lock);

	x = win->abs_x + (win->width - (int)display->mesure_text(display, win->text)) / 2;
	y = win->abs_y + win->height / 2;

	display->set_color(display, win->fore_color);
	display->draw_text(display, x, y, win->text);

	pthread_mutex_unlock(&display->lock);
}

void cavan_button_click_handler(struct cavan_window *win, cavan_touch_point_t *point)
{
	struct cavan_display_device *display;
	struct cavan_button *button = (struct cavan_button *)win;

	display = win->context->display;

	if (win->pressed)
	{
		button->back_color_backup = win->back_color;
		button->fore_color_backup = win->fore_color;
		button->border_color_backup = win->border_color;

		win->back_color = cavan_display_build_color3f(display, 0, 0, 1.0);
		win->fore_color = cavan_display_build_color3f(display, 0, 0, 0);
		win->border_color = cavan_display_build_color3f(display, 1.0, 0, 0);
	}
	else
	{
		win->back_color = button->back_color_backup;
		win->fore_color = button->fore_color_backup;
		win->border_color = button->border_color_backup;
	}
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
	win->back_color = cavan_display_build_color3f(display, 0.5, 0.5, 0.5);
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

	pthread_mutex_lock(&display->lock);

	width = rect.width * percent;
	display->set_color(display, bar->complete_color);
	display->fill_rect(display, rect.x, rect.y, width, rect.height);

	sprintf(buff, "%2.2lf%%", percent * 100);
	display->set_color(display, win->fore_color);
	cavan_display_draw_text_centre(display, &rect, buff);

	pthread_mutex_unlock(&display->lock);
}

int cavan_progress_bar_init_handler(struct cavan_window *win, struct cavan_application_context *context)
{
	int ret;
	struct cavan_progress_bar *bar;

	ret = cavan_window_init_handler(win, context);
	if (ret < 0)
	{
		return ret;
	}

	win->paint_handler = cavan_progress_bar_paint_handler;

	POINTER_ASSIGN_CASE(bar, win);

	bar->complete_color = cavan_display_build_color3f(context->display, 0, 0, 1.0);

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
	struct cavan_display_memory *mem;
	struct cavan_display_device *display = context->display;

	pthread_mutex_lock(&display->lock);
	mem = cavan_display_memory_alloc(display, CAVAN_MOUSE_SIZE, CAVAN_MOUSE_SIZE);
	pthread_mutex_unlock(&display->lock);

	return mem;
}

static inline int cavan_application_restore_mouse(struct cavan_application_context *context)
{
	int ret;
	struct cavan_display_device *display = context->display;

	pthread_mutex_lock(&display->lock);
	ret = cavan_display_memory_restore(display, context->mouse_backup);
	pthread_mutex_unlock(&display->lock);

	return ret;
}

static void cavan_application_draw_mouse(struct cavan_application_context *context)
{
	struct cavan_display_device *display = context->display;
	struct cavan_display_memory *mem = context->mouse_backup;

	pthread_mutex_lock(&display->lock);
	cavan_display_memory_backup(display, mem, context->x, context->y);
	display->set_color(display, context->mouse_color);
	display->draw_rect(display, context->x, context->y, CAVAN_MOUSE_SIZE, CAVAN_MOUSE_SIZE);
	display->refresh(display);
	pthread_mutex_unlock(&display->lock);
}

static void cavan_application_move(struct cavan_application_context *context, cavan_touch_point_t *point)
{
	struct cavan_window *win;

	win = context->win_active;
	if (win && win->pressed)
	{
		cavan_window_mouse_move(win, point);
	}
	else
	{
		win = cavan_window_find_by_point(&context->win_link, point);
		if (win)
		{
			if (win != context->win_curr)
			{
				if (context->win_curr)
				{
					cavan_window_mouse_exit(context->win_curr, point);
				}

				cavan_window_mouse_entry(win, point);
			}

			cavan_window_mouse_move(win, point);
		}
		else if (context->win_curr)
		{
			cavan_window_mouse_exit(context->win_curr, point);
		}

		context->win_curr = win;
	}

	context->x = point->x;
	context->y = point->y;
}

static void cavan_application_click(struct cavan_application_context *context, cavan_touch_point_t *point)
{
	struct cavan_window *win;

	win = cavan_window_find_by_point(&context->win_link, point);
	if (point->pressure > 0)
	{
		if (win)
		{
			struct cavan_window *parent = win->parent;
			struct double_link *link = parent ? &parent->child_link : &context->win_link;

			double_link_move_to_head(link, &win->node);
			cavan_window_clicked(win, point);
		}

		context->win_active = win;
	}
	else if (context->win_active)
	{
		cavan_window_clicked(context->win_active, point);
	}

	if (win != context->win_curr)
	{
		if (context->win_curr)
		{
			cavan_window_mouse_exit(context->win_curr, point);
		}

		if (win)
		{
			cavan_window_mouse_entry(win, point);
		}

		context->win_curr = win;
	}
}

// ================================================================================

static void cavan_application_key_handler(struct cavan_input_device *dev, const char *name, int code, int value, void *data)
{
	bool res;
	struct cavan_application_context *context = data;

	pthread_mutex_lock(&context->lock);

	if (context->on_key_pressed)
	{
		pthread_mutex_unlock(&context->lock);
		res = context->on_key_pressed(context, name, code, value, context->private_data);
		pthread_mutex_lock(&context->lock);
	}
	else
	{
		res = false;
	}

	if (res == false)
	{
		if (context->win_active)
		{
			cavan_window_key_pressed(context->win_active, name, code, value);
		}
		else if (context->win_curr)
		{
			cavan_window_key_pressed(context->win_curr, name, code, value);
		}
	}

	pthread_mutex_unlock(&context->lock);
}

static void cavan_application_mouse_wheel_handler(struct cavan_input_device *dev, int code, int value, void *data)
{
}

static void cavan_application_mouse_move_handler(struct cavan_input_device *dev, int x, int y, void *data)
{
	struct cavan_application_context *context = data;
	cavan_touch_point_t point =
	{
		.id = 0,
		.pressure = 1
	};

	pthread_mutex_lock(&context->lock);

	x = x * context->mouse_speed + context->x;
	if (x < 0)
	{
		point.x = 0;
	}
	else if (x > context->max_x)
	{
		point.x = context->max_x;
	}
	else
	{
		point.x = x;
	}

	y = y * context->mouse_speed + context->y;
	if (y < 0)
	{
		point.y = 0;
	}
	else if (y > context->max_y)
	{
		point.y = context->max_y;
	}
	else
	{
		point.y = y;
	}

	cavan_application_restore_mouse(context);
	cavan_application_move(context, &point);
	cavan_application_draw_mouse(context);

	pthread_mutex_unlock(&context->lock);
}

static void cavan_application_mouse_touch_handler(struct cavan_input_device *dev, int code, int value, void *data)
{
	struct cavan_application_context *context = data;
	cavan_touch_point_t point =
	{
		.id = 0,
		.pressure = value,
		.x = context->x,
		.y = context->y,
	};

	pthread_mutex_lock(&context->lock);

	cavan_application_restore_mouse(context);
	cavan_application_click(context, &point);
	cavan_application_draw_mouse(context);

	pthread_mutex_unlock(&context->lock);
}

static void cavan_application_touch_handler(struct cavan_input_device *dev, cavan_touch_point_t *point, void *data)
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

	cavan_application_click(context, point);

	pthread_mutex_unlock(&context->lock);
}

static void cavan_application_move_handler(struct cavan_input_device *dev, cavan_touch_point_t *point, void *data)
{
	struct cavan_application_context *context = data;

	pthread_mutex_lock(&context->lock);
	cavan_application_move(data, point);
	pthread_mutex_unlock(&context->lock);
}

// ================================================================================

int cavan_application_init(struct cavan_application_context *context, struct cavan_display_device *display, void *data)
{
	int ret;
	struct cavan_input_service *input_service;

	ret = pthread_mutex_init(&context->lock, NULL);
	if (ret < 0)
	{
		pr_error_info("pthread_mutex_init");
		return ret;
	}

	if (display == NULL)
	{
		display = cavan_fb_display_create();
		if (display == NULL)
		{
			ret = -EFAULT;
			pr_red_info("cavan_fb_display_create");
			goto out_pthread_mutex_destroy;
		}
	}

	context->display = display;

	ret = cavan_display_check(display);
	if (ret < 0)
	{
		pr_red_info("cavan_display_check");
		goto out_display_destory;
	}

	ret = double_link_init(&context->win_link, MEMBER_OFFSET(struct cavan_window, node));
	if (ret < 0)
	{
		pr_red_info("double_link_init");
		goto out_display_destory;
	}

	context->mouse_backup = cavan_application_mouse_alloc(context);
	if (context->mouse_backup == NULL)
	{
		ret = -ENOMEM;
		pr_red_info("cavan_display_memory_alloc");
		goto out_double_link_deinit;
	}

	context->private_data = data;
	context->win_curr = NULL;
	context->win_active = NULL;

	context->back_color = cavan_display_build_color3f(display, 0, 0, 0);
	context->mouse_color = cavan_display_build_color3f(display, 1.0, 0, 0);
	context->move_color = cavan_display_build_color3f(display, 0, 1.0, 0.5);

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

	ret = pipe(context->pipefd);
	if (ret < 0)
	{
		pr_error_info("pipe");
		goto out_display_memory_free;
	}

	context->on_key_pressed = NULL;

	return 0;

out_display_memory_free:
	cavan_display_memory_free(context->mouse_backup);
out_double_link_deinit:
	double_link_deinit(&context->win_link);
out_display_destory:
	display->destory(display);
out_pthread_mutex_destroy:
	pthread_mutex_destroy(&context->lock);

	return ret;
}

void cavan_application_uninit(struct cavan_application_context *context)
{
	close(context->pipefd[0]);
	close(context->pipefd[1]);

	double_link_deinit(&context->win_link);
	cavan_display_memory_free(context->mouse_backup);
	context->display->destory(context->display);

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

	cavan_application_update_data(context);

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

void cavan_application_update_data(struct cavan_application_context *context)
{
	struct cavan_display_device *display;

	pthread_mutex_lock(&context->lock);

	display = context->display;

	pthread_mutex_lock(&display->lock);
	display->set_color(display, context->back_color);
	display->fill_rect(display, 0, 0, display->xres, display->yres);
	pthread_mutex_unlock(&display->lock);

	cavan_window_paint_child(&context->win_link);
	cavan_display_refresh(display);

	pthread_mutex_unlock(&context->lock);
}

int cavan_application_add_window(struct cavan_application_context *context, struct cavan_window *win)
{
	int ret;

	ret = win->init_handler(win, context);
	if (ret  < 0)
	{
		pr_red_info("win->init_handler");
		return ret;
	}

	pthread_mutex_lock(&context->lock);

	if (win->x < 0)
	{
		win->x = 0;
	}

	if (win->y < 0)
	{
		win->y = 0;
	}

	if (win->width < 0)
	{
		win->width = context->display->xres - win->x;
	}

	if (win->height < 0)
	{
		win->height = context->display->yres - win->y;
	}

	win->parent = NULL;
	double_link_append(&context->win_link, &win->node);

	pthread_mutex_unlock(&context->lock);

	cavan_window_set_abs_position_base(win, win->x, win->y);

	return 0;
}

void cavan_application_remove_window(struct cavan_application_context *context, struct cavan_window *win)
{
	pthread_mutex_lock(&context->lock);
	double_link_remove(&context->win_link, &win->node);
	pthread_mutex_unlock(&context->lock);
}
