#pragma once

/*
 * File:		window.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2013-07-21 20:12:00
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

class Rect
{
protected:
	int x;
	int y;
	int width;
	int height;

public:
	int getX(void)
	{
		return x;
	}

	void setX(int x)
	{
		this->x = x;
	}

	int getY(void)
	{
		return y;
	}

	void setY(int y)
	{
		this->y = y;
	}

	void setXY(int x, int y)
	{
		this->x = x;
		this->y = y;
	}

	int getWidth(void)
	{
		return width;
	}

	void setWidth(int width)
	{
		this->width = width;
	}

	int getHeight(void)
	{
		return height;
	}

	void setHeight(int height)
	{
		this->height = height;
	}
};

class Window : public Rect
{
protected:
	int id;
	const char *text;

public:
	Window(int id, const char *text) : id(id), text(text) {};

	int getId(void)
	{
		return id;
	}

	void setId(int id)
	{
		this->id = id;
	}

	virtual const char *getText(void)
	{
		return text;
	}

	virtual void setText(const char *text)
	{
		this->text = text;
	}

	virtual void Paint(void) = 0;
};
