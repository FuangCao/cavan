#pragma once

#include <linux/input/hua_input.h>

struct hua_sensor_device
{
	struct hua_input_device dev;

	u32 min_delay;
	u32 max_range;
	u32 resolution;
	u32 power_consume;
};

#define HUA_INPUT_SENSOR_IOC_GET_MIN_DELAY		HUA_INPUT_IOC('S', 0x00, 0)
#define HUA_INPUT_SENSOR_IOC_GET_MAX_RANGE		HUA_INPUT_IOC('S', 0x01, 0)
#define HUA_INPUT_SENSOR_IOC_GET_RESOLUTION		HUA_INPUT_IOC('S', 0x02, 0)
#define HUA_INPUT_SENSOR_IOC_GET_POWER_CONSUME	HUA_INPUT_IOC('S', 0x03, 0)

static inline void hua_sensor_report_value(struct input_dev *input, int value)
{
	input_report_abs(input, ABS_MISC, value);
	input_sync(input);
}

static inline void hua_sensor_report_xaxis(struct input_dev *input, int x)
{
	input_report_abs(input, ABS_X, x);
	input_sync(input);
}

static inline void hua_sensor_report_yaxis(struct input_dev *input, int y)
{
	input_report_abs(input, ABS_Y, y);
	input_sync(input);
}

static inline void hua_sensor_report_zaxis(struct input_dev *input, int z)
{
	input_report_abs(input, ABS_Z, z);
	input_sync(input);
}

static inline void hua_sensor_report_vector(struct input_dev *input, int x, int y, int z)
{
	input_report_abs(input, ABS_X, x);
	input_report_abs(input, ABS_Y, y);
	input_report_abs(input, ABS_Z, z);
	input_sync(input);
}
