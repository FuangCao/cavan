/*
 * File:		ISuService.cpp
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-09-18 11:55:09
 *
 * Copyright (c) 2015 Fuang.Cao <cavan.cfa@gmail.com>
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

#include "ISuService.h"

namespace android {

const String16 ISuService::mServiceName("cavan.su");

class BpSuService: public BpInterface<ISuService>
{
public:
	BpSuService(const sp<IBinder> &impl) : BpInterface<ISuService>(impl) {}

	virtual int system(const char *command) {
        Parcel data, reply;

        data.writeInterfaceToken(ISuService::getInterfaceDescriptor());
		data.writeString8(String8(command));

        status_t status = remote()->transact(CMD_SYSTEM, data, &reply);
		if (status != NO_ERROR) {
			return -EFAULT;
		}

		int ret = reply.readInt32();
		if (ret < 0) {
			return ret;
		}

		return 0;
	}

	virtual int popen(const char *command, char *pathname, size_t size) {
        Parcel data, reply;

        data.writeInterfaceToken(ISuService::getInterfaceDescriptor());
		data.writeString8(String8(command));

        status_t status = remote()->transact(CMD_POPEN, data, &reply);
		if (status != NO_ERROR) {
			return -EFAULT;
		}

		int ret = reply.readInt32();
		if (ret < 0) {
			return ret;
		}

		String8 strPath = reply.readString8();
		strncpy(pathname, strPath, size);

		return 0;
	}
};

IMPLEMENT_META_INTERFACE(SuService, "com.cavan.ISuService");

status_t BnSuService::onTransact(uint32_t code, const Parcel &data, Parcel *reply, uint32_t flags)
{
	ALOGE("code = %d, flags = 0x%08x", code, flags);

    switch (code) {
	case CMD_SYSTEM: {
			CHECK_INTERFACE(IAudioFlinger, data, reply);
			String8 command = data.readString8();
			int ret = system(command.string());
			reply->writeInt32(ret);
		}
		break;

	case CMD_POPEN: {
			char pathname[1024];

			CHECK_INTERFACE(IAudioFlinger, data, reply);
			String8 command = data.readString8();
			int ret = popen(command.string(), pathname, sizeof(pathname));
			reply->writeInt32(ret);
			if (ret == 0) {
				reply->writeString8(String8(pathname));
			}
		}
		break;

	default:
		return BBinder::onTransact(code, data, reply, flags);
	}

	return NO_ERROR;
}

};
