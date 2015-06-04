#pragma once

#include <cavan/cavan_input.h>
#include <cavan/cavan_sysfs.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#elif defined(CONFIG_FB)
#include <linux/notifier.h>
#include <linux/fb.h>
#endif

struct cavan_ts_touch_key
{
	int code;
	int x;
	int y;
	int width;
	int height;
};

struct cavan_ts_device
{
	struct cavan_input_device dev;

	u32 xmax, xmin;
	u32 ymax, ymin;
	u32 pmax, pmin;
	int point_count;
	int touch_count;

	u32 key_count;
	struct cavan_ts_touch_key *keys;

#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#else
#if defined(CONFIG_FB) && defined(CONFIG_CAVAN_USE_FB_NOTIFILER)
	struct notifier_block fb_notifier;
#endif
	struct notifier_block pm_notifier;
#endif
};

struct cavan_i2c_request
{
	int addr;
	int read;
	void *data;
	size_t size;
};

static inline int cavan_ts_read_pending_firmware_name(char *buff, size_t size)
{
	return cavan_io_read_write_file("/data/property/persist.sys.tp.fw.pending", buff, size, false);
}

static inline void cavan_ts_mt_touch_release(struct input_dev *input)
{
	input_event(input, EV_SYN, SYN_MT_REPORT, 0);
	input_event(input, EV_KEY, BTN_TOUCH, 0);
	input_event(input, EV_SYN, SYN_REPORT, 0);
}

static inline void cavan_ts_report_mt_data(struct input_dev *input, int x, int y)
{
	input_event(input, EV_ABS, ABS_MT_POSITION_X, x);
	input_event(input, EV_ABS, ABS_MT_POSITION_Y, y);
	input_event(input, EV_SYN, SYN_MT_REPORT, 0);
	input_event(input, EV_KEY, BTN_TOUCH, 1);
}

static inline void cavan_ts_report_mt_data2(struct input_dev *input, int id, int x, int y)
{
	input_event(input, EV_ABS, ABS_MT_TRACKING_ID, id);
	cavan_ts_report_mt_data(input, x, y);
}
