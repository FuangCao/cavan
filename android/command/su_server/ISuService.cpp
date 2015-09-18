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

#define LOG_TAG		"Cavan"

#include "ISuService.h"

namespace android {

const String16 ISuService::mServiceName("cavan.su");

class BpSuService: public BpInterface<ISuService>
{
public:
	BpSuService(const sp<IBinder> &impl) : BpInterface<ISuService>(impl) {}

	virtual status_t runCommand(const char *command)
	{
        Parcel data, reply;

        data.writeInterfaceToken(ISuService::getInterfaceDescriptor());
		data.writeString8(String8(command));

        status_t status = remote()->transact(RUN_COMMAND, data, &reply);
		if (status == NO_ERROR) {
			status = reply.readInt32();
		}

		return status;
	}
};

IMPLEMENT_META_INTERFACE(SuService, "com.cavan.ISuService");

status_t BnSuService::onTransact(uint32_t code, const Parcel &data, Parcel *reply, uint32_t flags)
{
	ALOGE("code = %d, flags = 0x%08x", code, flags);

    switch (code) {
	case RUN_COMMAND: {
			CHECK_INTERFACE(IAudioFlinger, data, reply);
			String8 command = data.readString8();
			status_t status = runCommand(command.string());
			reply->writeInt32(status);
		}
		break;

	default:
		return BBinder::onTransact(code, data, reply, flags);
	}

	return NO_ERROR;
}

};
