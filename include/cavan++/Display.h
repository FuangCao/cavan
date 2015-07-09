#pragma once

/*
 * File:		Display.h
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

struct ColorComponent
{
	int max;
	int bits;
	int offset;
	u32 mask;

	void config(int bits, int offset)
	{
		this->bits = bits;
		this->offset = offset;
		this->max = (1 << bits) - 1;
		this->mask = this->max << offset;
	}

	u8 get(u32 color)
	{
		return (color & mask) >> offset;
	}

	void set(u32 &color, float value)
	{
		color &= ~mask;
		color |= build(value);
	}

	u32 build(float value)
	{
		return ((u32) (value * max)) << offset;
	}
};

class LineEquation
{
private:
	double a;
	double b;

public:
	LineEquation(int x1, int y1, int x2, int y2)
	{
		a = (x1 == x2) ? 1 : ((double) (y2 - y1)) / ((double) (x2 - x1));
		b = y1 - x1 * a;
	}

	int getY(int x)
	{
		return a * x + b;
	}

	int getX(int y)
	{
		return ((double) (y - b)) / a;
	}
};

class DisplayDevice
{
protected:
	int mWidth;
	int mHeight;
	u32 mColor;
	ColorComponent mRedComponent;
	ColorComponent mGreenComponent;
	ColorComponent mBlueComponent;
	ColorComponent mTranspComponent;

private:
	void drawLineVertical(int y1, int y2, int x);
	void drawLineVertical(int x1, int y1, int x2, int y2);
	void drawLineHorizon(int x1, int x2, int y);
	void drawLineHorizon(int x1, int y1, int x2, int y2);

public:
	DisplayDevice(void) {}
	~DisplayDevice(void) {}

	int getWidth(void)
	{
		return mWidth;
	}

	int getHeight(void)
	{
		return mHeight;
	}

	virtual u32 getColor(void)
	{
		return mColor;
	}

	virtual void setColor(u32 color)
	{
		mColor = color;
	}

	u32 buildColor(float red, float green, float blue, float transp)
	{
		return mRedComponent.build(red) | mGreenComponent.build(green) | mBlueComponent.build(blue) | mTranspComponent.build(transp);
	}

	u32 buildColor(float red, float green, float blue)
	{
		return buildColor(red, green, blue, 1.0);
	}

	virtual void drawPoint(int x, int y) = 0;
	virtual void refresh(void) = 0;
	virtual void drawLine(int x1, int y1, int x2, int y2);
	virtual void drawRect(int x, int y, int width, int height);
	virtual void fillRect(int x, int y, int width, int height);
};
