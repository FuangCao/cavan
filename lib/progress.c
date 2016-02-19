#include <cavan.h>
#include <cavan/time.h>
#include <cavan/timer.h>
#include <cavan/command.h>
#include <cavan/progress.h>

static struct progress_bar *bar_curr;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

s64 progress_bar_get_time_consume_ns(struct progress_bar *bar)
{
	struct timespec time_now;

	clock_gettime_mono(&time_now);

	return cavan_timespec_sub_ns(&time_now, &bar->time_start);
}

static void progress_bar_show(struct progress_bar *bar, char *buff, char *curr, char *buff_end)
{
	int length;

	length = curr - buff;
	if (length < bar->content_length) {
		char *last = buff + bar->content_length;

		if (last > buff_end) {
			last = buff_end;
		}

		while (curr < last) {
			*curr++ = ' ';
		}
	}

	bar->content_length = length;

	*curr++ = '\r';

	pthread_mutex_lock(&lock);
	print_ntext(buff, curr - buff);
	pthread_mutex_unlock(&lock);
}

static bool progress_bar_update_percent(struct progress_bar *bar)
{
	int percent = bar->current * 100 / bar->total;

	if (percent == bar->percent) {
		return false;
	}

	bar->percent = percent;

	return true;
}

static bool progress_bar_update_content(struct progress_bar *bar)
{
	int fill = bar->current * bar->full_length / bar->total;

	if (fill == bar->fill) {
		return false;
	}

	bar->fill = fill;

	return true;
}

static char *progress_bar_fflush_base(struct progress_bar *bar, char *p, char *p_end)
{
	*p++ = '[';

	if (bar->fill < bar->half_length) {
		memset(p, BAR_FULL_CHAR, bar->fill);
		memset(p + bar->fill, BAR_FREE_CHAR, bar->half_length - bar->fill);
	} else {
		memset(p, BAR_FULL_CHAR, bar->half_length);
	}

	p += bar->half_length;
	p += snprintf(p, p_end - p, " %d%% ", bar->percent);

	if (bar->fill > bar->half_length) {
		int progress = bar->fill - bar->half_length;

		memset(p, BAR_FULL_CHAR, progress);

		if (progress < bar->half_length) {
			memset(p + progress, BAR_FREE_CHAR, bar->half_length - progress);
		}
	} else {
		memset(p, BAR_FREE_CHAR, bar->half_length);
	}

	p += bar->half_length;
	*p++ = ']';

	return p;
}

// ================================================================================

static void progress_bar_speed_fflush(struct progress_bar *bar)
{
	char buff[1024];
	char *p = buff, *p_end = p + sizeof(buff);
#if BAR_SHOW_TIME
	u32 remain;
#endif

	p = progress_bar_fflush_base(bar, p, p_end);

	if (bar->speed >= 0) {
#if BAR_SHOW_TIME
		if (bar->speed > 0) {
			*p++ = ' ';
			remain = (bar->total - bar->current + bar->speed - 1) / bar->speed;
			p = cavan_time2text_simple2(remain, p, p_end - p);
		}
#endif

		*p++ = ' ';
		p = mem_speed_tostring(bar->speed, p, p_end - p - 2);
	}

	progress_bar_show(bar, buff, p, p_end);
}

static void progress_bar_speed_fflush2(struct progress_bar *bar)
{
	char buff[1024];
	char *p = buff, *p_end = p + sizeof(buff);
	double time = progress_bar_get_time_consume_ns(bar);

	p = cavan_time2text_simple2(NS_SEC(time), p, p_end - p);
	*p++ = ' ';
	p = mem_speed_tostring(bar->speed, p, p_end - p);
	*p++ = ' ';
	*p++ = '(';
	p = mem_size_tostring(bar->current, p, p_end - p);
	*p++ = ')';

	progress_bar_show(bar, buff, p, p_end);
}

static bool progress_bar_update_speed(struct progress_bar *bar)
{
	s64 interval;
	double speed;
	struct timespec time_now;

	clock_gettime_mono(&time_now);

	interval = cavan_timespec_sub_ns(&time_now, &bar->time_prev);
	if (interval < bar->interval) {
		return false;
	}

	if (bar->current > bar->last) {
		speed = (bar->current - bar->last) * 1000000000UL / interval;
	} else {
		speed = 0;
	}

	bar->last = bar->current;
	bar->time_prev = time_now;

	if (bar->speed == speed && bar->total > 0) {
		return false;
	}

	bar->speed = speed;

	return true;
}

static void progress_bar_speed_update_handler(struct progress_bar *bar)
{
	if (progress_bar_update_percent(bar) | progress_bar_update_content(bar) | progress_bar_update_speed(bar)) {
		progress_bar_speed_fflush(bar);
	}
}

static void progress_bar_speed_no_size_update_handler(struct progress_bar *bar)
{
	if (progress_bar_update_speed(bar)) {
		progress_bar_speed_fflush2(bar);
	}
}

// ================================================================================

static void progress_bar_time_fflush(struct progress_bar *bar)
{
	char buff[1024];
	char *p = buff, *p_end = p + sizeof(buff);

	p = progress_bar_fflush_base(bar, p, p_end);
	*p++ = ' ';
	p = cavan_time2text_simple2(bar->time, p, p_end - p);
	*p++ = ' ';
	*p++ = '/';
	*p++ = ' ';
	p = cavan_time2text_simple2(bar->time_total, p, p_end - p);

	progress_bar_show(bar, buff, p, p_end);
}

static bool progress_bar_update_time(struct progress_bar *bar)
{
	u32 time = MS_SEC(bar->current);

	if (time == bar->time) {
		return false;
	}

	bar->time = time;

	return true;
}

static void progress_bar_time_update_handler(struct progress_bar *bar)
{
	if (progress_bar_update_percent(bar) | progress_bar_update_content(bar) | progress_bar_update_time(bar)) {
		progress_bar_time_fflush(bar);
	}
}

// ================================================================================

static void progress_bar_sigint(int signum)
{
	struct progress_bar *bar = bar_curr;

	if (bar) {
		bar->normal = false;
		progress_bar_finish(bar);
	}

	exit(1);
}

void progress_bar_init(struct progress_bar *bar, double total, progress_bar_type_t type)
{
	int columns;

	bar_curr = bar;

	bar->total = total;
	bar->current = bar->last = 0;
	bar->percent = 0;
	bar->fill = 0;
	bar->content_length = 0;

	bar->interval = MS_NS(1000);

	if (tty_get_win_size2(stdout_fd, NULL, &columns) < 0 || columns <= 0) {
		bar->half_length = BAR_DEF_HALF_LEN;
	} else if (columns < (int) BAR_CONTENT_MIN) {
		bar->half_length = 0;
	} else {
		bar->half_length = (columns - BAR_CONTENT_MIN) / 2;
		if (bar->half_length > BAR_MAX_HALF_LEN) {
			bar->half_length = BAR_MAX_HALF_LEN;
		}
	}

	bar->full_length = bar->half_length * 2;
	bar->speed = -1;

	clock_gettime_mono(&bar->time_start);
	bar->time_prev = bar->time_start;

	bar->type = type;
	switch (type) {
	case PROGRESS_BAR_TYPE_TIME:
		bar->time = 0;
		bar->time_total = MS_SEC(bar->total);
		bar->update = progress_bar_time_update_handler;
		break;

	case PROGRESS_BAR_TYPE_DATA:
	default:
		if (bar->total > 0) {
			bar->update = progress_bar_speed_update_handler;
		} else {
			bar->update = progress_bar_speed_no_size_update_handler;
		}
	}

	progress_bar_update(bar);

	bar->normal = true;
	signal(SIGINT, progress_bar_sigint);
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

	bar_curr = NULL;

	if (bar->total > 0) {
		if (bar->normal) {
			bar->current = bar->total;
			progress_bar_update(bar);
		}

		print_char('\n');
	} else {
		char buff[bar->content_length + 1];

		memset(buff, ' ', sizeof(buff));
		buff[bar->content_length] = '\r';
		print_ntext(buff, sizeof(buff));
	}

	if (bar->type == PROGRESS_BAR_TYPE_DATA) {
		time = progress_bar_get_time_consume_ns(bar);
		if (time > 1000UL) {
			char size_buff[32];
			char speed_buff[32];

			mem_size_tostring(bar->current, size_buff, sizeof(size_buff));
			mem_speed_tostring(bar->current * 1000000000UL / time, speed_buff, sizeof(speed_buff));
			println("%s (%s in %lf ms)", speed_buff, size_buff, time / 1000000UL);
		}
	}
}
