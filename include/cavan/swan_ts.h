#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Thu Apr  5 18:45:28 CST 2012
 */

#include <cavan.h>

#define SWAN_TS_MISC_DEVICE			"/dev/swan_touchscreen"

struct swan_ts_i2c_request
{
	u16 offset;
	u16 size;
	void *data;
};

#define SWAN_TS_IOCTL_CALIBRATION	_IO('E', 0x01)
#define SWAN_TS_IOCTL_READ_I2C		_IOR('E', 0x02, struct swan_ts_i2c_request)
#define SWAN_TS_IOCTL_WRITE_I2C		_IOW('E', 0x03, struct swan_ts_i2c_request)

int swan_ts_do_calication(int argc, char *argv[]);

