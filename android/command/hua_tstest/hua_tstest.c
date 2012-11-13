#include "hua_input.h"

int huamobile_vk_matcher(struct huamobile_input_device *dev, void *data)
{
	return huamobile_input_name_matcher(dev->name, "FT5216", "CY8C242", NULL);
}

int huamobile_vk_point_handler(struct huamobile_input_device *dev, struct huamobile_touch_point *point, void *data)
{
	if (point == NULL)
	{
		pr_bold_info("Touch up");
	}
	else
	{
		pr_bold_info("p%d = [%d, %d]", point->id, point->x, point->y);
	}

	return 0;
}

int huamobile_vk_key_handler(struct huamobile_input_device *dev, int code, int value, void *data)
{
	pr_bold_info("code = %d, value = %d", code, value);

	return 0;
}

int main(int argc, char *argv[])
{
	int ret;
	struct huamobile_ts_device ts =
	{
		.matcher = huamobile_vk_matcher,
		.point_handler = huamobile_vk_point_handler,
		.key_handler = huamobile_vk_key_handler
	};

	ret = huamobile_ts_start(&ts, NULL);
	if (ret < 0)
	{
		pr_red_info("huamobile_ts_start");
		return ret;
	}

	ret = huamobile_input_thread_join(&ts.thread);
	if (ret < 0)
	{
		pr_error_info("huamobile_input_thread_join");
	}

	huamobile_ts_stop(&ts);

	return ret;
}
