#pragma once

#include <huamobile/hua_input.h>

#define HUA_SENSOR_EVENT_VALUE	ABS_MISC
#define HUA_SENSOR_EVENT_Z		ABS_Z

#ifndef CONFIG_HUAMOBILE_GSENSOR_EXCHANGE_XY
#define HUA_SENSOR_EVENT_X		ABS_X
#define HUA_SENSOR_EVENT_Y		ABS_Y
#else
#define HUA_SENSOR_EVENT_X		ABS_Y
#define HUA_SENSOR_EVENT_Y		ABS_X
#endif

#define HUA_INPUT_SENSOR_IOC_GET_MIN_DELAY		HUA_INPUT_IOC('S', 0x00, 0)
#define HUA_INPUT_SENSOR_IOC_GET_MAX_RANGE		HUA_INPUT_IOC('S', 0x01, 0)
#define HUA_INPUT_SENSOR_IOC_GET_RESOLUTION		HUA_INPUT_IOC('S', 0x02, 0)
#define HUA_INPUT_SENSOR_IOC_GET_POWER_CONSUME	HUA_INPUT_IOC('S', 0x03, 0)
#define HUA_INPUT_SENSOR_IOC_GET_AXIS_COUNT		HUA_INPUT_IOC('S', 0x04, 0)

struct hua_sensor_device
{
	struct hua_input_device dev;

	u32 axis_count;
	u32 min_delay;
	u32 max_range;
	u32 resolution;
	u32 power_consume;
};

static inline void hua_sensor_report_value(struct input_dev *input, int value)
{
	input_report_abs(input, HUA_SENSOR_EVENT_VALUE, value);
	input_sync(input);
}

static inline void hua_sensor_report_xaxis(struct input_dev *input, int x)
{
	input_report_abs(input, HUA_SENSOR_EVENT_X, x * CONFIG_HUAMOBILE_GSENSOR_SIGN_X);
	input_sync(input);
}

static inline void hua_sensor_report_yaxis(struct input_dev *input, int y)
{
	input_report_abs(input, HUA_SENSOR_EVENT_Y, y * CONFIG_HUAMOBILE_GSENSOR_SIGN_Y);
	input_sync(input);
}

static inline void hua_sensor_report_zaxis(struct input_dev *input, int z)
{
	input_report_abs(input, HUA_SENSOR_EVENT_Z, z * CONFIG_HUAMOBILE_GSENSOR_SIGN_Z);
	input_sync(input);
}

static inline void hua_sensor_report_vector(struct input_dev *input, int x, int y, int z)
{
	input_report_abs(input, HUA_SENSOR_EVENT_X, x * CONFIG_HUAMOBILE_GSENSOR_SIGN_X);
	input_report_abs(input, HUA_SENSOR_EVENT_Y, y * CONFIG_HUAMOBILE_GSENSOR_SIGN_Y);
	input_report_abs(input, HUA_SENSOR_EVENT_Z, z * CONFIG_HUAMOBILE_GSENSOR_SIGN_Z);
	input_sync(input);
}
