#pragma once

#include <linux/input/hua_input.h>

#define HUA_INPUT_SENSOR_IOC_GET_MIN_DELAY		HUA_INPUT_IOC('S', 0x00, 0)
#define HUA_INPUT_SENSOR_IOC_GET_MAX_RANGE		HUA_INPUT_IOC('S', 0x01, 0)
#define HUA_INPUT_SENSOR_IOC_GET_RESOLUTION		HUA_INPUT_IOC('S', 0x02, 0)
#define HUA_INPUT_SENSOR_IOC_GET_POWER_CONSUME	HUA_INPUT_IOC('S', 0x03, 0)
#define HUA_INPUT_SENSOR_IOC_GET_XYZ			HUA_INPUT_IOC('S', 0x04, 0)
#define HUA_INPUT_SENSOR_IOC_SET_OFFSET			HUA_INPUT_IOC('S', 0x05, 0)
#define HUA_INPUT_SENSOR_IOC_CALIBRATION		HUA_INPUT_IOC('S', 0x06, 0)

enum hua_sensor_orientation
{
	HUA_SENSOR_ORIENTATION_UPWARD_0,
	HUA_SENSOR_ORIENTATION_UPWARD_90,
	HUA_SENSOR_ORIENTATION_UPWARD_180,
	HUA_SENSOR_ORIENTATION_UPWARD_270,
	HUA_SENSOR_ORIENTATION_DOWNWARD_0,
	HUA_SENSOR_ORIENTATION_DOWNWARD_90,
	HUA_SENSOR_ORIENTATION_DOWNWARD_180,
	HUA_SENSOR_ORIENTATION_DOWNWARD_270,
};

struct hua_sensor_vector
{
	int x;
	int y;
	int z;
};

struct hua_sensor_device
{
	struct hua_input_device dev;

	u32 min_delay;
	u32 max_range;
	u32 resolution;
	u32 power_consume;

	struct hua_sensor_vector data;
	struct hua_sensor_vector offset;
	enum hua_sensor_orientation orientation;

	void (*report_vector)(struct hua_sensor_device *sensor, int x, int y, int z);
};

static inline void hua_sensor_report_value(struct input_dev *input, int value)
{
	input_report_abs(input, ABS_MISC, value);
	input_sync(input);
}
