#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Thu Nov 22 00:10:41 CST 2012
 */

#include <cavan.h>
#include <cavan/input.h>
#include <cavan/timer.h>

#define CAVAN_TOUCHPAD_EDGE_SHIFT	6

enum cavan_touchpad_state
{
	CAVAN_TOUCHPAD_STATE_IDEL,
	CAVAN_TOUCHPAD_STATE_DOWN1,
	CAVAN_TOUCHPAD_STATE_UP1,
	CAVAN_TOUCHPAD_STATE_DOWN2,
	CAVAN_TOUCHPAD_STATE_UP2
};

enum cavan_touchpad_mode
{
	CAVAN_TOUCHPAD_MODE_NONE,
	CAVAN_TOUCHPAD_MODE_MOVE,
	CAVAN_TOUCHPAD_MODE_WHEEL,
	CAVAN_TOUCHPAD_MODE_HWHEEL
};

struct cavan_touchpad_device
{
	struct cavan_input_device input_dev;
	struct cavan_timer timer;

	int x, xold;
	int y, yold;
	int right, bottom;
	int btn_code;
	float xspeed;
	float yspeed;

	enum cavan_touchpad_mode mode;
	enum cavan_touchpad_state state;
};

bool cavan_touchpad_device_match(uint8_t *key_bitmask, uint8_t *abs_bitmask);
bool cavan_touchpad_device_matcher(struct cavan_event_matcher *matcher, void *data);
struct cavan_input_device *cavan_touchpad_device_create(void);
