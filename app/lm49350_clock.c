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

#define PLL_M_MAX		0x7F
#define PLL_N_MAX		0xFF
#define PLL_N_MODE_MAX	0x1F
#define PLL_P_MAX		0xFF
#define FREQ_DIFF_MAX	10

static void show_usage(const char *command)
{
	println("Usage: %s Freq_IN Freq_OUT", command);
	println("Usage: %s Freq_IN M N N_MODE P1 [P2]", command);
}

static double lm49350_cal_freq(double Freq_IN, int M, int N, int N_MODE, int P, bool verbose)
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

	N1 = N + ((double) N_MODE) / 32;

	Freq_OUT = Freq_IN * N1 / (M1 * P1);

	if (verbose)
	{
		char buff_in[64];
		char buff_out[64];

		println("M = 0x%02x = %d, N = 0x%02x = %d, N_MODE = 0x%02x = %d, P = 0x%02x = %d", M, M, N, N, N_MODE, N_MODE, P, P);
		println("M1 = %lf, N1 = %lf, P1 = %lf", M1, N1, P1);
		println("%s => %s", frequency_tostring(Freq_IN, buff_in, sizeof(buff_in), NULL), frequency_tostring(Freq_OUT, buff_out, sizeof(buff_out), NULL));
	}

	return Freq_OUT;
}

int main(int argc, char *argv[])
{
	double Freq_IN;
	int M, N, N_MODE, P;

	if (argc > 5)
	{
		Freq_IN = text2frequency(argv[1], NULL, NULL);
		M = text2value_unsigned(argv[2], NULL, 10);
		N = text2value_unsigned(argv[3], NULL, 10);
		N_MODE = text2value_unsigned(argv[4], NULL, 10);

		P = text2value_unsigned(argv[5], NULL, 10);
		lm49350_cal_freq(Freq_IN, M, N, N_MODE, P, true);

		if (argc > 6)
		{
			P = text2value_unsigned(argv[6], NULL, 10);
			lm49350_cal_freq(Freq_IN, M, N, N_MODE, P, true);
		}
	}
	else if (argc > 2)
	{
		char buff[64];
		int M_BEST = 0;
		int N_BEST = 0;
		int P_BEST = 0;
		int N_MODE_BEST = 0;
		double Freq_OUT;
		double diff_min = -1;

		Freq_IN = text2frequency(argv[1], NULL, NULL);
		Freq_OUT = text2frequency(argv[2], NULL, NULL);

		println("Freq_IN = %s", frequency_tostring(Freq_IN, buff, sizeof(buff), NULL));
		println("Freq_OUT = %s", frequency_tostring(Freq_OUT, buff, sizeof(buff), NULL));

		for (M = PLL_M_MAX; M >= 0; M--)
		{
			for (N = PLL_N_MAX; N >= 0; N--)
			{
				for (N_MODE = 0; N_MODE <= PLL_N_MODE_MAX; N_MODE++)
				{
					for (P = 0; P <= PLL_P_MAX; P++)
					{
						double diff;
						double freq = lm49350_cal_freq(Freq_IN, M, N, N_MODE, P, false);

						diff = freq > Freq_OUT ? freq - Freq_OUT : Freq_OUT - freq;
						if (diff < FREQ_DIFF_MAX)
						{
							println("diff = %lf, M = 0x%02x = %d, N = 0x%02x = %d, N_MODE = 0x%02x = %d, P = 0x%02x = %d", diff, M, M, N, N, N_MODE, N_MODE, P, P);
						}

						if (diff < diff_min || diff_min < 0)
						{
							diff_min = diff;

							M_BEST = M;
							N_BEST = N;
							N_MODE_BEST = N_MODE;
							P_BEST = P;
						}
					}
				}
			}
		}

		lm49350_cal_freq(Freq_IN, M_BEST, N_BEST, N_MODE_BEST, P_BEST, true);
	}
	else
	{
		show_usage(argv[0]);
	}

	return 0;
}
