/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Tue Nov 20 16:09:41 CST 2012
 */

#include <cavan.h>
#include <cavan/fb.h>
#include <cavan/math.h>
#include <cavan/window.h>

static bool cavan_window_find_by_point_matcher(struct double_link *link, struct double_link_node *node, void *data);

static bool cavan_window_contian_point(struct cavan_window *win, struct cavan_input_message_point *point)
{
	if (win->visible == false || win->active == false)
	{
		return false;
	}

	if (point->x < win->xabs || point->x > win->right)
	{
		return false;
	}

	if (point->y < win->yabs || point->y > win->bottom)
	{
		return false;
	}

	return true;
}

void cavan_window_set_top(struct cavan_window *win)
{
	struct cavan_window *parent = win->parent;
	struct double_link *link = parent ? &parent->child_link : &win->context->win_link;

	double_link_move_to_head(link, &win->node);
}

static void cavan_window_paint_base(struct cavan_window *win)
{
	pthread_mutex_lock(&win->lock);

	if (win->visible)
	{
		win->paint_handler(win);

		if (win->on_paint)
		{
			pthread_mutex_unlock(&win->lock);
			win->on_paint(win, win->context->private_data);
			pthread_mutex_lock(&win->lock);
		}

		cavan_window_paint_child(&win->child_link);
	}

	pthread_mutex_unlock(&win->lock);
}

void cavan_window_paint_child(struct double_link *link)
{
	struct cavan_window *win;

	double_link_foreach(link, win)
	{
		cavan_window_paint_base(win);
	}
	end_link_foreach(link);
}

void cavan_window_paint(struct cavan_window *win)
{
	struct cavan_display_device *display = win->context->display;

	cavan_display_lock(display);
	cavan_window_paint_base(win);
	cavan_display_unlock(display);

	cavan_display_refresh(display);
}

bool cavan_window_clicked(struct cavan_window *win, struct cavan_input_message_point *point)
{
	int mask;
	bool pressed = win->pressed;

	pthread_mutex_lock(&win->lock);

	if (point->pressed)
	{
		mask = win->touch_mask | 1 << point->id;
	}
	else
	{
		mask = win->touch_mask & (~(1 << point->id));
	}

	if (win->touch_mask == mask)
	{
		pthread_mutex_unlock(&win->lock);
		return false;
	}

	win->touch_mask = mask;
	win->pressed = mask != 0;

	if (win->on_clicked)
	{
		pthread_mutex_unlock(&win->lock);

		if (win->on_clicked(win, point, win->context->private_data))
		{
			if (point->pressed > 0)
			{
				return true;
			}
		}

		pthread_mutex_lock(&win->lock);
	}

	if (pressed != win->pressed && win->click_handler(win, point))
	{
		pthread_mutex_unlock(&win->lock);
		cavan_window_paint(win);

		if (point->pressed > 0)
		{
			return true;
		}
	}

	if (win->parent)
	{
		pthread_mutex_unlock(&win->lock);
		cavan_window_clicked(win->parent, point);
	}
	else
	{
		pthread_mutex_unlock(&win->lock);
	}

	return false;
}

static bool cavan_window_mouse_move(struct cavan_window *win, struct cavan_input_message_point *point)
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

	if (win->parent)
	{
		pthread_mutex_unlock(&win->lock);
		cavan_window_mouse_move(win->parent, point);
		pthread_mutex_unlock(&win->lock);
	}

	pthread_mutex_unlock(&win->lock);

	return false;
}

static bool cavan_window_mouse_entry(struct cavan_window *win, struct cavan_input_message_point *point)
{
	int mask;

	pthread_mutex_lock(&win->lock);

	mask = win->move_mask | 1 << point->id;
	if (win->move_mask == mask)
	{
		pthread_mutex_unlock(&win->lock);
		return false;
	}

	win->move_mask = mask;

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

static bool cavan_window_mouse_exit(struct cavan_window *win, struct cavan_input_message_point *point)
{
	int mask;

	pthread_mutex_lock(&win->lock);

	mask = win->move_mask & (~(1 << point->id));
	if (win->move_mask == mask)
	{
		pthread_mutex_unlock(&win->lock);
		return false;
	}

	win->move_mask = mask;

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

static bool cavan_window_key_pressed(struct cavan_window *win, struct cavan_input_message_key *message)
{
	pthread_mutex_lock(&win->lock);

	if (win->on_key_pressed)
	{
		pthread_mutex_unlock(&win->lock);

		if (win->on_key_pressed(win, message, win->context->private_data))
		{
			return true;
		}

		pthread_mutex_lock(&win->lock);
	}

	win->key_handler(win, message);

	pthread_mutex_unlock(&win->lock);

	return false;
}

// ================================================================================

void cavan_window_key_handler(struct cavan_window *win, struct cavan_input_message_key *message)
{
}

bool cavan_window_click_handler(struct cavan_window *win, struct cavan_input_message_point *point)
{
	return false;
}

void cavan_window_move_handler(struct cavan_window *win, struct cavan_input_message_point *point)
{
}

void cavan_window_entry_handler(struct cavan_window *win, struct cavan_input_message_point *point)
{
}

void cavan_window_exit_handler(struct cavan_window *win, struct cavan_input_message_point *point)
{
}

void cavan_window_destroy_handler(struct cavan_window *win)
{
	double_link_deinit(&win->child_link);
	pthread_mutex_destroy(&win->lock);
}

int cavan_window_init(struct cavan_window *win, struct cavan_application_context *context)
{
	int ret;
	struct cavan_display_device *display;

	if (context == NULL)
	{
		pr_red_info("context == NULL");
		return -EINVAL;
	}

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
	win->border_width = 2;
	win->context = context;

	win->on_destroy = NULL;
	win->on_paint = NULL;
	win->on_clicked = NULL;
	win->on_double_clicked = NULL;
	win->on_move = NULL;
	win->on_entry = NULL;
	win->on_exit = NULL;
	win->on_key_pressed = NULL;

	win->pressed = false;
	win->visible = true;
	win->active = true;

	win->move_mask = 0;
	win->touch_mask = 0;

	win->destroy_handler = cavan_window_destroy_handler;
	win->paint_handler = cavan_window_paint_handler;
	win->click_handler = cavan_window_click_handler;
	win->move_handler = cavan_window_move_handler;
	win->entry_handler = cavan_window_entry_handler;
	win->exit_handler = cavan_window_exit_handler;
	win->key_handler = cavan_window_key_handler;
	win->get_rect_handler = cavan_window_get_rect_handler;

	display = context->display;
	win->back_color = cavan_display_build_color3f(display, 0, 0, 0);
	win->fore_color = cavan_display_build_color3f(display, 1.0, 1.0, 1.0);
	win->border_color = cavan_display_build_color3f(display, 1.0, 1.0, 1.0);

	return 0;

out_pthread_mutex_destroy:
	pthread_mutex_destroy(&win->lock);
	return ret;
}

void cavan_window_destroy(struct cavan_window *win)
{
	struct cavan_window *child;

	pthread_mutex_lock(&win->lock);

	if (win->on_destroy)
	{
		pthread_mutex_unlock(&win->lock);
		win->on_destroy(win, win->context->private_data);
		pthread_mutex_lock(&win->lock);
	}

	double_link_foreach(&win->child_link, child)
	{
		cavan_window_destroy(child);
	}
	end_link_foreach(&win->child_link);

	pthread_mutex_unlock(&win->lock);

	if (win->destroy_handler)
	{
		win->destroy_handler(win);
	}
}

void cavan_window_paint_handler(struct cavan_window *win)
{
	int x, y;
	int border_width;
	int width, height;
	struct cavan_application_context *context;
	struct cavan_display_device *display;

	context = win->context;
	display = context->display;

	x = win->xabs + win->border_width;
	y = win->yabs + win->border_width;
	width = win->width - win->border_width * 2;
	height = win->height - win->border_width * 2;

	if (width > 0 && height > 0)
	{
		display->set_color(display, win->back_color);
		display->fill_rect(display, x, y, width, height);
	}

	display->set_color(display, win->border_color);

	border_width = MIN(win->height, win->border_width);
	if (border_width > 0)
	{
		display->fill_rect(display, win->xabs, win->yabs, win->width, border_width);

		if (border_width > 1)
		{
			display->fill_rect(display, win->xabs, y + height, win->width, border_width);
		}
	}

	border_width = MIN(win->width, win->border_width);
	if (border_width > 0)
	{
		display->fill_rect(display, win->xabs, y, border_width, height);

		if (border_width > 1)
		{
			display->fill_rect(display, x + width, y, border_width, height);
		}
	}
}

void cavan_window_get_rect_handler(struct cavan_window *win, struct cavan_display_rect *rect)
{
	int sep_width = win->border_width ? CAVAN_WINDOW_SEP_WIDTH : 0;

	rect->x = win->border_width + sep_width;
	rect->y = win->border_width + sep_width;
	rect->width = win->width - (win->border_width + sep_width) * 2;
	rect->height = win->height - (win->border_width + sep_width) * 2;
}

static void cavan_window_set_abs_position_base(struct cavan_window *win, int x, int y)
{
	struct cavan_window *child;
	struct cavan_display_rect rect;

	win->xabs = x;
	win->yabs = y;
	win->right = x + win->width - 1;
	win->bottom = y + win->height - 1;

	win->get_rect_handler(win, &rect);

	double_link_foreach(&win->child_link, child)
	{
		cavan_window_set_abs_position_base(child, child->x + rect.x + x, child->y + rect.y + y);
	}
	end_link_foreach(&win->child_link);
}

static void cavan_window_set_abs_position(struct cavan_window *win, int xo, int yo, int x, int y)
{
	struct cavan_window *parent;
	struct cavan_application_context *context;

	pthread_mutex_lock(&win->lock);

	parent = win->parent;
	context = win->context;

	if (parent == NULL)
	{
		struct cavan_display_device *display = context->display;

		cavan_display_lock(display);
		display->set_color(display, context->back_color);
		display->fill_rect(display, xo, yo, win->width, win->height);
		cavan_display_unlock(display);
	}

	cavan_window_set_abs_position_base(win, x, y);

	pthread_mutex_unlock(&win->lock);

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

	cavan_window_set_abs_position(win, win->xabs, win->yabs, x, y);
}

struct cavan_window *cavan_window_find_by_point(struct double_link *link, struct cavan_input_message_point *point)
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

static inline bool cavan_window_find_by_point_matcher(struct double_link *link, struct double_link_node *node, void *data)
{
	struct cavan_window *win = double_link_get_container(link, node);

	return cavan_window_contian_point(win, data);
}

static void cavan_window_add_child_base(struct double_link *link, struct cavan_window *child, struct cavan_display_rect *rect)
{
	if (child->x < 0)
	{
		child->x = 0;
	}

	if (child->y < 0)
	{
		child->y = 0;
	}

	if (child->width < 0)
	{
		child->width = rect->width;
	}

	if (child->height < 0)
	{
		child->height = rect->height;
	}

	double_link_append(link, &child->node);
	cavan_window_set_abs_position_base(child, child->x + rect->x, child->y + rect->y);
}

void cavan_window_add_child(struct cavan_window *win, struct cavan_window *child)
{
	struct cavan_display_rect rect;

	pthread_mutex_lock(&win->lock);

	win->get_rect_handler(win, &rect);
	rect.x += win->xabs;
	rect.y += win->yabs;
	child->parent = win;

	cavan_window_add_child_base(&win->child_link, child, &rect);

	pthread_mutex_unlock(&win->lock);
}

void cavan_window_remove_child(struct cavan_window *win, struct cavan_window *child)
{
	pthread_mutex_lock(&win->lock);
	double_link_remove(&win->child_link, &child->node);
	pthread_mutex_unlock(&win->lock);
}

// ================================================================================

void cavan_label_paint_handler(struct cavan_window *win)
{
	int x;
	struct cavan_display_device *display;
	struct cavan_label *label = (struct cavan_label *)win;

	cavan_window_paint_handler(win);

	if (win->height < 0 || label->text == NULL)
	{
		return;
	}

	display = win->context->display;
	display->set_color(display, win->fore_color);

	switch (label->align)
	{
	case CAVAN_WIN_TEXT_ALIGN_CENTER:
		x = win->xabs + (win->width - (int)display->mesure_text(display, label->text)) / 2;
		break;

	case CAVAN_WIN_TEXT_ALIGN_RIGHT:
		x = win->xabs + (win->width - (int)display->mesure_text(display, label->text)) - 2;
		break;

	default:
		x = win->xabs + 2;
	}

	display->draw_text(display, x, win->yabs + win->height / 2, label->text);
}

int cavan_label_init(struct cavan_label *label, struct cavan_application_context *context, const char *text)
{
	int ret;
	struct cavan_window *win = &label->window;

	ret = cavan_window_init(win, context);
	if (ret < 0)
	{
		return ret;
	}

	if (win->height < 1)
	{
		win->height = context->display->font.cheight + win->border_width * 4;
	}

	if (win->width < 1 && text)
	{
		win->width = context->display->mesure_text(context->display, text) + win->border_width * 4;
	}

	label->text = text;
	label->align = CAVAN_WIN_TEXT_ALIGN_LEFT;
	win->paint_handler = cavan_label_paint_handler;

	return 0;
}

// ================================================================================

static void cavan_textview_key_handler(struct cavan_window *win, struct cavan_input_message_key *message)
{
	struct cavan_textview *view = (struct cavan_textview *)win;

	switch (message->code)
	{
	case KEY_LEFTSHIFT:
	case KEY_RIGHTSHIFT:
		view->shift_down = message->value > 0;
		return;

	case KEY_LEFT:
	case KEY_RIGHT:
		if (message->value > 0)
		{
			cavan_cursor_stop(&win->context->cursor);
			if (message->code == KEY_LEFT)
			{
				if (view->cursor > view->head)
				{
					view->cursor--;
				}
			}
			else
			{
				if (view->cursor < view->tail)
				{
					view->cursor++;
				}
			}

			cavan_cursor_start(&win->context->cursor, NULL);
		}
		break;

	case KEY_CAPSLOCK:
		if (message->value > 0)
		{
			view->caps_lock = !view->caps_lock;
		}
		return;

	case KEY_BACKSPACE:
		if (message->value > 0 && view->cursor > view->text)
		{
			cavan_cursor_stop(&win->context->cursor);

			mem_move(view->cursor - 1, view->cursor, view->tail - view->cursor);
			view->cursor--;
			*--view->tail = 0;

			if (view->head > view->text)
			{
				view->head--;
				view->label.text = view->head;
			}

			cavan_cursor_start(&win->context->cursor, NULL);
		}
		break;

	case KEY_DELETE:
		if (message->value > 0 && view->cursor < view->tail)
		{
			cavan_cursor_stop(&win->context->cursor);

			mem_move(view->cursor, view->cursor + 1, view->tail - view->cursor - 1);
			*--view->tail = 0;

			if (view->head > view->text)
			{
				view->head--;
				view->label.text = view->head;
			}

			cavan_cursor_start(&win->context->cursor, NULL);
		}
		break;

	default:
		if (message->value > 0 && view->tail - view->text < NELEM(view->text) - 1)
		{
			cavan_cursor_stop(&win->context->cursor);

			if (view->cursor < view->tail)
			{
				mem_move(view->cursor + 1, view->cursor, view->tail - view->cursor);
			}

			*view->cursor++ = cavan_keycode2ascii(message->code, view->caps_lock ? !view->shift_down : view->shift_down);
			*++view->tail = 0;

			if (view->tail - view->text > view->size)
			{
				view->head++;
				view->label.text = view->head;
			}

			cavan_cursor_start(&win->context->cursor, NULL);
		}
	}

	pthread_mutex_unlock(&win->lock);
	cavan_window_paint(win);
	pthread_mutex_lock(&win->lock);
}

static void cavan_textview_cursor_set_visual(struct cavan_cursor *cursor, bool visual, void *data)
{
	struct cavan_display_rect rect;
	struct cavan_window *win = data;
	struct cavan_textview *view = data;
	struct cavan_display_device *display = win->context->display;

	win->get_rect_handler(win, &rect);
	rect.x += win->xabs + display->font.cwidth * (view->cursor - view->head) - 2;
	rect.y += win->yabs + (rect.height - display->font.cheight) / 2;

	cavan_display_lock(display);
	display->set_color(display, visual ? win->fore_color : win->back_color);
	display->fill_rect(display, rect.x, rect.y, 1, display->font.cheight);
	cavan_display_unlock(display);

	cavan_display_refresh(display);
}

static bool cavan_textview_click_handler(struct cavan_window *win, struct cavan_input_message_point *message)
{
	struct cavan_cursor *cursor;

	if (message->pressed == false)
	{
		return false;
	}

	cursor = &win->context->cursor;
	cursor->period = 500;
	cursor->set_visual = cavan_textview_cursor_set_visual;
	cavan_cursor_start(cursor, win);

	return true;
}

int cavan_textview_init(struct cavan_textview *view, struct cavan_application_context *context, const char *text)
{
	int ret;

	if (text)
	{
		view->tail = text_copy(view->text, text);
	}
	else
	{
		view->text[0] = 0;
		view->tail = view->text;
	}

	view->cursor = view->tail;
	view->head = view->text;
	view->shift_down = false;
	view->caps_lock = false;

	view->size = (view->label.window.width / context->display->font.cwidth) - 1;
	if (view->size >= NELEM(view->text))
	{
		view->size = NELEM(view->text) - 1;
	}

	ret = cavan_label_init(&view->label, context, view->head);
	if (ret < 0)
	{
		return ret;
	}

	view->label.window.key_handler = cavan_textview_key_handler;
	view->label.window.click_handler = cavan_textview_click_handler;

	return 0;
}

// ================================================================================

void cavan_dialog_paint_handler(struct cavan_window *win)
{
	struct cavan_dialog *dialog = (struct cavan_dialog *)win;

	cavan_window_paint_handler(win);
	cavan_label_paint_handler(&dialog->title.window);
}

bool cavan_dialog_click_handler(struct cavan_window *win, struct cavan_input_message_point *point)
{
	struct cavan_dialog *dialog = (struct cavan_dialog *)win;

	if (win->pressed)
	{
		dialog->xofs = point->x - win->xabs;
		dialog->yofs = point->y - win->yabs;

		dialog->xbak = win->xabs;
		dialog->ybak = win->yabs;

		cavan_cursor_stop(&win->context->cursor);
	}
	else
	{
		if (dialog->backup)
		{
			struct cavan_display_memory_rect *mem = dialog->backup;

			if (dialog->xbak != mem->x || dialog->ybak != mem->y)
			{
				pthread_mutex_unlock(&win->lock);
				cavan_window_set_abs_position(win, dialog->xbak, dialog->ybak, mem->x, mem->y);
				pthread_mutex_lock(&win->lock);
			}

			cavan_display_memory_rect_free(mem);
			dialog->backup = NULL;
		}

		cavan_cursor_start(&win->context->cursor, NULL);
	}

	return false;
}

void cavan_dialog_move_handler(struct cavan_window *win, struct cavan_input_message_point *point)
{
	struct cavan_dialog *dialog = (struct cavan_dialog *)win;

	if (dialog->move_able && win->pressed && dialog->backup == NULL)
	{
		struct cavan_display_device *display;
		struct cavan_display_memory_rect *mem;

		display = win->context->display;
		cavan_display_lock(display);
		mem = cavan_display_memory_rect_alloc(display, win->width, win->height, 1);
		cavan_display_unlock(display);
		if (mem)
		{
			mem->x = win->xabs;
			mem->y = win->yabs;
		}

		dialog->backup = mem;
	}

	if (dialog->backup)
	{
		int x, y;
		struct cavan_application_context *context = win->context;
		struct cavan_display_device *display = context->display;
		struct cavan_display_memory_rect *mem = dialog->backup;

		cavan_display_lock(display);

		cavan_display_memory_rect_restore(display, mem);
		x = point->x - dialog->xofs;
		y = point->y - dialog->yofs;
		cavan_display_memory_rect_backup(display, mem, point->x - dialog->xofs, point->y - dialog->yofs);
		cavan_window_set_abs_position_base(win, x, y);

		display->set_color(display, context->move_color);
		display->draw_rect(display, mem->x, mem->y, mem->width, mem->height);

		cavan_display_unlock(display);

		cavan_display_refresh(display);
	}
}

int cavan_dialog_init(struct cavan_dialog *dialog, struct cavan_application_context *context, const char *text)
{
	int ret;
	struct cavan_display_rect rect;
	struct cavan_window *body = &dialog->body;
	struct cavan_window *win = &dialog->window;
	struct cavan_window *title = &dialog->title.window;

	ret = cavan_window_init(win, context);
	if (ret < 0)
	{
		return ret;
	}

	ret = cavan_label_init(&dialog->title, context, text);
	if (ret < 0)
	{
		goto out_win_destroy;
	}

	dialog->title.align = CAVAN_WIN_TEXT_ALIGN_CENTER;

	ret = cavan_window_init(&dialog->body, context);
	if (ret < 0)
	{
		goto out_title_destroy;
	}

	dialog->move_able = true;
	dialog->backup = NULL;
	dialog->xofs = dialog->yofs = 0;

	win->border_width = 0;
	win->paint_handler = cavan_dialog_paint_handler;
	win->click_handler = cavan_dialog_click_handler;
	win->move_handler = cavan_dialog_move_handler;

	win->get_rect_handler(win, &rect);

	title->x = 0;
	title->y = 0;
	title->width = rect.width;

	body->x = 0;
	body->width = rect.width;

	cavan_dialog_set_title_height(dialog, title->height);

	cavan_window_add_child(win, title);
	cavan_window_add_child(win, body);

out_title_destroy:
	title->destroy_handler(title);
out_win_destroy:
	win->destroy_handler(win);
	return 0;
}

void cavan_dialog_set_title_height(struct cavan_dialog *dialog, int height)
{
	int y;
	struct cavan_display_rect rect;
	struct cavan_window *title = &dialog->title.window;
	struct cavan_window *win = &dialog->window;
	struct cavan_window *body = &dialog->body;

	cavan_window_lock(title);
	cavan_window_lock(win);
	cavan_window_lock(body);

	title->height = height;
	title->bottom = title->yabs + height - 1;

	win->get_rect_handler(win, &rect);

	if (height)
	{
		y = title->y + height + CAVAN_WINDOW_SEP_WIDTH;
	}
	else
	{
		y = 0;
	}

	body->height = rect.height - y;

	if (body->y == y)
	{
		body->bottom = body->yabs + body->height - 1;
	}
	else
	{
		body->y = y;
		cavan_window_set_abs_position_base(body, body->xabs, win->yabs + body->y + rect.y);
	}

	cavan_window_unlock(body);
	cavan_window_unlock(win);
	cavan_window_unlock(title);
}

// ================================================================================

void cavan_button_paint_handler(struct cavan_window *win)
{
	cavan_label_paint_handler(win);
}

bool cavan_button_click_handler(struct cavan_window *win, struct cavan_input_message_point *point)
{
	struct cavan_button *button;
	struct cavan_display_device *display;

	display = win->context->display;
	button = (struct cavan_button *)win;

	if (win->pressed)
	{
		button->back_color_backup = win->back_color;
		button->fore_color_backup = win->fore_color;
		button->border_color_backup = win->border_color;

		win->back_color = cavan_display_build_color3f(display, 0, 0, 1.0);
		win->fore_color = cavan_display_build_color3f(display, 1.0, 1.0, 0);
		win->border_color = cavan_display_build_color3f(display, 1.0, 0, 0);
	}
	else
	{
		win->back_color = button->back_color_backup;
		win->fore_color = button->fore_color_backup;
		win->border_color = button->border_color_backup;
	}

	return true;
}

int cavan_button_init(struct cavan_button *button, struct cavan_application_context *context, const char *text)
{
	int ret;
	struct cavan_window *win;
	struct cavan_display_device *display;

	ret = cavan_label_init(&button->label, context, text);
	if (ret < 0)
	{
		return ret;
	}

	button->label.align = CAVAN_WIN_TEXT_ALIGN_CENTER;

	win = &button->label.window;
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

	cavan_label_paint_handler(win);

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
	rect.x += win->xabs;
	rect.y += win->yabs;

	display = win->context->display;

	width = rect.width * percent;
	display->set_color(display, bar->complete_color);
	display->fill_rect(display, rect.x, rect.y, width, rect.height);

	sprintf(buff, "%2.2lf%%", percent * 100);
	display->set_color(display, win->fore_color);
	cavan_display_draw_text_center(display, &rect, buff);
}

int cavan_progress_bar_init(struct cavan_progress_bar *bar, struct cavan_application_context *context, double total)
{
	int ret;
	struct cavan_window *win = &bar->window;

	ret = cavan_window_init(win, context);
	if (ret < 0)
	{
		return ret;
	}

	win->paint_handler = cavan_progress_bar_paint_handler;

	bar->pos = 0;
	bar->total = total;
	bar->complete_color = cavan_display_build_color3f(context->display, 0, 0, 1.0);

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

	cavan_display_lock(display);
	mem = cavan_display_memory_alloc(display, CAVAN_MOUSE_SIZE, CAVAN_MOUSE_SIZE);
	cavan_display_unlock(display);

	return mem;
}

static inline int cavan_application_restore_mouse(struct cavan_application_context *context)
{
	int ret;
	struct cavan_display_device *display = context->display;

	cavan_display_lock(display);
	ret = cavan_display_memory_restore(display, context->mouse_backup);
	cavan_display_unlock(display);

	return ret;
}

static void cavan_application_draw_mouse(struct cavan_application_context *context)
{
	struct cavan_display_device *display = context->display;
	struct cavan_display_memory *mem = context->mouse_backup;

	cavan_display_lock(display);
	cavan_display_memory_backup(display, mem, context->x, context->y);
	display->set_color(display, context->mouse_color);
	display->draw_rect(display, context->x, context->y, CAVAN_MOUSE_SIZE, CAVAN_MOUSE_SIZE);
	cavan_display_unlock(display);

	cavan_display_refresh(display);
}

static bool cavan_window_alias_has(struct double_link *link, struct cavan_window *win)
{
	struct cavan_window_alias *alias;

	double_link_foreach(link, alias)
	{
		if (win == alias->win)
		{
			link_foreach_return(link, true);
		}
	}
	end_link_foreach(link);

	return false;
}

static void cavan_application_move(struct cavan_application_context *context, struct cavan_input_message_point *point)
{
	struct cavan_window_alias *alias;
	struct cavan_window *win = cavan_window_find_by_point(&context->win_link, point);

	context->x = point->x;
	context->y = point->y;

	if (win && cavan_window_alias_has(&context->win_current, win) == false)
	{
		alias = cavan_data_pool_alloc(&context->win_pool);
		if (alias)
		{
			alias->win = win;
			double_link_append(&context->win_current, &alias->node.node);
		}

		cavan_window_mouse_entry(win, point);
	}

	double_link_foreach(&context->win_current, alias)
	{
		if (cavan_window_contian_point(alias->win, point))
		{
			cavan_window_mouse_move(alias->win, point);
		}
		else
		{
			cavan_window_mouse_exit(alias->win, point);
			double_link_remove_base(&alias->node.node);
			alias->node.destroy(&alias->node, alias);
		}
	}
	end_link_foreach(&context->win_current);
}

static void cavan_application_click(struct cavan_application_context *context, struct cavan_input_message_point *point)
{
	struct cavan_window *win;

	if (point->pressed)
	{
		bool is_active = false;
		struct cavan_window_alias *alias;

		win = cavan_window_find_by_point(&context->win_link, point);

		double_link_foreach(&context->win_active, alias)
		{
			struct cavan_window *active_win = alias->win;

			if (active_win == win)
			{
				is_active = true;
				continue;
			}

			point->pressed = false;
			cavan_window_clicked(active_win, point);
			if (active_win->pressed == false)
			{
				double_link_remove_base(&alias->node.node);
				alias->node.destroy(&alias->node, alias);
			}
		}
		end_link_foreach(&context->win_active);

		if (win)
		{
			point->pressed = true;
			cavan_window_clicked(win, point);
			cavan_window_set_top(win);
			win->move_mask |= point->id;

			if (is_active == false)
			{
				struct cavan_window_alias *alias;

				alias = cavan_data_pool_alloc(&context->win_pool);
				if (alias)
				{
					alias->win = win;
					double_link_append(&context->win_active, &alias->node.node);
				}
			}
		}
	}
	else
	{
		struct cavan_window_alias *alias;

		double_link_foreach(&context->win_active, alias)
		{
			win = alias->win;
			cavan_window_clicked(win, point);
		}
		end_link_foreach(&context->win_active);
	}
}

// ================================================================================

static void cavan_application_on_key_message(struct cavan_application_context *context, struct cavan_input_message_key *message)
{
	bool res;

	pthread_mutex_lock(&context->lock);

	if (context->on_key_pressed)
	{
		pthread_mutex_unlock(&context->lock);
		res = context->on_key_pressed(context, message, context->private_data);
		pthread_mutex_lock(&context->lock);
	}
	else
	{
		res = false;
	}

	if (res == false)
	{
		struct cavan_window_alias *alias;

		double_link_foreach(&context->win_active, alias)
		{
			cavan_window_key_pressed(alias->win, message);
		}
		end_link_foreach(&context->win_active);
	}

	pthread_mutex_unlock(&context->lock);
}

static void cavan_application_on_wheel_message(struct cavan_application_context *context, struct cavan_input_message_key *message)
{
}

static void cavan_application_on_mouse_move_message(struct cavan_application_context *context, struct cavan_input_message_vector *message)
{
	int x, y;
	struct cavan_input_message_point point =
	{
		.id = 0,
		.pressed = true
	};

	pthread_mutex_lock(&context->lock);

	x = message->x * context->mouse_speed + context->x;
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

	y = message->y * context->mouse_speed + context->y;
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

static void cavan_application_on_mouse_touch_message(struct cavan_application_context *context, struct cavan_input_message_key *message)
{
	struct cavan_input_message_point point =
	{
		.id = message->code - BTN_MOUSE,
		.pressed = message->value > 0,
		.x = context->x,
		.y = context->y
	};

	pthread_mutex_lock(&context->lock);

	cavan_application_restore_mouse(context);
	cavan_application_click(context, &point);
	cavan_application_draw_mouse(context);

	pthread_mutex_unlock(&context->lock);
}

static void cavan_application_on_touch_message(struct cavan_application_context *context, struct cavan_input_message_point *point)
{
	pthread_mutex_lock(&context->lock);

	context->x = point->x;
	context->y = point->y;

	cavan_application_click(context, point);

	pthread_mutex_unlock(&context->lock);
}

static void cavan_application_on_move_message(struct cavan_application_context *context, struct cavan_input_message_point *point)
{
	pthread_mutex_lock(&context->lock);
	cavan_application_move(context, point);
	pthread_mutex_unlock(&context->lock);
}

// ================================================================================

static void cavan_application_message_queue_handler(cavan_input_message_t *message, void *data)
{
	switch (message->type)
	{
	case CAVAN_INPUT_MESSAGE_KEY:
		cavan_application_on_key_message(data, &message->key);
		break;

	case CAVAN_INPUT_MESSAGE_MOVE:
		cavan_application_on_move_message(data, &message->point);
		break;

	case CAVAN_INPUT_MESSAGE_TOUCH:
		cavan_application_on_touch_message(data, &message->point);
		break;

	case CAVAN_INPUT_MESSAGE_WHEEL:
		cavan_application_on_wheel_message(data, &message->key);
		break;

	case CAVAN_INPUT_MESSAGE_MOUSE_MOVE:
		cavan_application_on_mouse_move_message(data, &message->vector);
		break;

	case CAVAN_INPUT_MESSAGE_MOUSE_TOUCH:
		cavan_application_on_mouse_touch_message(data, &message->key);
		break;

	default:
		pr_red_info("Invalid message type %d", message->type);
	}
}

static int cavan_application_main_thread_handler(struct cavan_thread *thread, void *data)
{
	int ret;
	u32 event;

	ret = cavan_thread_recv_event(thread, &event);
	if (ret < 0)
	{
		pr_red_info("cavan_thread_recv_event");
		return ret;
	}

	switch (event)
	{
	case CAVAN_APP_EVENT_STOP:
	case CAVAN_APP_EVENT_EXIT:
		cavan_thread_should_stop(thread);
		break;

	default:
		pr_red_info("unknown event %d", event);
	}

	return 0;
}

int cavan_application_init(struct cavan_application_context *context, struct cavan_display_device *display, void *data)
{
	int ret;
	struct cavan_thread *thread;
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

	ret = cavan_display_start(display);
	if (ret < 0)
	{
		pr_red_info("cavan_display_check");
		goto out_display_destroy;
	}

	ret = double_link_init(&context->win_link, MOFS(struct cavan_window, node));
	if (ret < 0)
	{
		pr_red_info("double_link_init");
		goto out_cavan_display_stop;
	}

	ret = cavan_data_pool_init(&context->win_pool, MOFS(struct cavan_window_alias, node), \
		sizeof(struct cavan_window_alias), CAVAN_WINDOW_POOL_SIZE);
	if (ret < 0)
	{
		pr_red_info("cavan_data_pool_init");
		goto out_double_link_deinit;
	}

	ret = double_link_init(&context->win_active, MOFS(struct cavan_window_alias, node.node));
	if (ret < 0)
	{
		pr_red_info("double_link_init");
		goto out_cavan_data_pool_deinit;
	}

	ret = double_link_init(&context->win_current, MOFS(struct cavan_window_alias, node.node));
	if (ret < 0)
	{
		pr_red_info("double_link_init");
		goto out_double_link_deinit_active;
	}

	context->mouse_backup = cavan_application_mouse_alloc(context);
	if (context->mouse_backup == NULL)
	{
		ret = -ENOMEM;
		pr_red_info("cavan_display_memory_alloc");
		goto out_double_link_deinit_current;
	}

	thread = &context->thread;
	thread->name = "AppMain";
	thread->wake_handker = NULL;
	thread->handler = cavan_application_main_thread_handler;
	ret = cavan_thread_init(thread, context);
	if (ret < 0)
	{
		pr_red_info("cavan_thread_init");
		goto out_display_memory_free;
	}

	context->private_data = data;

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
	input_service->handler = cavan_application_message_queue_handler;

	context->on_key_pressed = NULL;

	return 0;

out_display_memory_free:
	cavan_display_memory_free(context->mouse_backup);
out_double_link_deinit_current:
	double_link_deinit(&context->win_current);
out_double_link_deinit_active:
	double_link_deinit(&context->win_active);
out_cavan_data_pool_deinit:
	cavan_data_pool_deinit(&context->win_pool);
out_double_link_deinit:
	double_link_deinit(&context->win_link);
out_cavan_display_stop:
	cavan_display_stop(display);
out_display_destroy:
	display->destroy(display);
out_pthread_mutex_destroy:
	pthread_mutex_destroy(&context->lock);

	return ret;
}

void cavan_application_deinit(struct cavan_application_context *context)
{
	struct cavan_window *win;

	cavan_thread_deinit(&context->thread);

	double_link_foreach(&context->win_link, win)
	{
		cavan_window_destroy(win);
	}
	end_link_foreach(&context->win_link);

	double_link_deinit(&context->win_link);
	double_link_deinit(&context->win_active);
	double_link_deinit(&context->win_current);
	cavan_data_pool_deinit(&context->win_pool);

	cavan_display_memory_free(context->mouse_backup);
	cavan_display_stop(context->display);
	context->display->destroy(context->display);

	pthread_mutex_destroy(&context->lock);
}

int cavan_application_main_loop(struct cavan_application_context *context, void (*handler)(struct cavan_application_context *context, void *data), void *data)
{
	int ret;

	ret = cavan_timer_service_start(&context->timer_service);
	if (ret < 0)
	{
		pr_red_info("cavan_timer_service_start");
		return ret;
	}

	ret = cavan_cursor_init(&context->cursor, &context->timer_service);
	if (ret < 0)
	{
		pr_red_info("cavan_cursor_init");
		goto out_cavan_timer_service_stop;
	}

	ret = cavan_input_service_start(&context->input_service, context);
	if (ret < 0)
	{
		pr_red_info("cavan_input_service_start");
		goto out_cavan_timer_service_stop;
	}

	ret = cavan_thread_start(&context->thread);
	if (ret < 0)
	{
		pr_red_info("cavan_thread_start");
		goto out_cavan_input_service_stop;
	}

	cavan_application_update_data(context);

	if (handler)
	{
		handler(context, data);
	}

	cavan_thread_join(&context->thread);

out_cavan_input_service_stop:
	cavan_input_service_stop(&context->input_service);
	cavan_input_service_join(&context->input_service);
out_cavan_timer_service_stop:
	cavan_timer_service_stop(&context->timer_service);
	return ret;
}

void cavan_application_update_data(struct cavan_application_context *context)
{
	struct cavan_display_device *display;

	pthread_mutex_lock(&context->lock);

	display = context->display;

	cavan_display_lock(display);

	display->set_color(display, context->back_color);
	display->fill_rect(display, 0, 0, display->xres, display->yres);

	cavan_window_paint_child(&context->win_link);

	cavan_display_unlock(display);

	pthread_mutex_unlock(&context->lock);

	cavan_display_refresh(display);

}

void cavan_application_add_window(struct cavan_application_context *context, struct cavan_window *win)
{
	struct cavan_display_device *display = context->display;
	struct cavan_display_rect rect =
	{
		.x = 0,
		.y = 0,
		.width = display->xres,
		.height = display->yres
	};

	pthread_mutex_lock(&context->lock);

	win->parent = NULL;
	cavan_window_add_child_base(&context->win_link, win, &rect);

	pthread_mutex_unlock(&context->lock);
}

void cavan_application_remove_window(struct cavan_application_context *context, struct cavan_window *win)
{
	pthread_mutex_lock(&context->lock);
	double_link_remove(&context->win_link, &win->node);
	pthread_mutex_unlock(&context->lock);
}
