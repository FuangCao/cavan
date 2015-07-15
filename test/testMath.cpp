/*
 * File:		testMath.cpp
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-07-15 11:33:46
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
#include <cavan++/Math.h>

int main(int argc, char *argv[])
{
	Calculator calculator;

	assert(argc > 1);

	double result;
	if (!calculator.execute(argv[1], result))
	{
		pr_red_info("Failed to calculator.execute: %s", calculator.getErrMsg());
	}
	else
	{
		println("result = %lf", result);
	}

	return 0;
}
