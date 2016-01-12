#pragma once

#include <cavan.h>

#define CAVAN_TIME_SECONDS_PER_MIN			60UL
#define CAVAN_TIME_SECONDS_PER_HOUR			(60 * CAVAN_TIME_SECONDS_PER_MIN)
#define CAVAN_TIME_SECONDS_PER_DAY			(24 * CAVAN_TIME_SECONDS_PER_HOUR)

#define CAVAN_TIME_BASE_DAYS_1970			719527UL
#define CAVAN_TIME_BASE_DAYS_2000			730484UL

#define US_NS(time)							((time) * 1000UL)
#define MS_US(time)							((time) * 1000UL)
#define MS_NS(time)							((time) * 1000000UL)
#define SEC_MS(time)						((time) * 1000UL)
#define SEC_US(time)						((time) * 1000000UL)
#define SEC_NS(time)						((time) * 1000000000UL)

#define MIN_SEC(time)						((time) * 60UL)
#define MIN_MS(time)						SEC_MS(MIN_SEC(time))
#define MIN_US(time)						SEC_US(MIN_SEC(time))
#define MIN_NS(time)						SEC_NS(MIN_SEC(time))

#define HOUR_MIN(time)						((time) * 60UL)
#define HOUR_SEC(time)						MIN_SEC(HOUR_MIN(time))
#define HOUR_MS(time)						MIN_MS(HOUR_MIN(time))
#define HOUR_US(time)						MIN_US(HOUR_MIN(time))
#define HOUR_NS(time)						MIN_NS(HOUR_MIN(time))

#define DAY_HOUR(time)						((time) * 24UL)
#define DAY_MIN(time)						HOUR_MIN(DAY_HOUR(time))
#define DAY_SEC(time)						HOUR_SEC(DAY_HOUR(time))
#define DAY_MS(time)						HOUR_MS(DAY_HOUR(time))
#define DAY_US(time)						HOUR_US(DAY_HOUR(time))
#define DAY_NS(time)						HOUR_NS(DAY_HOUR(time))

#define WEEK_DAY(time)						((time) * 7UL)
#define WEEK_HOUR(time)						DAY_HOUR(WEEK_DAY(time))
#define WEEK_MIN(time)						DAY_MIN(WEEK_DAY(time))
#define WEEK_SEC(time)						DAY_SEC(WEEK_DAY(time))
#define WEEK_MS(time)						DAY_MS(WEEK_DAY(time))
#define WEEK_US(time)						DAY_US(WEEK_DAY(time))
#define WEEK_NS(time)						DAY_NS(WEEK_DAY(time))

#define NS_US(time)							((time) / 1000UL)
#define NS_MS(time)							((time) / 1000000UL)
#define NS_SEC(time)						((time) / 1000000000UL)
#define US_MS(time)							((time) / 1000UL)
#define US_SEC(time)						((time) / 1000000UL)
#define MS_SEC(time)						((time) / 1000UL)

struct cavan_time {
	u32 year;
	u8 month;	// 0 ~ 11
	u8 hour;	// 0 ~ 23
	u8 minute;	// 0 ~ 59
	u8 second;	// 0 ~ 59
	u8 mday;	// 1 ~ 31
	u8 yday;	// 0 ~ 365
	u8 wday;	// 0 ~ 6
};

struct cavan_time_simple {
	u32 day;
	u8 hour;
	u8 minute;
	u8 second;
};

bool cavan_time_year_is_leap(u32 year);
u32 cavan_time_get_days_of_year(u32 year, u8 month, u8 day);
u32 cavan_time_get_seconds_of_day(u8 hour, u8 min, u8 sec);
unsigned long cavan_time_build(const struct cavan_time *time, u32 base_days);
void cavan_time_parse(ulong timestamp, struct cavan_time *time, u32 base_days);

void cavan_second2time_simple(ulong second, struct cavan_time_simple *time);
char *cavan_time2text_simple(struct cavan_time_simple *time, char *buff, size_t size);
char *cavan_time2text_simple2(ulong second, char *buff, size_t size);
static inline unsigned long cavan_time_build_1970(const struct cavan_time *time)
{
	return cavan_time_build(time, CAVAN_TIME_BASE_DAYS_1970);
}

static inline void cavan_time_parse_1970(unsigned long timestamp, struct cavan_time *time)
{
	cavan_time_parse(timestamp, time, CAVAN_TIME_BASE_DAYS_1970);
}

static inline unsigned long cavan_time_build_2000(const struct cavan_time *time)
{
	return cavan_time_build(time, CAVAN_TIME_BASE_DAYS_2000);
}

static inline void cavan_time_parse_2000(unsigned long timestamp, struct cavan_time *time)
{
	cavan_time_parse(timestamp, time, CAVAN_TIME_BASE_DAYS_2000);
}

static inline int64_t cavan_timeval2nano(struct timeval *time)
{
	return time->tv_sec * 1000000000LL + time->tv_usec * 1000;
}
