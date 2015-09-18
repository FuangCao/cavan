#pragma once

/*
 * File:		ISuService.h
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

#include <utils/RefBase.h>
#include <utils/String8.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <binder/IServiceManager.h>

namespace android {

enum {
	CREATE = IBinder::FIRST_CALL_TRANSACTION,
	RUN_COMMAND,
};

class ISuService : public IInterface
{
public:
	static const String16 mServiceName;

public:
	DECLARE_META_INTERFACE(SuService);

	static sp<ISuService> getService(void)
	{
		sp<IBinder> binder = defaultServiceManager()->checkService(mServiceName);
		if (binder == NULL) {
			return NULL;
		}

		return interface_cast<ISuService>(binder);
	}

	virtual status_t runCommand(const char *command) = 0;
};

class BnSuService: public BnInterface<ISuService>
{
public:
	virtual status_t onTransact( uint32_t code, const Parcel &data, Parcel* reply, uint32_t flags = 0);
};

};
