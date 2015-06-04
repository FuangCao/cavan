#pragma once

#include <cavan/cavan_input.h>

#define CAVAN_SENSOR_EVENT_VALUE	ABS_MISC
#define CAVAN_SENSOR_EVENT_Z		ABS_Z

#ifndef CONFIG_CAVAN_GSENSOR_EXCHANGE_XY
#define CAVAN_SENSOR_EVENT_X		ABS_X
#define CAVAN_SENSOR_EVENT_Y		ABS_Y
#else
#define CAVAN_SENSOR_EVENT_X		ABS_Y
#define CAVAN_SENSOR_EVENT_Y		ABS_X
#endif

#define CAVAN_INPUT_SENSOR_IOC_GET_MIN_DELAY		CAVAN_INPUT_IOC('S', 0x00, 0)
#define CAVAN_INPUT_SENSOR_IOC_GET_MAX_RANGE		CAVAN_INPUT_IOC('S', 0x01, 0)
#define CAVAN_INPUT_SENSOR_IOC_GET_RESOLUTION		CAVAN_INPUT_IOC('S', 0x02, 0)
#define CAVAN_INPUT_SENSOR_IOC_GET_POWER_CONSUME	CAVAN_INPUT_IOC('S', 0x03, 0)
#define CAVAN_INPUT_SENSOR_IOC_GET_AXIS_COUNT		CAVAN_INPUT_IOC('S', 0x04, 0)

struct cavan_sensor_device
{
	struct cavan_input_device dev;

	u32 axis_count;
	u32 max_range;
	u32 resolution;
	u32 power_consume;
};

struct cavan_sensor_rate_table_node
{
	u8 value;
	u32 delay_ns;
};

const struct cavan_sensor_rate_table_node *cavan_sensor_find_rate_value(const struct cavan_sensor_rate_table_node *table, size_t count, u32 delay_ns);

static inline void cavan_sensor_report_value(struct input_dev *input, int value)
{
	input_report_abs(input, CAVAN_SENSOR_EVENT_VALUE, value);
	input_sync(input);
}

static inline void cavan_sensor_report_xaxis(struct input_dev *input, int x)
{
	input_report_abs(input, CAVAN_SENSOR_EVENT_X, x * CONFIG_CAVAN_GSENSOR_SIGN_X);
	input_sync(input);
}

static inline void cavan_sensor_report_yaxis(struct input_dev *input, int y)
{
	input_report_abs(input, CAVAN_SENSOR_EVENT_Y, y * CONFIG_CAVAN_GSENSOR_SIGN_Y);
	input_sync(input);
}

static inline void cavan_sensor_report_zaxis(struct input_dev *input, int z)
{
	input_report_abs(input, CAVAN_SENSOR_EVENT_Z, z * CONFIG_CAVAN_GSENSOR_SIGN_Z);
	input_sync(input);
}

static inline void cavan_sensor_report_vector(struct input_dev *input, int x, int y, int z)
{
	input_report_abs(input, CAVAN_SENSOR_EVENT_X, x * CONFIG_CAVAN_GSENSOR_SIGN_X);
	input_report_abs(input, CAVAN_SENSOR_EVENT_Y, y * CONFIG_CAVAN_GSENSOR_SIGN_Y);
	input_report_abs(input, CAVAN_SENSOR_EVENT_Z, z * CONFIG_CAVAN_GSENSOR_SIGN_Z);
	input_sync(input);
}
