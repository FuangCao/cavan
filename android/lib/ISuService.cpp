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

#include <cavan/android/ISuService.h>

namespace android {

const String16 ISuService::sServiceName("cavan.su");

class BpSuService: public BpInterface<ISuService> {
private:
	int mFlags;

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

	virtual int popen(const char *command, int flags) {
        Parcel data, reply;

		if (command == NULL) {
			command = "";
		}

		if ((flags & 0x07) == 0) {
			mFlags = CAVAN_EXECF_STDIN | CAVAN_EXECF_STDOUT | CAVAN_EXECF_ERR_TO_OUT;
		} else {
			mFlags = flags;
		}

		int size[2];
		tty_get_win_size(0, size);

        data.writeInterfaceToken(ISuService::getInterfaceDescriptor());
		data.writeString8(String8(command));
		data.writeInt32(size[0]);
		data.writeInt32(size[1]);
		data.writeInt32(mFlags);

        status_t status = remote()->transact(CMD_POPEN, data, &reply);
		if (status != NO_ERROR) {
			return -EFAULT;
		}

		int ret = reply.readInt32();
		if (ret < 0) {
			return ret;
		}

		reply.readInt32(&mPid);

		return 0;
	}

	int openPipeSlave(int ttyfds[3]) {
		return cavan_exec_open_temp_pipe_slave(ttyfds, mPid, mFlags);
	}

	int redirectSlaveStdio(void) {
		int ret;
		int ttyfds[3];

		ret = openPipeSlave(ttyfds);
		if (ret < 0) {
			ALOGE("Failed to openPipeSlave: %d", ret);
			return ret;
		}

		cavan_tty_redirect(ttyfds[0], ttyfds[1], ttyfds[2]);

		return cavan_exec_waitpid(mPid);
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
			mLines = data.readInt32();
			mColumns = data.readInt32();
			int flags = data.readInt32();
			int ret = popen(command.string(), flags);
			reply->writeInt32(ret);
			if (ret < 0) {
				return ret;
			}
			reply->writeInt32(mPid);
		}
		break;

	default:
		return BBinder::onTransact(code, data, reply, flags);
	}

	return NO_ERROR;
}

};
