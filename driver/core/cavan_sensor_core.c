#include <cavan/cavan_sensor.h>

static int cavan_sensor_device_ioctl(struct cavan_input_device *dev, unsigned int command, unsigned long args)
{
	struct cavan_sensor_device *sensor = (struct cavan_sensor_device *) dev;

	switch (command)
	{
	case CAVAN_INPUT_SENSOR_IOC_GET_MIN_DELAY:
		return cavan_input_copy_to_user_uint(args, dev->min_delay);

	case CAVAN_INPUT_SENSOR_IOC_GET_MAX_RANGE:
		return cavan_input_copy_to_user_uint(args, sensor->max_range);

	case CAVAN_INPUT_SENSOR_IOC_GET_RESOLUTION:
		return cavan_input_copy_to_user_uint(args, sensor->resolution);

	case CAVAN_INPUT_SENSOR_IOC_GET_POWER_CONSUME:
		return cavan_input_copy_to_user_uint(args, sensor->power_consume);

	case CAVAN_INPUT_SENSOR_IOC_GET_AXIS_COUNT:
		return cavan_input_copy_to_user_uint(args, sensor->axis_count);

	default:
		pr_red_info("Invalid IOCTL 0x%08x", command);
		return -EINVAL;
	}

	return -EFAULT;
}

static ssize_t cavan_sensor_device_attr_max_range_show(struct device *device, struct device_attribute *attr, char *buff)
{
	struct cavan_misc_device *mdev = dev_get_drvdata(device);
	struct cavan_sensor_device *sensor = (struct cavan_sensor_device *) cavan_misc_device_get_data(mdev);

	return sprintf(buff, "%d\n", sensor->max_range);
}

static ssize_t cavan_sensor_device_attr_resolution_show(struct device *device, struct device_attribute *attr, char *buff)
{
	struct cavan_misc_device *mdev = dev_get_drvdata(device);
	struct cavan_sensor_device *sensor = (struct cavan_sensor_device *) cavan_misc_device_get_data(mdev);

	return sprintf(buff, "%d\n", sensor->resolution);
}

static ssize_t cavan_sensor_device_attr_power_show(struct device *device, struct device_attribute *attr, char *buff)
{
	struct cavan_misc_device *mdev = dev_get_drvdata(device);
	struct cavan_sensor_device *sensor = (struct cavan_sensor_device *) cavan_misc_device_get_data(mdev);

	return sprintf(buff, "%d\n", sensor->power_consume);
}

static ssize_t cavan_sensor_device_attr_axis_count_show(struct device *device, struct device_attribute *attr, char *buff)
{
	struct cavan_misc_device *mdev = dev_get_drvdata(device);
	struct cavan_sensor_device *sensor = (struct cavan_sensor_device *) cavan_misc_device_get_data(mdev);

	return sprintf(buff, "%d\n", sensor->axis_count);
}

static struct device_attribute cavan_sensor_device_attr_max_range = __ATTR(max_range, S_IRUGO, cavan_sensor_device_attr_max_range_show, NULL);
static struct device_attribute cavan_sensor_device_attr_resolution = __ATTR(resolution, S_IRUGO, cavan_sensor_device_attr_resolution_show, NULL);
static struct device_attribute cavan_sensor_device_attr_power_consume = __ATTR(power_consume, S_IRUGO, cavan_sensor_device_attr_power_show, NULL);
static struct device_attribute cavan_sensor_device_attr_axis_count = __ATTR(axis_count, S_IRUGO, cavan_sensor_device_attr_axis_count_show, NULL);

static const struct attribute *cavan_sensor_device_attributes[] =
{
	&cavan_sensor_device_attr_max_range.attr,
	&cavan_sensor_device_attr_resolution.attr,
	&cavan_sensor_device_attr_power_consume.attr,
	&cavan_sensor_device_attr_axis_count.attr,
	NULL
};

static void cavan_sensor_device_remove(struct cavan_input_device *dev)
{
	pr_pos_info();

	sysfs_remove_files(&dev->misc_dev.dev->kobj, cavan_sensor_device_attributes);
}

int cavan_sensor_device_probe(struct cavan_input_device *dev)
{
	int ret;
	struct input_dev *input = dev->input;
	struct cavan_sensor_device *sensor = (struct cavan_sensor_device *) dev;

	switch (dev->type)
	{
	case CAVAN_INPUT_DEVICE_TYPE_ACCELEROMETER:
	case CAVAN_INPUT_DEVICE_TYPE_MAGNETIC_FIELD:
	case CAVAN_INPUT_DEVICE_TYPE_ORIENTATION:
	case CAVAN_INPUT_DEVICE_TYPE_GYROSCOPE:
	case CAVAN_INPUT_DEVICE_TYPE_GRAVITY:
	case CAVAN_INPUT_DEVICE_TYPE_ROTATION_VECTOR:
	case CAVAN_INPUT_DEVICE_TYPE_LINEAR_ACCELERATION:
		if (sensor->axis_count < 2)
		{
			sensor->axis_count = 3;
		}

		input_set_abs_params(input, ABS_X, 0, sensor->resolution, dev->fuzz, dev->flat);
		input_set_abs_params(input, ABS_Y, 0, sensor->resolution, dev->fuzz, dev->flat);
		input_set_abs_params(input, ABS_Z, 0, sensor->resolution, dev->fuzz, dev->flat);
		break;

	case CAVAN_INPUT_DEVICE_TYPE_LIGHT:
	case CAVAN_INPUT_DEVICE_TYPE_PRESSURE:
	case CAVAN_INPUT_DEVICE_TYPE_TEMPERATURE:
	case CAVAN_INPUT_DEVICE_TYPE_PROXIMITY:
		sensor->axis_count = 1;
		input_set_abs_params(input, ABS_MISC, 0, sensor->resolution, dev->fuzz, dev->flat);
		break;

	default:
		pr_red_info("Invalid sensor type %d", dev->type);
		return -EINVAL;
	}

	set_bit(EV_ABS, input->evbit);

	ret = sysfs_create_files(&dev->misc_dev.dev->kobj, cavan_sensor_device_attributes);
	if (ret < 0)
	{
		pr_red_info("sysfs_create_files");
		return ret;
	}

	dev->remove = cavan_sensor_device_remove;
	dev->ioctl = cavan_sensor_device_ioctl;

	pr_green_info("cavan sensor %s probe complete", dev->name);

	return 0;
}

EXPORT_SYMBOL_GPL(cavan_sensor_device_probe);

const struct cavan_sensor_rate_table_node *cavan_sensor_find_rate_value(const struct cavan_sensor_rate_table_node *table, size_t count, u32 delay_ns)
{
	const struct cavan_sensor_rate_table_node *p;

	for (p = table + count - 1; p > table && p->delay_ns > delay_ns; p--);

	pr_bold_info("value = 0x%02x, delay = %d, relly_delay = %d", p->value, delay_ns, p->delay_ns);

	return p;
}

EXPORT_SYMBOL_GPL(cavan_sensor_find_rate_value);

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Cavan Sensor Subsystem");
MODULE_LICENSE("GPL");
