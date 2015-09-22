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

#ifndef NELEM
#define NELEM(a) \
	(sizeof(a) / sizeof((a)[0]))
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "Cavan"
#endif

#define pr_error_info(format, args ...) \
	fprintf(stderr, format "\n", ##args)

#define pr_red_info(format, args ...) \
	fprintf(stderr, format "\n", ##args)

#define pr_pos_info() \
	ALOGE("%s => %s[%d]", __FILE__, __FUNCTION__, __LINE__);

namespace android {

enum {
	CREATE = IBinder::FIRST_CALL_TRANSACTION,
	CMD_SYSTEM,
	CMD_POPEN,
};

class ISuService : public IInterface
{
public:
	static const String16 mServiceName;

public:
	DECLARE_META_INTERFACE(SuService);

	static sp<ISuService> getService(void) {
		sp<IBinder> binder = defaultServiceManager()->checkService(mServiceName);
		if (binder == NULL) {
			return NULL;
		}

		return interface_cast<ISuService>(binder);
	}

	virtual int system(const char *command) = 0;
	virtual int popen(const char *command, char *pathname, size_t size) = 0;
};

class BnSuService: public BnInterface<ISuService>
{
public:
	virtual status_t onTransact( uint32_t code, const Parcel &data, Parcel* reply, uint32_t flags = 0);
};

};
