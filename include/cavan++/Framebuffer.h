#pragma once

/*
 * File:		Framebuffer.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2013-07-30 09:52:25
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
#include <cavan++/Display.h>

class Framebuffer : public DisplayDevice
{
private:
	int mFd;
	void *mMem;

public:
	Framebuffer(void) {}
	~Framebuffer(void) {}

	bool open(void);
	void close(void);

	virtual void drawPoint(int x, int y);
	virtual void refresh(void);
};
