#include <cavan.h>
#include <cavan/progress.h>

static void progress_bar_fflush(struct progress_bar *bar)
{
	print_ntext((char *) &bar->body, sizeof(struct progress_bar_body));
	print_char('\r');
}

static int update_percent(struct progress_bar *bar)
{
	int percent = (int) (((double) bar->current) * 100 / bar->total);

	if (percent == bar->percent)
	{
		return 0;
	}

	bar->percent = percent;
	sprintf(bar->body.percent, " %d%%", percent);
	bar->body.percent[5] = ' ';

	return 1;
}

static void fill_buffer(char *content, int fill, int size)
{
	char *end = content + size;

	while (content < end)
	{
		if (fill > 0)
		{
			*content = FULL_CHAR;
			fill--;
		}
		else
		{
			*content = FREE_CHAR;
		}

		content++;
	}
}

static int update_content(struct progress_bar *bar)
{
	int length;

	length = (int) (((double) bar->current) * HALF_LENGTH * 2 / bar->total);

	if (length == bar->length)
	{
		return 0;
	}

	bar->length = length;

	if (length <= HALF_LENGTH)
	{
		fill_buffer(bar->body.content1, length, HALF_LENGTH);
		memset(bar->body.content2, FREE_CHAR, HALF_LENGTH);
	}
	else
	{
		memset(bar->body.content1, FULL_CHAR, HALF_LENGTH);
		fill_buffer(bar->body.content2, length - HALF_LENGTH, HALF_LENGTH);
	}

	return 1;
}

void progress_bar_update(struct progress_bar *bar)
{
	if (bar->current > bar->total)
	{
		return;
	}

	if (update_percent(bar) | update_content(bar))
	{
		progress_bar_fflush(bar);
	}
}

void progress_bar_init(struct progress_bar *bar, u64 total)
{
	struct progress_bar_body *body = &bar->body;

	bar->total = total == 0 ? 1 : total;
	bar->current = 0;
	bar->percent = -1;
	bar->length = -1;

	body->head = '[';
	memset(body->percent, 0, sizeof(body->percent));
	body->tail = ']';

	progress_bar_update(bar);
}

void progress_bar_add(struct progress_bar *bar, u64 val)
{
	bar->current += val;

	progress_bar_update(bar);
}

void progress_bar_set(struct progress_bar *bar, u64 val)
{
	bar->current = val;

	progress_bar_update(bar);
}

void progress_bar_finish(struct progress_bar *bar)
{
	bar->current = bar->total;

	progress_bar_update(bar);

	print_char('\n');
}

