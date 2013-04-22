#include <cavan.h>
#include <cavan/fb.h>

// Fuang.Cao <cavan.cfa@gmail.com> 2011-11-16 15:48:51

void show_fb_bitfield(struct fb_bitfield *field, const char *msg)
{
	print_sep(60);
	if (msg)
	{
		print_string(msg);
	}
	println("field->offset = %d", field->offset);
	println("field->length = %d", field->length);
	println("field->msb_right = %d", field->msb_right);
}

void show_fb_var_info(struct fb_var_screeninfo *var)
{
	print_sep(60);
	println("var->xres = %d, var->xres_virtual = %d", var->xres, var->xres_virtual);
	println("var->yres = %d, var->yres_virtual = %d", var->yres, var->yres_virtual);
	println("var->xoffset = %d, var->yoffset = %d", var->xoffset, var->yoffset);
	println("var->bits_per_pixel = %d", var->bits_per_pixel);
	show_fb_bitfield(&var->red, "red fb_bitfield:");
	show_fb_bitfield(&var->green, "green fb_bitfield:");
	show_fb_bitfield(&var->blue, "blue fb_bitfield:");
	show_fb_bitfield(&var->transp, "transp fb_bitfield:");
}

void show_fb_fix_info(struct fb_fix_screeninfo *fix)
{
	print_sep(60);
	println("smem_start = 0x%08lx", fix->smem_start);
	println("smem_len = 0x%08x", fix->smem_len);
}

void show_fb_device_info(struct cavan_fb_device *dev)
{
	print_sep(60);
	println("fb_count = %d", dev->fb_count);
#if __WORDSIZE == 64
	println("fb_size = %ld", dev->fb_size);
	println("bpp_byte = %ld", dev->bpp_byte);
	println("line_size = %ld", dev->line_size);
#else
	println("fb_size = %d", dev->fb_size);
	println("bpp_byte = %d", dev->bpp_byte);
	println("line_size = %d", dev->line_size);
#endif
}

void cavan_fb_bitfield2element(struct fb_bitfield *field, struct cavan_fb_color_element *emt)
{
	emt->offset = field->offset;
	emt->max = (1 << field->length) - 1;
	emt->mask = emt->max << emt->offset;
	emt->index = emt->offset >> 3;
}

static void cavan_fb_draw_point8(struct cavan_fb_device *dev,int x, int y, u32 color)
{
	(((u8 *)dev->fb_cache) + y * dev->xres)[x] = color;
}

static void cavan_fb_draw_point16(struct cavan_fb_device *dev,int x, int y, u32 color)
{
	(((u16 *)dev->fb_cache) + y * dev->xres)[x] = color;
}

static void cavan_fb_draw_point24(struct cavan_fb_device *dev, int x, int y, u32 color)
{
	u8 *p;

	p = ((u8 *)dev->fb_cache) + (y * dev->xres + x) * 3;
	p[dev->red.index] = (color & dev->red.mask) >> dev->red.offset;
	p[dev->green.index] = (color & dev->green.mask) >> dev->green.offset;
	p[dev->blue.index] = (color & dev->blue.mask) >> dev->blue.offset;
}

static void cavan_fb_draw_point32(struct cavan_fb_device *dev, int x, int y, u32 color)
{
	(((u32 *)dev->fb_cache) + y * dev->xres)[x] = color;
}

int cavan_fb_refresh(struct cavan_fb_device *dev)
{
	int ret;
	int index = (dev->fb_active + 1) % dev->fb_count;
	struct fb_var_screeninfo *var = &dev->var_info;

	mem_copy((byte *)dev->fb_base + dev->fb_size * index, dev->fb_cache, dev->fb_size);

	if (dev->fb_active == index)
	{
		return 0;
	}

	var->yoffset = index * dev->yres;

	ret = ioctl(dev->fb, FBIOPUT_VSCREENINFO, var);
	if (ret < 0)
	{
		pr_error_info("ioctl FBIOPUT_VSCREENINFO");
		return ret;
	}

	dev->fb_active = index;

	return 0;
}

int cavan_fb_init(struct cavan_fb_device *dev, const char *fbpath)
{
	int ret;
	int fb;
	struct fb_fix_screeninfo *fix = &dev->fix_info;
	struct fb_var_screeninfo *var = &dev->var_info;

	if (fbpath)
	{
		fb = try_to_open(O_RDWR, fbpath, "/dev/fb0", "/dev/graphics/fb0", "/dev/fb1", "/dev/graphics/fb1", NULL);
	}
	else
	{
		fb = try_to_open(O_RDWR, "/dev/fb0", "/dev/graphics/fb0", "/dev/fb1", "/dev/graphics/fb1", NULL);
	}

	if (fb < 0)
	{
		print_error("open fb device failed");
		return fb;
	}

	dev->fb = fb;

	ret = ioctl(fb, FBIOGET_VSCREENINFO, var);
	if (ret < 0)
	{
		print_error("get screen var info failed");
		goto out_close_fb;
	}

	show_fb_var_info(var);

	dev->xres = var->xres;
	dev->yres = var->yres;

	switch (var->bits_per_pixel)
	{
	case 8:
		dev->bpp_byte = 1;
		dev->draw_point = cavan_fb_draw_point8;
		break;
	case 16:
		dev->bpp_byte = 2;
		dev->draw_point = cavan_fb_draw_point16;
		break;
	case 24:
		dev->bpp_byte = 3;
		dev->draw_point = cavan_fb_draw_point24;
		break;
	case 32:
		dev->bpp_byte = 4;
		dev->draw_point = cavan_fb_draw_point32;
		break;
	default:
		error_msg("unsported bits_per_pixel: %d", var->bits_per_pixel);
		ret = -EINVAL;
		goto out_close_fb;
	}

	dev->line_size = dev->bpp_byte * dev->xres;
	dev->fb_size = dev->line_size * dev->yres;
	dev->fb_count = var->yres_virtual / var->yres;
	dev->fb_active = var->yoffset / var->yres;

	ret = ioctl(fb, FBIOGET_FSCREENINFO, fix);
	if (ret < 0)
	{
		print_error("get screen fix info failed");
		goto out_close_fb;
	}

	show_fb_fix_info(fix);

	dev->fb_base = mmap(NULL, fix->smem_len, PROT_WRITE | PROT_READ, MAP_SHARED, fb, 0);
	if (dev->fb_base == NULL)
	{
		print_error("map framebuffer failed");
		ret = -1;
		goto out_close_fb;
	}

	dev->fb_cache = malloc(dev->fb_size);
	if (dev->fb_cache == NULL)
	{
		pr_error_info("malloc");
		goto out_munmap;
	}

	cavan_fb_bitfield2element(&var->red, &dev->red);
	cavan_fb_bitfield2element(&var->green, &dev->green);
	cavan_fb_bitfield2element(&var->blue, &dev->blue);
	cavan_fb_bitfield2element(&var->transp, &dev->transp);

	show_fb_device_info(dev);

	cavan_fb_refresh(dev);

	return 0;

out_munmap:
	munmap(dev->fb_base, dev->fix_info.smem_len);
out_close_fb:
	close(fb);

	return ret;
}

void cavan_fb_uninit(struct cavan_fb_device *dev)
{
	free(dev->fb_cache);
	munmap(dev->fb_base, dev->fix_info.smem_len);
	close(dev->fb);
}

// ================================================================================

static cavan_display_color_t cavan_fb_display_build_color_handler(struct cavan_display_device *display, float red, float green, float blue, float transp)
{
	return cavan_fb_build_color4f(display->private_data, red, green, blue, transp);
}

static void cavan_fb_display_refresh_handler(struct cavan_display_device *display)
{
	cavan_fb_refresh(display->private_data);
}

static void cavan_fb_display_draw_point_handler(struct cavan_display_device *display, int x, int y, cavan_display_color_t color)
{
	cavan_fb_draw_point(display->private_data, x, y, color);
}

static void cavan_fb_display_destory_handler1(struct cavan_display_device *display)
{
	cavan_fb_uninit(display->private_data);
	cavan_display_destory_dummy(display);
}

static void cavan_fb_display_destory_handler2(struct cavan_display_device *display)
{
	cavan_fb_display_destory_handler1(display);
	free(display);
}

int cavan_fb_display_init(struct cavan_display_device *display, struct cavan_fb_device *fb_dev)
{
	int ret;

	ret = cavan_fb_init(fb_dev, NULL);
	if (ret < 0)
	{
		pr_red_info("cavan_fb_init");
		return ret;
	}

	cavan_display_init(display);

	display->fb_base = fb_dev->fb_cache;

	display->private_data = fb_dev;
	display->xres= fb_dev->xres;
	display->yres = fb_dev->yres;
	display->bpp_byte = fb_dev->bpp_byte;

	display->destory = cavan_fb_display_destory_handler1;
	display->refresh = cavan_fb_display_refresh_handler;
	display->build_color = cavan_fb_display_build_color_handler;
	display->draw_point = cavan_fb_display_draw_point_handler;

	return 0;
}

struct cavan_display_device *cavan_fb_display_create(void)
{
	int ret;
	struct cavan_display_device *display;
	struct cavan_fb_device *fb_dev;

	display = malloc(sizeof(*display) + sizeof(*fb_dev));
	if (display == NULL)
	{
		pr_error_info("malloc");
		return NULL;
	}

	fb_dev = (struct cavan_fb_device *)(display + 1);
	ret = cavan_fb_display_init(display, fb_dev);
	if (ret < 0)
	{
		pr_red_info("cavan_fb_display_init");
		free(display);
		return NULL;
	}

	display->destory = cavan_fb_display_destory_handler2;

	return display;
}

struct cavan_display_device *cavan_fb_display_create_check(void)
{
	int ret;
	struct cavan_display_device *display;

	display = cavan_fb_display_create();
	if (display == NULL)
	{
		pr_red_info("cavan_fb_display_create");
		return NULL;
	}

	ret = cavan_display_check(display);
	if (ret < 0)
	{
		pr_red_info("cavan_display_check");
		display->destory(display);
		return NULL;
	}

	return display;
}
