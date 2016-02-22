#pragma once

#include <cavan.h>

#define BAR_SHOW_TIME		1

#define BAR_DEF_HALF_LEN	30
#define BAR_MAX_HALF_LEN	50
#define BAR_FREE_CHAR		'='
#define BAR_FULL_CHAR		'H'

#if BAR_SHOW_TIME
#define BAR_CONTENT_MIN		sizeof("[ 100% ] 00:00:00 1024.00 Byte/s")
#else
#define BAR_CONTENT_MIN		sizeof("[ 100% ] 1024.00 Byte/s")
#endif

typedef enum {
	PROGRESS_BAR_TYPE_DATA,
	PROGRESS_BAR_TYPE_TIME,
	PROGRESS_BAR_TYPE_COUNT
} progress_bar_type_t;

struct progress_bar {
	struct timespec time_prev;
	struct timespec time_start;
	pthread_mutex_t lock;

	double last;
	double speed;
	double skip;
	double total;
	double current;

	bool normal;
	s64 interval;

	int fill;
	int half_length;
	int full_length;
	int content_length;

	u32 time;
	u32 time_total;

	int percent;
	progress_bar_type_t type;

	void (*update)(struct progress_bar *bar);
};

__BEGIN_DECLS

s64 progress_bar_get_time_consume_ns(struct progress_bar *bar);
void progress_bar_init(struct progress_bar *bar, double total, double skip, progress_bar_type_t type);
void progress_bar_add(struct progress_bar *bar, double val);
void progress_bar_set(struct progress_bar *bar, double val);
void progress_bar_finish(struct progress_bar *bar);

static inline void progress_bar_update(struct progress_bar *bar)
{
	bar->update(bar);
}

static inline void progress_bar_lock(struct progress_bar *bar)
{
	pthread_mutex_lock(&bar->lock);
}

static inline void progress_bar_unlock(struct progress_bar *bar)
{
	pthread_mutex_unlock(&bar->lock);
}

__END_DECLS
