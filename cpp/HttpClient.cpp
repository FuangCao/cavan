/*
 * File:		HttpClient.cpp
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
#include <cavan++/HttpClient.h>

HttpStatusCode HttpStatusCode::OK = HttpStatusCode(200, "OK");
HttpStatusCode HttpStatusCode::Created = HttpStatusCode(201, "Created");
HttpStatusCode HttpStatusCode::Accepted = HttpStatusCode(202, "Accepted");

HttpStatusCode HttpStatusCode::BadRequest = HttpStatusCode(400, "Bad Request");
HttpStatusCode HttpStatusCode::Unauthorized = HttpStatusCode(401, "Unauthorized");
HttpStatusCode HttpStatusCode::Forbidden = HttpStatusCode(403, "Forbidden");
HttpStatusCode HttpStatusCode::NotFound = HttpStatusCode(404, "Not Found");

HttpStatusCode HttpStatusCode::InternalServerError = HttpStatusCode(500, "Internal Server Error");
HttpStatusCode HttpStatusCode::BadGateway = HttpStatusCode( 502, "Bad Gateway");

const char *HttpHeader::sHeaderNames[HTTP_HEADER_COUNT] = {
	[HTTP_HEADER_HOST] = "Host",
	[HTTP_HEADER_CONNECTION] = "Connection",
	[HTTP_HEADER_CONTENT_TYPE] = "Content-Type",
	[HTTP_HEADER_CONTENT_LENGTH] = "Content-Length",
};

int HttpHeader::getHeaderType(const char *name)
{
	for (int i = 0; i < HTTP_HEADER_COUNT; i++) {
		if (strcasecmp(name, sHeaderNames[i]) == 0) {
			return i;
		}
	}

	return -1;
}

void HttpHeader::reset(void)
{
	EpollBuffer::reset();
	mCompleted = false;
	mInvalid = false;
	mLineNo = 0;
	mValue = 0;
	mName = 0;

	for (int i = 0; i < HTTP_HEADER_COUNT; i++) {
		mHeaders[i] = 0;
	}
}

void HttpHeader::dump(void)
{
	println("%s %s %s", getType(), getPath(), getVersion());

	for (int i = 0; i < HTTP_HEADER_COUNT; i++) {
		int offset = mHeaders[i];
		if (offset > 0) {
			println("%s: %s", sHeaderNames[i], mData + offset);
		}
	}
}

int HttpHeader::write(const void *buff, u16 length)
{
	const char *p, *p_end;
	int ret;

	for (p = (const char *) buff, p_end = p + length; p < p_end; p++) {
		switch (*p) {
		case '\r':
			break;

		case '\n':
			if (mInvalid) {
				mInvalid = false;
				break;
			}

			if (mName == mOffset) {
				mCompleted = true;
				return p - (char *) buff + 1;
			}

			ret = putchar(0);
			if (ret < 0) {
				return ret;
			}

			if (++mLineNo == 1) {
				mVersion = mName;
			}

			mName = mOffset;
			mValue = 0;
			break;

		case ':':
			if (mInvalid) {
				break;
			}

			if (mValue == 0) {
				ret = putchar(0);
				if (ret < 0) {
					return ret;
				}

				int type = getHeaderType(mData + mName);
				if (type < 0) {
					mInvalid = true;
				} else {
					mHeaders[type] = mName;
					mValue = mName;
				}

				setOffset(mName);
			} else {
				ret = putchar(':');
				if (ret < 0) {
					return ret;
				}
			}
			break;

		case ' ':
			if (mLineNo == 0) {
				ret = putchar(0);
				if (ret < 0) {
					return ret;
				}

				if (mName > 0) {
					mPath = mName;
				}

				mName = mOffset;
				break;
			}
		case '\t':
			if (mValue == mOffset) {
				break;
			}
		default:
			if (mInvalid) {
				break;
			}

			ret = putchar(*p);
			if (ret < 0) {
				return ret;
			}
		}
	}

	return length;
}

u16 HttpHeader::getLength(void)
{
	const char *content_length = getHeader(HTTP_HEADER_CONTENT_LENGTH);
	if (content_length == NULL) {
		return 0;
	}

	return atoi(content_length);
}

EpollPacket *HttpClient::onEpollHeaderReceived(EpollBuffer *header)
{
	HttpPacket *packet = new HttpPacket(mHeader);
	if (packet != NULL) {
		mHeader = NULL;
	}

	return packet;
}
