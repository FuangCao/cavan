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
	println("var->xres = %d", var->xres);
	println("var->yres = %d", var->yres);
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

void cavan_bitfield2element(struct fb_bitfield *field, struct cavan_color_element *emt)
{
	emt->offset = field->offset;
	emt->max = (1 << field->length) - 1;
	emt->mask = emt->max << emt->offset;
	emt->index = emt->offset >> 3;
}

static void cavan_draw_point8(struct cavan_screen_descriptor * desc,int x, int y, u32 color)
{
	u8 *p1, *p2, *end1, *end2;

	p1 = ((u8 *)desc->fb_base) + y * desc->xres + x;

	for (end1 = p1 + (desc->xres * desc->bordersize); p1 < end1; p1 += desc->xres)
	{
		for (p2 = p1, end2 = p2 + desc->bordersize; p2 < end2; p2++)
		{
			*p2 = color;
		}
	}
}

static void cavan_draw_point16(struct cavan_screen_descriptor * desc,int x, int y, u32 color)
{
	u16 *p1, *p2, *end1, *end2;

	p1 = ((u16 *)desc->fb_base) + y * desc->xres + x;

	for (end1 = p1 + (desc->xres * desc->bordersize); p1 < end1; p1 += desc->xres)
	{
		for (p2 = p1, end2 = p2 + desc->bordersize; p2 < end2; p2++)
		{
			*p2 = color;
		}
	}
}

static void cavan_draw_point24(struct cavan_screen_descriptor *desc, int x, int y, u32 color)
{
	u8 *p1, *p2, *end1, *end2;
	int line_size;

	p1 = ((u8 *)desc->fb_base) + (y * desc->xres + x) * 3;
	line_size = desc->xres * 3;

	for (end1 = p1 + (line_size * desc->bordersize); p1 < end1; p1 += line_size)
	{
		for (p2 = p1, end2 = p2 + desc->bordersize * 3; p2 < end2; p2 += 3)
		{
			p2[desc->red.index] = (color & desc->red.mask) >> desc->red.offset;
			p2[desc->green.index] = (color & desc->green.mask) >> desc->green.offset;
			p2[desc->blue.index] = (color & desc->blue.mask) >> desc->blue.offset;
		}
	}
}

static void cavan_draw_point32(struct cavan_screen_descriptor * desc, int x, int y, u32 color)
{
	u32 *p1, *p2, *end1, *end2;

	p1 = ((u32 *)desc->fb_base) + y * desc->xres + x;

	for (end1 = p1 + (desc->xres * desc->bordersize); p1 < end1; p1 += desc->xres)
	{
		for (p2 = p1, end2 = p2 + desc->bordersize; p2 < end2; p2++)
		{
			*p2 = color;
		}
	}
}

int cavan_fb_init(struct cavan_screen_descriptor *desc, const char *fbpath)
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

	desc->fb = fb;

	ret = ioctl(fb, FBIOGET_VSCREENINFO, &desc->var_info);
	if (ret < 0)
	{
		print_error("get screen var info failed");
		goto out_close_fb;
	}

	show_fb_var_info(&desc->var_info);

	desc->xres = desc->var_info.xres;
	desc->yres = desc->var_info.yres;

	switch (desc->var_info.bits_per_pixel)
	{
	case 8:
		desc->line_size = desc->xres;
		desc->draw_point = cavan_draw_point8;
		break;
	case 16:
		desc->line_size = desc->xres * 2;
		desc->draw_point = cavan_draw_point16;
		break;
	case 24:
		desc->line_size = desc->xres * 3;
		desc->draw_point = cavan_draw_point24;
		break;
	case 32:
		desc->line_size = desc->xres * 4;
		desc->draw_point = cavan_draw_point32;
		break;
	default:
		error_msg("unsported bits_per_pixel: %d", desc->var_info.bits_per_pixel);
		ret = -EINVAL;
		goto out_close_fb;
	}

	desc->fb_size = desc->line_size * desc->yres;

	ret = ioctl(fb, FBIOGET_FSCREENINFO, &desc->fix_info);
	if (ret < 0)
	{
		print_error("get screen fix info failed");
		goto out_close_fb;
	}

	show_fb_fix_info(&desc->fix_info);

	desc->fb_base = mmap(NULL, desc->fix_info.smem_len, PROT_WRITE | PROT_READ, MAP_SHARED, fb, 0);
	if (desc->fb_base == NULL)
	{
		print_error("map framebuffer failed");
		ret = -1;
		goto out_close_fb;
	}

	cavan_bitfield2element(&desc->var_info.red, &desc->red);
	cavan_bitfield2element(&desc->var_info.green, &desc->green);
	cavan_bitfield2element(&desc->var_info.blue, &desc->blue);
	cavan_bitfield2element(&desc->var_info.transp, &desc->transp);

	cavan_set_background3f(desc, 0, 0, 0);
	cavan_set_foreground3f(desc, 1.0, 1.0, 1.0);
	cavan_set_bordercolor3f(desc, 1.0, 0, 0);
	desc->bordersize = 1;

	return 0;

out_close_fb:
	close(fb);

	return ret;
}

void cavan_fb_uninit(struct cavan_screen_descriptor *desc)
{
	munmap(desc->fb_base, desc->fix_info.smem_len);
	close(desc->fb);
}

void cavan_fb_clear(struct cavan_screen_descriptor *desc)
{
	mem_set32(desc->fb_base, desc->background, desc->fb_size);
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

static int cavan_draw_line_horizon(struct cavan_screen_descriptor *desc, int x1, int y1, int x2, int y2)
{
	double a, b;
	void (*draw_point_handle)(struct cavan_screen_descriptor *, int, int, u32);
	u32 color;

	draw_point_handle = desc->draw_point;
	color = desc->bordercolor;

	if (x1 == x2)
	{
		if (y1 < y2)
		{
			while (y1 <= y2)
			{
				draw_point_handle(desc, x1, y1, color);
				y1++;
			}
		}
		else
		{
			while (y1 >= y2)
			{
				draw_point_handle(desc, x1, y1, color);
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
			draw_point_handle(desc, x1, a * x1 + b, color);
			x1++;
		}
	}
	else
	{
		while (x1 >= x2)
		{
			draw_point_handle(desc, x1, a * x1 + b, color);
			x1--;
		}
	}

	return 0;
}

static int cavan_draw_line_vertical(struct cavan_screen_descriptor *desc, int x1, int y1, int x2, int y2)
{
	double a, b;
	void (*draw_point_handle)(struct cavan_screen_descriptor *, int, int, u32);
	u32 color;

	draw_point_handle = desc->draw_point;
	color = desc->bordercolor;

	if (y1 == y2)
	{
		if (x1 < x2)
		{
			while (x1 <= x2)
			{
				draw_point_handle(desc, x1, y1, color);
				x1++;
			}
		}
		else
		{
			while (x1 >= x2)
			{
				draw_point_handle(desc, x1, y1, color);
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
			draw_point_handle(desc, a * y1 + b, y1, color);
			y1++;
		}
	}
	else
	{
		while (y1 >= y2)
		{
			draw_point_handle(desc, a * y1 + b, y1, color);
			y1--;
		}
	}

	return 0;
}

int cavan_draw_line(struct cavan_screen_descriptor *desc, int x1, int y1, int x2, int y2)
{
	int ret;

	if (x1 < 0 || x1 >= desc->xres || x2 < 0 || x2 >= desc->xres)
	{
		return -EINVAL;
	}

	if (y1 < 0 || y1 >= desc->yres || y2 < 0 || y2 >= desc->yres)
	{
		return -EINVAL;
	}

	ret = cavan_draw_line_horizon(desc, x1, y1, x2, y2);

	return ret < 0 ? ret : cavan_draw_line_vertical(desc, x1, y1, x2, y2);
}

int cavan_draw_rect(struct cavan_screen_descriptor *desc, int left, int top, int width, int height)
{
	int right, bottom;
	int i;
	void (*draw_point_handle)(struct cavan_screen_descriptor *, int, int, u32);
	u32 color;

	if (left < 0 || top < 0)
	{
		return -EINVAL;
	}

	right = left + width - 1;
	bottom = top + height - 1;

	if (right >= desc->xres || bottom >= desc->yres)
	{
		return -EINVAL;
	}

	draw_point_handle = desc->draw_point;
	color = desc->bordercolor;

	for (i = left; i <= right; i++)
	{
		draw_point_handle(desc, i, top, color);
		draw_point_handle(desc, i, bottom, color);
	}

	for (i = top; i <= bottom; i++)
	{
		draw_point_handle(desc, left, i, color);
		draw_point_handle(desc, right, i, color);
	}

	return 0;
}

int cavan_fill_rect(struct cavan_screen_descriptor *desc, int left, int top, int width, int height)
{
	int right, bottom;
	int x, y;
	void (*draw_point_handle)(struct cavan_screen_descriptor *, int, int, u32);
	u32 color;

	if (left < 0 || top < 0)
	{
		return -EINVAL;
	}

	right = left + width - 1;
	bottom = top + height - 1;

	if (right >= desc->xres || bottom >= desc->yres)
	{
		return -EINVAL;
	}

	draw_point_handle = desc->draw_point;
	color = desc->foreground;

	for (y = top; y <= bottom; y++)
	{
		for (x = left; x <= right; x++)
		{
			draw_point_handle(desc, x, y, color);
		}
	}

	return 0;
}

int cavan_draw_circle(struct cavan_screen_descriptor *desc, int x, int y, int r)
{
	int rr;
	int i;
	int tmp;
	void (*draw_point_handle)(struct cavan_screen_descriptor *, int, int, u32);
	u32 color;

	if (x - r < 0 || x + r >= desc->xres || y - r < 0 || y + r >= desc->yres)
	{
		return -EINVAL;
	}

	draw_point_handle = desc->draw_point;
	rr = r * r;
	color = desc->bordercolor;

	for (i = 0; i < r; i++)
	{
		tmp = sqrt(rr - i * i);

		draw_point_handle(desc, x + i, y + tmp, color);
		draw_point_handle(desc, x + i, y - tmp, color);
		draw_point_handle(desc, x - i, y + tmp, color);
		draw_point_handle(desc, x - i, y - tmp, color);

		draw_point_handle(desc, x + tmp, y + i, color);
		draw_point_handle(desc, x + tmp, y - i, color);
		draw_point_handle(desc, x - tmp, y + i, color);
		draw_point_handle(desc, x - tmp, y - i, color);
	}

	return 0;
}

int cavan_fill_circle(struct cavan_screen_descriptor *desc, int x, int y, int r)
{
	int rr;
	int i;
	int tmp, left, right, top, bottom;
	void (*draw_point_handle)(struct cavan_screen_descriptor *, int, int, u32);
	u32 color;

	if (x - r < 0 || x + r >= desc->xres || y - r < 0 || y + r >= desc->yres)
	{
		return -EINVAL;
	}

	draw_point_handle = desc->draw_point;
	rr = r * r;
	color = desc->foreground;

	for (i = 0; i < r; i++)
	{
		tmp = sqrt(rr - i * i);

		for (left = x - i, right = x + i, top = y + tmp, bottom = y - tmp; top <= bottom; top++)
		{
			draw_point_handle(desc, left, top, color);
			draw_point_handle(desc, right, top, color);
		}

		for (left = x - tmp, right = x + tmp, top = y - i, bottom = y + i; left <= right; left++)
		{
			draw_point_handle(desc, left, top, color);
			draw_point_handle(desc, left, bottom, color);
		}
	}

	return 0;
}

int cavan_draw_ellipse(struct cavan_screen_descriptor *desc, int x, int y, int width, int height)
{
	double aa, bb;
	int tmp;
	int i;
	void (*draw_point_handle)(struct cavan_screen_descriptor *, int, int, u32);
	u32 color;

	aa = ((double)width) / 2;
	bb = ((double)height) / 2;

	if (x - aa < 0 || x + aa >= desc->xres || y - bb < 0 || y + bb >= desc->yres)
	{
		return -EINVAL;
	}

	aa *= aa;
	bb *= bb;
	draw_point_handle = desc->draw_point;
	color = desc->bordercolor;

	for (i = width >> 1; i >= 0; i--)
	{
		tmp = sqrt(bb - (bb * i * i / aa));

		draw_point_handle(desc, x + i, y + tmp, color);
		draw_point_handle(desc, x + i, y - tmp, color);
		draw_point_handle(desc, x - i, y + tmp, color);
		draw_point_handle(desc, x - i, y - tmp, color);
	}

	for (i = height >> 1; i >= 0; i--)
	{
		tmp = sqrt(aa - (aa * i * i / bb));

		draw_point_handle(desc, x + tmp, y + i, color);
		draw_point_handle(desc, x + tmp, y - i, color);
		draw_point_handle(desc, x - tmp, y + i, color);
		draw_point_handle(desc, x - tmp, y - i, color);
	}

	return 0;
}

int cavan_fill_ellipse(struct cavan_screen_descriptor *desc, int x, int y, int width, int height)
{
	double aa, bb;
	int tmp, left, right, top, bottom;
	int i;
	void (*draw_point_handle)(struct cavan_screen_descriptor *, int, int, u32);
	u32 color;

	aa = ((double)width) / 2;
	bb = ((double)height) / 2;

	if (x - aa < 0 || x + aa >= desc->xres || y - bb < 0 || y + bb >= desc->yres)
	{
		return -EINVAL;
	}

	aa *= aa;
	bb *= bb;
	draw_point_handle = desc->draw_point;
	color = desc->foreground;

	for (i = width >> 1; i >= 0; i--)
	{
		tmp = sqrt(bb - (bb * i * i / aa));

		for (left = x - i, right = x + i, top = y - tmp, bottom = y + tmp; top <= bottom; top++)
		{
			draw_point_handle(desc, left, y + tmp, color);
			draw_point_handle(desc, right, y + tmp, color);
		}
	}

	for (i = height >> 1; i >= 0; i--)
	{
		tmp = sqrt(aa - (aa * i * i / bb));

		for (left = x - tmp, right = x + tmp, top = y - i, bottom = y + i; left <= right; left++)
		{
			draw_point_handle(desc, left, top, color);
			draw_point_handle(desc, left, bottom, color);
		}
	}

	return 0;
}

int cavan_draw_polygon(struct cavan_screen_descriptor *desc, struct cavan_point *points, size_t count)
{
	unsigned int i;
	int ret;

	if (count < 3)
	{
		return -EINVAL;
	}

	for (i = 0, count--; i < count; i++)
	{
		ret = cavan_draw_line(desc, points[i].x, points[i].y, points[i + 1].x, points[i + 1].y);
		if (ret < 0)
		{
			return ret;
		}
	}

	return cavan_draw_line(desc, points[0].x, points[0].y, points[count].x, points[count].y);
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

void show_cavan_points(const struct cavan_point *points, size_t size)
{
	const struct cavan_point *end;

	for (end = points + size; points < end; points++)
	{
		println("[%d, %d]", points->x, points->y);
	}
}

void cavan_point_sort_x(struct cavan_point *start, struct cavan_point *end)
{
	struct cavan_point mid, *start_bak, *end_bak;

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
		cavan_point_sort_x(start_bak, start - 1);
	}

	if (end_bak > end)
	{
		cavan_point_sort_x(end + 1, end_bak);
	}
}

static int cavan_fill_triangle_half(struct cavan_screen_descriptor *desc, struct cavan_point *p1, struct cavan_point *p2, double a1, double b1, double a2, double b2)
{
	int left, right, top, bottom;
	void (*draw_point_handle)(struct cavan_screen_descriptor *, int, int, u32);
	u32 color;

	// println("left = %d, right = %d", left, right);
	// println("a1 = %lf, b1 = %lf, a2 = %lf, b2 = %lf", a1, b1, a2, b2);

	draw_point_handle = desc->draw_point;
	left = p1->x;
	right = p2->x;
	color = desc->foreground;

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
				cavan_draw_point(desc, left, i, color);
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
			draw_point_handle(desc, left, top, color);
			top++;
		}

		left++;
	}

	return 0;
}

int cavan_fill_triangle(struct cavan_screen_descriptor *desc, struct cavan_point *points)
{
	double a[3], b[3];
	struct cavan_point *p0, *p1, *p2;

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
		cavan_fill_triangle_half(desc, p0, p1, a[0], b[0], a[1], b[1]);
		cavan_fill_triangle_half(desc, p1, p2, a[2], b[2], a[1], b[1]);
	}
	else
	{
		// pr_bold_pos();
		cavan_fill_triangle_half(desc, p0, p1, a[1], b[1], a[0], b[0]);
		cavan_fill_triangle_half(desc, p1, p2, a[1], b[1], a[2], b[2]);
	}

	return 0;
}

int cavan_fill_polygon(struct cavan_screen_descriptor *desc, struct cavan_point *points, size_t count)
{
	int ret;
	struct cavan_point *p, *p_end;
	struct cavan_point point_buff[3];

	if (count < 3)
	{
		return -EINVAL;
	}

	for (p = points + 1, p_end = p + count - 2; p < p_end; p++)
	{
		point_buff[0] = points[0];
		point_buff[1] = p[0];
		point_buff[2] = p[1];

		ret = cavan_fill_triangle(desc, point_buff);
		if (ret < 0)
		{
			return ret;
		}
	}

	return 0;
}

static int cavan_build_polygon_points(struct cavan_screen_descriptor *desc, struct cavan_point *points, size_t count, int x, int y, int r, int rotation)
{
	double angle, avg_angle;
	struct cavan_point *point_end;

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

int cavan_draw_polygon_standard(struct cavan_screen_descriptor *desc, size_t count, int x, int y, int r, int rotation)
{
	int ret;
	struct cavan_point points[count];

	ret = cavan_build_polygon_points(desc, points, count, x, y, r, rotation);

	return ret < 0 ? ret : cavan_draw_polygon(desc, points, count);
}

int cavan_fill_polygon_standard(struct cavan_screen_descriptor *desc, size_t count, int x, int y, int r, int rotation)
{
	int ret;
	struct cavan_point points[count];

	ret = cavan_build_polygon_points(desc, points, count, x, y, r, rotation);

	return ret < 0 ? ret : cavan_fill_polygon(desc, points, count);
}

int cavan_draw_polygon_standard2(struct cavan_screen_descriptor *desc, size_t count, int x, int y, int r, int rotation)
{
	int ret;
	struct cavan_point points[count];
	struct cavan_point *p1, *p2, *end_p;

	ret = cavan_build_polygon_points(desc, points, count, x, y, r, rotation);
	if (ret < 0)
	{
		return ret;
	}

	for (p1 = points, end_p = points + count; p1 < end_p; p1++)
	{
		for (p2 = points; p2 < p1; p2++)
		{
			ret = cavan_draw_line(desc, p1->x, p1->y, p2->x, p2->y);
			if (ret < 0)
			{
				return ret;
			}
		}

		for (p2 = p1 + 1; p2 < end_p; p2++)
		{
			ret = cavan_draw_line(desc, p1->x, p1->y, p2->x, p2->y);
			if (ret < 0)
			{
				return ret;
			}
		}
	}

	return 0;
}

int cavan_calculate_line_cross_point(int x1, int x2, double a1, double b1, double a2, double b2, struct cavan_point *point)
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

int cavan_calculate_polygo_cross_points(struct cavan_point *points, struct cavan_point *cross_points, size_t count)
{
	size_t i;
	int ret;
	struct cavan_point *p0, *p1, *p2, *p3;
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

int cavan_fill_polygon_standard2(struct cavan_screen_descriptor *desc, size_t count, int x, int y, int r, int rotation)
{
	int ret;
	size_t i;
	struct cavan_point points[count];
	struct cavan_point cross_points[count];
	struct cavan_point triangle_points[3];

	if (count < 5)
	{
		return cavan_fill_polygon_standard(desc, count, x, y, r, rotation);
	}

	ret = cavan_build_polygon_points(desc, points, count, x, y, r, rotation);
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

		cavan_fill_triangle(desc, triangle_points);
	}

	return 0;
}

int cavan_draw_polygon_standard3(struct cavan_screen_descriptor *desc, size_t count, int x, int y, int r, int rotation)
{
	int ret;
	struct cavan_point points[count];
	size_t i, j;

	ret = cavan_build_polygon_points(desc, points, count, x, y, r, rotation);
	if (ret < 0)
	{
		return ret;
	}

	for (i = 0; i < count; i++)
	{
		j = (i + 2) % count;

		ret = cavan_draw_line(desc, points[i].x, points[i].y, points[j].x, points[j].y);
		if (ret < 0)
		{
			return ret;
		}
	}

	return 0;
}

int cavan_fill_polygon_standard3(struct cavan_screen_descriptor *desc, size_t count, int x, int y, int r, int rotation)
{
	int ret;
	unsigned int i;
	struct cavan_point points[count];
	struct cavan_point cross_points[count];

	if (count < 5)
	{
		return cavan_fill_polygon_standard(desc, count, x, y, r, rotation);
	}

	ret = cavan_build_polygon_points(desc, points, count, x, y, r, rotation);
	if (ret < 0)
	{
		return ret;
	}

	ret = cavan_calculate_polygo_cross_points(points, cross_points, count);
	if (ret < 0)
	{
		return ret;
	}

	cavan_fill_polygon(desc, cross_points, count);

	for (i = 0; i < count; i++)
	{
		struct cavan_point triangle_points[3];

		triangle_points[0] = cross_points[i];
		triangle_points[1] = cross_points[(i + 1) % count];
		triangle_points[2] = points[(i + 2) % count];

		cavan_fill_triangle(desc, triangle_points);
	}

	return 0;
}

int cavan_draw_polygon_standard4(struct cavan_screen_descriptor *desc, size_t count, int x, int y, int r, int rotation)
{
	int ret;

	ret = cavan_draw_polygon_standard3(desc, count, x, y, r, rotation);
	if (ret < 0)
	{
		return ret;
	}

	return cavan_draw_circle(desc, x, y, r);
}
