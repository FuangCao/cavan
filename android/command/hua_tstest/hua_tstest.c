#include <huamobile.h>

static int huamobile_vk_matcher(struct huamobile_input_device *dev, void *data)
{
	return huamobile_input_name_matcher(dev->name, "FT5216", "CY8C242", "sprd-keypad", "headset-keyboard", NULL);
}

static int huamobile_vk_point_handler(struct huamobile_ts_device *ts, struct huamobile_touch_point *point, void *data)
{
	if (point)
	{
		pr_bold_info("p%d = [%d, %d]", point->id, point->x, point->y);
	}
	else
	{
		pr_bold_info("release");
	}

	return 0;
}

static int huamobile_vk_key_handler(struct huamobile_ts_device *ts, const char *name, int code, int value, void *data)
{
	pr_bold_info("name = %s, code = %d, value = %d", name, code, value);

	return 0;
}

int main(int argc, char *argv[])
{
	int ret;
	struct huamobile_ts_service service =
	{
		.lcd_width = -1,
		.lcd_height = -1,
		.matcher = huamobile_vk_matcher,
		.probe = NULL,
		.remove = NULL,
		.point_handler = huamobile_vk_point_handler,
		.key_handler = huamobile_vk_key_handler
	};

	ret = huamobile_ts_service_start(&service, NULL);
	if (ret < 0)
	{
		pr_red_info("huamobile_ts_start");
		return ret;
	}

#if 1
	ret = huamobile_input_service_join(&service.input_service);
	if (ret < 0)
	{
		pr_error_info("huamobile_input_thread_join");
	}
#else
	huamobile_ssleep(10);
#endif

	huamobile_ts_service_stop(&service);

	return ret;
}
