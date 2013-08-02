/*
 * File:		Display.cpp
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2013-07-23 14:51:40
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
#include <cavan++/display.h>

void DisplayDevice::drawLineVertical(int y1, int y2, int x)
{
	if (y1 < y2)
	{
		while (y1 <= y2)
		{
			drawPoint(x, y1++);
		}
	}
	else
	{
		while (y1 >= y2)
		{
			drawPoint(x, y1--);
		}
	}
}

void DisplayDevice::drawLineVertical(int x1, int y1, int x2, int y2)
{
	LineEquation equation(y1, x1, y2, x2);

	if (y1 < y2)
	{
		while (y1 <= y2)
		{
			drawPoint(equation.getY(y1), y1);
			y1++;
		}
	}
	else
	{
		while (y1 >= y2)
		{
			drawPoint(equation.getY(y1), y1);
			y1--;
		}
	}
}

void DisplayDevice::drawLineHorizon(int x1, int x2, int y)
{
	if (x1 < x2)
	{
		while (x1 <= x2)
		{
			drawPoint(x1++, y);
		}
	}
	else
	{
		while (x1 >= x2)
		{
			drawPoint(x1--, y);
		}
	}
}

void DisplayDevice::drawLineHorizon(int x1, int y1, int x2, int y2)
{
	LineEquation equaltion(x1, y1, x2, y2);

	if (x1 < x2)
	{
		while (x1 <= x2)
		{
			drawPoint(x1, equaltion.getY(x1));
			x1++;
		}
	}
	else
	{
		while (x1 >= x2)
		{
			drawPoint(x1, equaltion.getY(x1));
			x1--;
		}
	}
}

void DisplayDevice::drawLine(int x1, int y1, int x2, int y2)
{
	int xdiff = VALUE_DIFF(x1, x2);
	int ydiff = VALUE_DIFF(y1, y2);

	if (xdiff > ydiff)
	{
		return ydiff < 5 ? drawLineHorizon(x1, x2, (y1 + y2) / 2) : drawLineHorizon(x1, y1, x2, y2);
	}
	else
	{
		return xdiff < 5 ? drawLineVertical(y1, y2, (x1 + x2) / 2) : drawLineVertical(x1, y1, x2, y2);
	}
}

void DisplayDevice::drawRect(int x, int y, int width, int height)
{
	int right = x + width;
	int bottom = y + height;

	if (x < 0)
	{
		x = 0;
	}

	if (y < 0)
	{
		y = 0;
	}

	if (right >= width)
	{
		right = width - 1;
	}

	if (bottom >= height)
	{
		bottom = height - 1;
	}

	drawLineHorizon(x, right, y);
	drawLineHorizon(x, right, bottom);

	drawLineVertical(y + 1, bottom - 1, x);
	drawLineVertical(y + 1, bottom - 1, right);
}

void DisplayDevice::fillRect(int x, int y, int width, int height)
{
}
