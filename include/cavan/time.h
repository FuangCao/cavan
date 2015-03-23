#pragma once

#include <cavan.h>

#define CAVAN_TIME_SECONDS_PER_MIN			60
#define CAVAN_TIME_SECONDS_PER_HOUR			(CAVAN_TIME_SECONDS_PER_MIN * 60)
#define CAVAN_TIME_SECONDS_PER_DAY			(CAVAN_TIME_SECONDS_PER_HOUR * 24)

#define CAVAN_TIME_DAYS_1970				719527
#define CAVAN_TIME_DAYS_2000				730484
#define CAVAN_TIME_DAYS_BASE				CAVAN_TIME_DAYS_1970

struct cavan_time
{
	u32 year;
	u8 month;
	u8 day;
	u8 hour;
	u8 minute;
	u8 second;
	u8 yday;
	u8 wday;
};

bool cavan_time_year_is_leap(u32 year);
u32 cavan_time_get_days_of_year(u32 year, u8 month, u8 day);
u32 cavan_time_get_seconds_of_day(u8 hour, u8 min, u8 sec);
unsigned long cavan_time_build(const struct cavan_time *time);
void cavan_time_parse(unsigned long timestap, struct cavan_time *time);
