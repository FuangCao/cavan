#include "hua_sensor_core.h"

ssize_t hua_i2c_read_data(struct i2c_client *client, u8 addr, void *buff, size_t size)
{
	int ret;
	struct i2c_msg msgs[] =
	{
		{
			.addr = client->addr,
			.flags = client->flags & I2C_M_TEN,
			.len = 1,
			.buf = (__u8 *)&addr
		},
		{
			.addr = client->addr,
			.flags = (client->flags & I2C_M_TEN) | I2C_M_RD,
			.len = size,
			.buf = (__u8 *)buff
		}
	};

	ret = i2c_transfer(client->adapter, msgs, 2);
	if (ret == 2)
	{
		return size;
	}

	return likely(ret < 0) ? ret : -EFAULT;
}

EXPORT_SYMBOL_GPL(hua_i2c_read_data);

ssize_t hua_i2c_write_data(struct i2c_client *client, u8 addr, const void *buff, size_t size)
{
	int ret;
	struct i2c_msg msgs[] =
	{
		{
			.addr = client->addr,
			.flags = client->flags & I2C_M_TEN,
			.len = 1,
			.buf = (__u8 *)&addr
		},
		{
			.addr = client->addr,
			.flags = client->flags & I2C_M_TEN,
			.len = size,
			.buf = (__u8 *)buff
		}
	};

	ret = i2c_transfer(client->adapter, msgs, 2);
	if (ret == 2)
	{
		return size;
	}

	return likely(ret < 0) ? ret : -EFAULT;
}

EXPORT_SYMBOL_GPL(hua_i2c_write_data);

int hua_sensor_read_register_i2c_smbus(struct hua_sensor_chip *chip, u8 addr, u8 *value)
{
	int ret;
	union i2c_smbus_data data;
	struct i2c_client *client = chip->private_data;

	ret = i2c_smbus_xfer(client->adapter, client->addr, client->flags, I2C_SMBUS_READ, addr, I2C_SMBUS_BYTE_DATA, &data);
	if (ret < 0)
	{
		return ret;
	}

	*value = data.byte;

	return 0;
}

EXPORT_SYMBOL_GPL(hua_sensor_read_register_i2c_smbus);

int hua_sensor_write_register_i2c_smbus(struct hua_sensor_chip *chip, u8 addr, u8 value)
{
	union i2c_smbus_data data;
	struct i2c_client *client = chip->private_data;

	data.byte = value;

	return i2c_smbus_xfer(client->adapter, client->addr, client->flags, I2C_SMBUS_WRITE, addr, I2C_SMBUS_BYTE_DATA, &data);
}

EXPORT_SYMBOL_GPL(hua_sensor_write_register_i2c_smbus);

// ================================================================================

static const char *hua_sensor_type_tostring(enum hua_sensor_type type)
{
	switch (type)
	{
	case HUA_SENSOR_TYPE_ACCELEROMETER:
		return "Acceleration";
	case HUA_SENSOR_TYPE_MAGNETIC_FIELD:
		return "Magnetic Field";
	case HUA_SENSOR_TYPE_ORIENTATION:
		return "Orientation";
	case HUA_SENSOR_TYPE_GYROSCOPE:
		return "Gyroscope";
	case HUA_SENSOR_TYPE_LIGHT:
		return "Light";
	case HUA_SENSOR_TYPE_PRESSURE:
		return "Pressure";
	case HUA_SENSOR_TYPE_TEMPERATURE:
		return "Gravity";
	case HUA_SENSOR_TYPE_PROXIMITY:
		return "Proximity";
	case HUA_SENSOR_TYPE_GRAVITY:
		return "Gravity";
	case HUA_SENSOR_TYPE_LINEAR_ACCELERATION:
		return "Linear Acceleration";
	case HUA_SENSOR_TYPE_ROTATION_VECTOR:
		return "Rotation Vector";
	default:
		return "unknown";
	}
}

static const char *hua_sensor_irq_trigger_type_tostring(unsigned long irq_flags)
{
	switch (irq_flags & IRQF_TRIGGER_MASK)
	{
	case IRQF_TRIGGER_FALLING:
		return "Falling Edge";
	case IRQF_TRIGGER_RISING:
		return "Rising Edge";
	case IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING:
		return "Both Edge";
	case IRQF_TRIGGER_HIGH:
		return "High Level";
	case IRQF_TRIGGER_LOW:
		return "Low Level";
	case IRQF_TRIGGER_NONE:
		return "None";
	default:
		return "Invalid";
	}
}

// ================================================================================

static void hua_sensor_event_thread_stop(struct hua_sensor_chip *chip)
{
	pr_pos_info();

	mutex_lock(&chip->lock);

	if (chip->event_task == NULL || chip->state == HUA_SENSOR_THREAD_STATE_STOPPED)
	{
		pr_func_info("Nothing to be done");
	}
	else
	{
		chip->state = HUA_SENSOR_THREAD_STATE_STOPPING;

		while (chip->state != HUA_SENSOR_THREAD_STATE_STOPPED)
		{
			mutex_unlock(&chip->lock);
			pr_pos_info();
			complete(&chip->event_completion);
			msleep(100);
			mutex_lock(&chip->lock);
		}
	}

	mutex_unlock(&chip->lock);
}

static int hua_sensor_chip_write_init_data(struct hua_sensor_chip *chip)
{
	int ret;
	const struct hua_sensor_init_data *data, *data_end;

	if (chip->init_data == NULL || chip->init_data_size == 0)
	{
		return 0;
	}

	for (data = chip->init_data, data_end = data + chip->init_data_size; data < data_end; data++)
	{
		pr_bold_info("Write register 0x%02x => 0x%02x", data->value, data->addr);

		ret = chip->write_register(chip, data->addr, data->value);
		if (ret < 0)
		{
			pr_red_info("hua_sensor_write_register");
			return ret;
		}

		if (data->delay)
		{
			msleep(data->delay);
		}
	}

	return 0;
}

static irqreturn_t hua_sensor_isr_level(int irq, void *dev_id)
{
	struct hua_sensor_chip *chip = (struct hua_sensor_chip *)dev_id;

	disable_irq_nosync(irq);
	complete(&chip->event_completion);

	return IRQ_HANDLED;
}

static irqreturn_t hua_sensor_isr_edge(int irq, void *dev_id)
{
	struct hua_sensor_chip *chip = (struct hua_sensor_chip *)dev_id;

	complete(&chip->event_completion);

	return IRQ_HANDLED;
}

static int hua_sensor_request_irq(struct hua_sensor_chip *chip)
{
	int ret;
	irq_handler_t handler;

	chip->irq_ctrl = false;

	if (chip->irq < 1 || (chip->irq_flags & IRQF_TRIGGER_MASK) == 0)
	{
		pr_func_info("Don't need request IRQ");
		return 0;
	}

	if (chip->irq_flags & (IRQF_TRIGGER_HIGH | IRQF_TRIGGER_LOW))
	{
		handler = hua_sensor_isr_level;
	}
	else
	{
		handler = hua_sensor_isr_edge;
		chip->irq_ctrl = true;
	}

	pr_green_info("%s irq trigger type is %s", chip->name, hua_sensor_irq_trigger_type_tostring(chip->irq_flags));

	ret = request_irq(chip->irq, handler, chip->irq_flags, chip->name, chip);
	if (ret < 0)
	{
		pr_red_info("request_irq %d", chip->irq);
		return ret;
	}

	disable_irq(chip->irq);

	return 0;
}

static int hua_sensor_set_delay(struct hua_sensor_device *sensor, struct hua_sensor_chip *chip, unsigned int delay)
{
	int ret;
	struct hua_sensor_device *sensor_end;

	pr_func_info("sensor = %s, delay = %d", sensor->name, delay);

	sensor->poll_delay = delay;

	for (sensor = chip->sensor_list, sensor_end = sensor + chip->sensor_count; sensor < sensor_end; sensor++)
	{
		if (sensor->enabled && sensor->poll_delay < delay)
		{
			delay = sensor->poll_delay;
		}
	}

	pr_func_info("chip = %s, delay = %d", chip->name, delay);

	if (chip->set_delay && (ret = chip->set_delay(chip, delay)) < 0)
	{
		pr_red_info("chip->set_delay");
		return ret;
	}

	chip->poll_delay = delay;

	return 0;
}

static int hua_sensor_set_delay_lock(struct hua_sensor_device *sensor, unsigned int delay)
{
	int ret;
	struct hua_sensor_chip *chip = sensor->chip;

	mutex_lock(&chip->lock);
	mutex_lock(&sensor->lock);
	ret = hua_sensor_set_delay(sensor, chip, delay);
	mutex_unlock(&sensor->lock);
	mutex_unlock(&chip->lock);

	return ret;
}

static int hua_sensor_chip_set_power(struct hua_sensor_chip *chip, bool enable)
{
	int ret;

	if (chip->powered == enable)
	{
		pr_func_info("Nothing to be done");
		return 0;
	}

	if (enable && chip->set_power)
	{
		ret = chip->set_power(chip, true);
		if (ret < 0)
		{
			pr_red_info("sensor->set_power %s", chip->name);
			return ret;
		}

		msleep(100);
	}

	if (enable && chip->power_on_init && (ret = hua_sensor_chip_write_init_data(chip)) < 0)
	{
		pr_red_info("hua_sensor_chip_write_init_data");
	}
	else
	{
		ret = 0;
		chip->powered = enable;
	}

	if (chip->powered == false && chip->set_power)
	{
		chip->set_power(chip, false);
	}

	pr_bold_info("sensor chip %s power %s", chip->name, chip->powered ? "enabled" : "disabled");

	return ret;
}

static int hua_sensor_chip_set_enable_base(struct hua_sensor_chip *chip, bool enable)
{
	int ret;

	if (chip->enabled == enable)
	{
		pr_func_info("Nothing to be done");
		return 0;
	}

	if (enable == false)
	{
		if (chip->irq_ctrl)
		{
			disable_irq_nosync(chip->irq);
		}

		chip->state = HUA_SENSOR_THREAD_STATE_SUSPEND;
	}

	ret = hua_sensor_chip_set_power(chip, enable);
	if (ret < 0)
	{
		pr_red_info("hua_sensor_chip_power_enable");
	}
	else
	{
		chip->enabled = enable;
	}

	if (chip->enabled)
	{
		if (chip->irq_ctrl)
		{
			enable_irq(chip->irq);
		}

		chip->state = HUA_SENSOR_THREAD_STATE_RUNNING;
		wake_up_process(chip->event_task);
	}

	pr_bold_info("sensor chip %s %s", chip->name, chip->enabled ? "enabled" : "disabled");

	return ret;
}

static int hua_sensor_chip_set_enable(struct hua_sensor_chip *chip, bool enable)
{
	int ret;

	if (enable)
	{
		if (chip->use_count == 0 && (ret = hua_sensor_chip_set_enable_base(chip, true)) < 0)
		{
			pr_red_info("hua_sensor_chip_set_enable_base");
			return ret;
		}

		chip->use_count++;
	}
	else
	{
		if (chip->use_count == 1 && (ret = hua_sensor_chip_set_enable_base(chip, false)) < 0)
		{
			pr_red_info("hua_sensor_chip_set_enable_base");
			return ret;
		}

		chip->use_count--;
	}

	pr_bold_info("sensor chip %s use count = %d", chip->name, chip->use_count);

	return 0;
}

static int hua_sensor_chip_readid(struct hua_sensor_chip *chip)
{
	int ret;

	if (chip->readid == NULL)
	{
		return 0;
	}

	ret = chip->readid(chip);
	if (ret < 0)
	{
		pr_red_info("sensor->readid %s", chip->name);
		return ret;
	}

	return 0;
}

// ================================================================================

static int hua_sensor_set_enable(struct hua_sensor_device *sensor, struct hua_sensor_chip *chip, bool enable)
{
	int ret;

	if (sensor->enabled == enable)
	{
		pr_func_info("Nothing to be done");
		return 0;
	}

	if (enable && (ret = hua_sensor_chip_set_enable(chip, true)) < 0)
	{
		pr_red_info("hua_sensor_chip_set_enable");
		return ret;
	}

	if (sensor->set_enable && (ret = sensor->set_enable(sensor, enable)) < 0)
	{
		pr_red_info("sensor->set_enable %s", sensor->name);
	}
	else
	{
		ret = 0;
		sensor->enabled = enable;
	}

	if (sensor->enabled == false)
	{
		hua_sensor_chip_set_enable(chip, false);
	}

	pr_bold_info("sensor %s %s %s", chip->name, sensor->name, sensor->enabled ? "enabled" : "disabled");

	if (chip->use_count > 0)
	{
		return hua_sensor_set_delay(sensor, chip, sensor->poll_delay);
	}

	return 0;
}

static int hua_sensor_set_enable_lock(struct hua_sensor_device *sensor, bool enable)
{
	int ret;
	struct hua_sensor_chip *chip = sensor->chip;

	mutex_lock(&chip->lock);
	mutex_lock(&sensor->lock);
	ret = hua_sensor_set_enable(sensor, chip, enable);
	mutex_unlock(&sensor->lock);
	mutex_unlock(&chip->lock);

	return ret;
}

static int hua_sensor_input_init(struct hua_sensor_device *sensor, struct input_dev *input)
{
	int xcode, ycode, zcode;

	switch (sensor->type)
	{
	case HUA_SENSOR_TYPE_ACCELEROMETER:
		xcode = ABS_X;
		ycode = ABS_Y;
		zcode = ABS_Z;
		break;

	case HUA_SENSOR_TYPE_MAGNETIC_FIELD:
		xcode = ABS_RX;
		ycode = ABS_RY;
		zcode = ABS_RZ;
		break;

	case HUA_SENSOR_TYPE_ORIENTATION:
		xcode = ABS_HAT0X;
		ycode = ABS_HAT0Y;
		zcode = ABS_RUDDER;
		break;

	case HUA_SENSOR_TYPE_GYROSCOPE:
		xcode = ABS_HAT1X;
		ycode = ABS_HAT1Y;
		zcode = ABS_WHEEL;
		break;

	case HUA_SENSOR_TYPE_GRAVITY:
		xcode = ABS_HAT2X;
		ycode = ABS_HAT2Y;
		zcode = ABS_GAS;
		break;

	case HUA_SENSOR_TYPE_ROTATION_VECTOR:
		xcode = ABS_HAT3X;
		ycode = ABS_HAT3Y;
		zcode = ABS_BRAKE;
		break;

	case HUA_SENSOR_TYPE_LINEAR_ACCELERATION:
		xcode = ABS_TILT_X;
		ycode = ABS_TILT_Y;
		zcode = ABS_TOOL_WIDTH;
		break;

	case HUA_SENSOR_TYPE_LIGHT:
		xcode = ycode = zcode = ABS_VOLUME;
		break;

	case HUA_SENSOR_TYPE_PRESSURE:
		xcode = ycode = zcode = ABS_PRESSURE;
		break;

	case HUA_SENSOR_TYPE_TEMPERATURE:
		xcode = ycode = zcode = ABS_THROTTLE;
		break;

	case HUA_SENSOR_TYPE_PROXIMITY:
		xcode = ycode = zcode = ABS_DISTANCE;
		break;

	default:
		pr_red_info("Invalid sensor type %d", sensor->type);
		return -EINVAL;
	}

	set_bit(EV_ABS, input->evbit);
	input_set_abs_params(input, xcode, 0, sensor->max_range, sensor->fuzz, sensor->flat);
	input_set_abs_params(input, ycode, 0, sensor->max_range, sensor->fuzz, sensor->flat);
	input_set_abs_params(input, zcode, 0, sensor->max_range, sensor->fuzz, sensor->flat);

	sensor->xcode = xcode;
	sensor->ycode = ycode;
	sensor->zcode = zcode;

	pr_func_info("xcode = 0x%02x, ycode = 0x%02x, zcode = 0x%02x", xcode, ycode, zcode);
	pr_green_info("sensor %s input init complete", sensor->name);

	return 0;
}

// ================================================================================

static int hua_sensor_device_init(struct hua_sensor_device *sensor, struct hua_sensor_chip *chip)
{
	pr_pos_info();

	if (sensor->event_handler == NULL)
	{
		pr_red_info("sensor->event_handler == NULL");
		return -EINVAL;
	}

	sensor->chip = chip;
	sensor->enabled = false;
	mutex_init(&sensor->lock);

	if (sensor->name == NULL)
	{
		sensor->name = hua_sensor_type_tostring(sensor->type);
	}

	return 0;
}

static int hua_sensor_copy_to_user_text(unsigned int command, unsigned long arg, const char *text)
{
	size_t size = HUA_SENSOR_IOC_GET_SIZE(command);
	size_t length = strlen(text) + 1;

	if (copy_to_user((void __user *)arg, text, length > size ? size : length))
	{
		pr_red_info("copy_to_user");
		return -EFAULT;
	}

	return 0;
}

static int hua_sensor_copy_to_user_uint(unsigned long arg, unsigned int value)
{
	if (copy_to_user((void __user *)arg, &value, sizeof(value)))
	{
		pr_red_info("copy_to_user");
		return -EFAULT;
	}

	return 0;
}

static int hua_sensor_chip_misc_ioctl_base(unsigned int command, unsigned long arg, struct hua_sensor_chip *chip)
{
	struct hua_sensor_device *sensor;
	int index = HUA_SENSOR_IOC_GET_INDEX(command);

	if (index > chip->sensor_count)
	{
		pr_red_info("Invalid sensor index");
		return -EINVAL;
	}

	sensor = chip->sensor_list + index;

	switch (HUA_SENSOR_IOC_GET_CMD_RAW(command))
	{
	case HUA_SENSOR_IOC_GET_SENSOR_TYPE(0):
		return hua_sensor_copy_to_user_uint(arg, sensor->type);

	case HUA_SENSOR_IOC_GET_SENSOR_NAME(0, 0):
		return hua_sensor_copy_to_user_text(command, arg, sensor->name);

	case HUA_SENSOR_IOC_GET_XCODE(0):
		return hua_sensor_copy_to_user_uint(arg, sensor->xcode);

	case HUA_SENSOR_IOC_GET_YCODE(0):
		return hua_sensor_copy_to_user_uint(arg, sensor->ycode);

	case HUA_SENSOR_IOC_GET_ZCODE(0):
		return hua_sensor_copy_to_user_uint(arg, sensor->zcode);

	case HUA_SENSOR_IOC_GET_MAX_RANGE(0):
		return hua_sensor_copy_to_user_uint(arg, sensor->max_range);

	case HUA_SENSOR_IOC_GET_RESOLUTION(0):
		return hua_sensor_copy_to_user_uint(arg, sensor->resolution);

	case HUA_SENSOR_IOC_GET_POWER_CONSUME(0):
		return hua_sensor_copy_to_user_uint(arg, sensor->power_consume);

	case HUA_SENSOR_IOC_SET_DELAY(0):
		return hua_sensor_set_delay_lock(sensor, arg);

	case HUA_SENSOR_IOC_SET_ENABLE(0):
		return hua_sensor_set_enable_lock(sensor, arg > 0);

	default:
		if (chip->ioctl)
		{
			return chip->ioctl(chip, command, arg);
		}

		pr_red_info("Invalid ioctl 0x%08x", command);
		return -EINVAL;
	}
}

static int hua_sensor_chip_misc_ioctl(struct inode *inode, struct file *file, unsigned int command, unsigned long arg)
{
	struct hua_sensor_chip *chip = file->private_data;

	switch (HUA_SENSOR_IOC_GET_CMD_RAW(command))
	{
	case HUA_SENSOR_IOC_GET_CHIP_NAME(0):
		return hua_sensor_copy_to_user_text(command, arg, chip->name);

	case HUA_SENSOR_IOC_GET_CHIP_VENDOR(0):
		return hua_sensor_copy_to_user_text(command, arg, chip->vendor);

	case HUA_SENSOR_IOC_GET_SENSOR_COUNT:
		return hua_sensor_copy_to_user_uint(arg, chip->sensor_count);

	case HUA_SENSOR_IOC_GET_MIN_DELAY:
		return hua_sensor_copy_to_user_uint(arg, chip->min_delay);

	default:
		return hua_sensor_chip_misc_ioctl_base(command, arg, chip);
	}
}

static ssize_t hua_sensor_chip_misc_write(struct file *file, const char __user *buff, size_t size, loff_t *offset)
{
	int ret;
	struct hua_sensor_device *sensor, *sensor_end;
	struct hua_sensor_chip *chip = file->private_data;
	bool enable = simple_strtoul(buff, NULL, 10) > 0;

	for (sensor = chip->sensor_list, sensor_end = sensor + chip->sensor_count; sensor < sensor_end; sensor++)
	{
		ret = hua_sensor_set_enable(sensor, chip, enable);
		if (ret < 0)
		{
			pr_red_info("hua_sensor_set_enable %s %s", chip->name, sensor->name);
			return ret;
		}
	}

	return size;
}

static int hua_sensor_chip_misc_open(struct inode *inode, struct file *file)
{
	struct hua_sensor_chip *chip = container_of(file->f_op, struct hua_sensor_chip, misc_fops);

	pr_func_info("sensor chip name = %s", chip->name);
	file->private_data = chip;

	return 0;
}

static int hua_sensor_chip_misc_release(struct inode *inode, struct file *file)
{
	pr_pos_info();

	return 0;
}

void hua_sensor_chip_report_event(struct hua_sensor_chip *chip, u32 mask)
{
	int count;
	struct hua_sensor_device *sensor, *sensor_end;

	for (count = 0, sensor = chip->sensor_list, sensor_end = sensor + chip->sensor_count; sensor < sensor_end; sensor++)
	{
		if (sensor->enabled && sensor->event_handler(sensor, mask))
		{
			count++;
		}
	}

	if (count > 0)
	{
		input_sync(chip->input);
	}
}

EXPORT_SYMBOL_GPL(hua_sensor_chip_report_event);

static void hua_sensor_main_loop_irq_level(struct hua_sensor_chip *chip)
{
	struct completion *event_completion = &chip->event_completion;

	pr_pos_info();

	while (1)
	{
		enable_irq(chip->irq);
		wait_for_completion(event_completion);

		if (chip->state != HUA_SENSOR_THREAD_STATE_RUNNING)
		{
			pr_pos_info();
			break;
		}

		hua_sensor_chip_report_event(chip, 0);
	}
}

static void hua_sensor_main_loop_irq_edge(struct hua_sensor_chip *chip)
{
	struct completion *event_completion = &chip->event_completion;

	pr_pos_info();

	while (1)
	{
		wait_for_completion(event_completion);

		if (chip->state != HUA_SENSOR_THREAD_STATE_RUNNING)
		{
			pr_pos_info();
			break;
		}

		hua_sensor_chip_report_event(chip, 0);
	}
}

static void hua_sensor_main_loop_no_irq(struct hua_sensor_chip *chip)
{
	pr_pos_info();

	while (1)
	{
		msleep(chip->poll_delay);

		if (chip->state != HUA_SENSOR_THREAD_STATE_RUNNING)
		{
			pr_pos_info();
			break;
		}

		hua_sensor_chip_report_event(chip, 0);
	}
}

static int hua_sensor_chip_init(struct hua_sensor_chip *chip, struct hua_sensor_core *core)
{
	int ret;
	struct miscdevice *misc;
	struct file_operations *fops;
	struct hua_sensor_device *sensor, *sensor_end;

	if (chip == NULL || chip->name == NULL)
	{
		pr_red_info("chip == NULL || chip->name == NULL");
		return -EINVAL;
	}

	if (chip->sensor_list == NULL || chip->sensor_count == 0)
	{
		pr_red_info("chip->sensor_list == NULL || chip->sensor_count == 0");
		return -EINVAL;
	}

	if (chip->read_data == NULL || chip->write_data == NULL)
	{
		pr_red_info("chip->main_loop == NULL || chip->read_data == NULL || chip->write_data == NULL");
		return -EINVAL;
	}

	if (chip->vendor == NULL)
	{
		chip->vendor = "HuaMobile";
	}

	if (chip->read_register == NULL)
	{
		chip->read_register = hua_sensor_read_register_dummy;
	}

	if (chip->write_register == NULL)
	{
		chip->write_register = hua_sensor_write_register_dummy;
	}

	if (chip->main_loop == NULL)
	{
		if (chip->irq > 0 && (chip->irq_flags & (IRQF_TRIGGER_HIGH | IRQF_TRIGGER_LOW)))
		{
			chip->main_loop = hua_sensor_main_loop_irq_level;
		}
		else if (chip->irq > 0 && (chip->irq_flags & (IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING)))
		{
			chip->main_loop = hua_sensor_main_loop_irq_edge;
		}
		else
		{
			chip->main_loop = hua_sensor_main_loop_no_irq;
		}
	}

	chip->core = core;
	mutex_init(&chip->lock);
	init_completion(&chip->event_completion);

	chip->state = HUA_SENSOR_THREAD_STATE_STOPPED;
	chip->powered = false;
	chip->enabled = false;
	chip->irq_ctrl = false;
	chip->use_count = 0;

	for (sensor = chip->sensor_list, sensor_end = sensor + chip->sensor_count; sensor < sensor_end; sensor++)
	{
		ret = hua_sensor_device_init(sensor, chip);
		if (ret < 0)
		{
			pr_red_info("hua_sensor_device_init");
			return ret;
		}
	}

	snprintf(chip->misc_name, sizeof(chip->misc_name), "sensor-%s", chip->name);

	fops = &chip->misc_fops;
	memset(fops, 0, sizeof(*fops));
	fops->open = hua_sensor_chip_misc_open;
	fops->release = hua_sensor_chip_misc_release;
	fops->ioctl = hua_sensor_chip_misc_ioctl;
	fops->write = hua_sensor_chip_misc_write;

	misc = &chip->misc_dev;
	memset(misc, 0, sizeof(*misc));
	misc->minor = MISC_DYNAMIC_MINOR;
	misc->mode = S_IRUGO | S_IWUGO;
	misc->name = chip->misc_name;
	misc->fops = fops;

	ret = misc_register(misc);
	if (ret < 0)
	{
		pr_red_info("misc_register");
		return ret;
	}

	return 0;
}

static void hua_sensor_chip_uninit(struct hua_sensor_chip *chip)
{
	misc_deregister(&chip->misc_dev);
}

static void hua_sensor_chip_remove(struct hua_sensor_chip *chip)
{
	hua_sensor_event_thread_stop(chip);

	if (chip->irq > 0)
	{
		free_irq(chip->irq, chip);
	}

	input_unregister_device(chip->input);

	if (chip->remove)
	{
		chip->remove(chip);
	}

	input_free_device(chip->input);
	hua_sensor_chip_set_power(chip, false);
}

static int hua_sensor_event_thread_handler(void *data)
{
	struct hua_sensor_chip *chip = data;

	mutex_lock(&chip->lock);

	while (chip->state != HUA_SENSOR_THREAD_STATE_STOPPING)
	{
		mutex_unlock(&chip->lock);
		chip->state = HUA_SENSOR_THREAD_STATE_RUNNING;
		pr_green_info("sensor chip %s enter main loop", chip->name);
		chip->main_loop(chip);
		pr_green_info("sensor chip %s exit main loop", chip->name);
		mutex_lock(&chip->lock);

		while (chip->state == HUA_SENSOR_THREAD_STATE_SUSPEND)
		{
			pr_green_info("sensor chip hip %s suspending", chip->name);

			mutex_unlock(&chip->lock);
			set_current_state(TASK_UNINTERRUPTIBLE);
			schedule();
			mutex_lock(&chip->lock);
		}
	}

	chip->state = HUA_SENSOR_THREAD_STATE_STOPPED;
	chip->event_task = NULL;
	pr_red_info("sensor chip %s event thread exit", chip->name);
	mutex_unlock(&chip->lock);

	return 0;
}

static int hua_sensor_chip_probe(struct hua_sensor_chip *chip)
{
	int ret;
	struct input_dev *input;
	struct hua_sensor_device *sensor, *sensor_end;

	ret = hua_sensor_chip_set_power(chip, true);
	if (ret < 0)
	{
		pr_red_info("hua_sensor_chip_set_power");
		return ret;
	}

	ret = hua_sensor_chip_readid(chip);
	if (ret < 0)
	{
		pr_red_info("hua_sensor_chip_readid");
		goto out_power_off;
	}

	if (chip->power_on_init == false && (ret = hua_sensor_chip_write_init_data(chip)) < 0)
	{
		pr_red_info("hua_sensor_chip_write_init_data");
		goto out_power_off;
	}

	input = input_allocate_device();
	if (input == NULL)
	{
		pr_red_info("input_allocate_device");
		ret = -ENOMEM;
		goto out_power_off;
	}

	chip->input = input;

	for (sensor = chip->sensor_list, sensor_end = sensor + chip->sensor_count; sensor < sensor_end; sensor++)
	{
		ret = hua_sensor_input_init(sensor, input);
		if (ret < 0)
		{
			pr_red_info("hua_sensor_input_init");
			goto out_input_free_device;
		}
	}

	if (chip->probe && (ret = chip->probe(chip)) < 0)
	{
		pr_red_info("chip->probe");
		goto out_input_free_device;
	}

	input->name = chip->misc_name;

	ret = input_register_device(input);
	if (ret < 0)
	{
		pr_red_info("input_register_device");
		goto out_chip_remove;
	}

	ret = hua_sensor_request_irq(chip);
	if (ret < 0)
	{
		pr_red_info("hua_sensor_request_irq");
		goto out_input_unregister_device;
	}

	chip->event_task = kthread_create(hua_sensor_event_thread_handler, chip, chip->name);
	if (chip->event_task == NULL)
	{
		pr_red_info("kthread_create");
		ret = -EFAULT;
		goto out_free_irq;
	}

#if HUA_SENSOR_ENABLE_ALL
	for (sensor = chip->sensor_list, sensor_end = sensor + chip->sensor_count; sensor < sensor_end; sensor++)
	{
		hua_sensor_set_enable(sensor, chip, true);
	}
#endif

	pr_green_info("sensor chip %s probe complete", chip->name);

	return 0;

out_free_irq:
	if (chip->irq > 0)
	{
		free_irq(chip->irq, chip);
	}
out_input_unregister_device:
	input_unregister_device(input);
out_chip_remove:
	if (chip->remove)
	{
		chip->remove(chip);
	}
out_input_free_device:
	input_free_device(input);
out_power_off:
	hua_sensor_chip_set_power(chip, false);

	return ret;
}

// ================================================================================

static struct hua_sensor_chip *hua_sensor_core_add_chip(struct hua_sensor_chip *head, struct hua_sensor_chip *chip)
{
	if (head)
	{
		head->prev = chip;
	}

	chip->next = head;
	chip->prev = NULL;

	return chip;
}

static bool hua_sensor_core_has_chip(struct hua_sensor_chip *head, struct hua_sensor_chip *chip)
{
	while (head)
	{
		if (head == chip)
		{
			return true;
		}

		head = head->next;
	}

	return false;
}

static struct hua_sensor_chip *hua_sensor_core_remove_chip(struct hua_sensor_chip *head, struct hua_sensor_chip *chip)
{
	struct hua_sensor_chip *next = chip->next;
	struct hua_sensor_chip *prev = chip->prev;

	if (next)
	{
		next->prev = prev;
	}

	if (prev)
	{
		prev->next = next;
		return head;
	}

	return next;
}

static int hua_sensor_detect_thread_handler(void *data)
{
	int ret;
	long timeout;
	struct hua_sensor_chip *chip;
	struct hua_sensor_core *core = data;

	pr_pos_info();

	mutex_lock(&core->lock);

	while (1)
	{
		for (chip = core->chip_head; chip; chip = chip->next)
		{
			mutex_lock(&chip->lock);

			pr_bold_info("Try sensor chip %s", chip->name);
			ret = hua_sensor_chip_probe(chip);
			if (ret >= 0)
			{
				core->chip_head = hua_sensor_core_remove_chip(core->chip_head, chip);
				core->detect_delay = 0;
			}

			mutex_unlock(&chip->lock);
		}

		if (core->chip_head == NULL)
		{
			pr_green_info("sensor detect thread enter sleep");
			timeout = MAX_SCHEDULE_TIMEOUT;
		}
		else
		{
			pr_func_info("Detct delay = %dms", core->detect_delay);
			timeout = msecs_to_jiffies(core->detect_delay) + 1;
		}

		mutex_unlock(&core->lock);
		set_current_state(TASK_UNINTERRUPTIBLE);
		schedule_timeout(timeout);
		mutex_lock(&core->lock);

		if (kthread_should_stop())
		{
			goto out_thread_exit;
		}

		if (core->detect_delay < 60 * 1000)
		{
			core->detect_delay = core->detect_delay << 1 | 1;
		}
	}

out_thread_exit:
	core->detect_task = NULL;

	mutex_unlock(&core->lock);

	pr_red_info("sensor core %s event thread exit", core->name);

	return 0;
}

static void hua_sensor_detect_thread_start(struct hua_sensor_core *core)
{
	mutex_lock(&core->lock);

	core->detect_delay = 0;

	if (core->detect_task == NULL)
	{
		core->detect_task = kthread_create(hua_sensor_detect_thread_handler, core, core->name);
	}

	if (core->detect_task)
	{
		wake_up_process(core->detect_task);
	}

	mutex_unlock(&core->lock);
}

static void hua_sensor_detect_thread_stop(struct hua_sensor_core *core)
{
	mutex_lock(&core->lock);

	if (core->detect_task)
	{
		mutex_unlock(&core->lock);
		kthread_stop(core->detect_task);
	}
	else
	{
		mutex_unlock(&core->lock);
	}
}

static ssize_t hua_sensor_core_misc_write(struct file *file, const char __user *buff, size_t size, loff_t *offset)
{
	struct hua_sensor_core *core = file->private_data;

	if (simple_strtoul(buff, NULL, 10) > 0)
	{
		hua_sensor_detect_thread_start(core);
	}
	else
	{
		hua_sensor_detect_thread_stop(core);
	}

	return size;
}

static int hua_sensor_core_misc_ioctl(struct inode *inode, struct file *file, unsigned int command, unsigned long arg)
{
	struct hua_sensor_core *core = file->private_data;

	switch (command)
	{
	case HUA_SENSOR_IOC_SET_DETECT:
		if (arg > 0)
		{
			hua_sensor_detect_thread_start(core);
		}
		else
		{
			hua_sensor_detect_thread_stop(core);
		}
		break;

	default:
		pr_red_info("Invalid command 0x%08x", command);
		return -EINVAL;
	}

	return 0;
}

static int hua_sensor_core_misc_open(struct inode *inode, struct file *file)
{
	struct hua_sensor_core *core = container_of(file->f_op, struct hua_sensor_core, misc_fops);

	pr_func_info("sensor core name = %s", core->name);
	file->private_data = core;

	return 0;
}

static int hua_sensor_core_misc_release(struct inode *inode, struct file *file)
{
	pr_pos_info();

	return 0;
}

static struct hua_sensor_core sensor_core =
{
	.name = "HuaMobile_SS",
	.misc_fops =
	{
		.open = hua_sensor_core_misc_open,
		.release = hua_sensor_core_misc_release,
		.write = hua_sensor_core_misc_write,
		.ioctl = hua_sensor_core_misc_ioctl
	},
	.misc_dev =
	{
		.minor = MISC_DYNAMIC_MINOR,
		.mode = S_IRUGO | S_IWUGO,
	}
};

int hua_sensor_register_chip(struct hua_sensor_chip *chip)
{
	int ret;

	ret = hua_sensor_chip_init(chip, &sensor_core);
	if (ret < 0)
	{
		pr_red_info("hua_sensor_chip_init");
		return ret;
	}

	mutex_lock(&sensor_core.lock);
	sensor_core.chip_head = hua_sensor_core_add_chip(sensor_core.chip_head, chip);
	mutex_unlock(&sensor_core.lock);

	hua_sensor_detect_thread_start(&sensor_core);

	return 0;
}

EXPORT_SYMBOL_GPL(hua_sensor_register_chip);

void hua_sensor_unregister_chip(struct hua_sensor_chip *chip)
{
	struct hua_sensor_core *core = chip->core;

	mutex_lock(&chip->lock);
	mutex_lock(&core->lock);

	if (hua_sensor_core_has_chip(core->chip_head, chip))
	{
		core->chip_head = hua_sensor_core_remove_chip(core->chip_head, chip);
	}
	else
	{
		mutex_unlock(&chip->lock);
		hua_sensor_chip_remove(chip);
		mutex_lock(&chip->lock);
	}

	hua_sensor_chip_uninit(chip);

	mutex_unlock(&core->lock);
	mutex_unlock(&chip->lock);
}

EXPORT_SYMBOL_GPL(hua_sensor_unregister_chip);

static int __init hua_sensor_core_init(void)
{
	int ret;
	struct miscdevice *misc_dev;

	pr_pos_info();

	mutex_init(&sensor_core.lock);

	misc_dev = &sensor_core.misc_dev;
	misc_dev->name = sensor_core.name;
	misc_dev->fops = &sensor_core.misc_fops;
	ret = misc_register(misc_dev);
	if (ret < 0)
	{
		pr_red_info("misc_register");
		return ret;
	}

	hua_sensor_detect_thread_start(&sensor_core);

	return 0;
}

static void __exit hua_sensor_core_exit(void)
{
	pr_pos_info();

	hua_sensor_detect_thread_stop(&sensor_core);
	misc_deregister(&sensor_core.misc_dev);
}

module_init(hua_sensor_core_init);
module_exit(hua_sensor_core_exit);

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Huamobile Sensor Subsystem");
MODULE_LICENSE("GPL");
