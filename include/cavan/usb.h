#pragma once

// Fuang.Cao <cavan.cfa@gmail.com> Wed Sep  7 09:54:06 CST 2011

#include <linux/usbdevice_fs.h>
#include <pthread.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 20)
#include <linux/usb/ch9.h>
#else
#include <linux/usb_ch9.h>
#endif

#define CAVAN_USB_DEBUG				0
#define CAVAN_USB_MAX_XFER_SIZE		4096

#ifndef USB_MAXENDPOINTS
#define USB_MAXENDPOINTS			32
#endif
#define USB_DEVICE_DIR				"/dev/bus/usb"

#define DEVICE_ADB_ENABLE_PATH	"/dev/android_adb_enable"
#define DEVICE_ADB_PATH			"/dev/android_adb"

struct cavan_usb_interface_descriptor
{
	struct usb_interface_descriptor if_desc;
	struct usb_endpoint_descriptor ep_descs[USB_MAXENDPOINTS];
};

struct cavan_usb_descriptor
{
	int fd;
	int if_count, if_curr;
	int epin_curr, epout_curr;
	char serial[256];
	char dev_path[256];

	pthread_mutex_t lock;
	pthread_t thread_notify;
	pthread_cond_t notify_read, notify_write;
	struct usbdevfs_urb urb_read, urb_write;

	struct usb_device_descriptor dev_desc;
	struct usb_config_descriptor cfg_descs[10];
	struct cavan_usb_interface_descriptor if_descs[10];
};

struct swan_adb_client_descriptor
{
	int pipefd[2];
	struct cavan_usb_descriptor *usb_desc;
};

enum cavan_usb_package_type
{
	CAVAN_UMSG_UNKNOWN,
	CAVAN_UMSG_DATA_STREAM,
	CAVAN_UMSG_WRITE_FILE,
	CAVAN_UMSG_READ_FILE,
	CAVAN_UMSG_ACK,
	CAVAN_UMSG_ERROR,
};

#pragma pack(1)
struct cavan_usb_data_header
{
	u16 data_length;
	u16 data_check;
};

#if 0
struct cavan_usb_data_option
{
	u32 blk_num;
	u32 data_length;
	u32 data_check;
};

struct cavan_usb_ack_option
{
	u32 blk_num;
};

struct cavan_usb_error_option
{
	u32 err_no;
};

struct cavan_usb_message
{
	u32 op_code;
	u32 op_check;

	union
	{
		struct cavan_usb_data_option data_opt;
		struct cavan_usb_ack_option ack_opt;
		struct cavan_usb_error_option err_opt;
	};
};

struct cavan_usb_file_descriptor
{
	u32 bs;
	u32 seek;
	u32 count;
	struct stat st;
	u8 pathname[1024];
};

struct cavan_usb_package
{
	struct cavan_usb_message msg;

	union
	{
		struct cavan_usb_file_descriptor file_desc;
		u8 data[CAVAN_USB_MAX_DATA_LENGTH];
	};
};
#endif
#pragma pack()

int dump_cavan_usb_descriptor(const void *buff, struct cavan_usb_descriptor *desc, size_t length);
int fusb_read_cavan_descriptor(int fd, struct cavan_usb_descriptor *desc);
int usb_read_cavan_descriptor(const char *dev_path, struct cavan_usb_descriptor *desc);

const char *usb_endpoint_xfertype_tostring(const struct usb_endpoint_descriptor *desc);
void show_usb_device_descriptor(const struct usb_device_descriptor *desc);
void show_usb_config_descriptor(const struct usb_config_descriptor *desc);
void show_usb_interface_descriptor(const struct usb_interface_descriptor *desc);
void show_usb_endpoint_descriptor(const struct usb_endpoint_descriptor *desc);
void show_cavan_usb_descriptor(const struct cavan_usb_descriptor *desc);

int cavan_usb_init(const char *dev_path, struct cavan_usb_descriptor *desc);
int cavan_find_usb_device(const char *dev_path, struct cavan_usb_descriptor *desc);
void cavan_usb_uninit(struct cavan_usb_descriptor *desc);

int cavan_usb_bluk_rw(struct cavan_usb_descriptor *desc, void *buff, size_t length, int read);
int cavan_usb_bluk_read(struct cavan_usb_descriptor *desc, void *buff, size_t length);
int cavan_usb_bluk_write(struct cavan_usb_descriptor *desc, const void *buff, size_t length);

int cavan_usb_bluk_xfer(struct cavan_usb_descriptor *desc, void *buff, size_t length, int ep);
int cavan_usb_bluk_read2(struct cavan_usb_descriptor *desc, void *buff, size_t length);
int cavan_usb_bluk_write2(struct cavan_usb_descriptor *desc, const void *buff, size_t length);

ssize_t cavan_usb_read_data(struct cavan_usb_descriptor *desc, void *buff, size_t size);
ssize_t cavan_usb_write_data(struct cavan_usb_descriptor *desc, const void *buff, size_t size);
ssize_t cavan_adb_read_data(int fd_adb, void *buff, size_t size);
ssize_t cavan_adb_write_data(int fd_adb, const void *buff, size_t size);

static inline int is_usb_device_descriptor(const struct usb_device_descriptor *desc)
{
	return desc->bDescriptorType == USB_DT_DEVICE && desc->bLength == USB_DT_DEVICE_SIZE;
}

static inline int is_usb_config_descriptor(const struct usb_config_descriptor *desc)
{
	return desc->bDescriptorType == USB_DT_CONFIG && desc->bLength == USB_DT_CONFIG_SIZE;
}

static inline int is_usb_interface_descriptor(const struct usb_interface_descriptor *desc)
{
	return desc->bDescriptorType == USB_DT_INTERFACE &&  desc->bLength == USB_DT_INTERFACE_SIZE;
}

static inline int is_usb_endpoint_descriptor(const struct usb_endpoint_descriptor *desc)
{
	return desc->bDescriptorType == USB_DT_ENDPOINT && desc->bLength == USB_DT_ENDPOINT_SIZE;
}

static inline int is_usb_string_descriptor(const struct usb_string_descriptor *desc)
{
	return desc->bDescriptorType == USB_DT_STRING;
}

static inline void cavan_cond_signal(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
	pthread_mutex_lock(mutex);
	pthread_cond_signal(cond);
	pthread_mutex_unlock(mutex);
}

static inline void cavan_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
	pthread_mutex_lock(mutex);
	pthread_cond_wait(cond, mutex);
	pthread_mutex_unlock(mutex);
}

static inline void cavan_cond_broadcast(pthread_cond_t *cond)
{
	pthread_cond_broadcast(cond);
}

