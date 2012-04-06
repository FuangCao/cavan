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

int swan_ts_open_misc_device(const char *devpath);
int swan_ts_calibration(const char *devpath);
int swan_ts_read_registers_fd(int fd, u16 addr, void *buff, size_t size);
int swan_ts_read_registers(const char *devpath, u16 addr, void *buff, size_t size);
int swan_ts_write_registers_fd(int fd, u16 addr, const void *buff, size_t size);
int swan_ts_write_registers(const char *devpath, u16 addr, const void *buff, size_t size);

int swan_ts_calication_main(int argc, char *argv[]);
int swan_ts_read_registers_main(int argc, char *argv[]);
int swan_ts_poll_registers_main(int argc, char *argv[]);
int swan_ts_write_registers_main(int argc, char *argv[]);
int ft5406_firmware_upgrade_main(int argc, char *argv[]);

