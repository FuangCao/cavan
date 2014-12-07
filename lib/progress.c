#include <cavan.h>
#include <cavan/timer.h>
#include <cavan/command.h>
#include <cavan/progress.h>

#define SPEED_INTERVAL_MS	1000

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

s64 progress_bar_get_time_consume_ns(struct progress_bar *bar)
{
	struct timespec time_now;

	clock_gettime_mono(&time_now);

	return cavan_timespec_sub_ns(&time_now, &bar->time_start);
}

static void progress_bar_fflush(struct progress_bar *bar)
{
	int length;
	char buff[1024];
	char *p = buff, *p_end = p + sizeof(buff);

	*p++ = '[';

	if (bar->fill < bar->half_length)
	{
		memset(p, BAR_FULL_CHAR, bar->fill);
		memset(p + bar->fill, BAR_FREE_CHAR, bar->half_length - bar->fill);
	}
	else
	{
		memset(p, BAR_FULL_CHAR, bar->half_length);
	}

	p += bar->half_length;
	p += snprintf(p, p_end - p, " %d%% ", bar->percent);

	if (bar->fill > bar->half_length)
	{
		int progress = bar->fill - bar->half_length;

		memset(p, BAR_FULL_CHAR, progress);
		memset(p + progress, BAR_FREE_CHAR, bar->half_length - progress);
	}
	else
	{
		memset(p, BAR_FREE_CHAR, bar->half_length);
	}

	p += bar->half_length;
	*p++ = ']';

	if (bar->speed >= 0)
	{
		*p++ = ' ';
		p = mem_speed_tostring(bar->speed, p, p_end - p - 2);
	}

	length = p - buff;
	if (length < bar->content_length)
	{
		for (p_end = buff + bar->content_length; p < p_end; p++)
		{
			*p = ' ';
		}
	}

	bar->content_length = length;

	*p++ = '\r';

	pthread_mutex_lock(&lock);
	print_ntext(buff, p - buff);
	pthread_mutex_unlock(&lock);
}

static bool update_percent(struct progress_bar *bar)
{
	int percent = bar->current * 100 / bar->total;

	if (percent == bar->percent)
	{
		return false;
	}

	bar->percent = percent;

	return true;
}

static bool update_content(struct progress_bar *bar)
{
	int fill = bar->current * bar->full_length / bar->total;

	if (fill == bar->fill)
	{
		return false;
	}

	bar->fill = fill;

	return true;
}

static bool update_speed(struct progress_bar *bar)
{
	s64 interval;
	double speed;
	struct timespec time_now;

	clock_gettime_mono(&time_now);

	interval = cavan_timespec_sub_ns(&time_now, &bar->time_prev);
	if (interval < SPEED_INTERVAL_MS * 1000000)
	{
		return false;
	}

	if (bar->current > bar->last)
	{
		speed = (bar->current - bar->last) * 1000000000 / interval;
	}
	else
	{
		speed = 0;
	}

	bar->last = bar->current;
	bar->time_prev = time_now;

	if (bar->speed == speed)
	{
		return false;
	}

	bar->speed = speed;

	return true;
}

void progress_bar_update(struct progress_bar *bar)
{
	if (bar->current > bar->total)
	{
		return;
	}

	if (update_percent(bar) | update_content(bar) | update_speed(bar))
	{
		progress_bar_fflush(bar);
	}
}

void progress_bar_init(struct progress_bar *bar, double total)
{
	u16 columns;

	bar->total = total == 0 ? 1 : total;
	bar->current = bar->last = 0;
	bar->percent = 0;
	bar->fill = 0;
	bar->content_length = 0;

	if (tty_get_win_size(fileno(stdout), NULL, &columns) < 0 || columns == 0)
	{
		bar->half_length = BAR_DEF_HALF_LEN;
	}
	else if (columns < BAR_CONTENT_MIN)
	{
		bar->half_length = 0;
	}
	else
	{
		bar->half_length = (columns - BAR_CONTENT_MIN) / 2;
		if (bar->half_length > BAR_MAX_HALF_LEN)
		{
			bar->half_length = BAR_MAX_HALF_LEN;
		}
	}

	bar->full_length = bar->half_length * 2;
	bar->speed = -1;

	clock_gettime_mono(&bar->time_start);
	bar->time_prev = bar->time_start;

	progress_bar_update(bar);
}

void progress_bar_add(struct progress_bar *bar, double val)
{
	bar->current += val;
	progress_bar_update(bar);
}

void progress_bar_set(struct progress_bar *bar, double val)
{
	bar->current = val;
	progress_bar_update(bar);
}

void progress_bar_finish(struct progress_bar *bar)
{
	double time;

	bar->current = bar->total;
	progress_bar_update(bar);

	print_char('\n');

	time = progress_bar_get_time_consume_ns(bar);
	if (time > 1000)
	{
		char size_buff[32];
		char speed_buff[32];

		mem_size_tostring(bar->total, size_buff, sizeof(size_buff));
		mem_speed_tostring(bar->total * 1000000000 / time, speed_buff, sizeof(speed_buff));
		println("%s (%s in %lf ms)", speed_buff, size_buff, time / 1000000);
	}
}
