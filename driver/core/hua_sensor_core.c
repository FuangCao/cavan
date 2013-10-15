#include <huamobile/hua_sensor.h>

static int hua_sensor_device_ioctl(struct hua_input_device *dev, unsigned int command, unsigned long args)
{
	struct hua_sensor_device *sensor = (struct hua_sensor_device *)dev;

	switch (command)
	{
	case HUA_INPUT_SENSOR_IOC_GET_MIN_DELAY:
		return hua_input_copy_to_user_uint(args, sensor->min_delay);

	case HUA_INPUT_SENSOR_IOC_GET_MAX_RANGE:
		return hua_input_copy_to_user_uint(args, sensor->max_range);

	case HUA_INPUT_SENSOR_IOC_GET_RESOLUTION:
		return hua_input_copy_to_user_uint(args, sensor->resolution);

	case HUA_INPUT_SENSOR_IOC_GET_POWER_CONSUME:
		return hua_input_copy_to_user_uint(args, sensor->power_consume);

	default:
		pr_red_info("Invalid IOCTL 0x%08x", command);
		return -EINVAL;
	}

	return -EFAULT;
}

static void hua_sensor_device_remove(struct hua_input_device *dev)
{
	pr_pos_info();
}

int hua_sensor_device_probe(struct hua_input_device *dev)
{
	struct input_dev *input = dev->input;
	struct hua_sensor_device *sensor = (struct hua_sensor_device *)dev;

	switch (dev->type)
	{
	case HUA_INPUT_DEVICE_TYPE_ACCELEROMETER:
	case HUA_INPUT_DEVICE_TYPE_MAGNETIC_FIELD:
	case HUA_INPUT_DEVICE_TYPE_ORIENTATION:
	case HUA_INPUT_DEVICE_TYPE_GYROSCOPE:
	case HUA_INPUT_DEVICE_TYPE_GRAVITY:
	case HUA_INPUT_DEVICE_TYPE_ROTATION_VECTOR:
	case HUA_INPUT_DEVICE_TYPE_LINEAR_ACCELERATION:
		input_set_abs_params(input, ABS_X, 0, sensor->max_range, dev->fuzz, dev->flat);
		input_set_abs_params(input, ABS_Y, 0, sensor->max_range, dev->fuzz, dev->flat);
		input_set_abs_params(input, ABS_Z, 0, sensor->max_range, dev->fuzz, dev->flat);
		break;

	case HUA_INPUT_DEVICE_TYPE_LIGHT:
	case HUA_INPUT_DEVICE_TYPE_PRESSURE:
	case HUA_INPUT_DEVICE_TYPE_TEMPERATURE:
	case HUA_INPUT_DEVICE_TYPE_PROXIMITY:
		input_set_abs_params(input, ABS_MISC, 0, sensor->max_range, dev->fuzz, dev->flat);
		break;

	default:
		pr_red_info("Invalid sensor type %d", dev->type);
		return -EINVAL;
	}

	set_bit(EV_ABS, input->evbit);

	dev->remove = hua_sensor_device_remove;
	dev->ioctl = hua_sensor_device_ioctl;

	pr_green_info("huamobile sensor %s probe complete", dev->name);

	return 0;
}

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Huamobile Sensor Subsystem");
MODULE_LICENSE("GPL");
