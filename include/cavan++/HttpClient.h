#pragma once

/*
 * File:		HttpClient.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-04-09 16:39:16
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
#include <cavan++/NetworkClient.h>

typedef enum {
	HTTP_HEADER_HOST,
	HTTP_HEADER_CONNECTION,
	HTTP_HEADER_CONTENT_TYPE,
	HTTP_HEADER_CONTENT_LENGTH,
	HTTP_HEADER_COUNT,
} http_header_t;

class HttpStatusCode {
public:
	static HttpStatusCode OK;
	static HttpStatusCode Created;
	static HttpStatusCode Accepted;

	static HttpStatusCode BadRequest;
	static HttpStatusCode Unauthorized;
	static HttpStatusCode Forbidden;
	static HttpStatusCode NotFound;

	static HttpStatusCode InternalServerError;
	static HttpStatusCode BadGateway;

private:
	int mCode;
	const char *mDesc;

public:
	HttpStatusCode(int code, const char *desc) : mCode(code), mDesc(desc) {}
	virtual ~HttpStatusCode() {}

	virtual int getCode(void) {
		return mCode;
	}

	virtual const char *getDesc(void) {
		return mDesc;
	}

	virtual void dump(void) {
		println("%d %s", mCode, mDesc);
	}
};

class HttpHeader : public EpollBufferAuto {
public:
	static const char *sHeaderNames[HTTP_HEADER_COUNT];
	static int getHeaderType(const char *name);

private:
	u16 mPath;
	u16 mVersion;
	u16 mName;
	u16 mValue;
	u16 mLineNo;
	bool mInvalid;
	u16 mHeaders[HTTP_HEADER_COUNT];

public:
	HttpHeader(void) {}
	virtual ~HttpHeader() {}

public:
	virtual char *getType(void) {
		return mData;
	}

	virtual char *getPath(void) {
		return mData + mPath;
	}

	virtual char *getVersion(void) {
		return mData + mVersion;
	}

	virtual char *getHeader(http_header_t header) {
		int offset = mHeaders[header];
		if (offset > 0) {
			return mData + mHeaders[header];
		}

		return NULL;
	}

	virtual void dump(void);
	virtual void reset(void);
	virtual int write(const void *buff, u16 length, bool &completed);
	virtual u16 getLength(void);
};

class HttpResponse : public EpollPacket {
private:
	HttpStatusCode &mStatusCode;

public:
	HttpResponse(HttpStatusCode &code) : mStatusCode(code) {}
	virtual ~HttpResponse() {}

	HttpStatusCode &getStatusCode(void) {
		return mStatusCode;
	}
};

class HttpClient : public NetworkEpollClient<HttpHeader> {
public:
	HttpClient(EpollService *service, NetworkClient *client) : NetworkEpollClient(service, client) {}

	virtual ~HttpClient() {}
};
