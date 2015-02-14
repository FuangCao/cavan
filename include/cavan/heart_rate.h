#pragma once

/*
 * File:		heart_rate.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-02-14 16:45:37
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

#define HEART_RATE_DECODE_MINUTE				60000
#define HEART_RATE_DECODE_SECOND				(HEART_RATE_DECODE_MINUTE / 60)
#define HEART_RATE_DECODE_PREPARE_COUNT			(HEART_RATE_DECODE_SECOND * 2)
#define HEART_RATE_DECODE_RATE_MIN				(HEART_RATE_DECODE_SECOND / 3)
#define HEART_RATE_DECODE_RATE_MAX				(HEART_RATE_DECODE_SECOND * 2)
#define HEART_RATE_DECODE_RATE_SAMPLE_COUNT		5

typedef enum {
	HEART_RATE_DECODE_STATE_INIT,
	HEART_RATE_DECODE_STATE_PREPARE,
	HEART_RATE_DECODE_STATE_INIT_P,
	HEART_RATE_DECODE_STATE_INIT_V,
	HEART_RATE_DECODE_STATE_FIND_P,
	HEART_RATE_DECODE_STATE_FIND_V,
} heart_rate_decode_state_t;

struct heart_rate_decode
{
	u32 count;
	heart_rate_decode_state_t state;

	void *private_data;

	u16 p, v, r, s;
	u16 p_avg, v_avg;

	u32 time_avg;
	u32 time_max;
	u32 r_time, s_time;
	u32 v_time, p_time;

	u32 rate_avg;
	int rate_count;
	u32 rate_list[HEART_RATE_DECODE_RATE_SAMPLE_COUNT];

	void (*handler)(struct heart_rate_decode *decode, int rate);
};

void heart_rate_decode_post(struct heart_rate_decode *decode, u16 value);

static inline void heart_rate_decode_init(struct heart_rate_decode *decode, void *data)
{
	decode->state = HEART_RATE_DECODE_STATE_INIT;
	decode->private_data = data;
}

static inline void heart_rate_set_data(struct heart_rate_decode *decode, void *data)
{
	decode->private_data = data;
}

static inline void *heart_rate_decode_get_data(struct heart_rate_decode *decode)
{
	return decode->private_data;
}
