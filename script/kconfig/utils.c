/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Tue Jan  3 15:37:38 CST 2012
 */

#include "kconfig.h"

void ncurses_fill_line(WINDOW *win, int x, int y, int width, int attr, chtype ch)
{
	int right;

	wattrset(win, attr);
	wmove(win, y, x);

	for (right = x + width; x < right; x++)
	{
		waddch(win, ch);
	}

	wrefresh(win);
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

	wrefresh(win);
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

	wrefresh(win);
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

	wrefresh(win);
}

void ncurses_set_title(WINDOW *win, const char *title, int attr1, int attr2)
{
	int width = getmaxx(win);
	int length = strlen(title);

	if (length + 4 < width)
	{
		int x = (width - length) >> 1;

		ncurses_fill_line(win, 1, 0, width - 2, attr1, ACS_HLINE);

		wattrset(win, attr2);
		wmove(win, 0, x);
		waddch(win, ' ');
		waddstr(win, title);
		waddch(win, ' ');
	}
}

void ncurses_draw_border(WINDOW *win, const char *title, int attr1, int attr2, int attr3)
{
	int width, height;

	width = getmaxx(win);
	height = getmaxy(win);

	ncurses_draw_rectangle(win, 0, 0, width, height, attr1, attr2);

	if (title)
	{
		int lenght = strlen(title);

		if (lenght < width)
		{
			int x = (width - lenght - 2) >> 1;

			wattrset(win, attr3);
			wmove(win, 0, x);
			waddch(win, ' ');
			waddstr(win, title);
			waddch(win, ' ');
			wrefresh(win);
		}
	}
}

void ncurses_draw_separate(WINDOW *win, int y, int width, int attrs[])
{
	ncurses_fill_line(win, 1, y, width - 2, attrs[0], ACS_HLINE);

	wattrset(win, attrs[0]);
	mvwaddch(win, y, 0, ACS_LTEE);

	wattrset(win, attrs[1]);
	mvwaddch(win, y, width - 1, ACS_RTEE);

	wrefresh(win);
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
		wrefresh(win);
		return y;
	}

	for (text_end = text + length; text < text_end; text += width)
	{
		wmove(win, y, x);
		waddnstr(win, text, width);
		y++;
	}

	wrefresh(win);

	return y;
}

WINDOW *ncurses_new_window(WINDOW *orig, int x, int y, int width, int height, const char *title, const char *prompt, int attrs[], size_t attr_count)
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

	if (prompt)
	{
		ncurses_print_text(win, 2, 1, width - 4, prompt, attrs[1]);
		ncurses_draw_separate(win, getcury(win) + 1, width, attrs);
	}

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

	wrefresh(win);
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

	wrefresh(win);
}

int ncurses_show_message_box(int width, int height, const char *prompt, const char *buttons[], size_t count)
{
	int x, y;
	int select;
	WINDOW *win;
	int attrs[] =
	{
		COLOR_PAIR(KCONFIG_COLOR_WHITE_WHITE) | A_BOLD,
		COLOR_PAIR(KCONFIG_COLOR_WHITE_BLACK),
		COLOR_PAIR(KCONFIG_COLOR_WHITE_BLUE),
		COLOR_PAIR(KCONFIG_COLOR_BLACK_BLACK),
	};

	if (width < 1 || height < 1 || width > COLS || height > LINES)
	{
		return -1;
	}

	x = (COLS - width) >> 1;
	y = (LINES - height) >> 1;

	win = ncurses_new_window(NULL, x, y, width, height, NULL, prompt, attrs, NELEM(attrs));
	if (win == NULL)
	{
		return -1;
	}

	select = 0;

	while (1)
	{
		ncurses_draw_buttons(win, 3, width - 2, buttons, count, select);

		switch (wgetch(win))
		{
		case KEY_LEFT:
		case KEY_TAB:
			select--;
			if (select < 0)
			{
				select = count - 1;
			}
			break;

		case KEY_RIGHT:
			select++;
			if (select >= count)
			{
				select = 0;
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
			return select;

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

void ncurses_draw_menu_item(WINDOW *win, int x, int y, int width, struct ncurses_menu_item *item, int current)
{
	int attr1, attr2;
	char buff[1024];

	if (current)
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
	wmove(win, y, x);
	sprintf(buff, "[%c]   ", item->select);
	waddstr(win, buff);
	wattrset(win, attr2);
	waddch(win, item->text[0]);
	wattrset(win, attr1);
	waddnstr(win, item->text + 1, width - 8);
}

void ncurses_draw_menu(WINDOW *win, int x, int y, int width, int height, struct ncurses_menu_item items[], size_t count, int current)
{
	int i;
	int bottom;

	for (i = 0, bottom = y + height; i < count && y < bottom; i++, y++)
	{
		ncurses_draw_menu_item(win, x, y, width, items + i, i == current);
	}

	wrefresh(win);
}

int ncurses_show_menu_box(const char *title, const char *prompt, struct ncurses_menu_item items[], size_t count)
{
	WINDOW *win;
	int attrs[] =
	{
		COLOR_PAIR(KCONFIG_COLOR_WHITE_WHITE) | A_BOLD,
		COLOR_PAIR(KCONFIG_COLOR_WHITE_BLACK),
		COLOR_PAIR(KCONFIG_COLOR_WHITE_BLUE),
		COLOR_PAIR(KCONFIG_COLOR_BLACK_BLACK),
	};
	int current;
	int x, y, width;

	win = ncurses_new_window(NULL, 1, 1, COLS - 4, LINES - 4, title, prompt, attrs, NELEM(attrs));
	if (win == NULL)
	{
		return -1;
	}

	ncurses_draw_rectangle(win, 2, 4, getmaxx(win) - 4, getmaxy(win) - 8, attrs[1], attrs[0]);

	current = 0;
	x = COLS / 3;
	y = 8;
	width = getmaxx(win) - x - 4;
	ncurses_draw_menu(win, x, y, width, getmaxy(win) - y - 8, items, count, current);

	while (1)
	{
		switch (wgetch(win))
		{
		case KEY_UP:
			ncurses_draw_menu_item(win, x, y + current, width, items + current, 0);
			current--;
			if (current < 0)
			{
				current = count - 1;
			}
			ncurses_draw_menu_item(win, x, y + current, width, items + current, 1);
			break;

		case KEY_DOWN:
			ncurses_draw_menu_item(win, x, y + current, width, items + current, 0);
			current++;
			if (current >= count)
			{
				current = 0;
			}
			ncurses_draw_menu_item(win, x, y + current, width, items  + current, 1);
			break;

		case KEY_ESC:
			delwin(win);
			return -1;

		case 'n':
			items[current].select = ' ';
			ncurses_draw_menu_item(win, x, y + current, width, items  + current, 1);
			break;

		case 'y':
			items[current].select = '*';
			ncurses_draw_menu_item(win, x, y + current, width, items  + current, 1);
			break;

		case ' ':
			if (items[current].select == '*')
			{
				items[current].select = ' ';
			}
			else
			{
				items[current].select = '*';
			}
			ncurses_draw_menu_item(win, x, y + current, width, items  + current, 1);
			break;

		case '\n':
			delwin(win);
			return current;
		}
	}

	return 0;
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

	wrefresh(stdscr);
}
