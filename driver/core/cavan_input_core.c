#include <cavan/cavan_input.h>
#include <cavan/cavan_i2c.h>
#include <linux/compat.h>

#define CAVAN_INPUT_CHIP_FW_CACHE_SIZE	1024

extern int cavan_ts_device_probe(struct cavan_input_device *dev);
extern int cavan_sensor_device_probe(struct cavan_input_device *dev);

static struct cavan_input_core input_core =
{
	.name = "CAVAN-INPUT-CORE"
};

struct class *cavan_input_class;
struct cavan_misc_device *cavan_misc_dev_map[CAVAN_INPUT_MINORS];

int cavan_input_debug_enable = CAVAN_INPUT_DEBUG;

module_param_named(debug_enable, cavan_input_debug_enable, int, S_IRUGO | S_IWUSR | S_IWGRP);
EXPORT_SYMBOL_GPL(cavan_input_debug_enable);

// ================================================================================

static void cavan_input_core_write_online_work(struct work_struct *data)
{
	int i;
	int err_count;
	struct cavan_input_chip *chip;
	struct cavan_input_list *list = &input_core.exclude_list;
	struct list_head *head = &list->head;

	for (i = 0, err_count = 0; i < ARRAY_SIZE(input_core.chip_online); i++)
	{
		if (input_core.chip_online[i] && cavan_input_chip_write_online(input_core.chip_online[i], true) < 0)
		{
			err_count++;
		}
	}

	mutex_lock(&list->lock);

	list_for_each_entry(chip, head, node)
	{
		if (cavan_input_chip_write_online(chip->name, false) < 0)
		{
			err_count++;
		}
	}

	mutex_unlock(&list->lock);

	if (err_count > 0)
	{
		queue_delayed_work(input_core.workqueue, &input_core.write_online_work, 10 * HZ);
	}
}

static int cavan_input_core_add_online_chip(const char *chip_name)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(input_core.chip_online); i++)
	{
		if (input_core.chip_online[i] == NULL)
		{
			input_core.chip_online[i] = chip_name;
			queue_delayed_work(input_core.workqueue, &input_core.write_online_work, 60 * HZ);
			return i;
		}
	}

	return -EBUSY;
}

// ================================================================================

static void cavan_input_list_init(struct cavan_input_list *list)
{
	mutex_init(&list->lock);
	INIT_LIST_HEAD(&list->head);
}

static void cavan_input_list_destory(struct cavan_input_list *list)
{
	mutex_destroy(&list->lock);
}

static void cavan_input_list_add(struct cavan_input_list *list, struct list_head *node)
{
	mutex_lock(&list->lock);
	list_add(node, &list->head);
	mutex_unlock(&list->lock);
}

static void cavan_input_list_del(struct cavan_input_list *list, struct list_head *node)
{
	mutex_lock(&list->lock);
	list_del(node);
	mutex_unlock(&list->lock);
}

static bool cavan_input_list_empty(struct cavan_input_list *list)
{
	bool res;

	mutex_lock(&list->lock);
	res = list_empty(&list->head);
	mutex_unlock(&list->lock);

	return res;
}

static bool cavan_input_list_has_node(struct cavan_input_list *list, struct list_head *node)
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

int cavan_input_copy_to_user_text(unsigned int command, unsigned long args, const char *text)
{
	size_t size = CAVAN_INPUT_IOC_GET_SIZE(command);
	size_t length = strlen(text) + 1;

	if (copy_to_user((void __user *)args, text, length > size ? size : length))
	{
		pr_red_info("copy_to_user");
		return -EFAULT;
	}

	return 0;
}

EXPORT_SYMBOL_GPL(cavan_input_copy_to_user_text);

int cavan_input_copy_to_user_uint(unsigned long args, unsigned int value)
{
	if (copy_to_user((void __user *)args, &value, sizeof(value)))
	{
		pr_red_info("copy_to_user");
		return -EFAULT;
	}

	return 0;
}

EXPORT_SYMBOL_GPL(cavan_input_copy_to_user_uint);

// ================================================================================

static int cavan_input_chip_write_init_data(struct cavan_input_chip *chip)
{
	int ret;
	const struct cavan_input_init_data *data, *data_end;

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
			pr_red_info("cavan_sensor_write_register");
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

static int cavan_misc_device_open(struct inode *inode, struct file *file)
{
	int minor = iminor(inode);
	struct cavan_misc_device *dev = cavan_misc_dev_map[minor];

	pr_pos_info();

	if (dev == NULL)
	{
		return -ENOENT;
	}

	file->private_data = dev;

	return dev->open ? dev->open(dev) : 0;
}

static int cavan_misc_device_release(struct inode *inode, struct file *file)
{
	struct cavan_misc_device *dev = file->private_data;

	pr_pos_info();

	return dev->release ? dev->release(dev) : 0;
}

static ssize_t cavan_misc_device_read(struct file *file, char __user *buff, size_t size, loff_t *offset)
{
	struct cavan_misc_device *dev = file->private_data;

	return dev->read ? dev->read(dev, buff, size, offset) : -EIO;
}

static ssize_t cavan_misc_device_write(struct file *file, const char __user *buff, size_t size, loff_t *offset)
{
	struct cavan_misc_device *dev = file->private_data;

	return dev->write ? dev->write(dev, buff, size, offset) : -EIO;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 0, 0)
static int cavan_misc_device_ioctl(struct inode *inode, struct file *file, unsigned int command, unsigned long args)
#else
static long cavan_misc_device_ioctl(struct file *file, unsigned int command, unsigned long args)
#endif
{
	struct cavan_misc_device *dev = file->private_data;

	return dev->ioctl ? dev->ioctl(dev, command, args) : -EINVAL;
}

#ifdef CONFIG_COMPAT
static long cavan_misc_device_ioctl_compat(struct file *file, unsigned int command, unsigned long args)
{
	struct cavan_misc_device *dev = file->private_data;

	return dev->ioctl ? dev->ioctl(dev, command, compat_ptr(args)) : -EINVAL;
}
#endif

static int cavan_misc_find_minor(void)
{
	int minor;

	for (minor = 0; minor < ARRAY_SIZE(cavan_misc_dev_map); minor++)
	{
		if (cavan_misc_dev_map[minor] == NULL)
		{
			return minor;
		}
	}

	return -EBUSY;
}

int cavan_misc_device_register(struct cavan_misc_device *dev, const char *name)
{
	int minor;

	if (cavan_input_class == NULL)
	{
		pr_red_info("cavan_input_class is null");
		return -EBUSY;
	}

	minor = cavan_misc_find_minor();
	if (minor < 0)
	{
		pr_red_info("cavan_misc_find_minor");
		return minor;
	}

	dev->dev = device_create(cavan_input_class, NULL, MKDEV(CAVAN_INPUT_MAJOR, minor), dev, name);
	if (IS_ERR(dev->dev))
	{
		pr_red_info("device_create");
		return PTR_ERR(dev->dev);
	}

	dev->minor = minor;
	cavan_misc_dev_map[minor] = dev;

	return 0;
}

EXPORT_SYMBOL_GPL(cavan_misc_device_register);

void cavan_misc_device_unregister(struct cavan_misc_device *dev)
{
	device_destroy(cavan_input_class, MKDEV(CAVAN_INPUT_MAJOR, dev->minor));
	cavan_misc_dev_map[dev->minor] = NULL;
}

EXPORT_SYMBOL_GPL(cavan_misc_device_unregister);

// ================================================================================

static const char *cavan_input_device_type_tostring(enum cavan_input_device_type type)
{
	switch (type)
	{
	case CAVAN_INPUT_DEVICE_TYPE_TOUCHSCREEN:
		return "TouchScreen";
	case CAVAN_INPUT_DEVICE_TYPE_ACCELEROMETER:
		return "Acceleration";
	case CAVAN_INPUT_DEVICE_TYPE_MAGNETIC_FIELD:
		return "MagneticField";
	case CAVAN_INPUT_DEVICE_TYPE_ORIENTATION:
		return "Orientation";
	case CAVAN_INPUT_DEVICE_TYPE_GYROSCOPE:
		return "Gyroscope";
	case CAVAN_INPUT_DEVICE_TYPE_LIGHT:
		return "Light";
	case CAVAN_INPUT_DEVICE_TYPE_PRESSURE:
		return "Pressure";
	case CAVAN_INPUT_DEVICE_TYPE_TEMPERATURE:
		return "Gravity";
	case CAVAN_INPUT_DEVICE_TYPE_PROXIMITY:
		return "Proximity";
	case CAVAN_INPUT_DEVICE_TYPE_GRAVITY:
		return "Gravity";
	case CAVAN_INPUT_DEVICE_TYPE_LINEAR_ACCELERATION:
		return "LinearAcceleration";
	case CAVAN_INPUT_DEVICE_TYPE_ROTATION_VECTOR:
		return "RotationVector";
	default:
		return "unknown";
	}
}

const char *cavan_input_irq_trigger_type_tostring(unsigned long irq_flags)
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

static irqreturn_t cavan_input_isr_edge(int irq, void *dev_id)
{
	struct cavan_input_chip *chip = (struct cavan_input_chip *)dev_id;

	complete(&chip->event_completion);

	return IRQ_HANDLED;
}

static irqreturn_t cavan_input_isr_level(int irq, void *dev_id)
{
	struct cavan_input_chip *chip = (struct cavan_input_chip *)dev_id;

	disable_irq_nosync(irq);
	complete(&chip->event_completion);

	return IRQ_HANDLED;
}

static inline void cavan_input_chip_wait_for_event_edge(struct cavan_input_thread *thread)
{
	struct cavan_input_chip *chip = cavan_input_thread_get_data(thread);

	wait_for_completion_timeout(&chip->event_completion, 5 * HZ);
}

static inline void cavan_input_chip_wait_for_event_none(struct cavan_input_thread *thread)
{
}

static inline void cavan_input_chip_wait_for_event_level(struct cavan_input_thread *thread)
{
	struct cavan_input_chip *chip = cavan_input_thread_get_data(thread);

	enable_irq(chip->irq);
	wait_for_completion(&chip->event_completion);
}

static inline void cavan_input_chip_wait_for_event_poll(struct cavan_input_thread *thread)
{
	struct cavan_input_chip *chip = cavan_input_thread_get_data(thread);

	set_current_state(TASK_UNINTERRUPTIBLE);
	schedule_timeout(chip->poll_jiffies);
}

static int cavan_input_chip_request_irq(struct cavan_input_chip *chip)
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
		chip->irq_type = CAVAN_INPUT_IRQ_TYPE_LEVEL;
		chip->isr_thread.wait_for_event = cavan_input_chip_wait_for_event_level;
		handler = cavan_input_isr_level;
	}
	else if (chip->irq_flags & (IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING))
	{
		chip->irq_type = CAVAN_INPUT_IRQ_TYPE_EDGE;
		chip->isr_thread.wait_for_event = cavan_input_chip_wait_for_event_edge;
		handler = cavan_input_isr_edge;
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

	pr_green_info("%s irq trigger type is %s", chip->name, cavan_input_irq_trigger_type_tostring(chip->irq_flags));

	return 0;
}

static inline void cavan_input_chip_free_irq(struct cavan_input_chip *chip)
{
	if (chip->irq >= 0)
	{
		free_irq(chip->irq, chip);
	}
}

char *cavan_input_print_memory(const void *mem, size_t size)
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

EXPORT_SYMBOL_GPL(cavan_input_print_memory);

// ================================================================================

int cavan_input_chip_set_power(struct cavan_input_chip *chip, bool enable)
{
	int ret = 0;

	if (chip->powered == enable)
	{
		pr_func_info("Nothing to be done");
		return 0;
	}

	if (enable)
	{
		if (chip->set_power && (ret = chip->set_power(chip, true)) < 0)
		{
			pr_red_info("chip->set_power");
			return ret;
		}

		if ((chip->flags & CAVAN_INPUT_CHIP_FLAG_POWERON_INIT) && (ret = cavan_input_chip_write_init_data(chip)) < 0)
		{
			pr_red_info("cavan_input_chip_write_init_data");
			enable = false;
		}
	}

	if (enable == false && chip->set_power)
	{
		chip->set_power(chip, false);
	}

	chip->powered = enable;
	chip->recovery_count = 0;

	pr_bold_info("cavan input chip %s power %s", chip->name, enable ? "enable" : "disable");

	return ret;
}

EXPORT_SYMBOL_GPL(cavan_input_chip_set_power);

int cavan_input_chip_set_power_lock(struct cavan_input_chip *chip, bool enable)
{
	int ret;

	mutex_lock(&chip->lock);
	ret = cavan_input_chip_set_power(chip, enable);
	mutex_unlock(&chip->lock);

	return ret;
}

EXPORT_SYMBOL_GPL(cavan_input_chip_set_power_lock);

int cavan_input_chip_set_active(struct cavan_input_chip *chip, bool enable)
{
	int ret = 0;

	if (chip->actived == enable)
	{
		pr_func_info("Nothing to be done");
		return 0;
	}

	if (enable && (ret = cavan_input_chip_set_power(chip, true)) < 0)
	{
		pr_red_info("cavan_input_chip_set_power");
		return ret;
	}

	if (chip->set_active)
	{
		ret = chip->set_active(chip, enable);
		if (ret < 0)
		{
			pr_red_info("chip->set_active");

			if (enable)
			{
				enable = false;
			}
			else
			{
				ret = 0;
			}

			chip->dead = true;
		}
		else if (enable)
		{
			chip->dead = false;
		}
	}

	if (enable == false)
	{
		cavan_input_chip_set_power(chip, false);
	}

	chip->actived = enable;

	pr_bold_info("cavan input chip %s is %s", chip->name, enable ? "actived" : "standby");

	return ret;
}

EXPORT_SYMBOL_GPL(cavan_input_chip_set_active);

int cavan_input_chip_set_active_lock(struct cavan_input_chip *chip, bool enable)
{
	int ret;

	mutex_lock(&chip->lock);
	ret = cavan_input_chip_set_active(chip, enable);
	mutex_unlock(&chip->lock);

	return ret;
}

EXPORT_SYMBOL_GPL(cavan_input_chip_set_active_lock);

static int cavan_input_chip_update_delay(struct cavan_input_chip *chip)
{
	int ret = 0;
	int count;
	int locked;
	unsigned int delay;
	struct list_head *head;
	struct cavan_input_device *dev;
	struct cavan_input_list *list = &chip->poll_list;

	locked = mutex_trylock(&list->lock);

	head = &list->head;
	if (list_empty(head))
	{
		pr_func_info("Nothing to be done");
		chip->poll_jiffies = MAX_SCHEDULE_TIMEOUT;
		goto out_mutex_unlock;
	}

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

	list_for_each_entry(dev, head, node)
	{
		if (delay < dev->min_delay)
		{
			delay = dev->min_delay;
		}
	}

	pr_green_info("cavan input chip poll count = %d, delay = %d(ms)", count, delay);

	list_for_each_entry(dev, head, node)
	{
		if (dev->enabled == false)
		{
			pr_red_info("device %s is not enable skipping", dev->name);
			continue;
		}

		if (dev->set_delay && (ret = dev->set_delay(dev, delay)))
		{
			pr_red_info("dev->set_delay");
			goto out_mutex_unlock;
		}
	}

	chip->poll_jiffies = msecs_to_jiffies(delay) + 1;

	pr_func_info("delay = %d(ms)", delay);

out_mutex_unlock:
	if (locked)
	{
		mutex_unlock(&list->lock);
	}

	return ret;
}

static int cavan_input_chip_update_thread_state(struct cavan_input_chip *chip)
{
	int count = 0;

	if (cavan_input_list_empty(&chip->isr_list))
	{
		cavan_input_thread_suspend(&chip->isr_thread);
	}
	else
	{
		cavan_input_thread_resume(&chip->isr_thread);
		count++;
	}

	if (cavan_input_list_empty(&chip->poll_list))
	{
		cavan_input_thread_suspend(&chip->poll_thread);
	}
	else
	{
		cavan_input_thread_resume(&chip->poll_thread);
		count++;
	}

	cavan_input_chip_set_active(chip, count > 0);

	return 0;
}

const struct cavan_input_rate_map *cavan_input_find_rate_map(const struct cavan_input_rate_map *map, size_t count, unsigned int delay)
{
	const struct cavan_input_rate_map *p, *map_end;

	if (map == NULL || count == 0)
	{
		return NULL;
	}

	for (p = map, map_end = map + count, map++; map < map_end; map++)
	{
		if (map->delay <= delay && (p->delay > delay || map->delay > p->delay))
		{
			p = map;
		}
	}

	pr_bold_info("delay = %d, real delay = %d, value = 0x%02x", delay, p->delay, p->value);

	return p;
}

EXPORT_SYMBOL_GPL(cavan_input_find_rate_map);

static int cavan_input_device_set_delay(struct cavan_input_device *dev, struct cavan_input_chip *chip, unsigned int delay)
{
	int ret;
	unsigned int delay_bak;

	if (chip->dead)
	{
		pr_red_info("chip %s is dead", chip->name);
		return -EIO;
	}

	pr_func_info("name = %s, delay = %d", dev->name, delay);

	delay_bak = dev->poll_delay;
	dev->poll_delay = delay;

	if (dev->use_irq)
	{
		ret = (dev->set_delay && dev->enabled) ? dev->set_delay(dev, delay) : 0;
	}
	else
	{
		ret = cavan_input_chip_update_delay(chip);

		if (dev->enabled && chip->poll_thread.task)
		{
			wake_up_process(chip->poll_thread.task);
		}
	}

	if (ret < 0)
	{
		dev->poll_delay = delay_bak;
		return ret;
	}

	return 0;
}

static int cavan_input_device_set_delay_lock(struct cavan_input_device *dev, unsigned int delay)
{
	int ret;
	struct cavan_input_chip *chip;

	mutex_lock(&dev->lock);

	chip = dev->chip;
	mutex_lock(&chip->lock);
	ret = cavan_input_device_set_delay(dev, chip, delay);
	mutex_unlock(&chip->lock);

	mutex_unlock(&dev->lock);

	return ret;
}

static int cavan_input_device_set_delay_no_sync(struct cavan_input_device *dev, unsigned int delay)
{
	struct cavan_input_core *core = dev->chip->core;

	pr_pos_info();

	if (core->workqueue == NULL)
	{
		return cavan_input_device_set_delay_lock(dev, delay);
	}

	mutex_lock(&dev->lock);
	dev->poll_delay = delay;
	mutex_unlock(&dev->lock);

	queue_work(core->workqueue, &dev->set_delay_work);

	return 0;
}

static int cavan_input_device_set_enable(struct cavan_input_device *dev, struct cavan_input_chip *chip, bool enable)
{
	int ret = 0;

	pr_pos_info();

	if (dev->enabled == enable)
	{
		pr_func_info("Nothing to be done");
		return 0;
	}

	if (enable)
	{
		ret = cavan_input_chip_set_active(chip, true);
		if (ret < 0)
		{
			pr_red_info("cavan_input_chip_set_active_lock");
			return ret;
		}

		if (dev->set_enable && (ret = dev->set_enable(dev, true)) < 0)
		{
			pr_red_info("dev->set_enable");
			enable = false;
		}

		if (dev->calibration)
		{
			ssize_t rdlen;
			char buff[32];

			rdlen = cavan_input_device_read_write_offset(dev, buff, sizeof(buff), false);
			if (rdlen > 0)
			{
				dev->calibration(dev, buff, ret, true);
			}
		}
	}

	if (dev->enabled != enable)
	{
		struct cavan_input_list *work_list = dev->use_irq ? &chip->isr_list : &chip->poll_list;

		if (enable == false)
		{
			cavan_input_list_del(work_list, &dev->node);
			cavan_input_list_add(&chip->dev_list, &dev->node);
		}
		else
		{
			cavan_input_list_del(&chip->dev_list, &dev->node);
			cavan_input_list_add(work_list, &dev->node);
		}
	}

	if (enable == false && dev->set_enable)
	{
		dev->set_enable(dev, false);
	}

	dev->enabled = enable;
	cavan_input_device_set_delay(dev, chip, dev->poll_delay);
	cavan_input_chip_update_thread_state(chip);

	pr_bold_info("cavan input device %s-%s %s", chip->name, dev->name, enable ? "enable" : "disable");

	return ret;
}

EXPORT_SYMBOL_GPL(cavan_input_device_set_enable);

int cavan_input_device_set_enable_lock(struct cavan_input_device *dev, bool enable)
{
	int ret;
	struct cavan_input_chip *chip;

	mutex_lock(&dev->lock);

	chip = dev->chip;
	mutex_lock(&chip->lock);
	ret = cavan_input_device_set_enable(dev, chip, enable);
	mutex_unlock(&chip->lock);

	mutex_unlock(&dev->lock);

	return ret;
}

EXPORT_SYMBOL_GPL(cavan_input_device_set_enable_lock);

int cavan_input_device_set_enable_no_sync(struct cavan_input_device *dev, bool enable)
{
	struct cavan_input_core *core = dev->chip->core;

	pr_pos_info();

	if (enable && core->workqueue)
	{
		queue_work(core->workqueue, &dev->resume_work);
		return 0;
	}

	return cavan_input_device_set_enable_lock(dev, enable);
}

EXPORT_SYMBOL_GPL(cavan_input_device_set_enable_no_sync);

int cavan_input_device_calibration(struct cavan_input_device *dev, struct cavan_input_chip *chip, char *buff, size_t size, bool store)
{
	int ret;

	if (dev->calibration == NULL)
	{
		pr_red_info("dev->calibration is null");
		return -EINVAL;
	}

	ret = cavan_input_device_set_enable(dev, chip, true);
	if (ret < 0)
	{
		pr_red_info("cavan_input_device_set_enable");
		return ret;
	}

	cavan_input_thread_set_state(&chip->isr_thread, CAVAN_INPUT_THREAD_STATE_SUSPEND);
	cavan_input_thread_set_state(&chip->poll_thread, CAVAN_INPUT_THREAD_STATE_SUSPEND);
	ret = dev->calibration(dev, buff, size, store);
	cavan_input_chip_update_thread_state(chip);

	if (ret > 0)
	{
		cavan_input_device_read_write_offset(dev, buff, ret, true);
	}

	return ret;
}

EXPORT_SYMBOL_GPL(cavan_input_device_calibration);

int cavan_input_device_calibration_lock(struct cavan_input_device *dev, char *buff, size_t size, bool store)
{
	int ret;
	struct cavan_input_chip *chip;

	mutex_lock(&dev->lock);
	chip = dev->chip;
	mutex_lock(&chip->lock);
	ret = cavan_input_device_calibration(dev, chip, buff, size, store);
	mutex_unlock(&chip->lock);
	mutex_unlock(&dev->lock);

	return ret;
}

EXPORT_SYMBOL_GPL(cavan_input_device_calibration_lock);

static void cavan_input_chip_recovery_devices(struct cavan_input_chip *chip, struct cavan_input_list *list)
{
	int list_locked;
	struct list_head *head;
	struct cavan_input_device *dev;

	list_locked = mutex_trylock(&list->lock);

	head = &list->head;

	list_for_each_entry(dev, head, node)
	{
		int dev_locked;

		cavan_input_chip_set_active(chip, true);

		dev_locked = mutex_trylock(&dev->lock);

		if (dev->set_enable)
		{
			dev->set_enable(dev, true);
		}

		if (dev_locked)
		{
			mutex_unlock(&dev->lock);
		}
	}

	if (list_locked)
	{
		mutex_unlock(&list->lock);
	}
}

void cavan_input_chip_recovery(struct cavan_input_chip *chip, bool force)
{
	int locked;

	locked = mutex_trylock(&chip->lock);

	chip->recovery_count++;
	pr_bold_info("recovery_count = %d", chip->recovery_count);

	if (force || chip->recovery_count > 5)
	{
		cavan_input_chip_set_active(chip, false);
		cavan_input_chip_recovery_devices(chip, &chip->isr_list);
		cavan_input_chip_recovery_devices(chip, &chip->poll_list);
		cavan_input_chip_update_delay(chip);

		chip->recovery_count = 0;
	}

	if (locked)
	{
		mutex_unlock(&chip->lock);
	}
}

EXPORT_SYMBOL_GPL(cavan_input_chip_recovery);

ssize_t cavan_input_chip_write_online(const char *chip_name, bool online)
{
	char pathname[512];

	cavan_input_chip_get_online_pathname(chip_name, pathname, sizeof(pathname));

	return cavan_io_read_write_file(pathname, online ? "1" : "0", 1, true);
}

EXPORT_SYMBOL_GPL(cavan_input_chip_write_online);

ssize_t cavan_input_chip_read_online(const char *chip_name)
{
	char buff[4];
	ssize_t rdlen;
	char pathname[512];

	cavan_input_chip_get_online_pathname(chip_name, pathname, sizeof(pathname));
	rdlen = cavan_io_read_write_file(pathname, buff, sizeof(buff), false);
	if (rdlen < 0)
	{
		return rdlen;
	}

	return rdlen > 0 && buff[0] != '0';
}

EXPORT_SYMBOL_GPL(cavan_input_chip_read_online);

static int cavan_input_chip_firmware_upgrade_handler(struct cavan_firmware *fw)
{
	int ret;
	bool powered;
	struct cavan_input_chip *chip = cavan_firmware_get_data(fw);

	cavan_input_thread_set_state(&chip->isr_thread, CAVAN_INPUT_THREAD_STATE_SUSPEND);
	cavan_input_thread_set_state(&chip->poll_thread, CAVAN_INPUT_THREAD_STATE_SUSPEND);

	mutex_lock(&chip->lock);
	wake_lock(&chip->wake_lock);

	powered = chip->powered;

	ret = cavan_input_chip_set_power(chip, true);
	if (ret < 0)
	{
		pr_red_info("cavan_input_chip_set_power");
	}
	else
	{
		ret = chip->firmware_upgrade(chip, fw);
		if (ret < 0)
		{
			pr_red_info("chip->firmware_upgrade");
		}
		else
		{
			pr_green_info("cavan input chip %s firmware upgrade complete", chip->name);
		}
	}

	cavan_input_chip_update_thread_state(chip);

	cavan_input_chip_set_power(chip, powered);

	wake_unlock(&chip->wake_lock);
	mutex_unlock(&chip->lock);

	return ret;
}

static struct cavan_firmware *cavan_input_chip_firmware_create(struct cavan_input_chip *chip)
{
	struct cavan_firmware *fw;

	fw = cavan_firmware_create(CAVAN_INPUT_CHIP_FW_CACHE_SIZE, cavan_input_chip_firmware_upgrade_handler);
	if (fw == NULL)
	{
		pr_red_info("cavan_firmware_create");
		return NULL;
	}

	cavan_firmware_set_data(fw, chip);

	return fw;
}

int cavan_input_chip_firmware_upgrade(struct cavan_input_chip *chip, void *buff, size_t size, int flags)
{
	struct cavan_firmware *fw;

	fw = cavan_input_chip_firmware_create(chip);
	if (fw == NULL)
	{
		pr_red_info("cavan_input_chip_firmware_create");
		return -ENOMEM;
	}

	cavan_firmware_write(fw, buff, size, flags);

	return cavan_firmware_destroy(fw);
}

EXPORT_SYMBOL_GPL(cavan_input_chip_firmware_upgrade);

int cavan_input_chip_read_firmware_id(struct cavan_input_chip *chip, char *buff, size_t size)
{
	int ret;
	bool powered;

	if (chip->read_firmware_id == NULL)
	{
		pr_red_info("dev->read_firmware_id is null");
		return -EINVAL;
	}

	powered = chip->powered;

	ret = cavan_input_chip_set_power(chip, true);
	if (ret < 0)
	{
		pr_red_info("cavan_input_chip_set_power");
		return ret;
	}

	ret = chip->read_firmware_id(chip, buff, size);

	cavan_input_chip_set_power(chip, powered);

	return ret;
}

EXPORT_SYMBOL_GPL(cavan_input_chip_read_firmware_id);

int cavan_input_chip_read_firmware_id_lock(struct cavan_input_chip *chip, char *buff, size_t size)
{
	int ret;

	mutex_lock(&chip->lock);
	ret = cavan_input_chip_read_firmware_id(chip, buff, size);
	mutex_unlock(&chip->lock);

	return ret;
}

EXPORT_SYMBOL_GPL(cavan_input_chip_read_firmware_id_lock);

static int cavan_input_chip_open(struct cavan_misc_device *dev)
{
	pr_pos_info();

	return 0;
}

static int cavan_input_chip_release(struct cavan_misc_device *dev)
{
	struct cavan_input_chip *chip = cavan_misc_device_get_data(dev);
	struct cavan_firmware *fw = cavan_input_chip_get_misc_data(chip);

	if (fw)
	{
		int ret;

		ret = cavan_firmware_destroy(fw);
		cavan_input_chip_set_misc_data(chip, NULL);

		return ret;
	}

	return 0;
}

static ssize_t cavan_input_chip_write(struct cavan_misc_device *dev, const char __user *buff, size_t size, loff_t *offset)
{
	ssize_t wrlen;
	struct cavan_input_chip *chip = cavan_misc_device_get_data(dev);
	struct cavan_firmware *fw = cavan_input_chip_get_misc_data(chip);

	if (chip->firmware_upgrade == NULL)
	{
		pr_red_info("Please implement firmware_upgrade method");
		return -EINVAL;
	}

	fw = cavan_input_chip_get_misc_data(chip);
	if (fw == NULL)
	{
		fw = cavan_input_chip_firmware_create(chip);
		if (fw == NULL)
		{
			pr_red_info("cavan_input_chip_firmware_create");
			return -ENOMEM;
		}

		cavan_input_chip_set_misc_data(chip, fw);
	}

	wrlen = cavan_firmware_write(fw, buff, size, CAVAN_FW_FLAG_USER);
	if (wrlen < 0)
	{
		pr_red_info("cavan_firmware_write");
		return wrlen;
	}

	*offset += wrlen;

	return wrlen;
}

static ssize_t cavan_input_chip_attr_firmware_id_show(struct device *device, struct device_attribute *attr, char *buff)
{
	struct cavan_misc_device *mdev = dev_get_drvdata(device);
	struct cavan_input_chip *chip = cavan_misc_device_get_data(mdev);

	return cavan_input_chip_read_firmware_id_lock(chip, buff, PAGE_SIZE);
}

static ssize_t cavan_input_chip_attr_info_show(struct device *device, struct device_attribute *attr, char *buff)
{
	char *buff_bak = buff;
	struct cavan_misc_device *mdev = dev_get_drvdata(device);
	struct cavan_input_chip *chip = cavan_misc_device_get_data(mdev);

	buff += sprintf(buff, "chip: %s\n", chip->name);
	buff += sprintf(buff, "irq: %d\n", chip->irq);
	buff += sprintf(buff, "irq_type: %d\n", chip->irq_type);
	buff += sprintf(buff, "irq_flags: 0x%08lx\n", chip->irq_flags);
	buff += sprintf(buff, "powered: %d\n", chip->powered);
	buff += sprintf(buff, "flags: 0x%08lx\n", chip->flags);
	buff += sprintf(buff, "delay: %d\n", jiffies_to_msecs(chip->poll_jiffies - 1));

	return buff - buff_bak;
}

static ssize_t cavan_input_chip_attr_enable_show(struct device *device, struct device_attribute *attr, char *buff)
{
	struct cavan_misc_device *mdev = dev_get_drvdata(device);
	struct cavan_input_chip *chip = cavan_misc_device_get_data(mdev);

	return sprintf(buff, "%d\n", chip->powered);
}

static ssize_t cavan_input_chip_attr_enable_store(struct device *device, struct device_attribute *attr, const char *buff, size_t size)
{
	int ret;
	struct cavan_misc_device *mdev = dev_get_drvdata(device);
	struct cavan_input_chip *chip = cavan_misc_device_get_data(mdev);

	ret = cavan_input_chip_set_power_lock(chip, simple_strtoul(buff, NULL, 10) > 0);
	if (ret < 0)
	{
		pr_red_info("cavan_input_chip_set_power_lock");
		return ret;
	}

	return size;
}

static struct device_attribute cavan_input_chip_attr_firmware_id = __ATTR(firmware_id, S_IRUGO, cavan_input_chip_attr_firmware_id_show, NULL);
static struct device_attribute cavan_input_chip_attr_info = __ATTR(info, S_IRUGO, cavan_input_chip_attr_info_show, NULL);
static struct device_attribute cavan_input_chip_attr_enable = __ATTR(enable, S_IRWXU | S_IRWXG | S_IRUGO, cavan_input_chip_attr_enable_show, cavan_input_chip_attr_enable_store);

static const struct attribute *cavan_input_chip_attributes[] =
{
	&cavan_input_chip_attr_firmware_id.attr,
	&cavan_input_chip_attr_info.attr,
	&cavan_input_chip_attr_enable.attr,
	NULL
};

static int cavan_input_chip_probe(struct cavan_input_chip *chip)
{
	int ret;
	const char *chip_name;
	struct cavan_misc_device *mdev;

	pr_pos_info();

	pr_green_info("Try input chip %s", chip->name);

	ret = cavan_input_chip_set_power_lock(chip, true);
	if (ret < 0)
	{
		pr_red_info("cavan_input_chip_set_power");
		return ret;
	}

	chip_name = chip->name;

	if (chip->probe_count > 5)
	{
		if (cavan_input_chip_read_online(chip_name) > 0)
		{
			pr_green_info("chip %s is online", chip_name);
			goto label_write_init_data;
		}
	}

	if (chip->readid && (ret = chip->readid(chip)) < 0)
	{
		pr_red_info("chip->readid");
		goto out_power_down;
	}

	cavan_input_core_add_online_chip(chip_name);

label_write_init_data:
	if ((chip->flags & CAVAN_INPUT_CHIP_FLAG_POWERON_INIT) == 0 && (ret = cavan_input_chip_write_init_data(chip)) < 0)
	{
		pr_red_info("cavan_input_chip_write_init_data");
		goto out_power_down;
	}

	ret = cavan_input_chip_request_irq(chip);
	if (ret < 0)
	{
		pr_red_info("cavan_input_chip_request_irq");
		goto out_power_down;
	}

	wake_lock_init(&chip->wake_lock, WAKE_LOCK_SUSPEND, chip->name);

	ret = chip->probe(chip);
	if (ret < 0)
	{
		pr_red_info("chip->probe");
		goto out_wake_lock_destroy;
	}

	chip->misc_name = kasprintf(GFP_KERNEL, "CAVAN-%s", chip->name);
	if (chip->misc_name == NULL)
	{
		pr_red_info("kasprintf");
		goto out_chip_remove;
	}

	mdev = &chip->misc_dev;
	cavan_misc_device_set_data(mdev, chip);
	mdev->open = cavan_input_chip_open;
	mdev->release = cavan_input_chip_release;
	mdev->write = cavan_input_chip_write;

	ret = cavan_misc_device_register(mdev, chip->misc_name);
	if (ret < 0)
	{
		pr_red_info("cavan_misc_device_register");
		goto out_kfree_chip_misc_name;
	}

	ret = sysfs_create_files(&mdev->dev->kobj, cavan_input_chip_attributes);
	if (ret < 0)
	{
		pr_red_info("sysfs_create_group");
		goto out_cavan_misc_device_unregister;
	}

	pr_green_info("cavan input chip %s probe complete", chip->name);

	return 0;

out_cavan_misc_device_unregister:
	cavan_misc_device_unregister(mdev);
out_kfree_chip_misc_name:
	kfree(chip->misc_name);
out_chip_remove:
	chip->remove(chip);
out_wake_lock_destroy:
	wake_lock_destroy(&chip->wake_lock);
	cavan_input_chip_free_irq(chip);
out_power_down:
	cavan_input_chip_set_power(chip, false);
	return ret;
}

static void cavan_input_chip_remove(struct cavan_input_chip *chip)
{
	sysfs_remove_files(&chip->misc_dev.dev->kobj, cavan_input_chip_attributes);
	cavan_misc_device_unregister(&chip->misc_dev);
	kfree(chip->misc_name);

	chip->remove(chip);

	wake_lock_destroy(&chip->wake_lock);

	cavan_input_thread_stop(&chip->poll_thread);
	cavan_input_thread_stop(&chip->isr_thread);

	cavan_input_chip_free_irq(chip);

	cavan_input_chip_set_active_lock(chip, false);
}

int cavan_input_chip_report_events(struct cavan_input_chip *chip, struct cavan_input_list *list)
{
	int count = 0;
	struct cavan_input_device *dev;
	struct list_head *head;

	mutex_lock(&list->lock);

	head = &list->head;

	list_for_each_entry(dev, head, node)
	{
		if (dev->event_handler(chip, dev) < 0)
		{
			count++;
		}
	}

	mutex_unlock(&list->lock);

	return count > 0 ? -EFAULT : 0;
}

EXPORT_SYMBOL_GPL(cavan_input_chip_report_events);

static int cavan_input_chip_event_handler_isr(struct cavan_input_thread *thread)
{
	struct cavan_input_chip *chip = cavan_input_thread_get_data(thread);

	return cavan_input_chip_report_events(chip, &chip->isr_list);
}

static int cavan_input_chip_event_handler_user_isr(struct cavan_input_thread *thread)
{
	struct cavan_input_chip *chip = cavan_input_thread_get_data(thread);

	return chip->event_handler_isr(chip);
}

static int cavan_input_chip_event_handler_user_poll(struct cavan_input_thread *thread)
{
	struct cavan_input_chip *chip = cavan_input_thread_get_data(thread);

	return chip->event_handler_poll(chip);
}

static int cavan_input_chip_event_handler_poll(struct cavan_input_thread *thread)
{
	struct cavan_input_chip *chip = cavan_input_thread_get_data(thread);

	return cavan_input_chip_report_events(chip, &chip->poll_list);
}

static void cavan_input_chip_error_handler(struct cavan_input_thread *thread)
{
	struct cavan_input_chip *chip = cavan_input_thread_get_data(thread);

	mutex_lock(&chip->lock);
	chip->dead = true;
	mutex_unlock(&chip->lock);
}

static void cavan_input_chip_isr_thread_stop(struct cavan_input_thread *thread)
{
	struct cavan_input_chip *chip = cavan_input_thread_get_data(thread);

	complete(&chip->event_completion);
}

static void cavan_input_chip_isr_thread_prepare(struct cavan_input_thread *thread, bool enable)
{
	struct cavan_input_chip *chip = cavan_input_thread_get_data(thread);

	if (chip->irq_type == CAVAN_INPUT_IRQ_TYPE_EDGE && chip->irq > 0)
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

static int cavan_input_chip_set_power_dummy(struct cavan_input_chip *chip, bool enable)
{
	pr_pos_info();

	if (enable)
	{
		int ret;

		cavan_io_gpio_set_value(chip->gpio_reset, 0);

		ret = cavan_io_set_power_regulator(chip, true);
		if (ret < 0)
		{
			return ret;
		}

		msleep(20);

		cavan_io_gpio_set_value(chip->gpio_reset, 1);
	}
	else
	{
		return cavan_io_set_power_regulator(chip, false);
	}

	return 0;
}

static int cavan_input_chip_init(struct cavan_input_core *core, struct cavan_input_chip *chip)
{
	int ret;
	struct cavan_input_thread *thread;

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

	ret = cavan_input_chip_io_init(chip);
	if (ret < 0)
	{
		pr_red_info("cavan_input_chip_io_init");
		return ret;
	}

	if (chip->vendor == NULL)
	{
		chip->vendor = "Cavan";
	}

	if (chip->read_register == NULL)
	{
		chip->read_register = cavan_input_read_register_dummy;
	}

	if (chip->write_register == NULL)
	{
		chip->write_register = cavan_input_write_register_dummy;
	}

	if (chip->read_register16 == NULL)
	{
		chip->read_register16 = cavan_input_read_register16_dummy;
	}

	if (chip->write_register16 == NULL)
	{
		chip->write_register16 = cavan_input_write_register16_dummy;
	}

	if (chip->master_recv == NULL)
	{
		chip->master_recv = cavan_input_master_recv_i2c;
	}

	if (chip->master_send == NULL)
	{
		chip->master_send = cavan_input_master_send_i2c;
	}

	if (chip->set_power == NULL)
	{
		chip->set_power = cavan_input_chip_set_power_dummy;
	}

	if (chip->i2c_rate == 0) {
		chip->i2c_rate = CAVAN_INPUT_I2C_RATE;
	}

	chip->core = core;
	chip->dead = false;
	chip->powered = false;
	chip->actived = false;
	chip->probe_count = 0;
	chip->recovery_count = 0;

	cavan_input_list_init(&chip->isr_list);
	cavan_input_list_init(&chip->poll_list);
	cavan_input_list_init(&chip->dev_list);

	mutex_init(&chip->lock);

	thread = &chip->isr_thread;
	thread->priority = MAX_PRIO;
	cavan_input_thread_set_data(thread, chip);
	thread->stop = cavan_input_chip_isr_thread_stop;
	thread->prepare = cavan_input_chip_isr_thread_prepare;
	thread->error_handle = cavan_input_chip_error_handler;
	thread->wait_for_event = cavan_input_chip_wait_for_event_edge;
	if (chip->event_handler_isr)
	{
		thread->event_handle = cavan_input_chip_event_handler_user_isr;
	}
	else
	{
		thread->event_handle = cavan_input_chip_event_handler_isr;
	}

	ret = cavan_input_thread_init(thread, "%s-ISR", chip->name);
	if (ret < 0)
	{
		pr_red_info("cavan_input_thread_init");
		goto out_cavan_input_chip_io_deinit;
	}

	thread = &chip->poll_thread;
	thread->priority = 0;
	cavan_input_thread_set_data(thread, chip);
	thread->stop = NULL;
	thread->prepare = NULL;
	thread->error_handle = cavan_input_chip_error_handler;
	if (chip->flags & CAVAN_INPUT_CHIP_FLAG_NO_WAIT)
	{
		thread->wait_for_event = cavan_input_chip_wait_for_event_none;
	}
	else
	{
		thread->wait_for_event = cavan_input_chip_wait_for_event_poll;
	}

	if (chip->event_handler_poll)
	{
		thread->event_handle = cavan_input_chip_event_handler_user_poll;
	}
	else
	{
		thread->event_handle = cavan_input_chip_event_handler_poll;
	}

	ret = cavan_input_thread_init(thread, "%s-POLL", chip->name);
	if (ret < 0)
	{
		pr_red_info("cavan_input_thread_init");
		goto out_cavan_input_thread_destroy_isr;
	}

	return 0;

out_cavan_input_thread_destroy_isr:
	cavan_input_thread_destroy(&chip->isr_thread);
out_cavan_input_chip_io_deinit:
	cavan_input_chip_io_deinit(chip);
	return ret;
}

static void cavan_input_chip_destroy(struct cavan_input_chip *chip)
{
	pr_pos_info();

	mutex_destroy(&chip->lock);

	cavan_input_list_destory(&chip->isr_list);
	cavan_input_list_destory(&chip->poll_list);
	cavan_input_list_destory(&chip->dev_list);

	cavan_input_chip_io_deinit(chip);
}

ssize_t cavan_input_device_read_write_offset(struct cavan_input_device *dev, char *buff, size_t size, bool store)
{
	char pathname[512];

	cavan_input_device_get_offset_pathname(dev, pathname, sizeof(pathname));

	return cavan_io_read_write_file(pathname, buff, size, store);
}

EXPORT_SYMBOL_GPL(cavan_input_device_read_write_offset);

static ssize_t cavan_input_device_read(struct cavan_misc_device *dev, char __user *buff, size_t size, loff_t *offset)
{
	char *p, *p_end;
	struct cavan_input_device *idev;

	if (*offset)
	{
		return 0;
	}

	idev = cavan_misc_device_get_data(dev);

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

static ssize_t cavan_input_device_write(struct cavan_misc_device *dev, const char __user *buff, size_t size, loff_t *offset)
{
	int ret;
	struct cavan_input_device *idev = cavan_misc_device_get_data(dev);

	ret = cavan_input_device_set_enable_no_sync(idev, size > 0 && buff[0] > '0');
	if (ret < 0)
	{
		pr_red_info("cavan_input_device_set_enable_no_sync");
		return ret;
	}

	return size;
}

static int cavan_input_device_ioctl(struct cavan_misc_device *dev, unsigned int command, unsigned long args)
{
	struct cavan_input_device *idev = cavan_misc_device_get_data(dev);

	switch (CAVAN_INPUT_IOC_GET_CMD_RAW(command))
	{
	case CAVAN_INPUT_CHIP_IOC_GET_NAME(0):
		return cavan_input_copy_to_user_text(command, args, idev->chip->name);

	case CAVAN_INPUT_CHIP_IOC_GET_VENDOR(0):
		return cavan_input_copy_to_user_text(command, args, idev->chip->vendor);

	case CAVAN_INPUT_DEVICE_IOC_GET_TYPE:
		return cavan_input_copy_to_user_uint(args, idev->type);

	case CAVAN_INPUT_DEVICE_IOC_GET_NAME(0):
		return cavan_input_copy_to_user_text(command, args, idev->name);

	case CAVAN_INPUT_DEVICE_IOC_SET_DELAY:
		return cavan_input_device_set_delay_no_sync(idev, args);

	case CAVAN_INPUT_DEVICE_IOC_SET_ENABLE:
		return cavan_input_device_set_enable_no_sync(idev, args > 0);

	case CAVAN_INPUT_DEVICE_IOC_GET_OFFSET(0):
	case CAVAN_INPUT_DEVICE_IOC_SET_OFFSET(0):
		return cavan_input_device_calibration_lock(idev, (void __user *)args, CAVAN_INPUT_IOC_GET_SIZE(command), CAVAN_INPUT_IOC_GET_CMD_RAW(command) == CAVAN_INPUT_DEVICE_IOC_SET_OFFSET(0));

	default:
		if (idev->ioctl)
		{
			return idev->ioctl(idev, command, args);
		}
	}

	pr_red_info("Invalid IOCTL 0x%08x", command);

	return -EINVAL;
}

static ssize_t cavan_input_device_attr_calibration_show(struct device *device, struct device_attribute *attr, char *buff)
{
	struct cavan_misc_device *mdev = dev_get_drvdata(device);
	struct cavan_input_device *idev = cavan_misc_device_get_data(mdev);

	return cavan_input_device_calibration_lock(idev, buff, PAGE_SIZE, false);
}

static ssize_t cavan_input_device_attr_calibration_store(struct device *device, struct device_attribute *attr, const char *buff, size_t size)
{
	int ret;
	struct cavan_misc_device *mdev = dev_get_drvdata(device);
	struct cavan_input_device *idev = cavan_misc_device_get_data(mdev);

	ret = cavan_input_device_calibration_lock(idev, (char *) buff, size, true);
	if (ret < 0)
	{
		pr_red_info("cavan_input_device_calibration_lock");
		return ret;
	}

	return size;
}

static ssize_t cavan_input_device_attr_info_show(struct device *device, struct device_attribute *attr, char *buff)
{
	char *buff_bak = buff;
	struct cavan_misc_device *mdev = dev_get_drvdata(device);
	struct cavan_input_device *idev = cavan_misc_device_get_data(mdev);

	buff += sprintf(buff, "name: %s\n", idev->name);
	buff += sprintf(buff, "type: %s\n", cavan_input_device_type_tostring(idev->type));
	buff += sprintf(buff, "enabled: %d\n", idev->enabled);
	buff += sprintf(buff, "delay: %d\n", idev->poll_delay);
	buff += sprintf(buff, "use_irq: %d\n", idev->use_irq);
	buff += sprintf(buff, "minor: %d\n", idev->misc_dev.minor);

	return buff - buff_bak;
}

static ssize_t cavan_input_device_attr_enable_show(struct device *device, struct device_attribute *attr, char *buff)
{
	struct cavan_misc_device *mdev = dev_get_drvdata(device);
	struct cavan_input_device *idev = cavan_misc_device_get_data(mdev);

	return sprintf(buff, "%d\n", idev->enabled);
}

static ssize_t cavan_input_device_attr_enable_store(struct device *device, struct device_attribute *attr, const char *buff, size_t size)
{
	int ret;
	struct cavan_misc_device *mdev = dev_get_drvdata(device);
	struct cavan_input_device *idev = cavan_misc_device_get_data(mdev);

	ret = cavan_input_device_set_enable_no_sync(idev, simple_strtoul(buff, NULL, 10) > 0);
	if (ret < 0)
	{
		pr_red_info("cavan_input_device_set_enable_no_sync");
		return ret;
	}

	return size;
}

static ssize_t cavan_input_device_attr_delay_show(struct device *device, struct device_attribute *attr, char *buff)
{
	struct cavan_misc_device *mdev = dev_get_drvdata(device);
	struct cavan_input_device *idev = cavan_misc_device_get_data(mdev);

	return sprintf(buff, "%d\n", idev->poll_delay);
}

static ssize_t cavan_input_device_attr_delay_store(struct device *device, struct device_attribute *attr, const char *buff, size_t size)
{
	int ret;
	struct cavan_misc_device *mdev = dev_get_drvdata(device);
	struct cavan_input_device *idev = cavan_misc_device_get_data(mdev);

	ret = cavan_input_device_set_delay_no_sync(idev, simple_strtoul(buff, NULL, 10));
	if (ret < 0)
	{
		pr_red_info("cavan_input_device_set_delay_no_sync");
		return ret;
	}

	return size;
}

static ssize_t cavan_input_device_attr_min_delay_show(struct device *device, struct device_attribute *attr, char *buff)
{
	struct cavan_misc_device *mdev = dev_get_drvdata(device);
	struct cavan_input_device *idev = cavan_misc_device_get_data(mdev);

	return sprintf(buff, "%d\n", idev->min_delay);
}

static struct device_attribute cavan_input_device_attr_calibration = __ATTR(calibration, S_IRWXU | S_IRWXG | S_IRUGO, cavan_input_device_attr_calibration_show, cavan_input_device_attr_calibration_store);
static struct device_attribute cavan_input_device_attr_info = __ATTR(info, S_IRUGO, cavan_input_device_attr_info_show, NULL);
static struct device_attribute cavan_input_device_attr_enable = __ATTR(enable, S_IRWXU | S_IRWXG | S_IRUGO, cavan_input_device_attr_enable_show, cavan_input_device_attr_enable_store);
static struct device_attribute cavan_input_device_attr_delay = __ATTR(delay, S_IRWXU | S_IRWXG | S_IRUGO, cavan_input_device_attr_delay_show, cavan_input_device_attr_delay_store);
static struct device_attribute cavan_input_device_attr_min_delay = __ATTR(min_delay, S_IRUGO, cavan_input_device_attr_min_delay_show, NULL);

static const struct attribute *cavan_input_device_attributes[] =
{
	&cavan_input_device_attr_calibration.attr,
	&cavan_input_device_attr_info.attr,
	&cavan_input_device_attr_enable.attr,
	&cavan_input_device_attr_delay.attr,
	&cavan_input_device_attr_min_delay.attr,
	NULL
};

static int cavan_input_device_probe(struct cavan_input_chip *chip, struct cavan_input_device *dev)
{
	int ret;
	u32 debound[2];
	const char *devname;
	struct input_dev *input;
	struct cavan_misc_device *mdev;

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

	devname = cavan_input_device_type_tostring(dev->type);

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

	dev->misc_name = kasprintf(GFP_KERNEL, "CAVAN-%s-%s", chip->name, devname);
	if (dev->misc_name == NULL)
	{
		ret = -ENOMEM;
		pr_red_info("asprintf");
		goto out_input_free_device;
	}

	input->name = dev->misc_name;
	dev->input = input;
	input_set_drvdata(input, dev);

	mdev = &dev->misc_dev;
	cavan_misc_device_set_data(mdev, dev);
	mdev->read = cavan_input_device_read;
	mdev->write = cavan_input_device_write;
	mdev->ioctl = cavan_input_device_ioctl;

	ret = cavan_misc_device_register(mdev, dev->misc_name);
	if (ret < 0)
	{
		pr_red_info("cavan_misc_device_register");
		goto out_kfree_input_name;
	}

#ifdef CONFIG_OF
	if (of_property_read_u32_array(chip->dev->of_node, "debound", debound, ARRAY_SIZE(debound)) >= 0)
	{
		dev->fuzz = debound[0];
		dev->flat = debound[1];
	}
#endif

	if (dev->type == CAVAN_INPUT_DEVICE_TYPE_TOUCHSCREEN)
	{
		ret = cavan_ts_device_probe(dev);
	}
	else
	{
		ret = cavan_sensor_device_probe(dev);
	}

	if (ret < 0)
	{
		pr_red_info("probe device failed");
		goto out_cavan_misc_device_unregister;
	}

	ret = input_register_device(input);
	if (ret < 0)
	{
		pr_red_info("input_register_device");
		goto out_device_remove;
	}

	ret = sysfs_create_files(&dev->misc_dev.dev->kobj, cavan_input_device_attributes);
	if (ret < 0)
	{
		pr_red_info("sysfs_create_group");
		goto out_input_unregister_device;
	}

	pr_green_info("cavan input deivce %s probe complete", dev->name);

	return 0;

out_input_unregister_device:
	input_unregister_device(input);
out_device_remove:
	if (dev->remove)
	{
		dev->remove(dev);
	}
out_cavan_misc_device_unregister:
	cavan_misc_device_unregister(mdev);
out_kfree_input_name:
	kfree(dev->misc_name);
out_input_free_device:
	input_free_device(input);
out_mutex_destroy:
	mutex_destroy(&dev->lock);
	return ret;
}

static void cavan_input_device_remove(struct cavan_input_chip *chip, struct cavan_input_device *dev)
{
	struct input_dev *input = dev->input;

	sysfs_remove_files(&dev->misc_dev.dev->kobj, cavan_input_device_attributes);
	cavan_misc_device_unregister(&dev->misc_dev);
	input_unregister_device(input);

	if (dev->remove)
	{
		dev->remove(dev);
	}

	kfree(dev->misc_name);
	input_free_device(input);
	mutex_destroy(&dev->lock);
}

static void cavan_input_device_resume_work_func(struct work_struct *work)
{
	struct cavan_input_device *dev = container_of(work, struct cavan_input_device, resume_work);

	pr_pos_info();

	cavan_input_device_set_enable_lock(dev, true);
}

static void cavan_input_device_set_delay_work_func(struct work_struct *work)
{
	struct cavan_input_device *dev = container_of(work, struct cavan_input_device, set_delay_work);

	pr_pos_info();

	cavan_input_device_set_delay_lock(dev, dev->poll_delay);
}

int cavan_input_device_register(struct cavan_input_chip *chip, struct cavan_input_device *dev)
{
	int ret;

	cavan_input_list_add(&chip->dev_list, &dev->node);

	ret = cavan_input_device_probe(chip, dev);
	if (ret < 0)
	{
		pr_red_info("cavan_input_device_probe");
		goto out_list_del;
	}

	INIT_WORK(&dev->resume_work, cavan_input_device_resume_work_func);
	INIT_WORK(&dev->set_delay_work, cavan_input_device_set_delay_work_func);

	pr_green_info("cavan input deivce %s register complete", dev->name);

	return 0;

out_list_del:
	cavan_input_list_del(&chip->dev_list, &dev->node);
	return ret;
}

EXPORT_SYMBOL_GPL(cavan_input_device_register);

void cavan_input_device_unregister(struct cavan_input_chip *chip, struct cavan_input_device *dev)
{
	mutex_lock(&chip->lock);

	mutex_lock(&dev->lock);
	cavan_input_device_set_enable(dev, chip, false);
	list_del(&dev->node);
	mutex_unlock(&dev->lock);

	cavan_input_device_remove(chip, dev);

	mutex_unlock(&chip->lock);
}

EXPORT_SYMBOL_GPL(cavan_input_device_unregister);

// ================================================================================

int cavan_input_chip_register(struct cavan_input_chip *chip, struct device *dev)
{
	int ret;

	pr_pos_info();

	chip->dev = dev;

	ret = cavan_input_chip_init(&input_core, chip);
	if (ret < 0)
	{
		pr_red_info("cavan_input_chip_init");
		goto out_list_del;
	}

	cavan_input_list_add(&input_core.chip_list, &chip->node);
	cavan_input_thread_resume(&input_core.detect_thread);

	return 0;

out_list_del:
	cavan_input_list_del(&input_core.chip_list, &chip->node);
	return ret;
}

EXPORT_SYMBOL_GPL(cavan_input_chip_register);

void cavan_input_chip_unregister(struct cavan_input_chip *chip)
{
	mutex_lock(&input_core.lock);
	mutex_lock(&chip->lock);

	if (cavan_input_list_has_node(&input_core.work_list, &chip->node))
	{
		mutex_unlock(&chip->lock);
		cavan_input_chip_remove(chip);
		mutex_lock(&chip->lock);
	}

	list_del(&chip->node);

	mutex_unlock(&chip->lock);
	cavan_input_chip_destroy(chip);

	input_core.devmask &= ~(chip->devmask);

	mutex_unlock(&input_core.lock);
}

EXPORT_SYMBOL_GPL(cavan_input_chip_unregister);

static void cavan_input_core_wait_for_event(struct cavan_input_thread *thread)
{
	struct cavan_input_core *core = cavan_input_thread_get_data(thread);

	set_current_state(TASK_UNINTERRUPTIBLE);
	schedule_timeout(core->poll_jiffies);
}

static int cavan_input_core_event_handler(struct cavan_input_thread *thread)
{
	struct cavan_input_chip *chip;
	struct cavan_input_core *core = cavan_input_thread_get_data(thread);
	struct cavan_input_list *list = &core->chip_list;
	struct list_head *head = &list->head;

	pr_pos_info();

	mutex_lock(&list->lock);

	list_for_each_entry(chip, head, node)
	{
		mutex_lock(&chip->lock);
		chip->probe_count++;

		if ((chip->devmask & core->devmask) || chip->probe_count > CAVAN_INPUT_CHIP_MAX_PROBE_COUNT)
		{
			list_del(&chip->node);
			cavan_input_list_add(&core->exclude_list, &chip->node);
			chip->poll_jiffies = 0;

			mutex_unlock(&chip->lock);
			break;
		}
		else
		{
			mutex_unlock(&chip->lock);

			if (cavan_input_chip_probe(chip) >= 0)
			{
				mutex_lock(&chip->lock);

				list_del(&chip->node);
				cavan_input_list_add(&core->work_list, &chip->node);
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
		thread->state = CAVAN_INPUT_THREAD_STATE_SUSPEND;
		core->poll_jiffies = 0;
	}
	else
	{
		core->poll_jiffies = core->poll_jiffies << 1 | 1;
	}

	mutex_unlock(&core->lock);

	mutex_unlock(&list->lock);

	return 0;
}

static const struct file_operations cavan_input_class_fops =
{
	.owner		= THIS_MODULE,
	.open = cavan_misc_device_open,
	.release = cavan_misc_device_release,
	.read = cavan_misc_device_read,
	.write = cavan_misc_device_write,

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 0, 0)
	.ioctl = cavan_misc_device_ioctl,
#else
	.unlocked_ioctl = cavan_misc_device_ioctl,
#endif
#ifdef CONFIG_COMPAT
	.compat_ioctl = cavan_misc_device_ioctl_compat,
#endif

	.llseek		= noop_llseek,
};

static int __init cavan_input_core_init(void)
{
	int ret;
	struct cavan_input_thread *thread;

	pr_pos_info();

	INIT_DELAYED_WORK(&input_core.write_online_work, cavan_input_core_write_online_work);

	cavan_input_list_init(&input_core.chip_list);
	cavan_input_list_init(&input_core.work_list);
	cavan_input_list_init(&input_core.exclude_list);

	mutex_init(&input_core.lock);

	cavan_input_class = class_create(THIS_MODULE, CAVAN_INPUT_CLASS_NAME);
	if (IS_ERR(cavan_input_class))
	{
		pr_red_info("class_create");
		ret = PTR_ERR(cavan_input_class);
		cavan_input_class = NULL;
		goto out_mutex_destroy;
	}

	ret = register_chrdev(CAVAN_INPUT_MAJOR, CAVAN_INPUT_CLASS_NAME, &cavan_input_class_fops);
	if (ret < 0)
	{
		pr_red_info("register_chrdev");
		goto out_class_destroy;
	}

	input_core.workqueue = create_singlethread_workqueue("cavan-input-wq");
	if (input_core.workqueue == NULL)
	{
		ret = -EFAULT;
		goto out_unregister_chrdev;
	}

	thread = &input_core.detect_thread;
	cavan_input_thread_set_data(thread, &input_core);
	thread->priority = 0;
	thread->stop = NULL;
	thread->prepare = NULL;
	thread->error_handle = NULL;
	thread->event_handle = cavan_input_core_event_handler;
	thread->wait_for_event = cavan_input_core_wait_for_event;
	ret = cavan_input_thread_init(thread, input_core.name);
	if (ret < 0)
	{
		pr_red_info("cavan_input_thread_init");
		goto out_destroy_workqueue;
	}

	return 0;

out_destroy_workqueue:
	destroy_workqueue(input_core.workqueue);
out_unregister_chrdev:
	unregister_chrdev(CAVAN_INPUT_MAJOR, CAVAN_INPUT_CLASS_NAME);
out_class_destroy:
	class_destroy(cavan_input_class);
out_mutex_destroy:
	mutex_destroy(&input_core.lock);
	cavan_input_list_destory(&input_core.chip_list);
	cavan_input_list_destory(&input_core.work_list);
	cavan_input_list_destory(&input_core.exclude_list);
	return ret;
}

static void __exit cavan_input_core_exit(void)
{
	pr_pos_info();

	cavan_input_thread_stop(&input_core.detect_thread);
	cavan_input_thread_destroy(&input_core.detect_thread);

	destroy_workqueue(input_core.workqueue);

	unregister_chrdev(CAVAN_INPUT_MAJOR, CAVAN_INPUT_CLASS_NAME);
	class_destroy(cavan_input_class);

	mutex_destroy(&input_core.lock);

	cavan_input_list_destory(&input_core.chip_list);
	cavan_input_list_destory(&input_core.work_list);
	cavan_input_list_destory(&input_core.exclude_list);
}

module_init(cavan_input_core_init);
module_exit(cavan_input_core_exit);

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Cavan Input Subsystem");
MODULE_LICENSE("GPL");
