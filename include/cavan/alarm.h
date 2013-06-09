#pragma once

/*
 * File:		alarm.h
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
#include <cavan/list.h>
#include <cavan/thread.h>

struct cavan_alarm_thread
{
	struct cavan_thread thread;

	pthread_mutex_t lock;
	struct double_link link;
};

struct cavan_alarm_node
{
	struct double_link_node node;

	time_t time;
	time_t repeat;
	void *private_data;

	void (*handler)(struct cavan_alarm_node *alarm, struct cavan_alarm_thread *thread, void *data);
};

void cavan_show_date(struct tm *date);
int cavan_text2date(const char *text, struct tm *date);
int cavan_date_cmp(struct tm *d1, struct tm *d2);
long cavan_date_diff(struct tm *d1, struct tm *d2);

int cavan_alarm_thread_init(struct cavan_alarm_thread *thread);
void cavan_alarm_thread_deinit(struct cavan_alarm_thread *thread);
int cavan_alarm_thread_start(struct cavan_alarm_thread *thread);
void cavan_alarm_thread_stop(struct cavan_alarm_thread *thread);
int cavan_alarm_insert_node(struct cavan_alarm_thread *thread, struct cavan_alarm_node *node, struct tm *date);
int cavan_alarm_insert_node2(struct cavan_alarm_thread *thread, struct cavan_alarm_node *node, int year, int mon, int day, int hour, int min, int sec);
void cavan_alarm_delete_node(struct cavan_alarm_thread *thread, struct cavan_alarm_node *node);

static inline void cavan_alarm_node_init(struct cavan_alarm_node *node, void *data, void (*handler)(struct cavan_alarm_node *alarm, struct cavan_alarm_thread *thread, void *data))
{
	node->private_data = data;
	node->handler = handler;
	node->repeat = 0;
	double_link_node_init(&node->node);
}

static inline int cavan_alarm_thread_join(struct cavan_alarm_thread *thread)
{
	return cavan_thread_join(&thread->thread);
}

static inline void cavan_alarm_set_sec_repeat(struct cavan_alarm_node *node, u32 count)
{
	node->repeat += count;
}

static inline void cavan_alarm_set_min_repeat(struct cavan_alarm_node *node, u32 count)
{
	node->repeat += 60 * count;
}

static inline void cavan_alarm_set_hour_repeat(struct cavan_alarm_node *node, u32 count)
{
	cavan_alarm_set_min_repeat(node, 60 * count);
}

static inline void cavan_alarm_set_day_repeat(struct cavan_alarm_node *node, u32 count)
{
	cavan_alarm_set_hour_repeat(node, 24 * count);
}

static inline void cavan_alarm_set_week_repeat(struct cavan_alarm_node *node, u32 count)
{
	cavan_alarm_set_day_repeat(node, 7 * count);
}

static inline void cavan_alarm_set_year_repeat(struct cavan_alarm_node *node, u32 count)
{
	cavan_alarm_set_day_repeat(node, 365 * count);
}
