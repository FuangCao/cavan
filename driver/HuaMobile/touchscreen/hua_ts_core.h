#pragma once

#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/gpio.h>
#include <linux/earlysuspend.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/firmware.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/wakelock.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/init.h>

#define HUAMOBILE_MAX_TOUCHSCREEN_COUNT		32

#define KB(size)	((size) << 10)
#define MB(size)	((size) << 20)
#define GB(size)	((size) << 30)

#define HUA_TS_IOCTL_GET_NAME(len)			_IOC(_IOC_READ, 0, 0x01, len)
#define HUA_TS_IOCTL_I2C_TRANFER			_IOWR(0, 0x02, struct huamobile_i2c_request)
#define HUA_TS_IOCTL_CALIBRATION			_IO(0, 0x03)
#define HUA_TS_IOCTL_LOCK					_IOW(0, 0x04, int)
#define HUA_TS_IOCTL_IRQ_ENABLE				_IOW(0, 0x05, int)
#define HUA_TS_IOCTL_SUSPEND_ENABLE			_IOW(0, 0x06, int)
#define HUA_TS_IOCTL_POWER_ENABLE			_IOW(0, 0x07, int)
#define HUA_TS_IOCTL_SET_FW_SIZE			_IOW(0, 0x08, int)
#define HUA_TS_IOCTL_RESET_ENABLE			_IOWR(0, 0x09, int)
#define HUA_TS_IOCTL_IRQ_OUTPUT				_IOWR(0, 0x0A, int)

#define BYTE_IS_LF(b) \
	((b) == '\n' || (b) == '\r')

#define BYTE_IS_SPACE(b) \
	((b) == ' ' || (b) == '\t' || BYTE_IS_LF(b))

#define BYTE_IS_NUMBER(b) \
	((b) >= '0' && (b) <= '9')

#define BYTE_IS_NUMBER16(b) \
	(BYTE_IS_NUMBER(b) || ((b) >= 'a' && (b) <= 'f') || ((b) >= 'A' && (b) <= 'F'))

#define pr_pos_info() \
	printk(KERN_INFO "%s => %s[%d]\n", __FILE__, __FUNCTION__, __LINE__)

#define pr_color_info(color, fmt, args ...) \
	printk(KERN_INFO "\033[" color "m%s[%d]: " fmt "\033[0m\n", __FUNCTION__, __LINE__, ##args)

#define pr_red_info(fmt, args ...) \
	pr_color_info("31", fmt, ##args)

#define pr_green_info(fmt, args ...) \
	pr_color_info("32", fmt, ##args)

#define pr_bold_info(fmt, args ...) \
	pr_color_info("1", fmt, ##args)

struct huamobile_ts_device;

enum huamobile_ts_state
{
	HUAMOBILE_TS_STATE_RUNNING,
	HUAMOBILE_TS_STATE_STOPED,
	HUAMOBILE_TS_STATE_STOPPING
};

struct huamobile_touch_key
{
	int code;
	int x;
	int y;
	int width;
	int height;
};

struct huamobile_ts_attribute
{
	struct attribute attr;
	char name[32];
	ssize_t (*show)(struct huamobile_ts_device *panel, struct huamobile_ts_attribute *attr, char *buff);
	ssize_t (*store)(struct huamobile_ts_device *panel, struct huamobile_ts_attribute *attr, const char *buff, size_t count);
};

struct huamobile_ts_kobject
{
	struct kobject kobj;
	const char *name;
	struct huamobile_ts_device *panel;
	int attr_count;
	struct huamobile_ts_attribute *attrs;
};

struct huamobile_ts_miscdevice
{
	struct file_operations fops;
	struct miscdevice misc;
	struct huamobile_ts_device *panel;
};

struct huamobile_ts_firmware
{
	size_t size;
	size_t max_size;
	void *data;
};

struct huamobile_ts_misc_data
{
	struct huamobile_ts_device *panel;
	struct huamobile_ts_firmware *fw;
};

struct huamobile_ts_device
{
	const char *name;
	size_t id_size;
	u8 dev_id[4];

	u32 xmax, xmin;
	u32 ymax, ymin;
	int point_count;

	struct i2c_client *client;
	unsigned long irq_flags;

	u32 key_count;
	const struct huamobile_touch_key *keys;

	bool is_poweron;
	bool is_suspend;
	size_t firmware_size;

	struct input_dev *input;
	struct mutex lock;
	struct wake_lock wake_lock;

	enum huamobile_ts_state state;
	struct completion event_completion;

#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif

	int (*readid)(struct huamobile_ts_device *ts);
	int (*firmware_upgrade)(struct huamobile_ts_device *ts, const void *buff, size_t size);
	int (*calibration)(struct huamobile_ts_device *ts);
	int (*ioctl)(struct huamobile_ts_device *ts, unsigned int command, unsigned long args);
	int (*event_loop)(struct huamobile_ts_device *ts);
	int (*power_enable)(struct huamobile_ts_device *ts, bool enable);
	int (*suspend_enable)(struct huamobile_ts_device *ts, bool enable);
};

struct huamobile_ts_core
{
	const char *name;

	struct mutex lock;
	struct task_struct *event_task;

	int panel_count;
	struct huamobile_ts_device *panels[HUAMOBILE_MAX_TOUCHSCREEN_COUNT];
};

struct huamobile_i2c_request
{
	int addr;
	int read;
	void *data;
	size_t size;
};

extern int sprd_ts_power_enable(bool enable);
extern void sprd_ts_reset_enable(bool enable);
extern void sprd_ts_irq_output(int value);

ssize_t huamobile_i2c_read_data(struct i2c_client *client, u8 addr, void *buff, size_t size);
ssize_t huamobile_i2c_write_data(struct i2c_client *client, u8 addr, const void *buff, size_t size);
int huamobile_i2c_test(struct i2c_client *client);
char *huamobile_print_memory(const void *mem, size_t size);
int huamobile_ts_register(struct huamobile_ts_device *ts);
int huamobile_ts_unregister(struct huamobile_ts_device *ts);

static inline ssize_t huamobile_i2c_read_register(struct i2c_client *client, u8 addr, u8 *value)
{
	return huamobile_i2c_read_data(client, addr, value, 1);
}

static inline ssize_t huamobile_i2c_write_register(struct i2c_client *client, u8 addr, u8 value)
{
	return huamobile_i2c_write_data(client, addr, &value, 1);
}

static inline int huamobile_ts_should_stop(struct huamobile_ts_device *ts)
{
	return ts->state != HUAMOBILE_TS_STATE_RUNNING;
}

static inline void huamobile_mt_touch_release(struct input_dev *input)
{
	input_event(input, EV_SYN, SYN_MT_REPORT, 0);
	input_event(input, EV_SYN, SYN_REPORT, 0);
}

static inline void huamobile_report_mt_axis(struct input_dev *input, int x, int y)
{
	input_event(input, EV_ABS, ABS_MT_POSITION_X, x);
	input_event(input, EV_ABS, ABS_MT_POSITION_Y, y);
}

static inline void huamobile_report_mt_axis2(struct input_dev *input, int id, int x, int y)
{
	input_report_abs(input, ABS_MT_TRACKING_ID, id);
	input_event(input, EV_ABS, ABS_MT_POSITION_X, x);
	input_event(input, EV_ABS, ABS_MT_POSITION_Y, y);
}

static inline void huamobile_report_mt_data(struct input_dev *input, int x, int y)
{
	input_event(input, EV_ABS, ABS_MT_WIDTH_MAJOR, 1);
	input_event(input, EV_ABS, ABS_MT_TOUCH_MAJOR, 1);
	input_event(input, EV_ABS, ABS_MT_POSITION_X, x);
	input_event(input, EV_ABS, ABS_MT_POSITION_Y, y);
	input_event(input, EV_SYN, SYN_MT_REPORT, 0);
}

static inline void huamobile_report_mt_data2(struct input_dev *input, int id, int x, int y)
{
	input_event(input, EV_ABS, ABS_MT_WIDTH_MAJOR, 1);
	input_event(input, EV_ABS, ABS_MT_TOUCH_MAJOR, 1);
	input_report_abs(input, ABS_MT_TRACKING_ID, id);
	input_event(input, EV_ABS, ABS_MT_POSITION_X, x);
	input_event(input, EV_ABS, ABS_MT_POSITION_Y, y);
	input_event(input, EV_SYN, SYN_MT_REPORT, 0);
}

static inline void huamobile_report_mt_pressure(struct input_dev *input, int pressure)
{
	input_event(input, EV_ABS, ABS_MT_WIDTH_MAJOR, pressure);
	input_event(input, EV_ABS, ABS_MT_TOUCH_MAJOR, pressure);
}
