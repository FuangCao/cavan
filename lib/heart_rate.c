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

static void heart_rate_decode_init_time(struct heart_rate_decode *decode, u32 time)
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

static bool heart_rate_decode_update_time(struct heart_rate_decode *decode, u32 time)
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

static void heart_rate_decode_init_value(struct heart_rate_decode *decode)
{
	decode->p_avg = decode->r;
	decode->v_avg = decode->s;
	decode->r_time = 0;
	decode->s_time = 0;
}

static bool heart_rate_decode_update_value_r(struct heart_rate_decode *decode, u16 value)
{
	u16 diff = decode->r - decode->p_avg;

	if (value > decode->p_avg + ((diff * 3) >> 2))
	{
		decode->r = ((u32) decode->r + value) >> 1;
		return true;
	}
	else
	{
		decode->p_avg = ((u32) decode->p_avg * 3 + value) >> 2;
		return false;
	}
}

static bool heart_rate_decode_update_value_s(struct heart_rate_decode *decode, u16 value)
{
	u16 diff = decode->v_avg - decode->s;

	if (value < decode->v_avg - ((diff * 3) >> 2))
	{
		decode->s = ((u32) decode->s + value) >> 1;
		return true;
	}
	else
	{
		decode->v_avg = ((u32) decode->v_avg * 3 + value) >> 2;
		return false;
	}
}

static void heart_rate_decode_update_rate(struct heart_rate_decode *decode, u32 rate)
{
	u32 *p, *q;

	for (p = decode->rate_list, q = p + decode->rate_count; p < q && *p > rate; p++);

	while (q-- > p)
	{
		q[1] = q[0];
	}

	*p = rate;

	if (++decode->rate_count >= NELEM(decode->rate_list))
	{
		rate = decode->rate_list[NELEM(decode->rate_list) >> 1];
		if (decode->rate_avg > 0)
		{
			decode->rate_avg = (decode->rate_avg * 3 + rate) >> 2;
		}
		else
		{
			decode->rate_avg = rate;
		}

		decode->handler(decode, HEART_RATE_DECODE_MINUTE / decode->rate_avg);
		decode->rate_count = 0;
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
		decode->rate_avg = 0;
		decode->rate_count = 0;

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
					heart_rate_decode_init_time(decode, decode->p_time - decode->v_time);
				}

				decode->state = HEART_RATE_DECODE_STATE_INIT_V;
			}
			else
			{
				heart_rate_decode_init_value(decode);
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
					heart_rate_decode_init_time(decode, decode->v_time - decode->p_time);
				}

				decode->state = HEART_RATE_DECODE_STATE_INIT_P;
			}
			else
			{
				heart_rate_decode_init_value(decode);
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
			if (value < decode->v_avg)
			{
				break;
			}

			decode->p_time = decode->count;
			if (heart_rate_decode_update_time(decode, decode->p_time - decode->v_time)
				| heart_rate_decode_update_value_r(decode, decode->p))
			{
				if (decode->r_time > 0)
				{
					u32 rate = decode->count - decode->r_time;

					if (rate > HEART_RATE_DECODE_RATE_MIN)
					{
						decode->r_time = decode->count;
						if (rate < HEART_RATE_DECODE_RATE_MAX)
						{
							heart_rate_decode_update_rate(decode, rate);
						}
					}
				}
				else
				{
					decode->r_time = decode->count;
				}
			}

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
			if (value > decode->p_avg)
			{
				break;
			}

			decode->v_time = decode->count;

			if (heart_rate_decode_update_time(decode, decode->v_time - decode->p_time)
				| heart_rate_decode_update_value_s(decode, decode->v))
			{
				if (decode->s_time > 0)
				{
					u32 rate = decode->count - decode->s_time;

					if (rate > HEART_RATE_DECODE_RATE_MIN)
					{
						decode->s_time = decode->count;
						if (rate < HEART_RATE_DECODE_RATE_MAX)
						{
							heart_rate_decode_update_rate(decode, rate);
						}
					}
				}
				else
				{
					decode->s_time = decode->count;
				}
			}

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
