#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <hardware/sensors.h>

#define NELEM(a) \
	(sizeof(a) / sizeof((a)[0]))

#define pr_std_info(fmt, args ...) \
	printf(fmt "\n", ##args)

#define pr_red_info(fmt, args ...) \
	pr_std_info("\033[31m" fmt "\033[0m", ##args);

#define pr_bold_info(fmt, args ...) \
	pr_std_info("\033[1m" fmt "\033[0m", ##args);

#define pr_green_info(fmt, args ...) \
	pr_std_info("\033[32m" fmt "\033[0m", ##args);

enum
{
	LOCAL_COMMAND_OPTION_UNKNOWN = 20,
	LOCAL_COMMAND_OPTION_HELP,
	LOCAL_COMMAND_OPTION_ALL,
	LOCAL_COMMAND_OPTION_DELAY,
};

static void show_usage(const char *command)
{
	pr_std_info("Usage: %s [option]", command);
	pr_std_info("--help, -H, -h\t\t\tshow this help");
	pr_std_info("--all, -A, -a\t\t\tenable all sensor");
	pr_std_info("--delay, -D, -d\t\t\tpoll delay ms");
	pr_std_info("--acc, --gsensor, -G, -g\tenable accelerometer sensor");
	pr_std_info("--mag, -M, -m\t\t\tenable magnetic sensor");
	pr_std_info("--orient, -O, -o\t\tenable orientation sensor");
	pr_std_info("--gyro\t\t\t\tenable gyroscope sensor");
	pr_std_info("--light, -L, -l\t\t\tenable light sensor");
	pr_std_info("--pressure\t\t\tenable pressure sensor");
	pr_std_info("--temp, -T, -t\t\t\tenable temperature sensor");
	pr_std_info("--prox, -P, -p\t\t\tenable proximity sensor");
	pr_std_info("--gravity\t\t\tenable gravity sensor");
	pr_std_info("--la, --lg\t\t\tenable liner accelerometer sensor");
	pr_std_info("--rv, -R, -r\t\t\tenable rotation vector sensor");
}

static int sensor_poll_main_loop(struct sensors_module_t *sensor_module, struct sensors_poll_device_t *sensor_device, int delay, int mask)
{
	int ret;
	int count, enable_count;
	sensors_event_t events[8], *pe, *pe_end;
	const struct sensor_t *sensor_list, *sensor;

	count = sensor_module->get_sensors_list(sensor_module, &sensor_list);
	if (count < 0)
	{
		pr_red_info("get_sensors_list");
		return count;
	}

	pr_bold_info("sensor count = %d", count);
	pr_bold_info("mask = 0x%08x, delay = %d(ms)", mask, delay);
	pr_std_info("============================================================");

	for (sensor = sensor_list + count - 1, enable_count = 0; sensor >= sensor_list; sensor--)
	{
		int enable = (mask >> sensor->type) & 0x01;

		sensor_device->activate(sensor_device, sensor->handle, enable);
		if (enable)
		{
			pr_green_info("Enable sensor %s", sensor->name);
			sensor_device->setDelay(sensor_device, sensor->handle, delay * 1000 * 1000);
			enable_count++;
		}
		else
		{
			pr_red_info("Disable sensor %s", sensor->name);
		}

		pr_bold_info("version = %d, type = %d, handle = %d", sensor->version, sensor->type, sensor->handle);
		pr_bold_info("maxRange = %f", sensor->maxRange);
		pr_bold_info("resolution = %f", sensor->resolution);
		pr_bold_info("power = %f", sensor->power);
		pr_bold_info("minDelay = %d", sensor->minDelay);

		pr_std_info("============================================================");
	}

	if (enable_count == 0)
	{
		pr_red_info("No sensor enabled");
		return -1;
	}

	pr_bold_info("enable_count = %d", enable_count);

	while (1)
	{
		ret = sensor_device->poll(sensor_device, events, NELEM(events));
		if (ret < 0)
		{
			return ret;
		}

		for (pe = events, pe_end = pe + ret; pe < pe_end; pe++)
		{
			switch (pe->type)
			{
			case SENSOR_TYPE_ACCELEROMETER:
				pr_std_info("Accelerometer: [%f, %f, %f]", pe->data[0], pe->data[1], pe->data[2]);
				break;

			case SENSOR_TYPE_MAGNETIC_FIELD:
				pr_std_info("Magnetic_Field: [%f, %f, %f]", pe->data[0], pe->data[1], pe->data[2]);
				break;

			case SENSOR_TYPE_ORIENTATION:
				pr_std_info("Orientation: [%f, %f, %f]", pe->data[0], pe->data[1], pe->data[2]);
				break;

			case SENSOR_TYPE_GYROSCOPE:
				pr_std_info("Gyroscope: [%f, %f, %f]", pe->data[0], pe->data[1], pe->data[2]);
				break;

			case SENSOR_TYPE_LIGHT:
				pr_std_info("Light: [%f]", pe->data[0]);
				break;

			case SENSOR_TYPE_PRESSURE:
				pr_std_info("Pressure: [%f]", pe->data[0]);
				break;

			case SENSOR_TYPE_TEMPERATURE:
				pr_std_info("Temperature: [%f]", pe->data[0]);
				break;

			case SENSOR_TYPE_PROXIMITY:
				pr_std_info("Proximity: [%f]", pe->data[0]);
				break;

			case SENSOR_TYPE_GRAVITY:
				pr_std_info("Gravity: [%f, %f, %f]", pe->data[0], pe->data[1], pe->data[2]);
				break;

			case SENSOR_TYPE_LINEAR_ACCELERATION:
				pr_std_info("Linear_Acceleration: [%f, %f, %f]", pe->data[0], pe->data[1], pe->data[2]);
				break;

			case SENSOR_TYPE_ROTATION_VECTOR:
				pr_std_info("Rotation_Vecto: [%f, %f, %f]", pe->data[0], pe->data[1], pe->data[2]);
				break;
			}
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int c;
	int ret;
	int mask;
	int delay;
	int option_index;
    struct sensors_module_t *sensor_module;
	struct sensors_poll_device_t *sensor_device;
	struct option long_option[] =
	{
		{
			.name = "help",
			.has_arg = no_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_HELP,
		},
		{
			.name = "all",
			.has_arg = no_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_ALL,
		},
		{
			.name = "delay",
			.has_arg = required_argument,
			.flag = NULL,
			.val = LOCAL_COMMAND_OPTION_DELAY,
		},
		{
			.name = "acc",
			.has_arg = no_argument,
			.flag = NULL,
			.val = SENSOR_TYPE_ACCELEROMETER,
		},
		{
			.name = "gsensor",
			.has_arg = no_argument,
			.flag = NULL,
			.val = SENSOR_TYPE_ACCELEROMETER,
		},
		{
			.name = "mag",
			.has_arg = no_argument,
			.flag = NULL,
			.val = SENSOR_TYPE_MAGNETIC_FIELD,
		},
		{
			.name = "orient",
			.has_arg = no_argument,
			.flag = NULL,
			.val = SENSOR_TYPE_ORIENTATION,
		},
		{
			.name = "gyro",
			.has_arg = no_argument,
			.flag = NULL,
			.val = SENSOR_TYPE_GYROSCOPE,
		},
		{
			.name = "light",
			.has_arg = no_argument,
			.flag = NULL,
			.val = SENSOR_TYPE_LIGHT,
		},
		{
			.name = "pressure",
			.has_arg = no_argument,
			.flag = NULL,
			.val = SENSOR_TYPE_PRESSURE,
		},
		{
			.name = "temp",
			.has_arg = no_argument,
			.flag = NULL,
			.val = SENSOR_TYPE_TEMPERATURE,
		},
		{
			.name = "prox",
			.has_arg = no_argument,
			.flag = NULL,
			.val = SENSOR_TYPE_PROXIMITY,
		},
		{
			.name = "gravity",
			.has_arg = no_argument,
			.flag = NULL,
			.val = SENSOR_TYPE_GRAVITY,
		},
		{
			.name = "la",
			.has_arg = no_argument,
			.flag = NULL,
			.val = SENSOR_TYPE_LINEAR_ACCELERATION,
		},
		{
			.name = "lg",
			.has_arg = no_argument,
			.flag = NULL,
			.val = SENSOR_TYPE_LINEAR_ACCELERATION,
		},
		{
			.name = "rv",
			.has_arg = no_argument,
			.flag = NULL,
			.val = SENSOR_TYPE_ROTATION_VECTOR,
		},
		{
			0, 0, 0, 0
		},
	};

	mask = 0;
	delay = 200;

	while ((c = getopt_long(argc, argv, "HhAaD:d:GgMmLlPpTtRr", long_option, &option_index)) != EOF)
	{
		switch (c)
		{
		case 'h':
		case 'H':
		case LOCAL_COMMAND_OPTION_HELP:
			show_usage(argv[0]);
			return 0;

		case 'a':
		case 'A':
		case LOCAL_COMMAND_OPTION_ALL:
			mask = -1;
			break;

		case 'd':
		case 'D':
		case LOCAL_COMMAND_OPTION_DELAY:
			delay = atoi(optarg);
			break;

		case 'g':
		case 'G':
			mask |= 1 << SENSOR_TYPE_ACCELEROMETER;
			break;

		case 'm':
		case 'M':
			mask |= 1 << SENSOR_TYPE_MAGNETIC_FIELD;
			break;

		case 'l':
		case 'L':
			mask |= 1 << SENSOR_TYPE_LIGHT;
			break;

		case 'p':
		case 'P':
			mask |= 1 << SENSOR_TYPE_PROXIMITY;
			break;

		case 't':
		case 'T':
			mask |= 1 << SENSOR_TYPE_TEMPERATURE;
			break;

		case 'r':
		case 'R':
			mask |= 1 << SENSOR_TYPE_ROTATION_VECTOR;
			break;

		case SENSOR_TYPE_ACCELEROMETER:
		case SENSOR_TYPE_MAGNETIC_FIELD:
		case SENSOR_TYPE_ORIENTATION:
		case SENSOR_TYPE_GYROSCOPE:
		case SENSOR_TYPE_LIGHT:
		case SENSOR_TYPE_PRESSURE:
		case SENSOR_TYPE_TEMPERATURE:
		case SENSOR_TYPE_PROXIMITY:
		case SENSOR_TYPE_GRAVITY:
		case SENSOR_TYPE_LINEAR_ACCELERATION:
		case SENSOR_TYPE_ROTATION_VECTOR:
			mask |= 1 << c;
			break;

		default:
			pr_red_info("Invalid option %c", c);
			show_usage(argv[0]);
			return -EINVAL;
		}
	}

	if (mask == 0)
	{
		pr_red_info("Please enable some sensor");
		show_usage(argv[0]);
		return -EINVAL;
	}

	ret = hw_get_module(SENSORS_HARDWARE_MODULE_ID, (const struct hw_module_t **)&sensor_module);
	if (ret < 0)
	{
		pr_red_info("hw_get_module");
		return ret;
	}

	ret = sensors_open(&sensor_module->common, &sensor_device);
	if (ret < 0)
	{
		pr_red_info("sensors_open");
		return ret;
	}

	ret = sensor_poll_main_loop(sensor_module, sensor_device, delay, mask);
	sensors_close(sensor_device);

	return ret;
}
