#pragma once

#include <linux/input.h>

#define CALIBRATION		_IO('E', 0x01)
#define TS_DEVICE_NAME	"Swan Touch Screen"

int touch_screen_calibration(void);
