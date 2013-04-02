#include <linux/input/hua_sensor.h>

static void hua_sensor_device_report_vector_base(struct hua_sensor_device *sensor, int x, int y, int z)
{
	struct input_dev *input = sensor->dev.input;
	struct hua_sensor_vector *data = &sensor->data;

	data->x = x;
	data->y = y;
	data->z = z;

	input_report_abs(input, ABS_X, x);
	input_report_abs(input, ABS_Y, y);
	input_report_abs(input, ABS_Z, z);
	input_sync(input);
}

static void hua_sensor_device_report_vector_upward_0(struct hua_sensor_device *sensor, int x, int y, int z)
{
	struct hua_sensor_vector *offset = &sensor->offset;

	hua_sensor_device_report_vector_base(sensor, offset->x + x, offset->y + y, offset->z + z);
}

static void hua_sensor_device_report_vector_upward_90(struct hua_sensor_device *sensor, int x, int y, int z)
{
	struct hua_sensor_vector *offset = &sensor->offset;

	hua_sensor_device_report_vector_base(sensor, offset->x + y, offset->y - x, offset->z + z);
}

static void hua_sensor_device_report_vector_upward_180(struct hua_sensor_device *sensor, int x, int y, int z)
{
	struct hua_sensor_vector *offset = &sensor->offset;

	hua_sensor_device_report_vector_base(sensor, offset->x - x, offset->y - y, offset->z + z);
}

static void hua_sensor_device_report_vector_upward_270(struct hua_sensor_device *sensor, int x, int y, int z)
{
	struct hua_sensor_vector *offset = &sensor->offset;

	hua_sensor_device_report_vector_base(sensor, offset->x - y, offset->y + x, offset->z + z);
}

static void hua_sensor_device_report_vector_downward_0(struct hua_sensor_device *sensor, int x, int y, int z)
{
	struct hua_sensor_vector *offset = &sensor->offset;

	hua_sensor_device_report_vector_base(sensor, offset->x + x, offset->y + y, offset->z - z);
}

static void hua_sensor_device_report_vector_downward_90(struct hua_sensor_device *sensor, int x, int y, int z)
{
	struct hua_sensor_vector *offset = &sensor->offset;

	hua_sensor_device_report_vector_base(sensor, offset->x + y, offset->y - x, offset->z - z);
}

static void hua_sensor_device_report_vector_downward_180(struct hua_sensor_device *sensor, int x, int y, int z)
{
	struct hua_sensor_vector *offset = &sensor->offset;

	hua_sensor_device_report_vector_base(sensor, offset->x - x, offset->y - y, offset->z - z);
}

static void hua_sensor_device_report_vector_downward_270(struct hua_sensor_device *sensor, int x, int y, int z)
{
	struct hua_sensor_vector *offset = &sensor->offset;

	hua_sensor_device_report_vector_base(sensor, offset->x - y, offset->y + x, offset->z - z);
}

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

	case HUA_INPUT_SENSOR_IOC_GET_XYZ:
		if (copy_to_user((void __user *)args, &sensor->data, sizeof(sensor->data)))
		{
			pr_red_info("copy_to_user");
			return -EFAULT;
		}
		return 0;

	case HUA_INPUT_SENSOR_IOC_SET_OFFSET:
		if (copy_from_user(&sensor->offset, (const void __user *)args, sizeof(sensor->offset)))
		{
			pr_red_info("copy_from_user");
			return -EFAULT;
		}
		return 0;

	case HUA_INPUT_SENSOR_IOC_CALIBRATION:
		sensor->offset.x -= sensor->data.x;
		sensor->offset.y -= sensor->data.y;
		sensor->offset.z -= sensor->data.z;
		if (copy_to_user((void __user *)args, &sensor->offset, sizeof(sensor->offset)))
		{
			pr_red_info("copy_to_user");
			return -EINVAL;
		}
		return 0;

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

static int hua_sensor_device_calibration(struct hua_input_device *dev, const void *buff, size_t size)
{
	int ret;
	struct hua_sensor_vector vector;
	struct hua_sensor_device *sensor = (struct hua_sensor_device *)dev;

	ret = sscanf(buff, "[%d,%d,%d]", &vector.x, &vector.y, &vector.z);
	if (ret != 3)
	{
		pr_red_info("sscanf");
		return -EINVAL;
	}

	pr_bold_info("x = %d, y = %d, z = %d", vector.x, vector.y, vector.z);

	sensor->offset = vector;

	return 0;
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

	switch (sensor->orientation)
	{
	case HUA_SENSOR_ORIENTATION_UPWARD_0:
		sensor->report_vector = hua_sensor_device_report_vector_upward_0;
		break;
	case HUA_SENSOR_ORIENTATION_UPWARD_90:
		sensor->report_vector = hua_sensor_device_report_vector_upward_90;
		break;
	case HUA_SENSOR_ORIENTATION_UPWARD_180:
		sensor->report_vector = hua_sensor_device_report_vector_upward_180;
		break;
	case HUA_SENSOR_ORIENTATION_UPWARD_270:
		sensor->report_vector = hua_sensor_device_report_vector_upward_270;
		break;
	case HUA_SENSOR_ORIENTATION_DOWNWARD_0:
		sensor->report_vector = hua_sensor_device_report_vector_downward_0;
		break;
	case HUA_SENSOR_ORIENTATION_DOWNWARD_90:
		sensor->report_vector = hua_sensor_device_report_vector_downward_90;
		break;
	case HUA_SENSOR_ORIENTATION_DOWNWARD_180:
		sensor->report_vector = hua_sensor_device_report_vector_downward_180;
		break;
	case HUA_SENSOR_ORIENTATION_DOWNWARD_270:
		sensor->report_vector = hua_sensor_device_report_vector_downward_270;
		break;
	default:
		pr_red_info("Invalid orientation %d", sensor->orientation);
		return -EINVAL;
	}

	set_bit(EV_ABS, input->evbit);

	dev->remove = hua_sensor_device_remove;
	dev->ioctl = hua_sensor_device_ioctl;

	if (dev->calibration == NULL)
	{
		dev->calibration = hua_sensor_device_calibration;
	}

	pr_green_info("huamobile sensor %s probe complete", dev->name);

	return 0;
}

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Huamobile Sensor Subsystem");
MODULE_LICENSE("GPL");
