#include <linux/input/hua_input.h>

extern int hua_ts_device_probe(struct hua_input_device *dev);
extern int hua_sensor_device_probe(struct hua_input_device *dev);

static struct hua_input_core input_core =
{
	.name = "HUA-INPUT-CORE"
};

ssize_t hua_input_read_data_i2c(struct hua_input_chip *chip, u8 addr, void *buff, size_t size)
{
	int ret;
	struct i2c_client *client = chip->bus_data;
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

EXPORT_SYMBOL_GPL(hua_input_read_data_i2c);

ssize_t hua_input_write_data_i2c(struct hua_input_chip *chip, u8 addr, const void *buff, size_t size)
{
	int ret;
	struct i2c_client *client = chip->bus_data;
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

EXPORT_SYMBOL_GPL(hua_input_write_data_i2c);

int hua_input_read_register_i2c_smbus(struct hua_input_chip *chip, u8 addr, u8 *value)
{
	int ret;
	union i2c_smbus_data data;
	struct i2c_client *client = chip->bus_data;

	ret = i2c_smbus_xfer(client->adapter, client->addr, client->flags, I2C_SMBUS_READ, addr, I2C_SMBUS_BYTE_DATA, &data);
	if (ret < 0)
	{
		return ret;
	}

	*value = data.byte;

	return 0;
}

EXPORT_SYMBOL_GPL(hua_input_read_register_i2c_smbus);

int hua_input_write_register_i2c_smbus(struct hua_input_chip *chip, u8 addr, u8 value)
{
	union i2c_smbus_data data;
	struct i2c_client *client = chip->bus_data;

	data.byte = value;

	return i2c_smbus_xfer(client->adapter, client->addr, client->flags, I2C_SMBUS_WRITE, addr, I2C_SMBUS_BYTE_DATA, &data);
}

EXPORT_SYMBOL_GPL(hua_input_write_register_i2c_smbus);

int hua_input_test_i2c(struct i2c_client *client)
{
	struct i2c_msg msg =
	{
		.addr = client->addr,
		.flags = (client->flags & I2C_M_TEN) | I2C_M_RD,
		.len = 0,
		.buf = NULL
	};

	if (i2c_transfer(client->adapter, &msg, 1) == 1)
	{
		return 0;
	}

	return -EFAULT;
}

EXPORT_SYMBOL_GPL(hua_input_test_i2c);

// ================================================================================

static void hua_input_list_init(struct hua_input_list *list)
{
	mutex_init(&list->lock);
	INIT_LIST_HEAD(&list->head);
}

static void hua_input_list_destory(struct hua_input_list *list)
{
	mutex_destroy(&list->lock);
}

static void hua_input_list_add(struct hua_input_list *list, struct list_head *node)
{
	mutex_lock(&list->lock);
	list_add(node, &list->head);
	mutex_unlock(&list->lock);
}

static void hua_input_list_del(struct hua_input_list *list, struct list_head *node)
{
	mutex_lock(&list->lock);
	list_del(node);
	mutex_unlock(&list->lock);
}

static bool hua_input_list_empty(struct hua_input_list *list)
{
	bool res;

	mutex_lock(&list->lock);
	res = list_empty(&list->head);
	mutex_unlock(&list->lock);

	return res;
}

static bool hua_input_list_has_node(struct hua_input_list *list, struct list_head *node)
{
	struct list_head *pos;
	struct list_head *head;

	mutex_lock(&list->lock);

	for (head = &list->head, pos = head->next; pos != head; pos = pos->next)
	{
		if (node == pos)
		{
			mutex_unlock(&list->lock);
			return true;
		}
	}

	mutex_unlock(&list->lock);

	return false;
}

// ================================================================================

int hua_input_copy_to_user_text(unsigned int command, unsigned long args, const char *text)
{
	size_t size = HUA_INPUT_IOC_GET_SIZE(command);
	size_t length = strlen(text) + 1;

	if (copy_to_user((void __user *)args, text, length > size ? size : length))
	{
		pr_red_info("copy_to_user");
		return -EFAULT;
	}

	return 0;
}

EXPORT_SYMBOL_GPL(hua_input_copy_to_user_text);

int hua_input_copy_to_user_uint(unsigned long args, unsigned int value)
{
	if (copy_to_user((void __user *)args, &value, sizeof(value)))
	{
		pr_red_info("copy_to_user");
		return -EFAULT;
	}

	return 0;
}

EXPORT_SYMBOL_GPL(hua_input_copy_to_user_uint);

// ================================================================================

static int hua_input_chip_write_init_data(struct hua_input_chip *chip)
{
	int ret;
	const struct hua_input_init_data *data, *data_end;

	pr_pos_info();

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

// ================================================================================

static ssize_t hua_input_sysfs_show(struct kobject *kobj, struct attribute *attr, char *buff)
{
	struct hua_input_attribute *hua_attr = (struct hua_input_attribute *)attr;

	if (hua_attr->show == NULL || hua_attr->dev == NULL)
	{
		pr_red_info("hua_attr->show == NULL || hua_kobj->dev == NULL");
		return -EINVAL;
	}

	return hua_attr->show(hua_attr->dev, hua_attr, buff);
}

static ssize_t hua_input_sysfs_store(struct kobject *kobj,struct attribute *attr, const char *buff, size_t size)
{
	struct hua_input_attribute *hua_attr = (struct hua_input_attribute *)attr;

	if (hua_attr->store == NULL || hua_attr->dev == NULL)
	{
		pr_red_info("hua_attr->store == NULL || hua_kobj->dev == NULL");
		return -EINVAL;
	}

	return hua_attr->store(hua_attr->dev, hua_attr, buff, size);
}

static struct sysfs_ops hua_input_sysfs_ops =
{
	.show = hua_input_sysfs_show,
	.store = hua_input_sysfs_store
};

static struct kobj_type hua_input_kobj_type =
{
	.sysfs_ops = &hua_input_sysfs_ops
};

int hua_input_add_kobject(struct kobject *kobj, const char *name)
{
	int ret;

	if (kobj->state_initialized == 0)
	{
		kobject_init(kobj, &hua_input_kobj_type);
	}

	ret = kobject_add(kobj, NULL, name);
	if (ret < 0)
	{
		pr_red_info("kobject_init_and_add");
		return ret;
	}

	return 0;
}

EXPORT_SYMBOL_GPL(hua_input_add_kobject);

void hua_input_remove_kobject(struct kobject *kobj)
{
	kobject_del(kobj);
	kobject_put(kobj);
}

EXPORT_SYMBOL_GPL(hua_input_remove_kobject);

int hua_input_create_sysfs_files(struct hua_input_device *dev, struct kobject *kobj, struct hua_input_attribute *attrs, size_t count)
{
	struct hua_input_attribute *p, *p_end;

	for (p = attrs, p_end = p + count; p < p_end; p++)
	{
		int ret;

		p->dev = dev;

		ret = sysfs_create_file(kobj, &p->attr);
		if (ret < 0)
		{
			pr_red_info("sysfs_create_file %s failed", p->attr.name);

			for (p--; p >= attrs; p--)
			{
				sysfs_remove_file(kobj, &p->attr);
			}

			return ret;
		}
	}

	return 0;
}

EXPORT_SYMBOL_GPL(hua_input_create_sysfs_files);

void hua_input_remove_sysfs_files(struct kobject *kobj, struct hua_input_attribute *attrs, size_t count)
{
	struct hua_input_attribute *p, *p_end;

	for (p = attrs, p_end = p + count; p < p_end; p++)
	{
		sysfs_remove_file(kobj, &p->attr);
	}
}

EXPORT_SYMBOL_GPL(hua_input_remove_sysfs_files);

// ================================================================================

static int hua_misc_device_open(struct inode *inode, struct file *file)
{
	struct hua_misc_device *dev = container_of(file->f_op, struct hua_misc_device, fops);

	pr_pos_info();

	file->private_data = dev;

	return dev->open ? dev->open(dev) : 0;
}

static int hua_misc_device_release(struct inode *inode, struct file *file)
{
	struct hua_misc_device *dev = file->private_data;

	pr_pos_info();

	return dev->release ? dev->release(dev) : 0;
}

static ssize_t hua_misc_device_read(struct file *file, char __user *buff, size_t size, loff_t *offset)
{
	struct hua_misc_device *dev = file->private_data;

	return dev->read ? dev->read(dev, buff, size, offset) : -EIO;
}

static ssize_t hua_misc_device_write(struct file *file, const char __user *buff, size_t size, loff_t *offset)
{
	struct hua_misc_device *dev = file->private_data;

	return dev->write ? dev->write(dev, buff, size, offset) : -EIO;
}

static int hua_misc_device_ioctl(struct inode *inode, struct file *file, unsigned int command, unsigned long args)
{
	struct hua_misc_device *dev = file->private_data;

	return dev->ioctl ? dev->ioctl(dev, command, args) : -EINVAL;
}

int hua_misc_device_register(struct hua_misc_device *dev, const char *name)
{
	struct miscdevice *mdev = &dev->dev;
	struct file_operations *fops = &dev->fops;

	fops->open = hua_misc_device_open;
	fops->release = hua_misc_device_release;
	fops->read = hua_misc_device_read;
	fops->write = hua_misc_device_write;
	fops->ioctl = hua_misc_device_ioctl;

	mdev->name = name;
	mdev->minor = MISC_DYNAMIC_MINOR;
	mdev->fops = fops;

	return misc_register(mdev);
}

EXPORT_SYMBOL_GPL(hua_misc_device_register);

void hua_misc_device_unregister(struct hua_misc_device *dev)
{
	misc_deregister(&dev->dev);
}

EXPORT_SYMBOL_GPL(hua_misc_device_unregister);

// ================================================================================

static int hua_input_thread_handler(void *data)
{
	struct hua_input_thread *thread = data;

	pr_pos_info();

	mutex_lock(&thread->lock);

	while (1)
	{
		while (thread->state == HUA_INPUT_THREAD_STATE_SUSPEND)
		{
			pr_green_info("huamobile input thread %s suspend", thread->name);

			set_current_state(TASK_UNINTERRUPTIBLE);
			mutex_unlock(&thread->lock);
			schedule();
			mutex_lock(&thread->lock);
		}

		if (thread->state == HUA_INPUT_THREAD_STATE_STOPPING)
		{
			break;
		}

		thread->state = HUA_INPUT_THREAD_STATE_RUNNING;
		pr_green_info("huamobile input thread %s running", thread->name);

		if (thread->prepare)
		{
			thread->prepare(thread, true);
		}

		while (1)
		{
			mutex_unlock(&thread->lock);
			thread->wait_for_event(thread);
			mutex_lock(&thread->lock);

			if (unlikely(thread->state != HUA_INPUT_THREAD_STATE_RUNNING))
			{
				break;
			}

			thread->event_handle(thread);
		}

		if (thread->prepare)
		{
			thread->prepare(thread, false);
		}
	}

	thread->task = NULL;
	thread->state = HUA_INPUT_THREAD_STATE_STOPPED;
	pr_green_info("huamobile input thread %s stoped", thread->name);

	mutex_unlock(&thread->lock);

	return 0;
}

static int hua_input_thread_prepare(struct hua_input_thread *thread, enum hua_input_thread_state state)
{
	pr_pos_info();

	mutex_lock(&thread->lock);

	if (thread->task == NULL)
	{
		thread->task = kthread_create(hua_input_thread_handler, thread, thread->name);
		if (thread->task == NULL)
		{
			pr_red_info("kthread_create");
			mutex_unlock(&thread->lock);
			return -EFAULT;
		}

		if (thread->priority > 0)
		{
			struct sched_param param =
			{
				.sched_priority = thread->priority
			};

			sched_setscheduler(thread->task, SCHED_FIFO, &param);
		}
	}

	thread->state = state;

	mutex_unlock(&thread->lock);

	return 0;
}

static void hua_input_thread_stop(struct hua_input_thread *thread)
{
	mutex_lock(&thread->lock);

	pr_pos_info();

	if (thread->task && thread->state != HUA_INPUT_THREAD_STATE_STOPPED)
	{
		thread->state = HUA_INPUT_THREAD_STATE_STOPPING;

		while (thread->state != HUA_INPUT_THREAD_STATE_STOPPED)
		{
			pr_func_info("thread name = %s", thread->name);

			if (thread->stop)
			{
				mutex_unlock(&thread->lock);
				thread->stop(thread);
				mutex_lock(&thread->lock);
			}

			wake_up_process(thread->task);

			mutex_unlock(&thread->lock);
			msleep(1);
			mutex_lock(&thread->lock);
		}
	}
	else
	{
		thread->state = HUA_INPUT_THREAD_STATE_STOPPED;
	}

	mutex_unlock(&thread->lock);
}

static int hua_input_thread_set_state(struct hua_input_thread *thread, enum hua_input_thread_state state)
{
	if (state == HUA_INPUT_THREAD_STATE_RUNNING)
	{
		int ret;

		ret = hua_input_thread_prepare(thread, state);
		if (ret < 0)
		{
			pr_red_info("hua_input_thread_prepare");
			return ret;
		}

		wake_up_process(thread->task);
	}
	else
	{
		int locked = mutex_trylock(&thread->lock);

		thread->state = state;

		if (locked)
		{
			mutex_unlock(&thread->lock);
		}
	}

	return 0;
}

static int hua_input_thread_suspend(struct hua_input_thread *thread)
{
	return hua_input_thread_set_state(thread, HUA_INPUT_THREAD_STATE_SUSPEND);
}

static int hua_input_thread_resume(struct hua_input_thread *thread)
{
	return hua_input_thread_set_state(thread, HUA_INPUT_THREAD_STATE_RUNNING);
}

static int hua_input_thread_init(struct hua_input_thread *thread, const char *format, ...)
{
	va_list ap;

	if (thread->event_handle == NULL || thread->wait_for_event == NULL)
	{
		pr_red_info("thread->event_handle == NULL || thread->wait_for_event == NULL");
		return -EINVAL;
	}

	va_start(ap, format);
	thread->name = kvasprintf(GFP_KERNEL, format, ap);
	va_end(ap);

	if (thread->name == NULL)
	{
		pr_red_info("kvasprintf");
		return -ENOMEM;
	}

	thread->task = NULL;
	thread->state = HUA_INPUT_THREAD_STATE_STOPPED;

	mutex_init(&thread->lock);

	return 0;
}

static void hua_input_thread_destroy(struct hua_input_thread *thread)
{
	kfree(thread->name);
	mutex_destroy(&thread->lock);
}

// ================================================================================

static const char *hua_input_device_type_tostring(enum hua_input_device_type type)
{
	switch (type)
	{
	case HUA_INPUT_DEVICE_TYPE_TOUCHSCREEN:
		return "TouchScreen";
	case HUA_INPUT_DEVICE_TYPE_ACCELEROMETER:
		return "Acceleration";
	case HUA_INPUT_DEVICE_TYPE_MAGNETIC_FIELD:
		return "MagneticField";
	case HUA_INPUT_DEVICE_TYPE_ORIENTATION:
		return "Orientation";
	case HUA_INPUT_DEVICE_TYPE_GYROSCOPE:
		return "Gyroscope";
	case HUA_INPUT_DEVICE_TYPE_LIGHT:
		return "Light";
	case HUA_INPUT_DEVICE_TYPE_PRESSURE:
		return "Pressure";
	case HUA_INPUT_DEVICE_TYPE_TEMPERATURE:
		return "Gravity";
	case HUA_INPUT_DEVICE_TYPE_PROXIMITY:
		return "Proximity";
	case HUA_INPUT_DEVICE_TYPE_GRAVITY:
		return "Gravity";
	case HUA_INPUT_DEVICE_TYPE_LINEAR_ACCELERATION:
		return "LinearAcceleration";
	case HUA_INPUT_DEVICE_TYPE_ROTATION_VECTOR:
		return "RotationVector";
	default:
		return "unknown";
	}
}

static const char *hua_input_irq_trigger_type_tostring(unsigned long irq_flags)
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

static irqreturn_t hua_input_isr_edge(int irq, void *dev_id)
{
	struct hua_input_chip *chip = (struct hua_input_chip *)dev_id;

	complete(&chip->event_completion);

	return IRQ_HANDLED;
}

static irqreturn_t hua_input_isr_level(int irq, void *dev_id)
{
	struct hua_input_chip *chip = (struct hua_input_chip *)dev_id;

	disable_irq_nosync(irq);
	complete(&chip->event_completion);

	return IRQ_HANDLED;
}

static inline void hua_input_chip_wait_for_event_edge(struct hua_input_thread *thread)
{
	struct hua_input_chip *chip = hua_input_thread_get_data(thread);

	wait_for_completion(&chip->event_completion);
}

static inline void hua_input_chip_wait_for_event_level(struct hua_input_thread *thread)
{
	struct hua_input_chip *chip = hua_input_thread_get_data(thread);

	enable_irq(chip->irq);
	wait_for_completion(&chip->event_completion);
}

static inline void hua_input_chip_wait_for_event_poll(struct hua_input_thread *thread)
{
	struct hua_input_chip *chip = hua_input_thread_get_data(thread);

	set_current_state(TASK_UNINTERRUPTIBLE);
	schedule_timeout(chip->poll_jiffies);
}

static int hua_input_chip_request_irq(struct hua_input_chip *chip)
{
	int ret;
	irq_handler_t handler;

	if (chip->irq < 0)
	{
		pr_func_info("chip %s don't have irq", chip->name);
		return 0;
	}

	if (chip->irq_flags & (IRQF_TRIGGER_HIGH | IRQF_TRIGGER_LOW))
	{
		chip->irq_type = HUA_INPUT_IRQ_TYPE_LEVEL;
		chip->isr_thread.wait_for_event = hua_input_chip_wait_for_event_level;
		handler = hua_input_isr_level;
	}
	else if (chip->irq_flags & (IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING))
	{
		chip->irq_type = HUA_INPUT_IRQ_TYPE_EDGE;
		chip->isr_thread.wait_for_event = hua_input_chip_wait_for_event_edge;
		handler = hua_input_isr_edge;
	}
	else
	{
		pr_red_info("invalid irq flag 0x%08lx", chip->irq_flags);
		return -EINVAL;
	}

	init_completion(&chip->event_completion);

	ret = request_irq(chip->irq, handler, chip->irq_flags, chip->name, chip);
	if (ret < 0)
	{
		pr_red_info("request_irq %d", chip->irq);
		return ret;
	}

	disable_irq(chip->irq);

	pr_green_info("%s irq trigger type is %s", chip->name, hua_input_irq_trigger_type_tostring(chip->irq_flags));

	return 0;
}

static inline void hua_input_chip_free_irq(struct hua_input_chip *chip)
{
	if (chip->irq >= 0)
	{
		free_irq(chip->irq, chip);
	}
}

static struct hua_input_firmware *hua_input_firmware_alloc(size_t size)
{
	struct hua_input_firmware *fw;

	pr_bold_info("Firmware Size = %d", size);

	fw = kmalloc(sizeof(struct hua_input_firmware) + size, GFP_KERNEL);
	if (fw == NULL)
	{
		pr_red_info("kmalloc");
		return NULL;
	}

	fw->data = (void *)(fw + 1);
	fw->max_size = size;
	fw->size = 0;
	fw->state = 0;

	return fw;
}

static void hua_input_firmware_free(struct hua_input_firmware *fw)
{
	kfree(fw);
}

char *hua_input_print_memory(const void *mem, size_t size)
{
	const u8 *p, *p_end;

	printk("Memory[%d] = ", size);

	for (p = mem, p_end = p + size; p < p_end; p++)
	{
		printk("%02x", *p);
	}

	printk("\n");

	return (char *)p;
}
EXPORT_SYMBOL_GPL(hua_input_print_memory);

// ================================================================================

static int hua_input_chip_set_power(struct hua_input_chip *chip, bool enable)
{
	int ret = 0;

	if (chip->powered == enable)
	{
		pr_func_info("Nothing to be done");
		return 0;
	}

	if (enable && chip->set_power && (ret = chip->set_power(chip, true)) < 0)
	{
		pr_red_info("chip->set_power");
		return ret;
	}

	if (enable && chip->poweron_init && (ret = hua_input_chip_write_init_data(chip)) < 0)
	{
		pr_red_info("hua_input_chip_write_init_data");
		enable = false;
	}

	if (enable == false && chip->set_power)
	{
		chip->set_power(chip, false);
	}

	chip->powered = enable;

	pr_bold_info("huamobie input chip %s power %s", chip->name, enable ? "enable" : "disable");

	return ret;
}

int hua_input_chip_set_power_lock(struct hua_input_chip *chip, bool enable)
{
	int ret;

	mutex_lock(&chip->lock);
	ret = hua_input_chip_set_power(chip, enable);
	mutex_unlock(&chip->lock);

	return ret;
}

EXPORT_SYMBOL_GPL(hua_input_chip_set_power_lock);

static int hua_input_chip_update_delay(struct hua_input_chip *chip)
{
	struct hua_input_list *list = &chip->poll_list;

	if (hua_input_list_empty(list))
	{
		chip->poll_jiffies = MAX_SCHEDULE_TIMEOUT;
	}
	else
	{
		int ret;
		int count;
		unsigned int delay;
		struct list_head *head = &list->head;
		struct hua_input_device *dev;

		mutex_lock(&list->lock);

		count = 0;
		delay = -1;

		list_for_each_entry(dev, head, node)
		{
			if (count == 0 || dev->poll_delay < delay)
			{
				delay = dev->poll_delay;
			}

			count++;
		}

		pr_green_info("haumobile input chip poll count = %d, delay = %d(ms)", count, delay);

		list_for_each_entry(dev, head, node)
		{
			if (dev->set_delay && (ret = dev->set_delay(dev, delay)))
			{
				pr_red_info("dev->set_delay");
				mutex_unlock(&list->lock);
				return ret;
			}
		}

		mutex_unlock(&list->lock);

		chip->poll_jiffies = msecs_to_jiffies(delay);

		pr_func_info("delay = %d(ms)", delay);
	}

	return 0;
}

static int hua_input_chip_update_delay_lock(struct hua_input_chip *chip)
{
	int ret;

	mutex_lock(&chip->lock);
	ret = hua_input_chip_update_delay(chip);
	mutex_unlock(&chip->lock);

	return ret;
}

static int hua_input_chip_update_thread_state(struct hua_input_chip *chip)
{
	int count = 0;

	if (hua_input_list_empty(&chip->isr_list))
	{
		hua_input_thread_suspend(&chip->isr_thread);
	}
	else
	{
		hua_input_thread_resume(&chip->isr_thread);
		count++;
	}

	if (hua_input_list_empty(&chip->poll_list))
	{
		hua_input_thread_suspend(&chip->poll_thread);
	}
	else
	{
		hua_input_thread_resume(&chip->poll_thread);
		count++;
	}

	hua_input_chip_set_power_lock(chip, count > 0);

	return 0;
}

static int hua_input_device_set_delay(struct hua_input_device *dev, unsigned int delay)
{
	int ret;
	unsigned int delay_bak;

	pr_func_info("name = %s, delay = %d", dev->name, delay);

	delay_bak = dev->poll_delay;
	dev->poll_delay = delay;

	ret = hua_input_chip_update_delay_lock(dev->chip);
	if (ret < 0)
	{
		pr_red_info("hua_input_chip_update_delay");
		dev->poll_delay = delay_bak;
		return ret;
	}

	return 0;
}

static int hua_input_device_set_delay_lock(struct hua_input_device *dev, unsigned int delay)
{
	int ret;

	mutex_lock(&dev->lock);
	ret = hua_input_device_set_delay(dev, delay);
	mutex_unlock(&dev->lock);

	return ret;
}

static int hua_input_device_set_enable(struct hua_input_device *dev, bool enable)
{
	int ret = 0;
	struct hua_input_chip *chip = dev->chip;
	struct hua_input_list *work_list;

	pr_pos_info();

	if (dev->enabled == enable)
	{
		pr_func_info("Nothing to be done");
		return 0;
	}

	if (enable)
	{
		ret = hua_input_chip_set_power_lock(chip, true);
		if (ret < 0)
		{
			pr_red_info("hua_input_chip_set_power");
			return ret;
		}

		if (dev->set_enable && (ret = dev->set_enable(dev, true)) < 0)
		{
			pr_red_info("dev->set_enable");
			enable = false;
		}
	}

	work_list = dev->use_irq ? &chip->isr_list : &chip->poll_list;

	if (enable == false)
	{
		hua_input_list_del(work_list, &dev->node);
		hua_input_list_add(&chip->dev_list, &dev->node);

		if (dev->set_enable)
		{
			dev->set_enable(dev, false);
		}
	}
	else
	{
		hua_input_list_del(&chip->dev_list, &dev->node);
		hua_input_list_add(work_list, &dev->node);
		hua_input_chip_update_delay_lock(chip);
	}

	hua_input_chip_update_thread_state(chip);
	dev->enabled = enable;

	pr_bold_info("huamobie input device %s-%s %s", chip->name, dev->name, enable ? "enable" : "disable");

	return ret;
}

int hua_input_device_set_enable_lock(struct hua_input_device *dev, bool enable)
{
	int ret;

	mutex_lock(&dev->lock);
	ret = hua_input_device_set_enable(dev, enable);
	mutex_unlock(&dev->lock);

	return ret;
}

EXPORT_SYMBOL_GPL(hua_input_device_set_enable_lock);

static int hua_input_chip_firmware_upgrade(struct hua_input_chip *chip, const void *buff, size_t size)
{
	int ret;

	pr_func_info("size = %d", size);

	hua_input_thread_set_state(&chip->isr_thread, HUA_INPUT_THREAD_STATE_SUSPEND);
	hua_input_thread_set_state(&chip->poll_thread, HUA_INPUT_THREAD_STATE_SUSPEND);

	mutex_lock(&chip->lock);
	wake_lock(&chip->wake_lock);

	ret = hua_input_chip_set_power(chip, true);
	if (ret < 0)
	{
		pr_red_info("hua_input_chip_set_power");
	}
	else
	{
		ret = chip->firmware_upgrade(chip, buff, size);
		if (ret < 0)
		{
			pr_red_info("chip->firmware_upgrade");
		}
		else
		{
			pr_green_info("huamobile input chip %s firmware upgrade complete", chip->name);
		}
	}

	wake_unlock(&chip->wake_lock);
	mutex_unlock(&chip->lock);

	hua_input_chip_update_thread_state(chip);

	return ret;
}

static int hua_input_chip_open(struct hua_misc_device *dev)
{
	struct hua_input_firmware *fw;
	struct hua_input_chip *chip = hua_misc_device_get_data(dev);

	pr_pos_info();

	mutex_lock(&chip->lock);

	if (chip->firmware_size == 0 || chip->firmware_upgrade == NULL)
	{
		pr_red_info("chip->firmware_size == 0 || chip->firmware_upgrade == NULL");
		mutex_unlock(&chip->lock);
		return -EINVAL;
	}

	fw = hua_input_firmware_alloc(chip->firmware_size);
	if (fw == NULL)
	{
		pr_red_info("hua_input_firmware_alloc");
		mutex_unlock(&chip->lock);
		return -ENOMEM;
	}

	hua_input_chip_set_misc_data(chip, fw);

	mutex_unlock(&chip->lock);

	return 0;
}

static int hua_input_chip_release(struct hua_misc_device *dev)
{
	int ret;
	struct hua_input_chip *chip = hua_misc_device_get_data(dev);
	struct hua_input_firmware *fw = hua_input_chip_get_misc_data(chip);

	if (fw->state < 0)
	{
		pr_red_info("fw->state < 0");
		ret = fw->state;
		goto out_hua_input_firmware_free;
	}

	ret = hua_input_chip_firmware_upgrade(chip, fw->data, fw->size);
	if (ret < 0)
	{
		pr_red_info("hua_input_chip_firmware_upgrade");
	}

out_hua_input_firmware_free:
	hua_input_firmware_free(fw);
	return ret;
}

static ssize_t hua_input_chip_write(struct hua_misc_device *dev, const char __user *buff, size_t size, loff_t *offset)
{
	struct hua_input_chip *chip = hua_misc_device_get_data(dev);
	struct hua_input_firmware *fw = hua_input_chip_get_misc_data(chip);

	if (fw->size + size > fw->max_size)
	{
		pr_red_info("fw->size + size > fw->max_size");
		fw->state = -ENOMEM;
		return fw->state;
	}

	if (copy_from_user(fw->data + fw->size, buff, size))
	{
		pr_red_info("copy_from_user");
		fw->state = -EFAULT;
		return fw->state;
	}

	fw->size += size;

	return size;
}

static int hua_input_chip_probe(struct hua_input_chip *chip)
{
	int ret;
	struct hua_misc_device *mdev;

	pr_pos_info();

	pr_green_info("Try input chip %s", chip->name);

	ret = hua_input_chip_set_power_lock(chip, true);
	if (ret < 0)
	{
		pr_red_info("hua_input_chip_set_power");
		return ret;
	}

	if (chip->readid && (ret = chip->readid(chip)) < 0)
	{
		pr_red_info("chip->readid");
		goto out_power_down;
	}

	if (chip->poweron_init == false && (ret = hua_input_chip_write_init_data(chip)) < 0)
	{
		pr_red_info("hua_input_chip_write_init_data");
		goto out_power_down;
	}

	ret = hua_input_chip_request_irq(chip);
	if (ret < 0)
	{
		pr_red_info("hua_input_chip_request_irq");
		goto out_power_down;
	}

	wake_lock_init(&chip->wake_lock, WAKE_LOCK_SUSPEND, chip->name);

	ret = chip->probe(chip);
	if (ret < 0)
	{
		pr_red_info("chip->probe");
		goto out_wake_lock_destroy;
	}

	chip->misc_name = kasprintf(GFP_KERNEL, "HUA-%s", chip->name);
	if (chip->misc_name == NULL)
	{
		pr_red_info("kasprintf");
		goto out_chip_remove;
	}

	mdev = &chip->misc_dev;
	hua_misc_device_set_data(mdev, chip);
	mdev->open = hua_input_chip_open;
	mdev->release = hua_input_chip_release;
	mdev->write = hua_input_chip_write;

	ret = hua_misc_device_register(mdev, chip->misc_name);
	if (ret < 0)
	{
		pr_red_info("hua_misc_device_register");
		goto out_kfree_chip_misc_name;
	}

	pr_green_info("huamobile input chip %s probe complete", chip->name);

	return 0;

out_kfree_chip_misc_name:
	kfree(chip->misc_name);
out_chip_remove:
	chip->remove(chip);
out_wake_lock_destroy:
	wake_lock_destroy(&chip->wake_lock);
	hua_input_chip_free_irq(chip);
out_power_down:
	hua_input_chip_set_power(chip, false);
	return ret;
}

static void hua_input_chip_remove(struct hua_input_chip *chip)
{
	hua_misc_device_unregister(&chip->misc_dev);
	kfree(chip->misc_name);

	chip->remove(chip);

	wake_lock_destroy(&chip->wake_lock);

	hua_input_thread_stop(&chip->poll_thread);
	hua_input_thread_stop(&chip->isr_thread);

	hua_input_chip_free_irq(chip);

	hua_input_chip_set_power_lock(chip, false);
}

void hua_input_chip_report_events(struct hua_input_chip *chip, struct hua_input_list *list)
{
	struct hua_input_device *dev;
	struct list_head *head;

	mutex_lock(&list->lock);

	head = &list->head;

	list_for_each_entry(dev, head, node)
	{
		dev->event_handler(chip, dev);
	}

	mutex_unlock(&list->lock);
}

EXPORT_SYMBOL_GPL(hua_input_chip_report_events);

static void hua_input_chip_event_handler_isr(struct hua_input_thread *thread)
{
	struct hua_input_chip *chip = hua_input_thread_get_data(thread);

	hua_input_chip_report_events(chip, &chip->isr_list);
}

static void hua_input_chip_event_handler_isr_user(struct hua_input_thread *thread)
{
	struct hua_input_chip *chip = hua_input_thread_get_data(thread);

	chip->event_handler(chip);
}

static void hua_input_chip_event_handler_poll(struct hua_input_thread *thread)
{
	struct hua_input_chip *chip = hua_input_thread_get_data(thread);

	hua_input_chip_report_events(chip, &chip->poll_list);
}

static void hua_input_chip_isr_thread_stop(struct hua_input_thread *thread)
{
	struct hua_input_chip *chip = hua_input_thread_get_data(thread);

	complete(&chip->event_completion);
}

static void hua_input_chip_isr_thread_prepare(struct hua_input_thread *thread, bool enable)
{
	struct hua_input_chip *chip = hua_input_thread_get_data(thread);

	if (chip->irq_type == HUA_INPUT_IRQ_TYPE_EDGE && chip->irq > 0)
	{
		if (enable)
		{
			enable_irq(chip->irq);
		}
		else
		{
			disable_irq(chip->irq);
		}
	}
}

static int hua_input_chip_init(struct hua_input_core *core, struct hua_input_chip *chip)
{
	int ret;
	struct hua_input_thread *thread;

	pr_pos_info();

	if (chip->name == NULL)
	{
		pr_red_info("chip->name == NULL");
		return -EINVAL;
	}

	if (chip->probe == NULL || chip->remove == NULL)
	{
		pr_red_info("chip->probe == NULL || chip->remove == NULL");
		return -EINVAL;
	}

	if (chip->read_data == NULL || chip->write_data == NULL)
	{
		pr_red_info("chip->read_data == NULL || chip->write_data == NULL");
		return -EINVAL;
	}

	if (chip->vendor == NULL)
	{
		chip->vendor = "HuaMobile";
	}

	if (chip->read_register == NULL)
	{
		chip->read_register = hua_input_read_register_dummy;
	}

	if (chip->write_register == NULL)
	{
		chip->write_register = hua_input_write_register_dummy;
	}

	chip->core = core;
	chip->powered = false;

	hua_input_list_init(&chip->isr_list);
	hua_input_list_init(&chip->poll_list);
	hua_input_list_init(&chip->dev_list);

	mutex_init(&chip->lock);

	thread = &chip->isr_thread;
	thread->priority = MAX_PRIO;
	hua_input_thread_set_data(thread, chip);
	thread->stop = hua_input_chip_isr_thread_stop;
	thread->prepare = hua_input_chip_isr_thread_prepare;
	thread->wait_for_event = hua_input_chip_wait_for_event_edge;
	if (chip->event_handler)
	{
		thread->event_handle = hua_input_chip_event_handler_isr_user;
	}
	else
	{
		thread->event_handle = hua_input_chip_event_handler_isr;
	}

	ret = hua_input_thread_init(thread, "%s-ISR", chip->name);
	if (ret < 0)
	{
		pr_red_info("hua_input_thread_init");
		return ret;
	}

	thread = &chip->poll_thread;
	thread->priority = 0;
	hua_input_thread_set_data(thread, chip);
	thread->stop = NULL;
	thread->prepare = NULL;
	thread->wait_for_event = hua_input_chip_wait_for_event_poll;
	thread->event_handle = hua_input_chip_event_handler_poll;

	ret = hua_input_thread_init(thread, "%s-POLL", chip->name);
	if (ret < 0)
	{
		pr_red_info("hua_input_thread_init");
		hua_input_thread_destroy(&chip->isr_thread);
		return ret;
	}

	return 0;
}

static void hua_input_chip_destroy(struct hua_input_chip *chip)
{
	pr_pos_info();

	mutex_destroy(&chip->lock);

	hua_input_list_destory(&chip->isr_list);
	hua_input_list_destory(&chip->poll_list);
	hua_input_list_destory(&chip->dev_list);
}

static ssize_t hua_input_device_read(struct hua_misc_device *dev, char __user *buff, size_t size, loff_t *offset)
{
	char *p, *p_end;
	struct hua_input_device *idev;

	if (*offset)
	{
		return 0;
	}

	idev = hua_misc_device_get_data(dev);

	p = buff;
	p_end = p + size - 1;

	p += snprintf(p, p_end - p, "name = %s\n", idev->name);
	p += snprintf(p, p_end - p, "type = %d\n", idev->type);
	p += snprintf(p, p_end - p, "enable = %d\n", idev->enabled);
	p += snprintf(p, p_end - p, "delay = %dms\n", idev->poll_delay);

	size = p - buff + 1;
	*offset += size;

	return size;
}

static ssize_t hua_input_device_write(struct hua_misc_device *dev, const char __user *buff, size_t size, loff_t *offset)
{
	int ret;
	struct hua_input_device *idev = hua_misc_device_get_data(dev);

	ret = hua_input_device_set_enable_lock(idev, size > 0 && buff[0] > '0');
	if (ret < 0)
	{
		pr_red_info("hua_input_device_set_enable_lock");
		return ret;
	}

	return size;
}

static int hua_input_device_ioctl(struct hua_misc_device *dev, unsigned int command, unsigned long args)
{
	struct hua_input_device *idev = hua_misc_device_get_data(dev);

	switch (HUA_INPUT_IOC_GET_CMD_RAW(command))
	{
	case HUA_INPUT_CHIP_IOC_GET_NAME(0):
		return hua_input_copy_to_user_text(command, args, idev->chip->name);

	case HUA_INPUT_CHIP_IOC_GET_VENDOR(0):
		return hua_input_copy_to_user_text(command, args, idev->chip->vendor);

	case HUA_INPUT_CHIP_IOC_SET_FW_SIZE:
		idev->chip->firmware_size = args;
		return 0;

	case HUA_INPUT_DEVICE_IOC_GET_TYPE:
		return hua_input_copy_to_user_uint(args, idev->type);

	case HUA_INPUT_DEVICE_IOC_GET_NAME(0):
		return hua_input_copy_to_user_text(command, args, idev->name);

	case HUA_INPUT_DEVICE_IOC_SET_DELAY:
		return hua_input_device_set_delay_lock(idev, args);

	case HUA_INPUT_DEVICE_IOC_SET_ENABLE:
		return hua_input_device_set_enable_lock(idev, args > 0);

	default:
		if (idev->ioctl)
		{
			return idev->ioctl(idev, command, args);
		}
	}

	pr_red_info("Invalid IOCTL 0x%08x", command);

	return -EINVAL;
}

static int hua_input_device_probe(struct hua_input_chip *chip, struct hua_input_device *dev)
{
	int ret;
	struct input_dev *input;
	const char *devname;
	struct hua_misc_device *mdev = &dev->misc_dev;

	pr_pos_info();

	if (chip->irq < 0 && dev->use_irq)
	{
		pr_red_info("chip %s don't have irq", chip->name);
		return -EINVAL;
	}

	if (dev->event_handler == NULL)
	{
		pr_red_info("dev->event_handler == NULL");
		return -EINVAL;
	}

	devname = hua_input_device_type_tostring(dev->type);

	if ((chip->devmask & (1 << dev->type)) == 0)
	{
		pr_red_info("chip %s don't support device %s", chip->name, devname);
		return -EINVAL;
	}

	mutex_init(&dev->lock);

	dev->chip = chip;
	dev->enabled = false;

	if (dev->name == NULL)
	{
		dev->name = devname;
	}

	input = input_allocate_device();
	if (input == NULL)
	{
		ret = -ENOMEM;
		pr_red_info("input_allocate_device");
		goto out_mutex_destroy;
	}

	dev->misc_name = kasprintf(GFP_KERNEL, "HUA-%s-%s", chip->name, devname);
	if (dev->misc_name == NULL)
	{
		ret = -ENOMEM;
		pr_red_info("asprintf");
		goto out_input_free_device;
	}

	input->name = dev->misc_name;
	dev->input = input;
	input_set_drvdata(input, dev);

	if (dev->type == HUA_INPUT_DEVICE_TYPE_TOUCHSCREEN)
	{
		ret = hua_ts_device_probe(dev);
	}
	else
	{
		ret = hua_sensor_device_probe(dev);
	}

	if (ret < 0)
	{
		pr_red_info("probe device failed");
		mutex_lock(&chip->lock);
		goto out_kfree_input_name;
	}

	ret = input_register_device(input);
	if (ret < 0)
	{
		pr_red_info("input_register_device");
		mutex_lock(&chip->lock);
		goto out_device_remove;
	}

	hua_misc_device_set_data(mdev, dev);
	mdev = &dev->misc_dev;
	mdev->read = hua_input_device_read;
	mdev->write = hua_input_device_write;
	mdev->ioctl = hua_input_device_ioctl;

	ret = hua_misc_device_register(mdev, dev->misc_name);
	if (ret < 0)
	{
		pr_red_info("hua_misc_device_register");
		goto out_input_unregister_device;
	}

	pr_green_info("huamobile input deivce %s probe complete", dev->name);

	return 0;

out_input_unregister_device:
	input_unregister_device(input);
out_device_remove:
	if (dev->remove)
	{
		dev->remove(dev);
	}
out_kfree_input_name:
	kfree(dev->misc_name);
out_input_free_device:
	input_free_device(input);
out_mutex_destroy:
	mutex_destroy(&dev->lock);
	return ret;
}

static void hua_input_device_remove(struct hua_input_chip *chip, struct hua_input_device *dev)
{
	struct input_dev *input = dev->input;

	hua_misc_device_unregister(&dev->misc_dev);
	input_unregister_device(input);

	if (dev->remove)
	{
		dev->remove(dev);
	}

	kfree(dev->misc_name);
	input_free_device(input);
	mutex_destroy(&dev->lock);
}

int hua_input_device_register(struct hua_input_chip *chip, struct hua_input_device *dev)
{
	int ret;

	hua_input_list_add(&chip->dev_list, &dev->node);

	ret = hua_input_device_probe(chip, dev);
	if (ret < 0)
	{
		pr_red_info("hua_input_device_probe");
		goto out_list_del;
	}

	pr_green_info("huamobile input deivce %s register complete", dev->name);

	return 0;

out_list_del:
	hua_input_list_del(&chip->dev_list, &dev->node);
	return ret;
}

EXPORT_SYMBOL_GPL(hua_input_device_register);

void hua_input_device_unregister(struct hua_input_chip *chip, struct hua_input_device *dev)
{
	mutex_lock(&dev->lock);
	hua_input_device_set_enable(dev, false);

	mutex_lock(&chip->lock);
	list_del(&dev->node);
	mutex_unlock(&dev->lock);

	hua_input_device_remove(chip, dev);

	mutex_unlock(&chip->lock);
}

EXPORT_SYMBOL_GPL(hua_input_device_unregister);

// ================================================================================

int hua_input_chip_register(struct hua_input_chip *chip)
{
	int ret;

	pr_pos_info();

	hua_input_list_add(&input_core.chip_list, &chip->node);

	ret = hua_input_chip_init(&input_core, chip);
	if (ret < 0)
	{
		pr_red_info("hua_input_chip_init");
		goto out_list_del;
	}

	hua_input_thread_resume(&input_core.detect_thread);

	return 0;

out_list_del:
	hua_input_list_del(&input_core.chip_list, &chip->node);
	return ret;
}

EXPORT_SYMBOL_GPL(hua_input_chip_register);

void hua_input_chip_unregister(struct hua_input_chip *chip)
{
	mutex_lock(&input_core.lock);
	mutex_lock(&chip->lock);

	if (hua_input_list_has_node(&input_core.work_list, &chip->node))
	{
		mutex_unlock(&chip->lock);
		hua_input_chip_remove(chip);
		mutex_lock(&chip->lock);
	}

	list_del(&chip->node);

	mutex_unlock(&chip->lock);
	hua_input_chip_destroy(chip);

	input_core.devmask &= ~(chip->devmask);

	mutex_unlock(&input_core.lock);
}

EXPORT_SYMBOL_GPL(hua_input_chip_unregister);

static void hua_input_core_wait_for_event(struct hua_input_thread *thread)
{
	struct hua_input_core *core = hua_input_thread_get_data(thread);

	set_current_state(TASK_UNINTERRUPTIBLE);
	schedule_timeout(core->poll_jiffies);
}

static void hua_input_core_event_handler(struct hua_input_thread *thread)
{
	struct hua_input_chip *chip;
	struct hua_input_core *core = hua_input_thread_get_data(thread);
	struct hua_input_list *list = &core->chip_list;
	struct list_head *head = &list->head;

	pr_pos_info();

	mutex_lock(&list->lock);

	list_for_each_entry(chip, head, node)
	{
		mutex_lock(&chip->lock);

		if ((chip->devmask & core->devmask))
		{
			list_del(&chip->node);
			hua_input_list_add(&core->exclude_list, &chip->node);
			chip->poll_jiffies = 0;

			mutex_unlock(&chip->lock);
			break;
		}
		else
		{
			mutex_unlock(&chip->lock);

			if (hua_input_chip_probe(chip) >= 0)
			{
				mutex_lock(&chip->lock);

				list_del(&chip->node);
				hua_input_list_add(&core->work_list, &chip->node);
				core->devmask |= chip->devmask;
				chip->poll_jiffies = 0;

				mutex_unlock(&chip->lock);
				break;
			}
		}
	}

	mutex_lock(&core->lock);

	if (list_empty(head))
	{
		thread->state = HUA_INPUT_THREAD_STATE_SUSPEND;
		core->poll_jiffies = 0;
	}
	else
	{
		core->poll_jiffies = core->poll_jiffies << 1 | 1;
	}

	mutex_unlock(&core->lock);

	mutex_unlock(&list->lock);
}

static int __init hua_input_core_init(void)
{
	int ret;
	struct hua_input_thread *thread;

	pr_pos_info();

	hua_input_list_init(&input_core.chip_list);
	hua_input_list_init(&input_core.work_list);
	hua_input_list_init(&input_core.exclude_list);

	mutex_init(&input_core.lock);

	thread = &input_core.detect_thread;
	hua_input_thread_set_data(thread, &input_core);
	thread->priority = 0;
	thread->stop = NULL;
	thread->prepare = NULL;
	thread->event_handle = hua_input_core_event_handler;
	thread->wait_for_event = hua_input_core_wait_for_event;
	ret = hua_input_thread_init(thread, input_core.name);
	if (ret < 0)
	{
		pr_red_info("hua_input_thread_init");
		goto out_mutex_destroy;
	}

	ret = hua_input_add_kobject(&input_core.prop_kobj, "board_properties");
	if (ret < 0)
	{
		pr_red_info("hua_input_add_kobject");
		goto out_hua_input_thread_destroy;
	}

	return 0;

out_hua_input_thread_destroy:
	hua_input_thread_destroy(thread);
out_mutex_destroy:
	mutex_destroy(&input_core.lock);
	hua_input_list_destory(&input_core.chip_list);
	hua_input_list_destory(&input_core.work_list);
	hua_input_list_destory(&input_core.exclude_list);
	return ret;
}

static void __exit hua_input_core_exit(void)
{
	pr_pos_info();

	hua_input_thread_stop(&input_core.detect_thread);
	hua_input_remove_kobject(&input_core.prop_kobj);
	hua_input_thread_destroy(&input_core.detect_thread);

	mutex_destroy(&input_core.lock);

	hua_input_list_destory(&input_core.chip_list);
	hua_input_list_destory(&input_core.work_list);
	hua_input_list_destory(&input_core.exclude_list);
}

module_init(hua_input_core_init);
module_exit(hua_input_core_exit);

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Huamobile Input Subsystem");
MODULE_LICENSE("GPL");
