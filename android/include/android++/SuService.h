#pragma once

/*
 * File:		SuService.h
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

#include <utils/String8.h>
#include <binder/BinderService.h>

#include "ISuService.h"

namespace android {

class SuService : public BinderService<SuService>, public BnSuService {
public:
	static const String16 &getServiceName(void) {
		return sServiceName;
	}

	virtual int system(const char *command);
	virtual int popen(const char *command, int flags);
};

};
