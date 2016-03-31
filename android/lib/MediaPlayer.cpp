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
#include <cavan/time.h>
#include <cavan/ctype.h>
#include <cavan/progress.h>
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
	mCommand[0] = 0;
	mCommandLen = 0;
	mBootAnimation = bootanimation;
	mVolume = android_getprop_int(PROP_NAME_VOLUME, 80);
	mSession = new SurfaceComposerClient();

	pd_info("mVolume = %d", mVolume);
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

	requestExit();
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
		status = NAME_NOT_FOUND;
		goto out_stop_process;
	}

	mVideoSize = lseek64(mVideoFd, 0, SEEK_END);
	if (mVideoSize < 0) {
		pd_err_info("lseek %s", mVideoPath);
		status = BAD_VALUE;
		goto out_close_fd;
	}

	if (lseek(mVideoFd, 0, SEEK_SET) != 0) {
		pd_err_info("lseek %s", mVideoPath);
		status = BAD_VALUE;
		goto out_close_fd;
	}

	dtoken = SurfaceComposerClient::getBuiltInDisplay(ISurfaceComposer::eDisplayIdMain);
	if (dtoken == 0) {
		pd_red_info("SurfaceComposerClient::getBuiltInDisplay");
		status = BAD_VALUE;
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
out_stop_process:
	IPCThreadState::self()->stopProcess();
	return status;
}

status_t CavanVideoPlayer::setVolume(int volume)
{
	if (volume > 100) {
		volume = 100;
	} else if (volume < 0) {
		volume = 0;
	}

	float precent = ((float) volume) / 100;
	float status = MediaPlayer::setVolume(precent, precent);
	if (status == NO_ERROR) {
		mVolume = volume;
	}

	android_setprop_int(PROP_NAME_VOLUME, volume);

	return status;
}

void CavanVideoPlayer::doCommand(char *command, size_t length)
{
	bool changed = true;
	char *last = command + length - 1;

	while(1) {
		if (last <= command) {
			if (mCommandLen > 0 && mCommand[0]) {
				command = mCommand;
				break;
			} else {
				return;
			}
		}

		if (cavan_isspace(*last)) {
			*last-- = 0;
		} else {
			strcpy(mCommand, command);
			mCommandLen = last - command + 1;
			break;
		}
	}

	if (text_lhcmp("seek", command) == 0) {
		int position;
		char buff[1024];

		getCurrentPosition(&mPosition);

		command = text_skip_space(command + 4, command + mCommandLen);
		switch (command[0]) {
		case '+':
			position = mPosition + text2value_unsigned(command + 1, NULL, 10) * 1000;
			break;

		case '-':
			position = mPosition - text2value_unsigned(command + 1, NULL, 10) * 1000;
			break;

		default:
			if (command[0] == 0) {
				position = mPosition;
				changed = false;
			} else {
				position = text2value_unsigned(command, NULL, 10) * 1000;
			}
		}

		if (changed) {
			if (position > mDuration) {
				position = mDuration;
			} else if (position < 0) {
				position = 0;
			}

			seekTo(position);
		}

		cavan_time2text_simple2(position / 1000, buff, sizeof(buff));
		pr_info("position = %s", buff);
	} else if (text_lhcmp("volume", command) == 0) {
		int volume;

		command = text_skip_space(command + 6, command + mCommandLen);
		switch (command[0]) {
		case '+':
			volume = mVolume + text2value_unsigned(command + 1, NULL, 10);
			break;

		case '-':
			volume = mVolume - text2value_unsigned(command + 1, NULL, 10);
			break;

		default:
			if (command[0] == 0) {
				volume = mVolume;
				changed = false;
			} else {
				volume = text2value_unsigned(command, NULL, 10);
			}
		}

		if (changed) {
			setVolume(volume);
		}

		pr_info("volume = %02d%%", mVolume);
	} else if (strcmp("stop", command) == 0) {
		stop();
	} else if (strcmp("exit", command) == 0) {
		requestExit();
	} else if (strcmp("progress", command) == 0 || strcmp("show", command) == 0) {
		mHideCount = 0;
	}
}

bool CavanVideoPlayer::threadLoop(void)
{
	status_t status;
	struct progress_bar bar;

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

	status = setVolume(mVolume);
	if (status != NO_ERROR) {
		pd_red_info("setVolume");
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

	mHideCount = 0;
	getDuration(&mDuration);
	progress_bar_init(&bar, mDuration, 0, PROGRESS_BAR_TYPE_TIME);

	while (isPlaying()) {
		ssize_t rdlen;
		char command[1024];

		if (exitPending()) {
			break;
		}

#if 0
		if (mBootAnimation && android_getprop_int("service.bootanim.exit", 0)) {
			break;
		}
#endif

		rdlen = file_read_timeout(stdin_fd, command, sizeof(command), 200);
		if (rdlen > 0) {
			mHideCount = 50;
			command[rdlen] = 0;
			doCommand(command, rdlen);
			print("MediaPlayer> ");
		}

		if (mHideCount > 0) {
			mHideCount--;
		} else {
			if (mHideCount == 0) {
				print_char('\r');
				mCommand[0] = 0;
				mCommandLen = 0;

				mHideCount = -1;
			}

			getCurrentPosition(&mPosition);
			progress_bar_set(&bar, mPosition);
		}
	}

	progress_bar_finish(&bar);

	pd_pos_info();

	stop();

out_surface_clean:
    mFlingerSurface.clear();
    mFlingerSurfaceControl.clear();
    IPCThreadState::self()->stopProcess();

	return false;
}

CavanMediaPlayer::CavanMediaPlayer(void)
{
	sp<ProcessState> proc(ProcessState::self());
	ProcessState::self()->startThreadPool();

	mInitSuccess = false;
}

CavanMediaPlayer::~CavanMediaPlayer(void)
{
	if (mInitSuccess) {
	    mSurface.clear();
	    mSurfaceControl.clear();
	}
}

bool CavanMediaPlayer::doInit(void)
{
	status_t status;
	DisplayInfo dinfo;
	sp<IBinder> dtoken;

	if (mInitSuccess) {
		return true;
	}

	dtoken = SurfaceComposerClient::getBuiltInDisplay(ISurfaceComposer::eDisplayIdMain);
	if (dtoken == 0) {
		pd_red_info("SurfaceComposerClient::getBuiltInDisplay");
		return false;
	}

	status = SurfaceComposerClient::getDisplayInfo(dtoken, &dinfo);
	if (status != NO_ERROR) {
		pd_red_info("SurfaceComposerClient::getDisplayInfo");
		return false;
	}

	mSession = new SurfaceComposerClient();
	if (mSession == NULL) {
		pd_red_info("new SurfaceComposerClient");
		return false;
	}

	if (dinfo.w > dinfo.h * 3) {
		mWidth = dinfo.w >> 1;
	} else {
		mWidth = dinfo.w;
	}

	mHeight = dinfo.h;

	mSurfaceControl = mSession->createSurface(String8("JwVideo"), mWidth, mHeight, PIXEL_FORMAT_RGB_565);
	if (mSurfaceControl == NULL) {
		pd_red_info("mSession->createSurface");
		return false;
	}

	SurfaceComposerClient::openGlobalTransaction();
	mSurfaceControl->setLayer(0x40000000);
	SurfaceComposerClient::closeGlobalTransaction();

	mSurface = mSurfaceControl->getSurface();

	mInitSuccess = true;

	return true;
}

bool CavanMediaPlayer::doPlay(const char *pathname)
{
	int fd;
	int64_t size;
	status_t status;
	bool success = false;
	struct progress_bar bar;

	if (!doInit()) {
		pd_red_info("doInit");
		return false;
	}

	fd = open(pathname, O_RDONLY);
	if (fd < 0) {
		pd_err_info("open file: %s", pathname);
		return false;
	}

	pd_info("pathname = %s", pathname);

	size = ffile_get_size(fd);
	if (size == 0) {
		size = lseek(fd, 0, SEEK_END);
		if (size < 0 && errno != EOVERFLOW) {
			pd_err_info("lseek: size = %lld", size);
			goto out_close_fd;
		}
	}

	pd_info("size = %s", size2text(size));

	status = reset();
	if (status != NO_ERROR) {
		pd_red_info("reset");
		goto out_close_fd;
	}

	status = setLooping(false);
	if (status != NO_ERROR) {
		pd_red_info("setLooping");
		goto out_close_fd;
	}

	status = setAudioStreamType(AUDIO_STREAM_MUSIC);
	if (status != NO_ERROR) {
		pd_red_info("setAudioStreamType");
		goto out_close_fd;
	}

	status = setVolume(100, 100);
	if (status != NO_ERROR) {
		pd_red_info("setVolume");
		goto out_close_fd;
	}

	status = setDataSource(fd, 0, size);
	if (status != NO_ERROR) {
		pd_red_info("setDataSource");
		goto out_close_fd;
	}

	status = setVideoSurfaceTexture(mSurface->getIGraphicBufferProducer());
	if (status != NO_ERROR) {
		pd_red_info("setVideoSurfaceTexture");
		goto out_close_fd;
	}

	status = prepare();
	if (status != NO_ERROR) {
		pd_red_info("prepare");
		goto out_close_fd;
	}

	status = start();
	if (status != NO_ERROR) {
		pd_red_info("start");
		goto out_close_fd;
	}

	getDuration(&mDuration);
	progress_bar_init(&bar, mDuration, 0, PROGRESS_BAR_TYPE_TIME);

	while (isPlaying()) {
		getCurrentPosition(&mPosition);
		progress_bar_set(&bar, mPosition);

		msleep(200);
	}

	progress_bar_finish(&bar);

	stop();
	success = true;

out_close_fd:
	close(fd);
	return success;
}

}
