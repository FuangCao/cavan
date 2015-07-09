/*
 * File:		Window.cpp
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
#include <cavan++/Window.h>

static void WindowPaintAllHandler(DoubleLinkNode *node, void *data)
{
	CavanWindow *win = static_cast<CavanWindow *>(node);

	win->PaintAll();
}

void CavanWindow::PaintAll(void)
{
	AutoLock lock(mLock);

	Paint();
	mChildLink.traversal(WindowPaintAllHandler, NULL);
}

void CavanWindow::Paint()
{
	AutoLock lock(mLock);

	if (onPaint)
	{
		onPaint(this);
	}
}
