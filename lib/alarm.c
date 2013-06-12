/*
 * File:		alarm.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2013-06-09 09:51:29
 *
 * Copyright (c) 2013 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <time.h>
#include <cavan/alarm.h>

static struct cavan_alarm_thread *global_alarm_thread;

void cavan_show_date(struct tm *date, const char *prompt)
{
	if (prompt == NULL)
	{
		prompt = "";
	}

	pr_bold_info("%s%04d-%02d-%02d %02d:%02d:%02d", prompt,
		date->tm_year + 1900, date->tm_mon + 1, date->tm_mday,
		date->tm_hour, date->tm_min, date->tm_sec);

	// pr_bold_info("tm_wday = %d, tm_yday = %d, tm_isdst = %d", date->tm_wday, date->tm_yday, date->tm_isdst);
}

void cavan_show_date2(const time_t *time, const char *prompt)
{
	struct tm date;

	localtime_r(time, &date);
	cavan_show_date(&date, prompt);
}

int cavan_date_cmp(struct tm *d1, struct tm *d2)
{
	if (d1->tm_year > d2->tm_year)
	{
		return 1;
	}

	if (d1->tm_year < d2->tm_year)
	{
		return -1;
	}

	if (d1->tm_mon > d2->tm_mon)
	{
		return 1;
	}

	if (d1->tm_mon < d2->tm_mon)
	{
		return -1;
	}

	if (d1->tm_mday > d2->tm_mday)
	{
		return 1;
	}

	if (d1->tm_mday < d2->tm_mday)
	{
		return -1;
	}

	if (d1->tm_hour > d2->tm_hour)
	{
		return 1;
	}

	if (d1->tm_hour < d2->tm_hour)
	{
		return -1;
	}

	if (d1->tm_min > d2->tm_min)
	{
		return 1;
	}

	if (d1->tm_min < d2->tm_min)
	{
		return -1;
	}

	if (d1->tm_sec > d2->tm_sec)
	{
		return 1;
	}

	if (d1->tm_sec < d2->tm_sec)
	{
		return -1;
	}

	return 0;
}

long cavan_date_diff(struct tm *d1, struct tm *d2)
{
	long diff = d1->tm_year - d2->tm_year;

	if (diff > 1)
	{
		diff = 1;
	}
	else if (diff < -1)
	{
		diff = -1;
	}

	diff = diff * 365 + (d1->tm_yday - d2->tm_yday);
	diff = diff * 24 + (d1->tm_hour - d2->tm_hour);
	diff = diff * 60 + (d1->tm_min - d2->tm_min);
	diff = diff * 60 + (d1->tm_sec - d2->tm_sec);

	return diff;
}

static bool cavan_alarm_insert_matcher(struct double_link *link, struct double_link_node *node, void *data)
{
	struct cavan_alarm_node *alarm_node = double_link_get_container(link, node);

	return alarm_node->time > *(time_t *)data;
}

static inline void cavan_alarm_insert_node_base(struct cavan_alarm_thread *thread, struct cavan_alarm_node *node)
{
	double_link_cond_insert_append(&thread->link, &node->node, &node->time, cavan_alarm_insert_matcher);
}

static int cavan_alarm_thread_handler(struct cavan_thread *thread, void *data)
{
	struct cavan_alarm_thread *alarm_thread = data;
	struct double_link_node *node;

	pthread_mutex_lock(&alarm_thread->lock);

	node = double_link_get_first_node(&alarm_thread->link);
	if (node == NULL)
	{
		alarm(0);
		cavan_thread_suspend(thread);
	}
	else
	{
		struct cavan_alarm_node *alarm_node = double_link_get_container(&alarm_thread->link, node);
		time_t curr_time;

		curr_time = time(NULL);
		if (curr_time < 0)
		{
			pr_error_info("get current time");
			pthread_mutex_unlock(&thread->lock);
			return curr_time;
		}

		cavan_show_date2(&curr_time, "curr_time = ");
		cavan_show_date2(&alarm_node->time, "alarm_time = ");

		if (curr_time < alarm_node->time)
		{
			alarm(alarm_node->time - curr_time);
			cavan_thread_suspend(thread);
		}
		else
		{
			double_link_remove(&alarm_thread->link, node);
			alarm_node->handler(alarm_node, alarm_thread, alarm_node->private_data);

			if (alarm_node->repeat)
			{
				alarm_node->time += alarm_node->repeat;
				cavan_alarm_insert_node_base(alarm_thread, alarm_node);
			}
			else if (alarm_node->destroy)
			{
				alarm_node->destroy(alarm_node, alarm_node->private_data);
			}
		}
	}

	pthread_mutex_unlock(&alarm_thread->lock);

	return 0;
}

static void cavan_alarm_sighandler(int signum)
{
	if (signum == SIGALRM && global_alarm_thread)
	{
		cavan_thread_resume(&global_alarm_thread->thread);
	}
}

int cavan_alarm_thread_init(struct cavan_alarm_thread *alarm_thread)
{
	int ret;
	struct cavan_thread *thread;

	ret = pthread_mutex_init(&alarm_thread->lock, NULL);
	if (ret < 0)
	{
		pr_error_info("pthread_mutex_init");
		return ret;
	}

	ret = double_link_init(&alarm_thread->link, MOFS(struct cavan_alarm_node, node));
	if (ret < 0)
	{
		pr_red_info("double_link_init");
		goto out_pthread_mutex_destroy;
	}

	thread = &alarm_thread->thread;
	thread->name = "ALARM";
	thread->wake_handker = NULL;
	thread->handler = cavan_alarm_thread_handler;

	ret = cavan_thread_init(thread, alarm_thread);
	if (ret < 0)
	{
		pr_red_info("cavan_thread_init");
		goto out_double_link_deinit;
	}

	signal(SIGALRM, cavan_alarm_sighandler);

	return 0;

out_pthread_mutex_destroy:
	pthread_mutex_destroy(&thread->lock);
out_double_link_deinit:
	double_link_deinit(&alarm_thread->link);
	return ret;
}

void cavan_alarm_thread_deinit(struct cavan_alarm_thread *thread)
{
	cavan_thread_deinit(&thread->thread);
	double_link_deinit(&thread->link);
	pthread_mutex_destroy(&thread->lock);
}

int cavan_alarm_thread_start(struct cavan_alarm_thread *thread)
{
	global_alarm_thread = thread;

	return cavan_thread_start(&thread->thread);
}

void cavan_alarm_thread_stop(struct cavan_alarm_thread *thread)
{
	cavan_thread_stop(&thread->thread);
}

int cavan_alarm_insert_node(struct cavan_alarm_thread *thread, struct cavan_alarm_node *node, struct tm *date)
{
	if (node->handler == NULL)
	{
		pr_red_info("node->handler == NULL");
		return -EINVAL;
	}

	if (date)
	{
		time_t curr_time = time(NULL);

		node->time = mktime(date);
		if (node->time < curr_time)
		{
			node->time += TIME_DAY(1);
			if (node->time < curr_time)
			{
				pr_red_info("Date too old");
				return -EINVAL;
			}
		}
	}

	cavan_show_date2(&node->time, "date = ");

	pthread_mutex_lock(&thread->lock);

	double_link_remove(&thread->link, &node->node);
	cavan_alarm_insert_node_base(thread, node);
	cavan_thread_resume(&thread->thread);

	pthread_mutex_unlock(&thread->lock);

	return 0;
}

int cavan_alarm_insert_node2(struct cavan_alarm_thread *thread, struct cavan_alarm_node *node, int year, int mon, int day, int hour, int min, int sec)
{
	struct tm time =
	{
		.tm_sec = sec,
		.tm_min = min,
		.tm_hour = hour,
		.tm_mday = day,
		.tm_mon = mon - 1,
		.tm_year = year - 1900,
		.tm_wday = 0,
		.tm_yday = 0,
		.tm_isdst = 0
	};

	return cavan_alarm_insert_node(thread, node, &time);
}

void cavan_alarm_delete_node(struct cavan_alarm_thread *thread, struct cavan_alarm_node *node)
{
	pthread_mutex_lock(&thread->lock);

	double_link_remove(&thread->link, &node->node);

	pthread_mutex_unlock(&thread->lock);
}
