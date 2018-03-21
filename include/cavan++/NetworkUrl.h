#pragma once

/*
 * File:		NetworkUrl.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-03-21 10:30:06
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

class NetworkUrl {
public:
	static const char *DEFAULT_PROTOCOL;
	static const char *DEFAULT_HOST;

private:
	const char *mProtocol;
	const char *mHost;
	const char *mPath;
	const char *mArgs;
	char *mData;
	u16 mPort;

public:
	NetworkUrl(void) : mData(NULL) {
		clear();
	}

	virtual ~NetworkUrl() {
		if (mData != NULL) {
			delete[] mData;
		}
	}

	const char *getProtocol(void) {
		return mProtocol;
	}

	void setProtocol(const char *protocol) {
		mProtocol = protocol;
	}

	const char *getHost(void) {
		return mHost;
	}

	void setHost(const char *host) {
		mHost = host;
	}

	const char *getPath(void) {
		return mPath;
	}

	void setPath(const char *path) {
		mPath = path;
	}

	u16 getPort(void) {
		return mPort;
	}

	void setPort(u16 port) {
		mPort = port;
	}

public:
	void dump(void);
	void clear(void);
	bool parse(const char *url, size_t size);

	bool parse(const char *url) {
		return parse(url, strlen(url));
	}
};
