#pragma once

/*
 * File:		drm.h
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2019-09-23 10:47:02
 *
 * Copyright (c) 2019 Fuang.Cao <cavan.cfa@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifdef CONFIG_CAVAN_DRM
#include <cavan/display.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#define CAVAN_DRM_BPP		32
#define CAVAN_DRM_DEPTH		24

struct cavan_drm_fb {
	void *vaddr;
	u32 handle;
	u32 size;
	u32 id;
};

struct cavan_drm_device {
	int fd;
	drmModeCrtc *crtc;
	drmModeConnector *conn;
	struct cavan_drm_fb fbs[2];
	struct cavan_drm_fb *fb_acquired;
	struct cavan_drm_fb *fb_dequeued;
};

int cavan_drm_init(struct cavan_drm_device *dev);
void cavan_drm_deinit(struct cavan_drm_device *dev);
int cavan_drm_add_fb(struct cavan_drm_device *dev, struct cavan_drm_fb *fb);
void cavan_drm_rm_fb(struct cavan_drm_device *dev, struct cavan_drm_fb *fb);
int cavan_drm_refresh(struct cavan_drm_device *dev);

int cavan_drm_display_init(struct cavan_display_device *display, struct cavan_drm_device *drm);
struct cavan_display_device *cavan_drm_display_create(void);

static inline int cavan_drm_set_crtc(struct cavan_drm_device *dev, struct cavan_drm_fb *fb)
{
	return drmModeSetCrtc(dev->fd, dev->crtc->crtc_id, fb->id, 0, 0, &dev->conn->connector_id, 1, dev->conn->modes);
}

static inline int cavan_drm_page_flip(struct cavan_drm_device *dev, struct cavan_drm_fb *fb)
{
	return drmModePageFlip(dev->fd, dev->crtc->crtc_id, fb->id, DRM_MODE_PAGE_FLIP_EVENT, dev);
}

static inline int cavan_drm_xres(struct cavan_drm_device *dev)
{
	return dev->conn->modes[0].hdisplay;
}

static inline int cavan_drm_yres(struct cavan_drm_device *dev)
{
	return dev->conn->modes[0].vdisplay;
}
#endif
