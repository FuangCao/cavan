#pragma once

#include <cavan.h>

#define CAVAN_TIME_SECONDS_PER_MIN			60
#define CAVAN_TIME_SECONDS_PER_HOUR			(CAVAN_TIME_SECONDS_PER_MIN * 60)
#define CAVAN_TIME_SECONDS_PER_DAY			(CAVAN_TIME_SECONDS_PER_HOUR * 24)

#define CAVAN_TIME_DAYS_1970				719527
#define CAVAN_TIME_DAYS_2000				730484
#define CAVAN_TIME_DAYS_BUILD_BASE			CAVAN_TIME_DAYS_1970
#define CAVAN_TIME_DAYS_PARSE_BASE			CAVAN_TIME_DAYS_1970

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
unsigned long cavan_time_build(const struct cavan_time *time);
void cavan_time_parse(unsigned long timestap, struct cavan_time *time);
