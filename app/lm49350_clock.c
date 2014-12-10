/*
 * File:		lm49350_clock.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2014-12-09 17:39:41
 *
 * Copyright (c) 2014 Fuang.Cao <cavan.cfa@gmail.com>
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

#define PLL_M_MIN		0x00
#define PLL_M_MAX		0x7F

#define PLL_N_MIN		10
#define PLL_N_MAX		250

#define PLL_N_MOD_MIN	0x00
#define PLL_N_MOD_MAX	0x1F

#define PLL_P_MIN		0x00

#if 0
#define PLL_P_MAX		0x1FF
#else
#define PLL_P_MAX		0xFF
#endif

#define FREQ_DIFF_MAX	10

static u32 lm49350_fps_list[] = { 8000, 11025, 16000, 22050, 32000, 44100, 48000, 64000, 88200, 96000, 176400, 192000 };
static double lm49350_osr_list[] = { 32, 64, 125, 128 };

static void show_usage(const char *command)
{
	println("Usage: %s Freq_IN", command);
	println("Usage: %s Freq_IN Freq_OUT", command);
	println("Usage: %s Freq_IN FPS OSR", command);
	println("Usage: %s Freq_IN M N N_MOD P1 [P2]", command);
}

static double lm49350_cal_freq(double Freq_IN, int M, int N, int N_MOD, int P, bool verbose)
{
	double Freq_OUT;
	double M1, N1, P1;

	M1 = ((double) M + 1) / 2;
	P1 = ((double) P + 1) / 2;

	if (N < 10)
	{
		N = 10;
	}
	else if (N > 250)
	{
		N = 250;
	}

	N1 = N + ((double) N_MOD) / 32;

	Freq_OUT = Freq_IN * N1 / (M1 * P1);

	if (verbose)
	{
		char buff_in[64];
		char buff_out[64];

		println("M = 0x%02x, N = 0x%02x, N_MOD = 0x%02x, P = 0x%02x", M, N, N_MOD, P);
		println("M = %d, N = %d, N_MOD = %d, P = %d", M, N, N_MOD, P);
		println("M1 = %lf, N1 = %lf, P1 = %lf", M1, N1, P1);
		println("%s => %s", frequency_tostring(Freq_IN, buff_in, sizeof(buff_in), NULL), frequency_tostring(Freq_OUT, buff_out, sizeof(buff_out), NULL));
	}

	return Freq_OUT;
}

static int lm49350_find_pll_config(double Freq_IN, double Freq_OUT, int *M_BEST, int *N_BEST, int *N_MOD_BEST, int *P_BEST, bool verbose)
{
	int M, N, N_MOD, P;
	double diff_min = -1;

	for (M = PLL_M_MIN; M <= PLL_M_MAX; M++)
	{
		for (N = PLL_N_MIN; N <= PLL_M_MAX; N++)
		{
			for (N_MOD = PLL_N_MOD_MIN; N_MOD <= PLL_N_MOD_MAX; N_MOD++)
			{
				for (P = PLL_P_MIN; P <= PLL_P_MAX; P++)
				{
					double diff;
					double freq = lm49350_cal_freq(Freq_IN, M, N, N_MOD, P, false);

					diff = freq > Freq_OUT ? freq - Freq_OUT : Freq_OUT - freq;

					if (diff < diff_min || diff_min < 0)
					{
						diff_min = diff;

						*M_BEST = M;
						*N_BEST = N;
						*N_MOD_BEST = N_MOD;
						*P_BEST = P;
					}

					if (verbose && diff == diff_min)
					{
						println("M = 0x%02x, N = 0x%02x, N_MOD = 0x%02x, P = 0x%02x, diff = %lf", M, N, N_MOD, P, diff);
					}
				}
			}
		}
	}

	if (diff_min != 0)
	{
		return -EFAULT;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	double Freq_IN;
	int M, N, N_MOD, P;

	if (argc > 5)
	{
		Freq_IN = text2frequency(argv[1], NULL, NULL);
		M = text2value_unsigned(argv[2], NULL, 10);
		N = text2value_unsigned(argv[3], NULL, 10);
		N_MOD = text2value_unsigned(argv[4], NULL, 10);

		P = text2value_unsigned(argv[5], NULL, 10);
		lm49350_cal_freq(Freq_IN, M, N, N_MOD, P, true);

		if (argc > 6)
		{
			P = text2value_unsigned(argv[6], NULL, 10);
			lm49350_cal_freq(Freq_IN, M, N, N_MOD, P, true);
		}
	}
	else if (argc > 2)
	{
		int ret;
		char buff[64];
		double Freq_OUT;
		int M_BEST, N_BEST, N_MOD_BEST, P_BEST;

		Freq_IN = text2frequency(argv[1], NULL, NULL);
		Freq_OUT = text2frequency(argv[2], NULL, NULL);

		if (argc > 3)
		{
			int osr = text2value_unsigned(argv[3], NULL, 10);

			println("FPS = %s, OSR = %d", frequency_tostring(Freq_OUT, buff, sizeof(buff), NULL), osr);

			Freq_OUT *= osr * 2;
		}

		println("Freq_IN = %s", frequency_tostring(Freq_IN, buff, sizeof(buff), NULL));
		println("Freq_OUT = %s", frequency_tostring(Freq_OUT, buff, sizeof(buff), NULL));

		ret = lm49350_find_pll_config(Freq_IN, Freq_OUT, &M_BEST, &N_BEST, &N_MOD_BEST, &P_BEST, true);
		if (ret < 0)
		{
			pr_red_info("Not found!");
			return ret;
		}

		lm49350_cal_freq(Freq_IN, M_BEST, N_BEST, N_MOD_BEST, P_BEST, true);
	}
	else if (argc > 1)
	{
		int i, j;
		char buff[64];

		Freq_IN = text2frequency(argv[1], NULL, NULL);
		println("Freq_IN = %s", frequency_tostring(Freq_IN, buff, sizeof(buff), NULL));

		for (i = 0; i < NELEM(lm49350_osr_list); i++)
		{
			int osr = lm49350_osr_list[i];

			println("OSR = %d", osr);

			for (j = 0; j < NELEM(lm49350_fps_list); j++)
			{
				int ret;
				u32 fps = lm49350_fps_list[j];
				double Freq_OUT = fps * osr * 2;
				int M_BEST, N_BEST, N_MOD_BEST, P_BEST;

				ret = lm49350_find_pll_config(Freq_IN, Freq_OUT, &M_BEST, &N_BEST, &N_MOD_BEST, &P_BEST, false);
				if (ret < 0)
				{
					pr_red_info("fps = %d not found!", fps);
				}
				else
				{
					println("{ %d, 0x%02x, 0x%02x, 0x%02x, 0x%02x }, // %s", fps, M_BEST, N_BEST, N_MOD_BEST, P_BEST, frequency_tostring(Freq_OUT, buff, sizeof(buff), NULL));
				}
			}
		}
	}
	else
	{
		show_usage(argv[0]);
	}

	return 0;
}
