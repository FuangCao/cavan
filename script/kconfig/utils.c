/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Tue Jan  3 15:37:38 CST 2012
 */

#include <stdlib.h>
#include "kconfig.h"

void ncurses_fill_character(WINDOW *win, chtype ch, size_t count)
{
	while (count--)
	{
		waddch(win, ch);
	}
}

void ncurses_fill_line(WINDOW *win, int x, int y, int width, int attr, chtype ch)
{
	wattrset(win, attr);
	wmove(win, y, x);
	ncurses_fill_character(win, ch, width);
}

void ncurses_fill_column(WINDOW *win, int x, int y, int height, int attr, chtype ch)
{
	int bottom;

	wattrset(win, attr);

	for (bottom = y + height; y < bottom; y++)
	{
		wmove(win, y, x);
		waddch(win, ch);
	}
}

void ncurses_fill_rectangle(WINDOW *win, int x, int y, int width, int height, int attr, chtype ch)
{
	int i, bottom;

	wattrset(win, attr);

	for (bottom = y + height; y < bottom; y++)
	{
		wmove(win, y, x);

		for (i = 0; i < width; i++)
		{
			waddch(win, ch);
		}
	}
}

void ncurses_draw_rectangle(WINDOW *win, int x, int y, int width, int height, int attr1, int attr2)
{
	ncurses_fill_line(win, x + 1, y, width - 2, attr1, ACS_HLINE);
	ncurses_fill_column(win, x, y + 1, height - 2, attr1, ACS_VLINE);
	mvwaddch(win, y, x, ACS_ULCORNER);
	mvwaddch(win, y + height - 1, x, ACS_LLCORNER);

	ncurses_fill_line(win, x + 1, y + height - 1, width - 2, attr2, ACS_HLINE);
	ncurses_fill_column(win, x + width - 1, y + 1, height - 2, attr2, ACS_VLINE);
	mvwaddch(win, y, x + width - 1, ACS_URCORNER);
	mvwaddch(win, y + height - 1, x + width - 1, ACS_LRCORNER);
}

void ncurses_init(void)
{
	initscr();
	cbreak();
	noecho();

	start_color();
	init_pair(KCONFIG_COLOR_WHITE_WHITE, COLOR_WHITE, COLOR_WHITE);
	init_pair(KCONFIG_COLOR_WHITE_BLACK, COLOR_BLACK, COLOR_WHITE);
	init_pair(KCONFIG_COLOR_WHITE_RED, COLOR_RED, COLOR_WHITE);
	init_pair(KCONFIG_COLOR_WHITE_YELLOW, COLOR_YELLOW, COLOR_WHITE);
	init_pair(KCONFIG_COLOR_WHITE_BLUE, COLOR_BLUE, COLOR_WHITE);
	init_pair(KCONFIG_COLOR_BLUE_WHITE, COLOR_WHITE, COLOR_BLUE);
	init_pair(KCONFIG_COLOR_BLUE_BLACK, COLOR_BLACK, COLOR_BLUE);
	init_pair(KCONFIG_COLOR_BLUE_YELLOW, COLOR_YELLOW, COLOR_BLUE);
	init_pair(KCONFIG_COLOR_BLACK_BLACK, COLOR_BLACK, COLOR_BLACK);

	keypad(stdscr, TRUE);
	ncurses_fill_rectangle(stdscr, 0, 0, COLS, LINES, COLOR_PAIR(KCONFIG_COLOR_BLUE_WHITE), ' ');
}

void ncurses_set_title(WINDOW *win, int x, int y, int width, const char *title, int attr1, int attr2)
{
	int length = strlen(title) + 2;

	ncurses_fill_line(win, x, y, width, attr1, ACS_HLINE);

	if (length > width)
	{
		return;
	}

	x = (width - length) >> 1;
	wattrset(win, attr2);
	wmove(win, 0, x);
	waddch(win, ' ');
	waddstr(win, title);
	waddch(win, ' ');
}

void ncurses_draw_border(WINDOW *win, const char *title, int attr1, int attr2, int attr3)
{
	int width, height;

	width = getmaxx(win);
	height = getmaxy(win);

	ncurses_draw_rectangle(win, 0, 0, width, height, attr1, attr2);

	if (title)
	{
		ncurses_set_title(win, 1, 0, width - 2, title, attr1, attr3);
	}
}

void ncurses_draw_separate(WINDOW *win, int y, int width, int attrs[])
{
	ncurses_fill_line(win, 1, y, width - 2, attrs[0], ACS_HLINE);

	wattrset(win, attrs[0]);
	mvwaddch(win, y, 0, ACS_LTEE);

	wattrset(win, attrs[1]);
	mvwaddch(win, y, width - 1, ACS_RTEE);
}

int ncurses_print_text(WINDOW *win, int x, int y, int width, const char *text, int attr)
{
	int length;
	const char *text_end;

	wattrset(win, attr);
	length = strlen(text);

	if (length < width)
	{
		mvwaddstr(win, y, (width - length) >> 1, text);
		return y;
	}

	for (text_end = text + length; text < text_end; text += width)
	{
		wmove(win, y, x);
		waddnstr(win, text, width);
		y++;
	}

	return y;
}

WINDOW *ncurses_new_window(WINDOW *orig, int x, int y, int width, int height, const char *title, int attrs[], size_t attr_count)
{
	WINDOW *win;

	if (attr_count < 3)
	{
		return NULL;
	}

	if (orig)
	{
		win = subwin(orig, height, width, y, x);
	}
	else
	{
		win = newwin(height, width, y, x);
	}

	if (win == NULL)
	{
		return NULL;
	}

	if (attr_count > 3)
	{
		ncurses_fill_rectangle(stdscr, x + 2, y + 1, width, height, attrs[3], ' ');
	}

	ncurses_fill_rectangle(win, 1, 1, width - 2, height - 2, attrs[0], ' ');
	ncurses_draw_border(win, title, attrs[0], attrs[1], attrs[2]);

	keypad(win, TRUE);

	return win;
}

void ncurses_draw_button(WINDOW *win, int x, int y, const char *text, int attr1, int attr2)
{
	wmove(win, y, x - 2);

	wattrset(win, attr1);
	waddstr(win, "< ");
	wattrset(win, attr2);
	waddch(win, text[0]);
	wattrset(win, A_BOLD | attr1);
	waddstr(win, text + 1);
	wattrset(win, attr1);
	waddstr(win, " >");
}

void ncurses_draw_buttons(WINDOW *win, int y, int width, const char *texts[], size_t count, int selected)
{
	int i;
	int x, sx;
	float step;
	int attr1, attr2, attr3, attr4;

	attr1 = COLOR_PAIR(KCONFIG_COLOR_WHITE_BLACK);
	attr2 = COLOR_PAIR(KCONFIG_COLOR_WHITE_RED);
	attr3 = COLOR_PAIR(KCONFIG_COLOR_BLUE_WHITE);
	attr4 = COLOR_PAIR(KCONFIG_COLOR_BLUE_YELLOW);

	for (i = 0, step = (float)width / (count + 1); i < count; i++)
	{
		x = (i + 1) * step - (strlen(texts[i]) >> 1);

		if (i == selected)
		{
			sx = x;
			ncurses_draw_button(win, x, y, texts[i], attr3, attr4);
		}
		else
		{
			ncurses_draw_button(win, x, y, texts[i], attr1, attr2);
		}
	}

	wmove(win, y, sx);
}

int ncurses_show_message_box(int width, int height, const char *prompt, const char *buttons[], size_t count)
{
	int x, y;
	int state;
	WINDOW *win;
	int attrs[] =
	{
		COLOR_PAIR(KCONFIG_COLOR_WHITE_WHITE) | A_BOLD,
		COLOR_PAIR(KCONFIG_COLOR_WHITE_BLACK),
		COLOR_PAIR(KCONFIG_COLOR_WHITE_BLUE),
		COLOR_PAIR(KCONFIG_COLOR_BLACK_BLACK),
	};

	ncurses_init();

	if (width < 1 || height < 1 || width > COLS || height > LINES)
	{
		return -1;
	}

	x = (COLS - width) >> 1;
	y = (LINES - height) >> 1;

	win = ncurses_new_window(NULL, x, y, width, height, NULL, attrs, NELEM(attrs));
	if (win == NULL)
	{
		return -1;
	}

	if (prompt)
	{
		y = ncurses_print_text(win, 2, 1, width - 4, prompt, attrs[1]);
		ncurses_draw_separate(win, y + 1, width, attrs);
	}

	state = 0;
	wrefresh(stdscr);

	while (1)
	{
		ncurses_draw_buttons(win, 3, width - 2, buttons, count, state);
		wrefresh(win);

		switch (wgetch(win))
		{
		case KEY_UP:
		case KEY_TAB:
		case KEY_LEFT:
			state--;
			if (state < 0)
			{
				state = count - 1;
			}
			break;

		case KEY_DOWN:
		case KEY_RIGHT:
			state++;
			if (state >= count)
			{
				state = 0;
			}
			break;

		case '0':
		case 'y':
		case 'Y':
			delwin(win);
			return 0;

		case '1':
		case 'n':
		case 'N':
			delwin(win);
			return 1;

		case '\n':
			delwin(win);
			return state;

		case KEY_ESC:
			delwin(win);
			return -1;
		}
	}

	return 0;
}

int ncurses_show_yes_no_dialog(int width, int height, const char *prompt)
{
	const char *buttons[] =
	{
		"Yes", "No"
	};

	return ncurses_show_message_box(width, height, prompt, buttons, NELEM(buttons));
}

// ================================================================================

const char *kconfig_state_tostring(enum kconfig_state state)
{
	switch (state)
	{
	case KCONFIG_STATE_SELECTED:
		return "[*]";
	case KCONFIG_STATE_DESELED:
		return "[ ]";
	default:
		return "[-]";
	}
}

void ncurses_draw_menu_item_base(WINDOW *win, struct kconfig_menu_item *item, int selected)
{
	int attr1, attr2;

	if (selected)
	{
		attr1 = COLOR_PAIR(KCONFIG_COLOR_BLUE_WHITE);
		attr2 = COLOR_PAIR(KCONFIG_COLOR_BLUE_YELLOW);
	}
	else
	{
		attr1 = COLOR_PAIR(KCONFIG_COLOR_WHITE_BLACK);
		attr2 = COLOR_PAIR(KCONFIG_COLOR_WHITE_BLUE);
	}

	wattrset(win, attr1);
	wmove(win, item->y, item->x);
	waddstr(win, kconfig_state_tostring(item->state));
	ncurses_fill_character(win, ' ', (item->level << 1) + 1);
	wattrset(win, attr2);
	waddch(win, item->text[0]);
	wattrset(win, attr1);
	waddnstr(win, item->text + 1, item->width - 8);
}

void ncurses_draw_menu_base(WINDOW *win, struct kconfig_menu_descriptor *desc, int y, int level, struct cavan_list_node *head)
{
	struct cavan_list_node *node;
	struct kconfig_menu_item *item;

	if (head == NULL)
	{
		return;
	}

	for (node = head; ; node = node->next)
	{
		item = list_node_to_menu_item(node);

		item->x = desc->x;
		item->y = y;
		item->level = level;
		item->width = desc->width;
		ncurses_draw_menu_item_base(win, item, 0);

		if (item->state != KCONFIG_STATE_DESELED && item->child)
		{
			ncurses_draw_menu_base(win, desc, y + 1, level + 1, &item->child->list_node);
		}

		if (node->next == head)
		{
			break;
		}

		y = getcury(win) + 1;
	}
}

struct kconfig_menu_item *list_node_to_menu_item(struct cavan_list_node *node)
{
	if (node == NULL)
	{
		return NULL;
	}

	return GET_STRUCT_POINTER(struct kconfig_menu_item, list_node, node);
}

void ncurses_draw_menu(WINDOW *win, struct kconfig_menu_descriptor *desc)
{
	ncurses_fill_rectangle(win, desc->x, desc->y, desc->width, desc->height, COLOR_PAIR(KCONFIG_COLOR_WHITE_WHITE), ' ');
	ncurses_draw_menu_base(win, desc, desc->y, 0, desc->head);
}

struct kconfig_menu_item *kconfig_menu_get_next_item(struct kconfig_menu_item *curr_item)
{
	struct kconfig_menu_item *item;

	if (curr_item->state != KCONFIG_STATE_DESELED && curr_item->child)
	{
		return curr_item->child;
	}

	item = list_node_to_menu_item(curr_item->list_node.next);
	if (item->parent)
	{
		return list_node_to_menu_item(item->parent->list_node.next);
	}

	return item;
}

struct kconfig_menu_item *kconfig_menu_get_prev_item(struct kconfig_menu_item *curr_item)
{
	struct kconfig_menu_item *item;

	if (curr_item->parent)
	{
		return curr_item->parent;
	}

	item = list_node_to_menu_item(curr_item->list_node.prev);
	while (item->state != KCONFIG_STATE_DESELED && item->child)
	{
		item = list_node_to_menu_item(item->child->list_node.prev);
	}

	return item;
}


int ncurses_show_menu_box(struct kconfig_menu_descriptor *desc)
{
	WINDOW *win;
	int attrs[] =
	{
		COLOR_PAIR(KCONFIG_COLOR_WHITE_WHITE) | A_BOLD,
		COLOR_PAIR(KCONFIG_COLOR_WHITE_BLACK),
		COLOR_PAIR(KCONFIG_COLOR_WHITE_BLUE),
		COLOR_PAIR(KCONFIG_COLOR_BLACK_BLACK),
	};
	struct kconfig_menu_item *curr_item;
	int width, height, y;
	int need_update;

	ncurses_init();

	width = COLS - 4;
	height = LINES - 4;

	win = ncurses_new_window(NULL, 1, 1, width, height, desc->title, attrs, NELEM(attrs));
	if (win == NULL)
	{
		return -1;
	}

	if (desc->prompt)
	{
		y = ncurses_print_text(win, 2, 2, width - 4, desc->prompt, attrs[1]);
		ncurses_draw_separate(win, y + 2, width, attrs);
	}

	y = getcury(win) + 1;
	ncurses_draw_rectangle(win, 2, y, width - 4, height - y - 2, attrs[1], attrs[0]);

	desc->x = width / 3;
	desc->y = y + 2;
	desc->width = width - desc->x - 4;
	desc->height = height - desc->y - 4;

	curr_item = list_node_to_menu_item(desc->head);

	wrefresh(stdscr);
	need_update = 1;

	while (1)
	{
		if (need_update && curr_item->child)
		{
			ncurses_draw_menu(win, desc);
			need_update = 0;
		}

		ncurses_draw_menu_item_base(win, curr_item, 1);
		wmove(win, curr_item->y, curr_item->x + 1);
		wrefresh(win);

		switch (wgetch(win))
		{
		case KEY_UP:
			ncurses_draw_menu_item_base(win, curr_item, 0);
			curr_item = kconfig_menu_get_prev_item(curr_item);
			break;

		case KEY_DOWN:
			ncurses_draw_menu_item_base(win, curr_item, 0);
			curr_item = kconfig_menu_get_next_item(curr_item);
			break;

		case KEY_ESC:
			delwin(win);
			return -1;

		case 'n':
			curr_item->state = KCONFIG_STATE_DESELED;
			need_update = 1;
			break;

		case 'y':
			curr_item->state = KCONFIG_STATE_SELECTED;
			need_update = 1;
			break;

		case ' ':
			if (curr_item->state == KCONFIG_STATE_SELECTED)
			{
				curr_item->state = KCONFIG_STATE_DESELED;
			}
			else
			{
				curr_item->state = KCONFIG_STATE_SELECTED;
			}
			need_update = 1;
			break;

		case '\n':
			delwin(win);
			return 0;
		}
	}

	return 0;
}

void kconfig_menu_init(struct kconfig_menu_descriptor *desc)
{
	desc->head = NULL;
}

void kconfig_menu_item_init(struct kconfig_menu_item *item, const char *text)
{
	if (text)
	{
		strcpy(item->text, text);
	}

	item->child = NULL;
}

struct kconfig_menu_item *kconfig_menu_new_item(const char *text)
{
	struct kconfig_menu_item *item;

	item = malloc(sizeof(*item));
	if (item == NULL)
	{
		return NULL;
	}

	kconfig_menu_item_init(item, text);

	return item;
}

void kconfig_menu_add_item(struct kconfig_menu_descriptor *desc, struct kconfig_menu_item *item)
{
	item->parent = NULL;
	item->child = NULL;

	if (desc->head == NULL)
	{
		desc->head = &item->list_node;
		cavan_list_head_init(&item->list_node);
	}
	else
	{
		cavan_list_append(desc->head, &item->list_node);
	}
}

void kconfig_menu_add_child(struct kconfig_menu_item *item, struct kconfig_menu_item *child)
{
	child->child = NULL;

	if (item->child == NULL)
	{
		item->child = child;
		child->parent = item;
		cavan_list_head_init(&child->list_node);
	}
	else
	{
		child->parent = NULL;
		cavan_list_append(&item->child->list_node, &child->list_node);
	}
}
