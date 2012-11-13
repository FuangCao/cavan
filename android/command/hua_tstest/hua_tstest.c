#include "hua_input.h"

int huamobile_vk_match(struct huamobile_input_device *dev, void *data)
{
	int i;
	const char *dev_names[] =
		{"FT5216", "CY8C242"};

	pr_pos_info();

	for (i = 0; i < (int)NELEM(dev_names); i++)
	{
		if (huamobile_input_text_lhcmp(dev_names[i], dev->name) == 0)
		{
			return 0;
		}
	}

	return -EINVAL;
}

int huamobile_vk_handler(struct huamobile_touch_point *point, void *data)
{
	pr_bold_info("p%d = [%d, %d]", point->id, point->x, point->y);

	return 0;
}

int main(int argc, char *argv[])
{
	int ret;
	struct huamobile_ts_device ts =
	{
		.match = huamobile_vk_match,
		.point_handler = huamobile_vk_handler
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
