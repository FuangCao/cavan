/*
 * File:		heart_rate.c
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
#include <cavan/heart_rate.h>

static void heart_rate_decode_time_init(struct heart_rate_decode *decode, u32 time)
{
	if (decode->time_avg > 0)
	{
		decode->time_avg = (decode->time_avg + time) >> 1;

		if (time > decode->time_max)
		{
			decode->time_max = time;
		}
	}
	else
	{
		decode->time_max = decode->time_avg = time;
	}
}

static bool heart_rate_decode_time_update(struct heart_rate_decode *decode, u32 time)
{
	u32 diff = decode->time_max - decode->time_avg;

	if (time > decode->time_avg + ((diff * 3) >> 2))
	{
		decode->time_max = (decode->time_max + time) >> 1;
		return true;
	}
	else
	{
		decode->time_avg = (decode->time_avg * 3 + time) >> 2;
		return false;
	}
}

void heart_rate_decode_post(struct heart_rate_decode *decode, u16 value)
{
	decode->count++;

	switch (decode->state)
	{
	case HEART_RATE_DECODE_STATE_INIT:
		decode->r = 0x0000;
		decode->s = 0xFFFF;
		decode->count = 0;
		decode->v_time = 0;
		decode->p_time = 0;
		decode->time_avg = 0;
		decode->time_max = 0;

		decode->p = decode->v = value;
		decode->state = HEART_RATE_DECODE_STATE_PREPARE;
		break;

	case HEART_RATE_DECODE_STATE_PREPARE:
		if (value > decode->p)
		{
			decode->p = value;
			decode->state = HEART_RATE_DECODE_STATE_INIT_P;
		}
		else if (value < decode->v)
		{
			decode->v = value;
			decode->state = HEART_RATE_DECODE_STATE_INIT_V;
		}
		break;

	case HEART_RATE_DECODE_STATE_INIT_P:
		if (value < decode->p)
		{
			if (decode->p > decode->r)
			{
				decode->r = decode->p;
			}

			decode->p_time = decode->count;

			if (decode->count < HEART_RATE_DECODE_PREPARE_COUNT)
			{
				if (decode->v_time > 0)
				{
					heart_rate_decode_time_init(decode, decode->p_time - decode->v_time);
				}

				decode->state = HEART_RATE_DECODE_STATE_INIT_V;
			}
			else
			{
				decode->state = HEART_RATE_DECODE_STATE_FIND_V;
			}

			decode->v = value;
		}
		else
		{
			decode->p = value;
		}
		break;

	case HEART_RATE_DECODE_STATE_INIT_V:
		if (value > decode->v)
		{
			if (decode->v < decode->s)
			{
				decode->s = decode->v;
			}

			decode->v_time = decode->count;

			if (decode->count < HEART_RATE_DECODE_PREPARE_COUNT)
			{
				if (decode->p_time > 0)
				{
					heart_rate_decode_time_init(decode, decode->v_time - decode->p_time);
				}

				decode->state = HEART_RATE_DECODE_STATE_INIT_P;
			}
			else
			{
				decode->state = HEART_RATE_DECODE_STATE_FIND_P;
			}

			decode->p = value;
		}
		else
		{
			decode->v = value;
		}
		break;

	case HEART_RATE_DECODE_STATE_FIND_P:
		if (value < decode->p)
		{
			u32 time;

			decode->p_time = decode->count;
			time = decode->p_time - decode->v_time;
			heart_rate_decode_time_update(decode, time);

			decode->state = HEART_RATE_DECODE_STATE_FIND_V;
			decode->v = value;
		}
		else
		{
			decode->p = value;
		}
		break;

	case HEART_RATE_DECODE_STATE_FIND_V:
		if (value > decode->v)
		{
			u32 time;

			decode->v_time = decode->count;
			time = decode->v_time - decode->p_time;
			heart_rate_decode_time_update(decode, time);

			decode->state = HEART_RATE_DECODE_STATE_FIND_P;
			decode->p = value;
		}
		else
		{
			decode->v = value;
		}
		break;
	}
}
