#include <cavan.h>
#include <cavan/fb.h>
#include <cavan/calculator.h>
#include <math.h>

// Fuang.Cao <cavan.cfa@gmail.com> 2011-11-16 15:48:51

void show_bitfield(struct fb_bitfield *field, const char *msg)
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
	println("var->bits_per_pixel = %d", var->bits_per_pixel);
	show_bitfield(&var->red, "red fb_bitfield:");
	show_bitfield(&var->green, "green fb_bitfield:");
	show_bitfield(&var->blue, "blue fb_bitfield:");
	show_bitfield(&var->transp, "transp fb_bitfield:");
}

void show_fb_fix_info(struct fb_fix_screeninfo *fix)
{
	print_sep(60);
	println("smem_start = 0x%08lx", fix->smem_start);
	println("smem_len = 0x%08x", fix->smem_len);
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
	(((u8 *)dev->fb_base) + y * dev->xres)[x] = color;
}

static void cavan_fb_draw_point16(struct cavan_fb_device *dev,int x, int y, u32 color)
{
	(((u16 *)dev->fb_base) + y * dev->xres)[x] = color;
}

static void cavan_fb_draw_point24(struct cavan_fb_device *dev, int x, int y, u32 color)
{
	u8 *p;

	p = ((u8 *)dev->fb_base) + (y * dev->xres + x) * 3;
	p[dev->red.index] = (color & dev->red.mask) >> dev->red.offset;
	p[dev->green.index] = (color & dev->green.mask) >> dev->green.offset;
	p[dev->blue.index] = (color & dev->blue.mask) >> dev->blue.offset;
}

static void cavan_fb_draw_point32(struct cavan_fb_device * dev, int x, int y, u32 color)
{
	(((u32 *)dev->fb_base) + y * dev->xres)[x] = color;
}

int cavan_fb_init(struct cavan_fb_device *dev, const char *fbpath)
{
	int ret;
	int fb;

	if (fbpath == NULL)
	{
		fbpath = "/dev/fb0";
	}

	fb = open(fbpath, O_RDWR);
	if (fb < 0)
	{
		print_error("open device %s failed", fbpath);
		return fb;
	}

	dev->fb = fb;

	ret = ioctl(fb, FBIOGET_VSCREENINFO, &dev->var_info);
	if (ret < 0)
	{
		print_error("get screen var info failed");
		goto out_close_fb;
	}

	show_fb_var_info(&dev->var_info);

	dev->xres = dev->var_info.xres;
	dev->yres = dev->var_info.yres;

	switch (dev->var_info.bits_per_pixel)
	{
	case 8:
		dev->byte_per_pixel = 1;
		dev->draw_point = cavan_fb_draw_point8;
		break;
	case 16:
		dev->byte_per_pixel = 2;
		dev->draw_point = cavan_fb_draw_point16;
		break;
	case 24:
		dev->byte_per_pixel = 3;
		dev->draw_point = cavan_fb_draw_point24;
		break;
	case 32:
		dev->byte_per_pixel = 4;
		dev->draw_point = cavan_fb_draw_point32;
		break;
	default:
		error_msg("unsported bits_per_pixel: %d", dev->var_info.bits_per_pixel);
		ret = -EINVAL;
		goto out_close_fb;
	}

	dev->line_size = dev->byte_per_pixel * dev->xres;
	dev->fb_size = dev->line_size * dev->yres;

	ret = ioctl(fb, FBIOGET_FSCREENINFO, &dev->fix_info);
	if (ret < 0)
	{
		print_error("get screen fix info failed");
		goto out_close_fb;
	}

	show_fb_fix_info(&dev->fix_info);

	dev->fb_base = mmap(NULL, dev->fix_info.smem_len, PROT_WRITE | PROT_READ, MAP_SHARED, fb, 0);
	if (dev->fb_base == NULL)
	{
		print_error("map framebuffer failed");
		ret = -1;
		goto out_close_fb;
	}

	cavan_fb_bitfield2element(&dev->var_info.red, &dev->red);
	cavan_fb_bitfield2element(&dev->var_info.green, &dev->green);
	cavan_fb_bitfield2element(&dev->var_info.blue, &dev->blue);
	cavan_fb_bitfield2element(&dev->var_info.transp, &dev->transp);

	cavan_fb_set_pen_color3f(dev, 1.0, 1.0, 1.0, 1.0);

	return 0;

out_close_fb:
	close(fb);

	return ret;
}

void cavan_fb_uninit(struct cavan_fb_device *dev)
{
	munmap(dev->fb_base, dev->fix_info.smem_len);
	close(dev->fb);
}

void cavan_fb_clear(struct cavan_fb_device *dev)
{
	mem_set32(dev->fb_base, dev->pen_color.value, dev->fb_size);
}

int cavan_build_line_equation(int x1, int y1, int x2, int y2, double *a, double *b)
{
	int x_diff;

	// println("x1 = %d, y1 = %d, x2 = %d, y2 = %d", x1, y1, x2, y2);

	x_diff = x2 - x1;
	if (x_diff > -5 && x_diff < 5)
	{
		*a = 0;
		*b = 0;
		return -EINVAL;
	}

	*a = ((double)(y2 - y1)) / x_diff;
	*b = y1 - *a * x1;

	return 0;
}

static int cavan_fb_draw_line_horizon(struct cavan_fb_device *dev, int x1, int y1, int x2, int y2)
{
	double a, b;
	void (*draw_point_handle)(struct cavan_fb_device *, int, int, u32);
	u32 color;

	draw_point_handle = dev->draw_point;
	color = dev->pen_color.value;

	if (x1 == x2)
	{
		if (y1 < y2)
		{
			while (y1 <= y2)
			{
				draw_point_handle(dev, x1, y1, color);
				y1++;
			}
		}
		else
		{
			while (y1 >= y2)
			{
				draw_point_handle(dev, x1, y1, color);
				y1--;
			}
		}

		return 0;
	}

	cavan_build_line_equation(x1, y1, x2, y2, &a, &b);

	if (x1 < x2)
	{
		while (x1 <= x2)
		{
			draw_point_handle(dev, x1, a * x1 + b, color);
			x1++;
		}
	}
	else
	{
		while (x1 >= x2)
		{
			draw_point_handle(dev, x1, a * x1 + b, color);
			x1--;
		}
	}

	return 0;
}

static int cavan_fb_draw_line_vertical(struct cavan_fb_device *dev, int x1, int y1, int x2, int y2)
{
	double a, b;
	void (*draw_point_handle)(struct cavan_fb_device *, int, int, u32);
	u32 color;

	draw_point_handle = dev->draw_point;
	color = dev->pen_color.value;

	if (y1 == y2)
	{
		if (x1 < x2)
		{
			while (x1 <= x2)
			{
				draw_point_handle(dev, x1, y1, color);
				x1++;
			}
		}
		else
		{
			while (x1 >= x2)
			{
				draw_point_handle(dev, x1, y1, color);
				x1--;
			}
		}

		return 0;
	}

	cavan_build_line_equation(y1, x1, y2, x2, &a, &b);

	if (y1 < y2)
	{
		while (y1 <= y2)
		{
			draw_point_handle(dev, a * y1 + b, y1, color);
			y1++;
		}
	}
	else
	{
		while (y1 >= y2)
		{
			draw_point_handle(dev, a * y1 + b, y1, color);
			y1--;
		}
	}

	return 0;
}

int cavan_fb_draw_line(struct cavan_fb_device *dev, int x1, int y1, int x2, int y2)
{
	int ret;

	if (x1 < 0 || x1 >= dev->xres || x2 < 0 || x2 >= dev->xres)
	{
		return -EINVAL;
	}

	if (y1 < 0 || y1 >= dev->yres || y2 < 0 || y2 >= dev->yres)
	{
		return -EINVAL;
	}

	ret = cavan_fb_draw_line_horizon(dev, x1, y1, x2, y2);

	return ret < 0 ? ret : cavan_fb_draw_line_vertical(dev, x1, y1, x2, y2);
}

int cavan_fb_draw_rect(struct cavan_fb_device *dev, int left, int top, int width, int height)
{
	int i;
	int right, bottom;
	void (*draw_point_handle)(struct cavan_fb_device *, int, int, u32);
	u32 color;

	right = left + width - 1;
	if (right >= dev->xres)
	{
		right = dev->xres - 1;
	}

	if (left < 0)
	{
		left = 0;
	}

	bottom = top + height - 1;
	if (bottom >= dev->yres)
	{
		bottom = dev->yres - 1;
	}

	if (top < 0)
	{
		top = 0;
	}

	draw_point_handle = dev->draw_point;
	color = dev->pen_color.value;

	for (i = left; i <= right; i++)
	{
		draw_point_handle(dev, i, top, color);
		draw_point_handle(dev, i, bottom, color);
	}

	for (i = top; i <= bottom; i++)
	{
		draw_point_handle(dev, left, i, color);
		draw_point_handle(dev, right, i, color);
	}

	return 0;
}

int cavan_fb_fill_rect(struct cavan_fb_device *dev, int left, int top, int width, int height)
{
	int right, bottom;
	int x, y;
	void (*draw_point_handle)(struct cavan_fb_device *, int, int, u32);
	u32 color;

	right = left + width - 1;
	if (right >= dev->xres)
	{
		right = dev->xres - 1;
	}

	if (left < 0)
	{
		left = 0;
	}

	bottom = top + height - 1;
	if (bottom >= dev->yres)
	{
		bottom = dev->yres - 1;
	}

	if (top < 0)
	{
		top = 0;
	}

	draw_point_handle = dev->draw_point;
	color = dev->pen_color.value;

	for (y = top; y <= bottom; y++)
	{
		for (x = left; x <= right; x++)
		{
			draw_point_handle(dev, x, y, color);
		}
	}

	return 0;
}

int cavan_fb_draw_circle(struct cavan_fb_device *dev, int x, int y, int r)
{
	int rr;
	int i;
	int tmp;
	void (*draw_point_handle)(struct cavan_fb_device *, int, int, u32);
	u32 color;

	if (x - r < 0 || x + r >= dev->xres || y - r < 0 || y + r >= dev->yres)
	{
		return -EINVAL;
	}

	draw_point_handle = dev->draw_point;
	rr = r * r;
	color = dev->pen_color.value;

	for (i = 0; i < r; i++)
	{
		tmp = sqrt(rr - i * i);

		draw_point_handle(dev, x + i, y + tmp, color);
		draw_point_handle(dev, x + i, y - tmp, color);
		draw_point_handle(dev, x - i, y + tmp, color);
		draw_point_handle(dev, x - i, y - tmp, color);

		draw_point_handle(dev, x + tmp, y + i, color);
		draw_point_handle(dev, x + tmp, y - i, color);
		draw_point_handle(dev, x - tmp, y + i, color);
		draw_point_handle(dev, x - tmp, y - i, color);
	}

	return 0;
}

int cavan_fb_fill_circle(struct cavan_fb_device *dev, int x, int y, int r)
{
	int rr;
	int i;
	int tmp, left, right, top, bottom;
	void (*draw_point_handle)(struct cavan_fb_device *, int, int, u32);
	u32 color;

	if (x - r < 0 || x + r >= dev->xres || y - r < 0 || y + r >= dev->yres)
	{
		return -EINVAL;
	}

	draw_point_handle = dev->draw_point;
	rr = r * r;
	color = dev->pen_color.value;

	for (i = 0; i < r; i++)
	{
		tmp = sqrt(rr - i * i);

		for (left = x - i, right = x + i, top = y + tmp, bottom = y - tmp; top <= bottom; top++)
		{
			draw_point_handle(dev, left, top, color);
			draw_point_handle(dev, right, top, color);
		}

		for (left = x - tmp, right = x + tmp, top = y - i, bottom = y + i; left <= right; left++)
		{
			draw_point_handle(dev, left, top, color);
			draw_point_handle(dev, left, bottom, color);
		}
	}

	return 0;
}

int cavan_fb_draw_ellipse(struct cavan_fb_device *dev, int x, int y, int width, int height)
{
	double aa, bb;
	int tmp;
	int i;
	void (*draw_point_handle)(struct cavan_fb_device *, int, int, u32);
	u32 color;

	aa = ((double)width) / 2;
	bb = ((double)height) / 2;

	if (x - aa < 0 || x + aa >= dev->xres || y - bb < 0 || y + bb >= dev->yres)
	{
		return -EINVAL;
	}

	aa *= aa;
	bb *= bb;
	draw_point_handle = dev->draw_point;
	color = dev->pen_color.value;

	for (i = width >> 1; i >= 0; i--)
	{
		tmp = sqrt(bb - (bb * i * i / aa));

		draw_point_handle(dev, x + i, y + tmp, color);
		draw_point_handle(dev, x + i, y - tmp, color);
		draw_point_handle(dev, x - i, y + tmp, color);
		draw_point_handle(dev, x - i, y - tmp, color);
	}

	for (i = height >> 1; i >= 0; i--)
	{
		tmp = sqrt(aa - (aa * i * i / bb));

		draw_point_handle(dev, x + tmp, y + i, color);
		draw_point_handle(dev, x + tmp, y - i, color);
		draw_point_handle(dev, x - tmp, y + i, color);
		draw_point_handle(dev, x - tmp, y - i, color);
	}

	return 0;
}

int cavan_fb_fill_ellipse(struct cavan_fb_device *dev, int x, int y, int width, int height)
{
	double aa, bb;
	int tmp, left, right, top, bottom;
	int i;
	void (*draw_point_handle)(struct cavan_fb_device *, int, int, u32);
	u32 color;

	aa = ((double)width) / 2;
	bb = ((double)height) / 2;

	if (x - aa < 0 || x + aa >= dev->xres || y - bb < 0 || y + bb >= dev->yres)
	{
		return -EINVAL;
	}

	aa *= aa;
	bb *= bb;
	draw_point_handle = dev->draw_point;
	color = dev->pen_color.value;

	for (i = width >> 1; i >= 0; i--)
	{
		tmp = sqrt(bb - (bb * i * i / aa));

		for (left = x - i, right = x + i, top = y - tmp, bottom = y + tmp; top <= bottom; top++)
		{
			draw_point_handle(dev, left, y + tmp, color);
			draw_point_handle(dev, right, y + tmp, color);
		}
	}

	for (i = height >> 1; i >= 0; i--)
	{
		tmp = sqrt(aa - (aa * i * i / bb));

		for (left = x - tmp, right = x + tmp, top = y - i, bottom = y + i; left <= right; left++)
		{
			draw_point_handle(dev, left, top, color);
			draw_point_handle(dev, left, bottom, color);
		}
	}

	return 0;
}

int cavan_fb_draw_polygon(struct cavan_fb_device *dev, cavan_display_point_t *points, size_t count)
{
	unsigned int i;
	int ret;

	if (count < 3)
	{
		return -EINVAL;
	}

	for (i = 0, count--; i < count; i++)
	{
		ret = cavan_fb_draw_line(dev, points[i].x, points[i].y, points[i + 1].x, points[i + 1].y);
		if (ret < 0)
		{
			return ret;
		}
	}

	return cavan_fb_draw_line(dev, points[0].x, points[0].y, points[count].x, points[count].y);
}

int max3i(int a, int b, int c)
{
	if (b > a)
	{
		a = b;
	}

	return c > a ? c : a;
}

int min3i(int a, int b, int c)
{
	if (b < a)
	{
		a = b;
	}

	return c < a ? c : a;
}

void cavan_fb_show_points(const cavan_display_point_t *points, size_t size)
{
	const cavan_display_point_t *end;

	for (end = points + size; points < end; points++)
	{
		println("[%d, %d]", points->x, points->y);
	}
}

void cavan_fb_point_sort_x(cavan_display_point_t *start, cavan_display_point_t *end)
{
	cavan_display_point_t mid, *start_bak, *end_bak;

	if (start >= end)
	{
		return;
	}

	mid = *start;
	start_bak = start;
	end_bak = end;

	while (1)
	{
		for (; start < end && mid.x <= end->x; end--);
		if (start < end)
		{
			*start++ = *end;
		}
		else
		{
			break;
		}

		for (; start < end && mid.x >= start->x; start++);
		if (start < end)
		{
			*end-- = *start;
		}
		else
		{
			break;
		}
	}

	*start = mid;

	if (start_bak < start)
	{
		cavan_fb_point_sort_x(start_bak, start - 1);
	}

	if (end_bak > end)
	{
		cavan_fb_point_sort_x(end + 1, end_bak);
	}
}

static int cavan_fb_fill_triangle_half(struct cavan_fb_device *dev, cavan_display_point_t *p1, cavan_display_point_t *p2, double a1, double b1, double a2, double b2)
{
	int left, right, top, bottom;
	void (*draw_point_handle)(struct cavan_fb_device *, int, int, u32);
	u32 color;

	// println("left = %d, right = %d", left, right);
	// println("a1 = %lf, b1 = %lf, a2 = %lf, b2 = %lf", a1, b1, a2, b2);

	draw_point_handle = dev->draw_point;
	left = p1->x;
	right = p2->x;
	color = dev->pen_color.value;

	if ((a1 == 0 && b1 == 0) || (a2 == 0 && b2 == 0))
	{
		if (p1->y < p2->y)
		{
			top = p1->y;
			bottom = p2->y;
		}
		else
		{
			top = p2->y;
			bottom = p1->y;
		}

		while (left <= right)
		{
			int i;

			for (i = top; i <= bottom; i++)
			{
				draw_point_handle(dev, left, i, color);
			}

			left++;
		}

		return 0;
	}

	while (left <= right)
	{
		top = a1 * left + b1;
		bottom = a2 * left + b2;

		while (top <= bottom)
		{
			draw_point_handle(dev, left, top, color);
			top++;
		}

		left++;
	}

	return 0;
}

int cavan_fb_fill_triangle(struct cavan_fb_device *dev, cavan_display_point_t *points)
{
	double a[3], b[3];
	cavan_display_point_t *p0, *p1, *p2;

	if (points[0].x < points[1].x)
	{
		p0 = points;
		p1 = points + 1;
	}
	else
	{
		p0 = points + 1;
		p1 = points;
	}

	if (points[2].x < p0->x)
	{
		p2 = p1;
		p1 = p0;
		p0 = points + 2;
	}
	else if (points[2].x < p1->x)
	{
		p2 = p1;
		p1 = points + 2;
	}
	else
	{
		p2 = points + 2;
	}

	cavan_build_line_equation(p0->x, p0->y, p1->x, p1->y, a, b);
	cavan_build_line_equation(p0->x, p0->y, p2->x, p2->y, a + 1, b + 1);
	cavan_build_line_equation(p2->x, p2->y, p1->x, p1->y, a + 2, b + 2);

	if (a[1] == 0 && b[1] == 0)
	{
		return -EINVAL;
	}

	if (p1->y < (p1->x * a[1] + b[1]))
	{
		// pr_bold_pos();
		cavan_fb_fill_triangle_half(dev, p0, p1, a[0], b[0], a[1], b[1]);
		cavan_fb_fill_triangle_half(dev, p1, p2, a[2], b[2], a[1], b[1]);
	}
	else
	{
		// pr_bold_pos();
		cavan_fb_fill_triangle_half(dev, p0, p1, a[1], b[1], a[0], b[0]);
		cavan_fb_fill_triangle_half(dev, p1, p2, a[1], b[1], a[2], b[2]);
	}

	return 0;
}

int cavan_fb_fill_polygon(struct cavan_fb_device *dev, cavan_display_point_t *points, size_t count)
{
	int ret;
	cavan_display_point_t *p, *p_end;
	cavan_display_point_t point_buff[3];

	if (count < 3)
	{
		return -EINVAL;
	}

	for (p = points + 1, p_end = p + count - 2; p < p_end; p++)
	{
		point_buff[0] = points[0];
		point_buff[1] = p[0];
		point_buff[2] = p[1];

		ret = cavan_fb_fill_triangle(dev, point_buff);
		if (ret < 0)
		{
			return ret;
		}
	}

	return 0;
}

static int cavan_build_polygon_points(struct cavan_fb_device *dev, cavan_display_point_t *points, size_t count, int x, int y, int r, int rotation)
{
	double angle, avg_angle;
	cavan_display_point_t *point_end;

	if (count < 3)
	{
		return -EINVAL;
	}

	avg_angle = PI * 2 / count;

	for (point_end = points + count, angle = (PI * rotation) / 180 ; points < point_end; points++, angle += avg_angle)
	{
		points->x = x + r * cos(angle);
		points->y = y + r * sin(angle);
	}

	return 0;
}

int cavan_fb_draw_polygon_standard(struct cavan_fb_device *dev, size_t count, int x, int y, int r, int rotation)
{
	int ret;
	cavan_display_point_t points[count];

	ret = cavan_build_polygon_points(dev, points, count, x, y, r, rotation);

	return ret < 0 ? ret : cavan_fb_draw_polygon(dev, points, count);
}

int cavan_fb_fill_polygon_standard(struct cavan_fb_device *dev, size_t count, int x, int y, int r, int rotation)
{
	int ret;
	cavan_display_point_t points[count];

	ret = cavan_build_polygon_points(dev, points, count, x, y, r, rotation);

	return ret < 0 ? ret : cavan_fb_fill_polygon(dev, points, count);
}

int cavan_fb_draw_polygon_standard2(struct cavan_fb_device *dev, size_t count, int x, int y, int r, int rotation)
{
	int ret;
	cavan_display_point_t points[count];
	cavan_display_point_t *p1, *p2, *end_p;

	ret = cavan_build_polygon_points(dev, points, count, x, y, r, rotation);
	if (ret < 0)
	{
		return ret;
	}

	for (p1 = points, end_p = points + count; p1 < end_p; p1++)
	{
		for (p2 = points; p2 < p1; p2++)
		{
			ret = cavan_fb_draw_line(dev, p1->x, p1->y, p2->x, p2->y);
			if (ret < 0)
			{
				return ret;
			}
		}

		for (p2 = p1 + 1; p2 < end_p; p2++)
		{
			ret = cavan_fb_draw_line(dev, p1->x, p1->y, p2->x, p2->y);
			if (ret < 0)
			{
				return ret;
			}
		}
	}

	return 0;
}

int cavan_calculate_line_cross_point(int x1, int x2, double a1, double b1, double a2, double b2, cavan_display_point_t *point)
{
	if (a1 == a2)
	{
		return -EINVAL;
	}

	if (a1 == 0 && b1 == 0)
	{
		point->x = x1;
		point->y = a2 * x1 + b2;
	}
	else if (a2 == 0 && b2 == 0)
	{
		point->x = x2;
		point->y = a1 * x2 + b1;
	}
	else
	{
		point->x = (b1 - b2) / (a2 - a1);
		point->y = a1 * point->x + b1;
	}

	return 0;
}

int cavan_calculate_polygo_cross_points(cavan_display_point_t *points, cavan_display_point_t *cross_points, size_t count)
{
	size_t i;
	int ret;
	cavan_display_point_t *p0, *p1, *p2, *p3;
	double a1, a2, b1, b2;

	if (count < 5)
	{
		return -EINVAL;
	}

	for (i = 0; i < count; i++)
	{
		p0 = points + i;
		p2 = points + (i + 2) % count;
		cavan_build_line_equation(p0->x, p0->y, p2->x, p2->y, &a1, &b1);

		p1 = points + (i + 1) % count;
		p3 = points + (i + 3) % count;
		cavan_build_line_equation(p1->x, p1->y, p3->x, p3->y, &a2, &b2);

		ret = cavan_calculate_line_cross_point(p0->x, p1->x, a1, b1, a2, b2, cross_points + i);
		if (ret < 0)
		{
			return ret;
		}
	}

	return 0;
}

int cavan_fb_fill_polygon_standard2(struct cavan_fb_device *dev, size_t count, int x, int y, int r, int rotation)
{
	int ret;
	size_t i;
	cavan_display_point_t points[count];
	cavan_display_point_t cross_points[count];
	cavan_display_point_t triangle_points[3];

	if (count < 5)
	{
		return cavan_fb_fill_polygon_standard(dev, count, x, y, r, rotation);
	}

	ret = cavan_build_polygon_points(dev, points, count, x, y, r, rotation);
	if (ret < 0)
	{
		return ret;
	}

	ret = cavan_calculate_polygo_cross_points(points, cross_points, count);
	if (ret < 0)
	{
		return ret;
	}

	for (i = 0; i < count; i++)
	{
		triangle_points[0] = cross_points[i];
		triangle_points[1] = cross_points[(i + 1) % count];
		triangle_points[2] = points[(i + 2) % count];

		cavan_fb_fill_triangle(dev, triangle_points);
	}

	return 0;
}

int cavan_fb_draw_polygon_standard3(struct cavan_fb_device *dev, size_t count, int x, int y, int r, int rotation)
{
	int ret;
	cavan_display_point_t points[count];
	size_t i, j;

	ret = cavan_build_polygon_points(dev, points, count, x, y, r, rotation);
	if (ret < 0)
	{
		return ret;
	}

	for (i = 0; i < count; i++)
	{
		j = (i + 2) % count;

		ret = cavan_fb_draw_line(dev, points[i].x, points[i].y, points[j].x, points[j].y);
		if (ret < 0)
		{
			return ret;
		}
	}

	return 0;
}

int cavan_fb_fill_polygon_standard3(struct cavan_fb_device *dev, size_t count, int x, int y, int r, int rotation)
{
	int ret;
	unsigned int i;
	cavan_display_point_t points[count];
	cavan_display_point_t cross_points[count];

	if (count < 5)
	{
		return cavan_fb_fill_polygon_standard(dev, count, x, y, r, rotation);
	}

	ret = cavan_build_polygon_points(dev, points, count, x, y, r, rotation);
	if (ret < 0)
	{
		return ret;
	}

	ret = cavan_calculate_polygo_cross_points(points, cross_points, count);
	if (ret < 0)
	{
		return ret;
	}

	cavan_fb_fill_polygon(dev, cross_points, count);

	for (i = 0; i < count; i++)
	{
		cavan_display_point_t triangle_points[3];

		triangle_points[0] = cross_points[i];
		triangle_points[1] = cross_points[(i + 1) % count];
		triangle_points[2] = points[(i + 2) % count];

		cavan_fb_fill_triangle(dev, triangle_points);
	}

	return 0;
}

int cavan_fb_draw_polygon_standard4(struct cavan_fb_device *dev, size_t count, int x, int y, int r, int rotation)
{
	int ret;

	ret = cavan_fb_draw_polygon_standard3(dev, count, x, y, r, rotation);
	if (ret < 0)
	{
		return ret;
	}

	return cavan_fb_draw_circle(dev, x, y, r);
}

int cavan_fb_display_memory_xfer(struct cavan_fb_device *dev, struct cavan_display_memory *mem, bool read)
{
	char *data;
	size_t width;
	size_t line_size;
	char *p, *p_end;
	int right, bottom;

	right = mem->x + mem->width;
	if (right > dev->xres)
	{
		right = dev->xres;
	}

	if (mem->x < 0)
	{
		mem->x = 0;
	}

	bottom = mem->y + mem->height;
	if (bottom > dev->yres)
	{
		bottom = dev->yres;
	}

	if (mem->y < 0)
	{
		mem->y = 0;
	}

	if (mem->x >= right || mem->y >= bottom)
	{
		return 0;
	}

	mem->width = right - mem->x;
	mem->height = bottom - mem->y;

	width = mem->width * dev->byte_per_pixel;
	line_size = dev->line_size;

	p = (char *)dev->fb_base + mem->y * line_size + mem->x * dev->byte_per_pixel;
	p_end = p + mem->height * line_size;

	data = mem->data;

	if (read)
	{
		while (p < p_end)
		{
			mem_copy(data, p, width);

			data += width;
			p += line_size;
		}
	}
	else
	{
		while (p < p_end)
		{
			mem_copy(p, data, width);

			data += width;
			p += line_size;
		}
	}

	return 0;
}
