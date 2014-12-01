#include <cavan.h>
#include <cavan/command.h>
#include <cavan/progress.h>

#define PROGRESS_BAR_SPEED_UPDATE_INTERVAL	1000

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static void progress_bar_fflush(struct progress_bar *bar, struct speed_detector *detector)
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

	if (detector->loop_count > 0)
	{
		*p++ = ' ';
		p = mem_speed_tostring(speed_detector_get_speed(detector, 1000), p, p_end - p - 2);
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

void progress_bar_update(struct progress_bar *bar)
{
	if (bar->current > bar->total)
	{
		return;
	}

	if (update_percent(bar) | update_content(bar))
	{
		progress_bar_fflush(bar, &bar->detector);
	}
}

static void progress_bar_speed_notify(struct speed_detector *detector, u32 speed)
{
	struct progress_bar *bar = (struct progress_bar *) detector;

	if (bar->speed == speed)
	{
		return;
	}

	bar->speed = speed;
	progress_bar_fflush(bar, detector);
}

void progress_bar_init(struct progress_bar *bar, double total)
{
	u16 columns;
	struct speed_detector *detector = &bar->detector;

	bar->total = total == 0 ? 1 : total;
	bar->current = 0;
	bar->percent = 0;
	bar->fill = 0;

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
	}

	bar->full_length = bar->half_length * 2;

	bar->speed = 0;
	detector->notify = progress_bar_speed_notify;
	speed_detector_start(detector, PROGRESS_BAR_SPEED_UPDATE_INTERVAL);

	progress_bar_update(bar);
}

void progress_bar_add(struct progress_bar *bar, double val)
{
	speed_detector_post(&bar->detector, val);

	bar->current += val;
	progress_bar_update(bar);
}

void progress_bar_set(struct progress_bar *bar, double val)
{
	if (val > bar->current)
	{
		speed_detector_post(&bar->detector, val - bar->current);
	}

	bar->current = val;
	progress_bar_update(bar);
}

void progress_bar_finish(struct progress_bar *bar)
{
	double time;
	struct speed_detector *detector = &bar->detector;

	speed_detector_stop(&bar->detector);

	bar->current = bar->total;
	progress_bar_update(bar);

	print_char('\n');

	time = speed_detector_get_time_consume_ns(detector);
	if (time > 1000)
	{
		char size_buff[32];
		char speed_buff[32];

		mem_size_tostring(bar->total, size_buff, sizeof(size_buff));
		mem_speed_tostring(bar->total * 1000000000 / time, speed_buff, sizeof(speed_buff));
		println("%s (%s in %lf ms)", speed_buff, size_buff, time / 1000000);
	}
}
