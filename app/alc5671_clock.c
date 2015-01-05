/*
 * File:		alc5671_clock.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-01-05 17:10:00
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

static void show_usage(const char *command)
{
	println("Usage: %s MCLK M N [K]", command);
}

static inline double alc5671_cal_pll(double MCLK, int M, int N, int K)
{
	return (MCLK * (N + 2)) / ((M + 2) * (K + 2));
}

int main(int argc, char *argv[])
{
	double MCLK, OUT;
	int M, N, K;

	if (argc > 3)
	{
		char buff_in[1024];
		char buff_out[1024];

		MCLK = text2frequency(argv[1], NULL, NULL);
		M = text2value_unsigned(argv[2], NULL, 10);
		N = text2value_unsigned(argv[3], NULL, 10);
		K = argc > 4 ? text2value_unsigned(argv[4], NULL, 10) : 2;

		OUT = alc5671_cal_pll(MCLK, M, N, K);
		println("%s => %s", frequency_tostring(MCLK, buff_in, sizeof(buff_in), NULL), frequency_tostring(OUT, buff_out, sizeof(buff_out), NULL));
	}
	else
	{
		show_usage(argv[0]);
		return -EINVAL;
	}

	return 0;
}
