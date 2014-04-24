#pragma once

#include <huamobile/hua_input.h>
#include <huamobile/hua_sysfs.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#elif defined(CONFIG_FB)
#include <linux/notifier.h>
#include <linux/fb.h>
#endif

struct hua_ts_touch_key
{
	int code;
	int x;
	int y;
	int width;
	int height;
};

struct hua_ts_device
{
	struct hua_input_device dev;

	u32 xmax, xmin;
	u32 ymax, ymin;
	u32 pmax, pmin;
	int point_count;
	int touch_count;

	u32 key_count;
	const struct hua_ts_touch_key *keys;

#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#else
#if defined(CONFIG_FB) && defined(CONFIG_HUAMOBILE_USE_FB_NOTIFILER)
	struct notifier_block fb_notifier;
#endif
	struct notifier_block pm_notifier;
#endif
};

struct hua_i2c_request
{
	int addr;
	int read;
	void *data;
	size_t size;
};

int hua_ts_read_pending_firmware_name(char *buff, size_t size);

static inline void hua_ts_mt_touch_release(struct input_dev *input)
{
	input_event(input, EV_SYN, SYN_MT_REPORT, 0);
	input_event(input, EV_KEY, BTN_TOUCH, 0);
	input_event(input, EV_SYN, SYN_REPORT, 0);
}

static inline void hua_ts_report_mt_data(struct input_dev *input, int x, int y)
{
	input_event(input, EV_ABS, ABS_MT_POSITION_X, x);
	input_event(input, EV_ABS, ABS_MT_POSITION_Y, y);
	input_event(input, EV_SYN, SYN_MT_REPORT, 0);
	input_event(input, EV_KEY, BTN_TOUCH, 1);
}

static inline void hua_ts_report_mt_data2(struct input_dev *input, int id, int x, int y)
{
	input_event(input, EV_ABS, ABS_MT_TRACKING_ID, id);
	hua_ts_report_mt_data(input, x, y);
}
