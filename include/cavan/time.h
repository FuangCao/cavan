#pragma once

#include <cavan.h>

#define CAVAN_TIME_SECONDS_PER_MIN			60UL
#define CAVAN_TIME_SECONDS_PER_HOUR			(60 * CAVAN_TIME_SECONDS_PER_MIN)
#define CAVAN_TIME_SECONDS_PER_DAY			(24 * CAVAN_TIME_SECONDS_PER_HOUR)

#define CAVAN_TIME_BASE_DAYS_1970			719527UL
#define CAVAN_TIME_BASE_DAYS_2000			730484UL

struct cavan_time
{
	u32 year;
	u8 month;	// 0 ~ 11
	u8 hour;	// 0 ~ 23
	u8 minute;	// 0 ~ 59
	u8 second;	// 0 ~ 59
	u8 mday;	// 1 ~ 31
	u8 yday;	// 0 ~ 365
	u8 wday;	// 0 ~ 6
};

bool cavan_time_year_is_leap(u32 year);
u32 cavan_time_get_days_of_year(u32 year, u8 month, u8 day);
u32 cavan_time_get_seconds_of_day(u8 hour, u8 min, u8 sec);
unsigned long cavan_time_build(const struct cavan_time *time, u32 base_days);
void cavan_time_parse(unsigned long timestamp, struct cavan_time *time, u32 base_days);

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
