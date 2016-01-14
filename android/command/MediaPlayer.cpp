/*
 * File:			MediaPlayer.cpp
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2016-01-14 12:01:25
 *
 * Copyright (c) 2016 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <android++/MediaPlayer.h>
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>

using namespace android;

int main(int argc, char *argv[])
{
	const char *pathname;

	if (argc > 1) {
		pathname = argv[1];
	} else {
		pathname = "/data/test.mp4";
	}

	pd_func_info("pathname = %s", pathname);

	sp<ProcessState> proc(ProcessState::self());
	ProcessState::self()->startThreadPool();

	sp<CavanPlayer> player = new CavanPlayer(pathname, strcmp(argv[0], "bootanimation") == 0);
	IPCThreadState::self()->joinThreadPool();

	return 0;
}
