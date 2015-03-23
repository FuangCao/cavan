/*
 * File:		time.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-03-23 18:47:41
 *
 * Copyright (c) 2015 Fuang.Cao <cavan.cfa@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <cavan.h>
#include <cavan/time.h>

/* How many days come before each month (0-12). */
static const u16 cavan_time_mon_yday[2][12] = {
	/* Normal years. */
	{ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 },
	/* Leap years. */
	{ 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 }
};

bool cavan_time_year_is_leap(u32 year)
{
	return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

u32 cavan_time_get_days_of_year(u32 year, u8 month, u8 day)
{
	const u16 *yday;
	u32 days = year / 4 - year / 100 + year / 400 + year * 365 + day;

	if (cavan_time_year_is_leap(year))
	{
		yday = cavan_time_mon_yday[1];
		days--;
	}
	else
	{
		yday = cavan_time_mon_yday[0];
	}

	return yday[month - 1] + days - 1;
}

u32 cavan_time_get_seconds_of_day(u8 hour, u8 min, u8 sec)
{
	return (((u32) hour) * 60 + min) * 60 + sec;
}

unsigned long cavan_time_build(const struct cavan_time *time)
{
	return ((unsigned long) cavan_time_get_days_of_year(time->year, time->month, time->day) - CAVAN_TIME_DAYS_BASE) * CAVAN_TIME_SECONDS_PER_DAY + cavan_time_get_seconds_of_day(time->hour, time->minute, time->second);
}

void cavan_time_parse(unsigned long timestap, struct cavan_time *time)
{
	u32 year;
	u32 remain;
	unsigned long days;
	const u16 *p, *yday;

	days = timestap / CAVAN_TIME_SECONDS_PER_DAY + CAVAN_TIME_DAYS_BASE;
	time->wday = days % 7;
	year = days / 366;

	while (cavan_time_get_days_of_year(year + 1, 1, 1) <= days)
	{
		year++;
	}

	yday = cavan_time_mon_yday[cavan_time_year_is_leap(year)];
	days -= cavan_time_get_days_of_year(year, 1, 1);
	for (p = yday + 11; *p > days; p--);

	time->year = year;
	time->month = p - yday + 1;
	time->day = days - *p + 1;
	time->yday = days;

	remain = timestap % CAVAN_TIME_SECONDS_PER_DAY;
	time->hour = remain / CAVAN_TIME_SECONDS_PER_HOUR;
	time->minute = remain % CAVAN_TIME_SECONDS_PER_HOUR / CAVAN_TIME_SECONDS_PER_MIN;
	time->second = timestap % CAVAN_TIME_SECONDS_PER_MIN;
}
