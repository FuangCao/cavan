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

	virtual int popen(const char *command, int lines, int columns, pid_t *ppid, int flags) {
        Parcel data, reply;

		if (command == NULL) {
			command = "";
		}

		if (lines == 0 && columns == 0) {
			tty_get_win_size2(0, &lines, &columns);
		}

		if ((flags & 0x07) == 0) {
			flags = CAVAN_EXECF_STDIN | CAVAN_EXECF_STDOUT | CAVAN_EXECF_ERR_TO_OUT;
		}

        data.writeInterfaceToken(ISuService::getInterfaceDescriptor());
		data.writeString8(String8(command ? command : ""));
		data.writeInt32(lines);
		data.writeInt32(columns);
		data.writeInt32(flags);

        status_t status = remote()->transact(CMD_POPEN, data, &reply);
		if (status != NO_ERROR) {
			return -EFAULT;
		}

		int ret = reply.readInt32();
		if (ret < 0) {
			return ret;
		}

		reply.readInt32(ppid);

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
			CHECK_INTERFACE(IAudioFlinger, data, reply);
			String8 command = data.readString8();
			int lines = data.readInt32();
			int columns = data.readInt32();
			int flags = data.readInt32();
			pid_t pid;
			int ret = popen(command.string(), lines, columns, &pid, flags);
			reply->writeInt32(ret);
			if (ret < 0) {
				return ret;
			}
			reply->writeInt32(pid);
		}
		break;

	default:
		return BBinder::onTransact(code, data, reply, flags);
	}

	return NO_ERROR;
}

};
