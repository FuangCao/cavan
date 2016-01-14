/*
 * File:			MediaPlayer.cpp
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2016-01-14 12:31:31
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
#include <ui/PixelFormat.h>
#include <ui/Rect.h>
#include <ui/Region.h>
#include <ui/DisplayInfo.h>
#include <binder/IPCThreadState.h>

namespace android {

CavanPlayer::CavanPlayer(const char *pathname) : MediaPlayer(), Thread(false)
{
	pr_pos_info();

	mPathName = pathname;
    mSession = new SurfaceComposerClient();
}

CavanPlayer::~CavanPlayer(void)
{
	pr_pos_info();
}

void CavanPlayer::onFirstRef(void)
{
	run("CavanPlayer", PRIORITY_DISPLAY);
}

status_t CavanPlayer::readyToRun(void)
{
	pr_pos_info();

	sp<IBinder> dtoken(SurfaceComposerClient::getBuiltInDisplay(ISurfaceComposer::eDisplayIdMain));
	DisplayInfo dinfo;
	status_t status = SurfaceComposerClient::getDisplayInfo(dtoken, &dinfo);
	if (status) {
		return false;
	}

	if (dinfo.w > dinfo.h * 3) {
		mWidth = dinfo.w >> 1;
	} else {
		mWidth = dinfo.w;
	}

	mHeight = dinfo.h;

	mFlingerSurfaceControl = mSession->createSurface(String8("CavanPlayer"), mWidth, mHeight, PIXEL_FORMAT_RGB_565);

	SurfaceComposerClient::openGlobalTransaction();
	mFlingerSurfaceControl->setLayer(0x40000000);
	SurfaceComposerClient::closeGlobalTransaction();

	mFlingerSurface = mFlingerSurfaceControl->getSurface();

	return NO_ERROR;
}

bool CavanPlayer::threadLoop(void)
{
	pr_pos_info();

	mFd = open(mPathName, O_RDONLY);
	if (mFd < 0) {
		pr_err_info("open %s", mPathName);
		return false;
	}

	mSize = lseek(mFd, 0, SEEK_END);
	if (mSize < 0) {
		pr_err_info("lseek %s", mPathName);
		return false;
	}

	if (lseek(mFd, 0, SEEK_SET) != 0) {
		pr_err_info("lseek %s", mPathName);
		return false;
	}

	status_t status;

	status = setDataSource(mFd, 0, mSize);
	if (status != NO_ERROR) {
		pr_red_info("setDataSource");
		return false;
	}

	status = setVideoSurfaceTexture(mFlingerSurface->getIGraphicBufferProducer());
	if (status != NO_ERROR) {
		pr_red_info("setVideoSurfaceTexture");
		return false;
	}

	status = prepare();
	if (status != NO_ERROR) {
		pr_red_info("prepare");
		return false;
	}

	status = start();
	if (status != NO_ERROR) {
		pr_red_info("start");
		return false;
	}

	while (1) {
		pr_pos_info();
		msleep(2000);
	}

    mFlingerSurface.clear();
    mFlingerSurfaceControl.clear();
    IPCThreadState::self()->stopProcess();
}

}
