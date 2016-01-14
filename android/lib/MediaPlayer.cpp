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
#include <cavan/android.h>
#include <android++/MediaPlayer.h>
#include <ui/PixelFormat.h>
#include <ui/Rect.h>
#include <ui/Region.h>
#include <ui/DisplayInfo.h>
#include <binder/IPCThreadState.h>

namespace android {

CavanPlayer::CavanPlayer(const char *pathname, bool bootanimation) : MediaPlayer(), Thread(false)
{
	pd_pos_info();

	mFd = -1;
	mPathName = pathname;
	mBootAnimation = bootanimation;
    mSession = new SurfaceComposerClient();
}

CavanPlayer::~CavanPlayer(void)
{
	pd_pos_info();

	if (mFd >= 0) {
		close(mFd);
		mFd = -1;
	}
}

void CavanPlayer::onFirstRef(void)
{
	pd_pos_info();

	if (mPathName == NULL) {
		pd_red_info("Nothing to be done");
		return;
	}

	pd_info("mPathName = %s", mPathName);

    status_t err = mSession->linkToComposerDeath(this);
	if (err != NO_ERROR) {
		pd_red_info("linkToComposerDeath");
		return;
	}

	run("CavanPlayer", PRIORITY_DISPLAY);
}

void CavanPlayer::binderDied(const wp<IBinder> &binder)
{
	pd_pos_info();

	mShouldStop = true;
}

status_t CavanPlayer::readyToRun(void)
{
	status_t status;
	DisplayInfo dinfo;
	sp<IBinder> dtoken;

	pd_pos_info();

	mFd = open(mPathName, O_RDONLY);
	if (mFd < 0) {
		pd_err_info("open %s", mPathName);
		return NAME_NOT_FOUND;
	}

	mSize = lseek(mFd, 0, SEEK_END);
	if (mSize < 0) {
		pd_err_info("lseek %s", mPathName);
		goto out_close_fd;
	}

	if (lseek(mFd, 0, SEEK_SET) != 0) {
		pd_err_info("lseek %s", mPathName);
		goto out_close_fd;
	}

	dtoken = SurfaceComposerClient::getBuiltInDisplay(ISurfaceComposer::eDisplayIdMain);
	if (dtoken == 0) {
		pd_red_info("SurfaceComposerClient::getBuiltInDisplay");
		goto out_close_fd;
	}

	status = SurfaceComposerClient::getDisplayInfo(dtoken, &dinfo);
	if (status != NO_ERROR) {
		pd_red_info("SurfaceComposerClient::getDisplayInfo");
		goto out_close_fd;
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

	mDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	eglInitialize(mDisplay, 0, 0);

	return NO_ERROR;

out_close_fd:
	close(mFd);
	return FDS_NOT_ALLOWED;
}

bool CavanPlayer::threadLoop(void)
{
	status_t status;

	pd_pos_info();

	status = reset();
	if (status != NO_ERROR) {
		pd_red_info("reset");
		goto out_surface_clean;
	}

	status = setAudioStreamType(AUDIO_STREAM_MUSIC);
	if (status != NO_ERROR) {
		pd_red_info("setAudioStreamType");
		goto out_surface_clean;
	}

	status = setDataSource(mFd, 0, mSize);
	if (status != NO_ERROR) {
		pd_red_info("setDataSource");
		goto out_surface_clean;
	}

	status = setVideoSurfaceTexture(mFlingerSurface->getIGraphicBufferProducer());
	if (status != NO_ERROR) {
		pd_red_info("setVideoSurfaceTexture");
		goto out_surface_clean;
	}

	status = prepare();
	if (status != NO_ERROR) {
		pd_red_info("prepare");
		goto out_surface_clean;
	}

	status = start();
	if (status != NO_ERROR) {
		pd_red_info("start");
		goto out_surface_clean;
	}

	mShouldStop = false;

	while (1) {
		pd_pos_info();

		if (mShouldStop) {
			break;
		}

		if (mBootAnimation && android_getprop_int("service.bootanim.exit", 0)) {
			break;
		}

		msleep(200);
	}

	pd_pos_info();

	stop();

out_surface_clean:
    mFlingerSurface.clear();
    mFlingerSurfaceControl.clear();
    IPCThreadState::self()->stopProcess();

	return false;
}

}
