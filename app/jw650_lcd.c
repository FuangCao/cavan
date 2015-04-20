/*
 * File:		test.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-02-11 16:22:25
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

static int get_char_value(char c, const char *weight)
{
	const char *p;

	if (c == '-')
	{
		c = 'G';
	}

	for (p = weight; *p; p++)
	{
		if (c == *p)
		{
			return p - weight;
		}
	}

	return -1;
}

int main(int argc, char *argv[])
{
	int i, j;
	const char *weights[] = { "CGB-DEFA", "AFEDBGC-" };
							// 0         1     2        3        4       5        6         7      8          9         A         B        C       D         E        F       G         H        I     J      K       L      M    N        O         P        Q        R         S        T       U        V    W    X    Y       Z    -    =    ' '   
	const char *encoded[] = { "ABCDEF", "BC", "ABGED", "ABGCD", "FGBC", "AFGCD", "AFEDCG", "ABC", "ABCDEFG", "ABCDFG", "EFABCG", "FEGCD", "AFED", "ABCDEF", "AFEDG", "AFEG", "AFEDGC", "FEGBC", "FE", "BCD", "FEGD", "FED", "-", "EFABC", "ABCDEF", "FEABG", "ABCFG", "ABCFEG", "AFGCD", "FEDG", "BCDEF", "-", "-", "-", "BCFG", "-", "G", "GD", ""};

	print("// ");

	for (j = 0; ; j++)
	{
		if (j < 10)
		{
			print("%02d'%d' ", j, j);
		}
		else if (j < 10 + 26)
		{
			print("%02d'%c' ", j, j - 10 + 'A');
		}
		else
		{
			char c;

			switch (j)
			{
			case 36:
				c = '-';
				break;

			case 37:
				c = '=';
				break;

			case 38:
				c = ' ';
				break;

			default:
				c = '-';
				break;
			}

			if (j < NELEM(encoded) - 1)
			{
				print("%02d'%c' ", j, c);
			}
			else
			{
				print("%02d'%c'", j, c);
				break;
			}
		}
	}

	for (i = 0; i < NELEM(weights); i++)
	{
		const char *weight = weights[i];

		print("\n{  ");

		for (j = 0; ; j++)
		{
			const char *p;
			int value = 0;

			for (p = encoded[j]; *p; p++)
			{
				value |= 1 << get_char_value(*p, weight);
			}

			if (j < NELEM(encoded) - 1)
			{
				print("0x%02x, ", value);
			}
			else
			{
				print("0x%02x", value);
				break;
			}
		}

		print(" },");
	}

	return 0;
}
