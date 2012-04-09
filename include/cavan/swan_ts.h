#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Thu Apr  5 18:45:28 CST 2012
 */

#include <cavan.h>

#define SWAN_TS_DEVICE_NAME			"Swan Touch Screen"
#define SWAN_TS_MISC_DEVICE			"/dev/swan_touchscreen"

struct swan_ts_i2c_request
{
	u16 offset;
	u16 size;
	void *data;
};

enum swan_ts_ioctl_type
{
	SWAN_TS_IOCTL_TYPE_CORE,
	SWAN_TS_IOCTL_TYPE_DEVICE,
	SWAN_TS_IOCTL_TYPE_OTHER
};

struct ft5406_firmware_data_package
{
	u32 size;
	void *data;
};

#define SWAN_TS_CORE_IO(nr) \
	_IO(SWAN_TS_IOCTL_TYPE_CORE, nr)

#define SWAN_TS_CORE_IOR(nr, size) \
	_IOR(SWAN_TS_IOCTL_TYPE_CORE, nr, size)

#define SWAN_TS_CORE_IOW(nr, size) \
	_IOW(SWAN_TS_IOCTL_TYPE_CORE, nr, size)

#define SWAN_TS_DEVICE_IO(nr) \
	_IO(SWAN_TS_IOCTL_TYPE_DEVICE, nr)

#define SWAN_TS_DEVICE_IOR(nr, size) \
	_IOR(SWAN_TS_IOCTL_TYPE_DEVICE, nr, size)

#define SWAN_TS_DEVICE_IOW(nr, size) \
	_IOW(SWAN_TS_IOCTL_TYPE_DEVICE, nr, size)

#define SWAN_TS_IOCTL_CALIBRATION			SWAN_TS_CORE_IO(0x01)
#define SWAN_TS_INPUT_IOCTL_CALIBRATION		_IO('E', 0x01)
#define SWAN_TS_IOCTL_READ_REGISTERS		SWAN_TS_CORE_IOR(0x02, struct swan_ts_i2c_request)
#define SWAN_TS_IOCTL_WRITE_REGISTERS		SWAN_TS_CORE_IOW(0x03, struct swan_ts_i2c_request)
#define SWAN_TS_IOCTL_READ_DATA				SWAN_TS_CORE_IOR(0x04, struct swan_ts_i2c_request)
#define SWAN_TS_IOCTL_WRITE_DATA			SWAN_TS_CORE_IOW(0x05, struct swan_ts_i2c_request)
#define SWAN_TS_IOCTL_GET_CLIENT_ADDRESS	SWAN_TS_CORE_IOR(0x06, u16)
#define SWAN_TS_IOCTL_SET_CLIENT_ADDRESS	SWAN_TS_CORE_IOW(0x07, u16)
#define SWAN_TS_IOCTL_TEST_CLIENT			SWAN_TS_CORE_IOW(0x08, u16)
#define SWAN_TS_IOCTL_DETECT_CLIENT			SWAN_TS_CORE_IOW(0x09, u32)

#define FT5406_IOCTL_UPGRADE_ENTER		SWAN_TS_DEVICE_IO(0x01)
#define FT5406_IOCTL_UPGRADE_FINISH		SWAN_TS_DEVICE_IO(0x02)
#define FT5406_IOCTL_ERASE_APP_CONFIG	SWAN_TS_DEVICE_IO(0x03)
#define FT5406_IOCTL_ERASE_APP			SWAN_TS_DEVICE_IO(0x04)
#define FT5406_IOCTL_ERASE_CONFIG		SWAN_TS_DEVICE_IO(0x05)
#define FT5406_IOCTL_ERASE_PARAM		SWAN_TS_DEVICE_IO(0x06)
#define FT5406_IOCTL_READ_CHECKSUM		SWAN_TS_DEVICE_IOR(0x07, u8)
#define FT5406_IOCTL_UPGRADE_APP		SWAN_TS_DEVICE_IOW(0x08, struct ft5406_firmware_data_package)
#define FT5406_IOCTL_SINGLE_WRITE		SWAN_TS_DEVICE_IOW(0x09, struct ft5406_firmware_data_package)

int swan_ts_open_misc_device(const char *devpath, int flags);

int swan_ts_input_calibration(const char *devname);
int swan_ts_calibration(const char *devpath);
int swan_ts_calication_main(int argc, char *argv[]);

int swan_ts_read_registers_fd(int fd, u16 addr, void *buff, size_t size);
int swan_ts_read_registers(const char *devpath, u16 addr, void *buff, size_t size);
int swan_ts_read_registers_main(int argc, char *argv[]);
int swan_ts_poll_registers_main(int argc, char *argv[]);

int swan_ts_read_data_fd(int fd, void *buff, size_t size);
int swan_ts_read_data(const char *devpath, void *buff, size_t size);
int swan_ts_read_data_main(int argc, char *argv[]);
int swan_ts_poll_data_main(int argc, char *argv[]);

int swan_ts_write_registers_fd(int fd, u16 addr, const void *buff, size_t size);
int swan_ts_write_registers(const char *devpath, u16 addr, const void *buff, size_t size);
int swan_ts_write_registers_main(int argc, char *argv[]);

int swan_ts_write_data_fd(int fd, const void *buff, size_t size);
int swan_ts_write_data(const char *devpath, const void *buff, size_t size);
int swan_ts_write_data_main(int argc, char *argv[]);

int swan_ts_get_client_address(const char *devpath, u16 *addr);
int swan_ts_get_client_address_main(int argc, char *argv[]);

int swan_ts_set_client_address(const char *devpath, u16 addr);
int swan_ts_set_client_address_main(int argc, char *argv[]);

int swan_ts_detect_clients_fd(int fd, u16 start, u16 end);
int swan_ts_detect_clients(const char *devpath, u16 start, u16 end);
int swan_ts_detect_clients_main(int argc, char *argv[]);

int swan_ts_test_client_fd(int fd, u16 addr);
int swan_ts_test_client(const char *devpath, u16 addr);
int swan_ts_test_client_main(int argc, char *argv[]);

int ft5406_firmware_upgrade_main(int argc, char *argv[]);

static inline int swan_ts_get_client_address_fd(int fd, u16 *addr)
{
	return ioctl(fd, SWAN_TS_IOCTL_GET_CLIENT_ADDRESS, addr);
}

static inline int swan_ts_set_client_address_fd(int fd, u16 addr)
{
	return ioctl(fd, SWAN_TS_IOCTL_SET_CLIENT_ADDRESS, &addr);
}

static inline int ft5406_upgrade_enter(int fd)
{
	return ioctl(fd, FT5406_IOCTL_UPGRADE_ENTER);
}

static inline int ft5406_upgrade_finish(int fd)
{
	return ioctl(fd, FT5406_IOCTL_UPGRADE_FINISH);
}

static inline int ft5406_erase_app_and_config(int fd)
{
	return ioctl(fd, FT5406_IOCTL_ERASE_APP_CONFIG);
}

static inline int ft5406_erase_app(int fd)
{
	return ioctl(fd, FT5406_IOCTL_ERASE_APP);
}

static inline int ft5406_erase_config(int fd)
{
	return ioctl(fd, FT5406_IOCTL_ERASE_CONFIG);
}

static inline int ft5406_erase_param(int fd)
{
	return ioctl(fd, FT5406_IOCTL_ERASE_PARAM);
}

static inline int ft5406_read_checksum(int fd, u8 *checksum)
{
	return ioctl(fd, FT5406_IOCTL_READ_CHECKSUM, checksum);
}

