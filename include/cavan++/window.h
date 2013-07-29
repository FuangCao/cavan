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
#include <cavan++/link.h>

class CavanRect
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

class CavanWindow : public DoubleLinkNode, CavanRect
{
protected:
	int id;
	const char *text;
	MutexLock mLock;
	CavanWindow *mParent;
	DoubleLoopLink mChildLink;

	void (*onPaint)(CavanWindow *win);
	void (*onClick)(CavanWindow *win, int x, int y, bool pressed);
	void (*onMove)(CavanWindow *win, int x, int y);
	void (*onEnter)(CavanWindow *win, int x, int y);
	void (*onExit)(CavanWindow *win, int x, int y);
	void (*onKey)(CavanWindow *win, int code, int value, const char *name);

public:
	CavanWindow(int id, const char *text) : mLock(), mChildLink()
	{
		this->id = id;
		this->text = text;
	}

	void PaintAll(void);

	void setOnPaint(void (*handler)(CavanWindow *win))
	{
		AutoLock lock(mLock);

		onPaint = handler;
	}

	int getId(void)
	{
		AutoLock lock(mLock);

		return id;
	}

	void setId(int id)
	{
		AutoLock lock(mLock);

		this->id = id;
	}

	void setParent(CavanWindow *parent)
	{
		AutoLock lock(mLock);

		mParent = parent;
	}

	void addChild(CavanWindow *win)
	{
		AutoLock lock(mLock);

		win->setParent(this);
		mChildLink.push(win);
	}

	virtual void Paint(void);

	virtual const char *getText(void)
	{
		AutoLock lock(mLock);

		return text;
	}

	virtual void setText(const char *text)
	{
		AutoLock lock(mLock);

		this->text = text;
	}
};
