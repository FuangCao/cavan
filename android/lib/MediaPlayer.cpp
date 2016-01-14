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

CavanVideoPlayer::CavanVideoPlayer(const char *pathname, bool bootanimation, const char *name) : MediaPlayer(), Thread(false)
{
	pd_pos_info();

	mVideoFd = -1;
	mVideoPath = pathname;
	mName = name;
	mBootAnimation = bootanimation;
    mSession = new SurfaceComposerClient();
}

CavanVideoPlayer::~CavanVideoPlayer(void)
{
	pd_pos_info();

	if (mVideoFd >= 0) {
		close(mVideoFd);
		mVideoFd = -1;
	}
}

void CavanVideoPlayer::onFirstRef(void)
{
	pd_pos_info();

	if (mVideoPath == NULL) {
		pd_red_info("Nothing to be done");
		return;
	}

	pd_info("mVideoPath = %s", mVideoPath);

    status_t err = mSession->linkToComposerDeath(this);
	if (err != NO_ERROR) {
		pd_red_info("linkToComposerDeath");
		return;
	}

	run(mName, PRIORITY_DISPLAY);
}

void CavanVideoPlayer::binderDied(const wp<IBinder> &binder)
{
	pd_pos_info();

	mShouldStop = true;
}

status_t CavanVideoPlayer::readyToRun(void)
{
	status_t status;
	DisplayInfo dinfo;
	sp<IBinder> dtoken;

	pd_pos_info();

	mVideoFd = open(mVideoPath, O_RDONLY);
	if (mVideoFd < 0) {
		pd_err_info("open %s", mVideoPath);
		return NAME_NOT_FOUND;
	}

	mVideoSize = lseek64(mVideoFd, 0, SEEK_END);
	if (mVideoSize < 0) {
		pd_err_info("lseek %s", mVideoPath);
		goto out_close_fd;
	}

	if (lseek(mVideoFd, 0, SEEK_SET) != 0) {
		pd_err_info("lseek %s", mVideoPath);
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

	mFlingerSurfaceControl = mSession->createSurface(String8(mName), mWidth, mHeight, PIXEL_FORMAT_RGB_565);

	SurfaceComposerClient::openGlobalTransaction();
	mFlingerSurfaceControl->setLayer(0x40000000);
	SurfaceComposerClient::closeGlobalTransaction();

	mFlingerSurface = mFlingerSurfaceControl->getSurface();

	return NO_ERROR;

out_close_fd:
	close(mVideoFd);
	return FDS_NOT_ALLOWED;
}

bool CavanVideoPlayer::threadLoop(void)
{
	status_t status;

	pd_pos_info();

	status = reset();
	if (status != NO_ERROR) {
		pd_red_info("reset");
		goto out_surface_clean;
	}

	status = setLooping(false);
	if (status != NO_ERROR) {
		pd_red_info("setLooping");
		goto out_surface_clean;
	}

	status = setAudioStreamType(AUDIO_STREAM_MUSIC);
	if (status != NO_ERROR) {
		pd_red_info("setAudioStreamType");
		goto out_surface_clean;
	}

	status = setDataSource(mVideoFd, 0, mVideoSize);
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
