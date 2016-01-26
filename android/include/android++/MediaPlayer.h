#pragma once

/*
 * File:			MediaPlayer.h
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
#include <media/mediaplayer.h>
#include <gui/ISurfaceComposer.h>
#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>

#define PROP_NAME_VOLUME	"sys.cavan.volume"

namespace android {

class CavanVideoPlayer : public MediaPlayer, public Thread, public IBinder::DeathRecipient {
private:
	const char *mName;

	int mVideoFd;
	off64_t mVideoSize;
	const char *mVideoPath;

	int mVolume;
	int mPosition;
	int mDuration;
	int mHideCount;
	bool mBootAnimation;
	int mWidth, mHeight;
	int mCommandLen;
	char mCommand[1024];
	sp<Surface> mFlingerSurface;
	sp<SurfaceComposerClient> mSession;
	sp<SurfaceControl> mFlingerSurfaceControl;

public:
	CavanVideoPlayer(const char *pathname, bool bootanimation = false, const char *name = "CavanVideoPlayer");
	~CavanVideoPlayer(void);

	virtual void binderDied(const wp<IBinder> &binder);
	virtual void onFirstRef(void);
	virtual status_t readyToRun(void);
	virtual bool threadLoop(void);

	void doCommand(char *command, size_t length);
	status_t setVolume(int volume);
};

}
