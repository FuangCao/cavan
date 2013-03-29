#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Wed Mar 27 18:25:16 CST 2013
 */

#include <cavan.h>
#include <cavan/display.h>
#include <cavan/service.h>

#define CAVAN_SCREEN_MIRROR_PORT	1234

struct screen_mirror_info
{
	u32 xres, yres;
	u32 bytes_per_pixel;
};

struct screen_mirror_service
{
	struct cavan_service_description service;
	struct cavan_display_device display;
	struct screen_mirror_info info;
};

int screen_mirror_service_run(struct screen_mirror_service *service);
int screen_mirror_client(const char *buff, size_t size);
