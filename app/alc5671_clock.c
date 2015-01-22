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

#define ALC5671_PLL_K_MAX	0x1F
#define ALC5671_PLL_N_MAX	0x1FF
#define ALC5671_PLL_M_MAX	0x0F

struct alc5671_pll_config {
	bool bypass;
	u16 M, N, K;
};

static void show_usage(const char *command)
{
	println("Usage: %s MCLK M N [K]", command);
}

static inline double alc5671_pll_cal(double MCLK, int M, int N, int K)
{
#if 0
	return (MCLK * (N + 2)) / ((M + 2) * (K + 2));
#else
	return (MCLK * (N + 2)) / (M + 2) / (K + 2);
#endif
}

static u32 alc5671_pll_find_config(u32 MCLK, u32 OUT, struct alc5671_pll_config *config)
{
	int M1, N1, K1;
	u32 diff_min = OUT;

	for (K1 = 2; K1 <= ALC5671_PLL_K_MAX + 2; K1++)
	{
		N1 = OUT * K1 / MCLK;
		if (N1 < 2)
		{
			N1 = 2;
		}

		for (; N1 <= ALC5671_PLL_N_MAX + 2; N1++)
		{
			u32 diff;
			u32 O1 = MCLK * N1 / K1;

			M1 = O1 / OUT;
			if (M1 < 1 || M1 > ALC5671_PLL_M_MAX + 2)
			{
				continue;
			}

			O1 /= M1;

			diff = O1 > OUT ? O1 - OUT : OUT - O1;
			if (diff <= diff_min)
			{
				println("M1 = %d, N1 = %d, K1 = %d, diff = %d", M1, N1, K1, diff);

				if (diff < diff_min)
				{
					config->N = N1 - 2;
					config->K = K1 - 2;
					if (M1 < 2)
					{
						config->bypass = true;
						config->M = 0;
					}
					else
					{
						config->bypass = false;
						config->M = M1 - 2;
					}

					if (diff == 0)
					{
						return 0;
					}

					diff_min = diff;
				}
			}
		}
	}

	return diff_min;
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

		OUT = alc5671_pll_cal(MCLK, M, N, K);
		println("%s => %s", frequency_tostring(MCLK, buff_in, sizeof(buff_in), NULL), frequency_tostring(OUT, buff_out, sizeof(buff_out), NULL));
	}
	else if (argc > 2)
	{
		double OUT;
		char buff_in[1024];
		char buff_out[1024];
		struct alc5671_pll_config config;

		MCLK = text2frequency(argv[1], NULL, NULL);
		OUT = text2frequency(argv[2], NULL, NULL);
		println("%s => %s", frequency_tostring(MCLK, buff_in, sizeof(buff_in), NULL), frequency_tostring(OUT, buff_out, sizeof(buff_out), NULL));

		alc5671_pll_find_config(MCLK, OUT, &config);
		println("M = %d, N = %d, K = %d, bypass = %d", config.M, config.N, config.K, config.bypass);
	}
	else
	{
		show_usage(argv[0]);
		return -EINVAL;
	}

	return 0;
}
