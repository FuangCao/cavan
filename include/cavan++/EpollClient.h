#pragma once

/*
 * File:		EpollClient.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2018-03-27 11:22:18
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
#include <cavan++/Lock.h>
#include <cavan++/Link.h>

class EpollService;
class EpollClient;

class EpollBuffer {
protected:
	u16 mOffset;

public:
	EpollBuffer(void) {
		reset();
	}

	virtual ~EpollBuffer() {}

public:
	virtual u16 getSize(void) = 0;
	virtual char *getData(void) = 0;

	virtual int writeTo(EpollClient *client);
	virtual int write(const void *buff, u16 length, bool &completed);

	virtual void reset(void) {
		mOffset = 0;
	}

	virtual u16 getLength(void) {
		return mOffset;
	}

	virtual char *getDataHead(void) {
		return getData() + mOffset;
	}

	virtual u16 getOffset(void) {
		return mOffset;
	}

	virtual void seek(u16 offset) {
		mOffset = offset;
	}

	virtual u16 getRemain(void) {
		return getSize() - mOffset;
	}
};

class EpollBufferU16 : public EpollBuffer {
private:
	u16 mLength;

public:
	virtual u16 getSize(void) {
		return sizeof(mLength);
	};

	virtual u16 getLength(void) {
		return mLength;
	}

	virtual char *getData(void) {
		return (char *) &mLength;
	}
};

class EpollBufferAuto : public EpollBuffer {
protected:
	char *mData;
	u16 mSize;

public:
	EpollBufferAuto(void) : mData(NULL), mSize(0) {}

	virtual ~EpollBufferAuto() {
		if (mData != NULL) {
			delete[] mData;
		}
	}

	virtual int alloc(u16 length);
	virtual int write(char c);
	virtual int write(const void *buff, u16 length, bool &completed);

	virtual u16 getSize(void) {
		return mSize;
	}

	virtual char *getData(void) {
		return mData;
	}
};

class EpollPacket : public EpollBuffer {
friend EpollClient;

protected:
	char *mData;
	u16 mLength;
	EpollPacket *mNext;

public:
	EpollPacket(u16 length = 0) : mLength(length) {
		if (length > 0) {
			mData = new char[length];
		} else {
			mData = NULL;
		}
	}

	virtual ~EpollPacket() {
		if (mData != NULL) {
			delete[] mData;
		}
	}

	virtual u16 getSize(void) {
		return mLength;
	}

	virtual char *getData(void) {
		return mData;
	}
};

class EpollClient : public SimpleLink<EpollClient> {
friend class EpollBuffer;
friend class EpollService;

private:
	EpollPacket *mWrHead;
	EpollPacket *mWrTail;
	ThreadLock mWrLock;
	u32 mEpollEvents;

protected:
	EpollService *mEpollService;
	EpollClient *mEpollNext;

public:
	EpollClient(EpollService *service) : mEpollService(service) {
		mEpollNext = this;
		mEpollEvents = 0;
		mWrHead = NULL;
	}

	virtual ~EpollClient() {
		EpollPacket *pack = mWrHead;

		while (pack != NULL) {
			EpollPacket *next = pack->mNext;
			delete pack;
			pack = next;
		}
	}

	virtual u32 getEpollEvents(void) {
		return mEpollEvents;
	}

	virtual void setEpollEvents(u32 events) {
		mEpollEvents |= events;
	}

	virtual int addEpollTo(EpollService *service);
	virtual int removeEpollFrom(EpollService *service);
	virtual void sendEpollPacket(EpollPacket *packet);

protected:
	virtual int getEpollFd(void) = 0;

	virtual int doEpollRead(void *buff, int size) {
		pr_red_info("doEpollRead no implement!");
		return -ENOENT;
	}

	virtual int doEpollWrite(const void *buff, int size) {
		pr_red_info("doEpollWrite no implement!");
		return -ENOENT;
	}

protected:
	virtual bool onEpollEvent(EpollService *service);
	virtual int onEpollIn(EpollService *service);
	virtual int onEpollOut(EpollService *service);
	virtual void onEpollErr(EpollService *service);

	virtual int onEpollDataReceived(EpollService *service, const void *buff, int size) {
		println("onEpollDataReceived[%d]: %s", size, (const char *) buff);
		return size;
	}
};

template <class T>
class EpollPackClient : public EpollClient {
protected:
	T mHeader;
	EpollPacket *mWrPacket;

public:
	EpollPackClient(EpollService *service) : EpollClient(service), mWrPacket(NULL) {}

protected:
	virtual int onEpollDataReceived(EpollService *service, const void *buff, int size) {
		bool completed = false;
		int wrlen;

		if (mWrPacket != NULL) {
			wrlen = mWrPacket->write(buff, size, completed);
			if (wrlen < 0) {
				return wrlen;
			}

			if (completed) {
				int ret = onEpollPackReceived(mWrPacket);
				if (ret < 0) {
					return ret;
				}

				delete mWrPacket;
				mWrPacket = NULL;
				mHeader.seek(0);
			}
		} else {
			wrlen = mHeader.write(buff, size, completed);
			if (wrlen < 0) {
				return wrlen;
			}

			if (completed) {
				u16 length = mHeader.getLength();

				println("length = %d", length);

				if (length > 0) {
					mWrPacket = new EpollPacket(length);
					if (mWrPacket == NULL) {
						return -ENOMEM;
					}
				} else {
					int ret = onEpollPackReceived(NULL);
					if (ret < 0) {
						return ret;
					}
				}
			}
		}

		return wrlen;
	}

	virtual int onEpollPackReceived(EpollPacket *packet) {
		println("onEpollPackReceived");
		return 0;
	}
};
