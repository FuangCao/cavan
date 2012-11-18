#include <huamobile.h>
#include <huamobile/input.h>

static bool huamobile_vk_matcher(int fd, const char *name, void *data)
{
#if 0
	return huamobile_event_name_matcher(name, "FT5216", "CY8C242", "sprd-keypad", "headset-keyboard", "accelerometer", NULL);
#else
	return true;
#endif
}

int main(int argc, char *argv[])
{
	int ret;
	struct huamobile_input_service service =
	{
		.event_service =
		{
			.matcher = huamobile_vk_matcher
		},

		.lcd_width = 1024,
		.lcd_height = 768,
		.mouse_speed = 1.5,
		.gsensor_handler = NULL,
		.touch_handler = NULL,
		.move_handler = NULL,
		.release_handler = NULL,
		.key_handler = NULL
	};

	ret = huamobile_input_service_start(&service, NULL);
	if (ret < 0)
	{
		pr_red_info("huamobile_ts_start");
		return ret;
	}

#if 1
	ret = huamobile_event_service_join(&service.event_service);
	if (ret < 0)
	{
		pr_error_info("huamobile_input_thread_join");
	}
#else
	huamobile_ssleep(10);
#endif

	huamobile_input_service_stop(&service);

	return ret;
}
