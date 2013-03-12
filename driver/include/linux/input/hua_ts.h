#pragma once

#include <linux/input/hua_input.h>
#include <linux/earlysuspend.h>

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

	u32 key_count;
	const struct hua_ts_touch_key *keys;

#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
};

struct hua_i2c_request
{
	int addr;
	int read;
	void *data;
	size_t size;
};

extern int sprd_ts_power_enable(bool enable);
extern void sprd_ts_reset_enable(bool enable);
extern void sprd_ts_irq_output(int value);

static inline void hua_ts_mt_touch_release(struct input_dev *input)
{
	input_event(input, EV_SYN, SYN_MT_REPORT, 0);
	input_event(input, EV_SYN, SYN_REPORT, 0);
}

static inline void hua_ts_report_mt_pressure(struct input_dev *input, int pressure)
{
	input_event(input, EV_ABS, ABS_MT_WIDTH_MAJOR, pressure);
	input_event(input, EV_ABS, ABS_MT_TOUCH_MAJOR, pressure);
}

static inline void hua_ts_report_mt_axis(struct input_dev *input, int x, int y)
{
	input_event(input, EV_ABS, ABS_MT_POSITION_X, x);
	input_event(input, EV_ABS, ABS_MT_POSITION_Y, y);
	input_event(input, EV_SYN, SYN_MT_REPORT, 0);
}

static inline void hua_ts_report_mt_axis2(struct input_dev *input, int id, int x, int y)
{
	input_event(input, EV_ABS, ABS_MT_TRACKING_ID, id);
	hua_ts_report_mt_axis(input, x, y);
}

static inline void hua_ts_report_mt_data(struct input_dev *input, int x, int y)
{
	input_event(input, EV_ABS, ABS_MT_WIDTH_MAJOR, 1);
	input_event(input, EV_ABS, ABS_MT_TOUCH_MAJOR, 1);
	hua_ts_report_mt_axis(input, x, y);
}

static inline void hua_ts_report_mt_data2(struct input_dev *input, int id, int x, int y)
{
	input_event(input, EV_ABS, ABS_MT_WIDTH_MAJOR, 1);
	input_event(input, EV_ABS, ABS_MT_TOUCH_MAJOR, 1);
	hua_ts_report_mt_axis2(input, id, x, y);
}
