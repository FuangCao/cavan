/*
 * File:		drm.c
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
#include <cavan.h>
#include <cavan/drm.h>

int cavan_drm_init(struct cavan_drm_device *dev)
{
	drmModeRes *res;
	int ret;

	dev->fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
	if (dev->fd < 0) {
		pr_err_info("open: %d", dev->fd);
		return dev->fd;
	}

	res = drmModeGetResources(dev->fd);
	if (res == NULL) {
		pr_err_info("drmModeGetResources");
		ret = -EFAULT;
		goto out_close_fd;
	}

	println("count_crtcs = %d", res->count_crtcs);

	if (res->count_crtcs <= 0) {
		ret = -EINVAL;
		goto out_free_res;
	}

	dev->crtc = drmModeGetCrtc(dev->fd, res->crtcs[0]);
	if (dev->crtc == NULL) {
		pr_err_info("drmModeGetCrtc");
		ret = -EFAULT;
		goto out_free_res;
	}

	println("count_connectors = %d", res->count_connectors);

	if (res->count_connectors <= 0) {
		ret = -EINVAL;
		goto out_free_crtc;
	}

	dev->conn = drmModeGetConnector(dev->fd, res->connectors[0]);
	if (dev->conn == NULL) {
		pr_err_info("drmModeGetConnector");
		ret = -EINVAL;
		goto out_free_crtc;
	}

	println("count_modes = %d", dev->conn->count_modes);

	if (dev->conn->count_modes <= 0) {
		ret = -EINVAL;
		goto out_free_conn;
	}

	dev->fb_acquired = dev->fbs;
	dev->fb_dequeued = dev->fbs + 1;

	ret = cavan_drm_add_fb(dev, dev->fb_acquired);
	if (ret < 0) {
		pr_err_info("cavan_drm_add_fb: %d", ret);
		goto out_free_conn;
	}

	ret = cavan_drm_add_fb(dev, dev->fb_dequeued);
	if (ret < 0) {
		pr_err_info("cavan_drm_add_fb: %d", ret);
		goto out_rm_fb_acquired;
	}

	cavan_drm_set_crtc(dev, dev->fb_acquired);
	drmModeFreeResources(res);

	return 0;

#if 0
out_rm_fb_dequeued:
	cavan_drm_rm_fb(dev, dev->fb_dequeued);
#endif
out_rm_fb_acquired:
	cavan_drm_rm_fb(dev, dev->fb_acquired);
out_free_conn:
	drmModeFreeConnector(dev->conn);
out_free_crtc:
	drmModeFreeCrtc(dev->crtc);
out_free_res:
	drmModeFreeResources(res);
out_close_fd:
	close(dev->fd);
	return ret;
}

void cavan_drm_deinit(struct cavan_drm_device *dev)
{
	drmModeSetCrtc(dev->fd, dev->crtc->crtc_id, 0, 0, 0, NULL, 0, NULL);
	cavan_drm_rm_fb(dev, dev->fb_acquired);
	cavan_drm_rm_fb(dev, dev->fb_dequeued);
	drmModeFreeConnector(dev->conn);
	drmModeFreeCrtc(dev->crtc);
	close(dev->fd);
}

int cavan_drm_add_fb(struct cavan_drm_device *dev, struct cavan_drm_fb *fb)
{
	struct drm_mode_destroy_dumb destroy;
	struct drm_mode_create_dumb create;
	struct drm_mode_map_dumb map;
	int ret;

	/* create a dumb-buffer, the pixel format is XRGB888 */
	create.bpp = CAVAN_DRM_BPP;
	create.width = cavan_drm_xres(dev);
	create.height = cavan_drm_yres(dev);
	ret = drmIoctl(dev->fd, DRM_IOCTL_MODE_CREATE_DUMB, &create);
	if (ret < 0) {
		pr_err_info("drmIoctl: %d", ret);
		return ret;
	}

	/* bind the dumb-buffer to an FB object */
	ret = drmModeAddFB(dev->fd, create.width, create.height, CAVAN_DRM_DEPTH, create.bpp, create.pitch, create.handle, &fb->id);
	if (ret < 0) {
		pr_err_info("drmModeAddFB: %d", ret);
		goto out_destory_dumb;
	}

	/* map the dumb-buffer to userspace */
	map.handle = create.handle;
	ret = drmIoctl(dev->fd, DRM_IOCTL_MODE_MAP_DUMB, &map);
	if (ret < 0) {
		pr_err_info("drmIoctl: %d", ret);
		goto out_rm_fb;
	}

	fb->vaddr = mmap(0, create.size, PROT_READ | PROT_WRITE, MAP_SHARED, dev->fd, map.offset);
	if (fb->vaddr == MAP_FAILED) {
		pr_err_info("mmap");
		ret = -EFAULT;
		goto out_rm_fb;
	}

	fb->handle = create.handle;
	fb->size = create.size;

	return 0;

out_rm_fb:
	drmModeRmFB(dev->fd, fb->id);
out_destory_dumb:
	destroy.handle = create.handle;
	drmIoctl(dev->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy);
	return ret;
}

void cavan_drm_rm_fb(struct cavan_drm_device *dev, struct cavan_drm_fb *fb)
{
	struct drm_mode_destroy_dumb destroy;

	munmap(fb->vaddr, fb->size);
	drmModeRmFB(dev->fd, fb->id);

	destroy.handle = fb->handle;
	drmIoctl(dev->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy);
}

int cavan_drm_refresh(struct cavan_drm_device *dev)
{
	struct cavan_drm_fb *fb = dev->fb_dequeued;
	int ret;

	ret = cavan_drm_page_flip(dev, fb);
	if (ret < 0) {
		pr_err_info("cavan_drm_page_flip");
		return ret;
	}

	dev->fb_dequeued = dev->fb_acquired;
	dev->fb_acquired = fb;

	return 0;
}

// ================================================================================

static void *cavan_drm_display_get_dequeued_buff(struct cavan_display_device *display)
{
	struct cavan_drm_device *drm = (struct cavan_drm_device *) display->private_data;
	return drm->fb_dequeued->vaddr;
}

static void *cavan_drm_display_get_acquired_buff(struct cavan_display_device *display)
{
	struct cavan_drm_device *drm = (struct cavan_drm_device *) display->private_data;
	return drm->fb_acquired->vaddr;
}

static void cavan_drm_display_destroy_handler1(struct cavan_display_device *display)
{
	struct cavan_drm_device *drm = (struct cavan_drm_device *) display->private_data;
	cavan_display_deinit(display);
	cavan_drm_deinit(drm);
}

static void cavan_drm_display_destroy_handler2(struct cavan_display_device *display)
{
	cavan_drm_display_destroy_handler1(display);
	free(display);
}

static void cavan_drm_display_refresh_handler(struct cavan_display_device *display)
{
	struct cavan_drm_device *drm = (struct cavan_drm_device *) display->private_data;
	cavan_drm_refresh(drm);
}

static cavan_display_color_t cavan_drm_display_build_color_handler(struct cavan_display_device *display, float red, float green, float blue, float transp)
{
	cavan_display_color_t color;

	color.red = blue * 255;
	color.green = green * 255;
	color.blue = red * 255;
	color.transp = transp * 255;

	return color;
}

static void cavan_drm_display_draw_point_handler(struct cavan_display_device *display, int x, int y, cavan_display_color_t color)
{
	struct cavan_drm_device *drm = (struct cavan_drm_device *) display->private_data;
	u32 *fb = (u32 *) drm->fb_dequeued->vaddr;
	fb[y * display->xres + x] = color.value;
}

int cavan_drm_display_init(struct cavan_display_device *display, struct cavan_drm_device *drm)
{
	int ret;

	ret = cavan_drm_init(drm);
	if (ret < 0) {
		pr_err_info("cavan_drm_init: %d", ret);
		return ret;
	}

	cavan_display_init(display);

	display->private_data = drm;
	display->xres = cavan_drm_xres(drm);
	display->yres = cavan_drm_yres(drm);
	display->bpp_byte = CAVAN_DRM_BPP;

	display->get_dequeued_buff = cavan_drm_display_get_dequeued_buff;
	display->get_acquired_buff = cavan_drm_display_get_acquired_buff;
	display->destroy = cavan_drm_display_destroy_handler1;
	display->refresh = cavan_drm_display_refresh_handler;
	display->build_color = cavan_drm_display_build_color_handler;
	display->draw_point = cavan_drm_display_draw_point_handler;

	return 0;
}

struct cavan_display_device *cavan_drm_display_create(void)
{
	struct cavan_display_device *display;
	int ret;

	display = malloc(sizeof(struct cavan_display_device) + sizeof(struct cavan_drm_device));
	if (display == NULL) {
		return NULL;
	}

	ret = cavan_drm_display_init(display, (struct cavan_drm_device *) (display + 1));
	if (ret < 0) {
		pr_err_info("cavan_drm_init: %d", ret);
		goto out_free;
	}

	display->destroy = cavan_drm_display_destroy_handler2;

	return display;

out_free:
	free(display);
	return NULL;
}
#endif
