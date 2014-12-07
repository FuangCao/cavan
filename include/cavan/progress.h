#pragma once

#include <cavan.h>

#define BAR_CONTENT_MIN		23 // sizeof("[ 100% ] 1024.00 Byte/s")
#define BAR_DEF_HALF_LEN	30
#define BAR_MAX_HALF_LEN	50
#define BAR_FREE_CHAR		'='
#define BAR_FULL_CHAR		'H'

struct progress_bar
{
	struct timespec time_prev;
	struct timespec time_start;

	double last;
	double speed;
	double total;
	double current;

	int fill;
	int half_length;
	int full_length;
	int content_length;

	int percent;
};

s64 progress_bar_get_time_consume_ns(struct progress_bar *bar);
void progress_bar_update(struct progress_bar *bar);
void progress_bar_init(struct progress_bar *bar, double total);
void progress_bar_add(struct progress_bar *bar, double val);
void progress_bar_set(struct progress_bar *bar, double val);
void progress_bar_finish(struct progress_bar *bar);
