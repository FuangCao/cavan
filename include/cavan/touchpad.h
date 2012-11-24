#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Thu Nov 22 00:10:41 CST 2012
 */

#include <cavan.h>
#include <cavan/input.h>

struct cavan_touchpad_device
{
	struct cavan_input_device input_dev;
	int x, xold;
	int y, yold;
	int pressed;
	int released;
	float xspeed;
	float yspeed;
	u32 time;
};

bool cavan_touchpad_device_match(uint8_t *key_bitmask, uint8_t *abs_bitmask);
bool cavan_touchpad_device_matcher(struct cavan_event_matcher *matcher, void *data);
struct cavan_input_device *cavan_touchpad_device_create(void);
