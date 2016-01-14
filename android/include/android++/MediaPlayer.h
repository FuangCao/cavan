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

namespace android {

class CavanPlayer : public MediaPlayer, public Thread, public IBinder::DeathRecipient {
private:
	int mFd;
	off_t mSize;
	bool mShouldStop;
	bool mBootAnimation;
	int mWidth, mHeight;
	const char *mPathName;
	EGLDisplay mDisplay;
	sp<SurfaceComposerClient> mSession;
	sp<SurfaceControl> mFlingerSurfaceControl;
	sp<Surface> mFlingerSurface;

public:
	CavanPlayer(const char *pathname, bool bootanimation = false);
	~CavanPlayer(void);

	virtual void binderDied(const wp<IBinder> &binder);
	virtual void onFirstRef(void);
	virtual status_t readyToRun(void);
	virtual bool threadLoop(void);
};

}
