/*
 * File:		SuService.cpp
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-09-18 11:55:09
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <android++/SuService.h>

namespace android {

int SuService::system(const char *command)
{
	ALOGE("%s[%d]: command = %s", __FUNCTION__, __LINE__, command);

	if (::system(command)) {
		return -EFAULT;
	}

	return 0;
}

int SuService::popen(const char *command, int flags)
{
	int ret;

	ALOGE("%s[%d]: command = %s, lines = %d, columns = %d, flags = 0x%08x", __FUNCTION__, __LINE__, command, mLines, mColumns, flags);

	ret = cavan_exec_redirect_stdio_popen2(command, mLines, mColumns, &mPid, flags);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

};
