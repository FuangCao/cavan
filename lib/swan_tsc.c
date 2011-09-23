#include <cavan.h>
#include <cavan/swan_tsc.h>
#include <cavan/event.h>

int touch_screen_calibration(void)
{
	int ret;
	struct event_desc desc;

	ret = event_init_by_name(&desc, TS_DEVICE_NAME);
	if (ret < 0)
	{
		error_msg("event_init_by_name");
		return ret;
	}

	stand_msg("Device \"%s\" Is Touchscreen, Start Calibration Now", desc.dev_path);

	ret = ioctl(desc.fd, CALIBRATION);
	if (ret < 0)
	{
		print_error("ioctl");
	}
	else
	{
		right_msg("Touch Screen Calibration Is OK");
	}

	event_uninit(&desc);

	return ret;
}
