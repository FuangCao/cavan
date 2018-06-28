/*
 * File:		NetworkBase.cpp
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-03-22 10:47:36
 *
 * Copyright (c) 2018 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan++/NetworkBase.h>

bool NetworkBase::setBlockEnable(bool enable)
{
	int flags = fcntl(mSockfd, F_GETFL);
	if (flags == -1) {
		pr_err_info("fcntl F_GETFL");
		return false;
	}

	int newFlags;

	if (enable) {
		newFlags = flags & (~(O_NONBLOCK));
	} else {
		newFlags = flags | O_NONBLOCK;
	}

	if (flags != newFlags && fcntl(mSockfd, F_SETFL, newFlags)) {
		pr_err_info("fcntl F_SETFL");
		return false;
	}

	pd_info("setBlockEnable: %d", enable);

	return true;
}
