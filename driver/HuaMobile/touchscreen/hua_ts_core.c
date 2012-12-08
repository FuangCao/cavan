#include "hua_ts_core.h"

static struct huamobile_ts_core ts_core =
{
	.name = "HuaMobile_TS",
	.lock = __MUTEX_INITIALIZER(ts_core.lock)
};

static struct huamobile_ts_firmware *huamobile_ts_firmware_alloc(size_t size)
{
	struct huamobile_ts_firmware *fw;

	pr_bold_info("Firmware Size = %d", size);

	fw = kmalloc(sizeof(struct huamobile_ts_firmware) + size, GFP_KERNEL);
	if (fw == NULL)
	{
		pr_red_info("kmalloc");
		return NULL;
	}

	fw->data = (void *)(fw + 1);
	fw->max_size = size;
	fw->size = 0;

	return fw;
}

static void huamobile_ts_firmware_free(struct huamobile_ts_firmware *fw)
{
	kfree(fw);
}

ssize_t huamobile_i2c_read_data(struct i2c_client *client, u8 addr, void *buff, size_t size)
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
EXPORT_SYMBOL_GPL(huamobile_i2c_read_data);

ssize_t huamobile_i2c_write_data(struct i2c_client *client, u8 addr, const void *buff, size_t size)
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
EXPORT_SYMBOL_GPL(huamobile_i2c_write_data);

int huamobile_i2c_test(struct i2c_client *client)
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
EXPORT_SYMBOL_GPL(huamobile_i2c_test);

char *huamobile_print_memory(const void *mem, size_t size)
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
EXPORT_SYMBOL_GPL(huamobile_print_memory);

static int huamobile_ts_readid(struct huamobile_ts_device *ts)
{
	pr_pos_info();

	if (ts->readid)
	{
		return ts->readid(ts);
	}

	ts->id_size = 0;

	return huamobile_i2c_test(ts->client);
}

static int huamobile_ts_power_enable(struct huamobile_ts_device *ts, bool enable)
{
	int ret;

	if (ts->is_poweron == enable)
	{
		pr_green_info("Don't need switch power state");
		return 0;
	}

	ret = ts->power_enable ? ts->power_enable(ts, enable) : sprd_ts_power_enable(enable);
	if (ret < 0)
	{
		pr_red_info("power_enable");
		return ret;
	}

	if (enable)
	{
		msleep(100);
	}

	ts->is_poweron = enable;

	return 0;
}

static int huamobile_ts_power_enable_lock(struct huamobile_ts_device *ts, bool enable)
{
	int ret;

	mutex_lock(&ts->lock);
	ret = huamobile_ts_power_enable(ts, enable);
	mutex_unlock(&ts->lock);

	return ret;
}

static int huamobile_ts_suspend_enable_base(struct huamobile_ts_device *ts, bool enable)
{
	if (ts->suspend_enable)
	{
		return ts->suspend_enable(ts, enable);
	}

	return 0;
}

static int huamobile_ts_suspend_enable(struct huamobile_ts_device *ts, bool enable)
{
	int ret;

	pr_pos_info();

	if (ts->is_suspend == enable)
	{
		pr_green_info("Don't need switch suspend state");
		return 0;
	}

	if (enable)
	{
		disable_irq_nosync(ts->client->irq);

		ret = huamobile_ts_suspend_enable_base(ts, true);
		if (ret < 0)
		{
			pr_red_info("huamobile_ts_suspend_enable_base");
			enable_irq(ts->client->irq);
			return ret;
		}

		ret = huamobile_ts_power_enable(ts, false);
		if (ret < 0)
		{
			pr_red_info("huamobile_ts_power_enable");
			huamobile_ts_suspend_enable_base(ts, false);
			enable_irq(ts->client->irq);
			return ret;
		}

		pr_green_info("`%s' enter suspend", ts->name);
	}
	else
	{
		ret = huamobile_ts_power_enable(ts, true);
		if (ret < 0)
		{
			pr_red_info("huamobile_ts_power_enable");
			return ret;
		}

		ret = huamobile_ts_suspend_enable_base(ts, false);
		if (ret < 0)
		{
			pr_red_info("huamobile_ts_suspend_enable_base");
			huamobile_ts_power_enable(ts, false);
			return ret;
		}

		enable_irq(ts->client->irq);

		pr_green_info("`%s' resume", ts->name);
	}

	ts->is_suspend = enable;

	return 0;
}

static int huamobile_ts_suspend_enable_lock(struct huamobile_ts_device *ts, bool enable)
{
	int ret;

	mutex_lock(&ts->lock);
	ret = huamobile_ts_suspend_enable(ts, enable);
	mutex_unlock(&ts->lock);

	return ret;
}

static irqreturn_t huamobile_isr_level(int irq, void *dev_id)
{
	struct huamobile_ts_device *ts = (struct huamobile_ts_device *)dev_id;

	disable_irq_nosync(irq);
	complete(&ts->event_completion);

	return IRQ_HANDLED;
}

static irqreturn_t huamobile_isr_edge(int irq, void *dev_id)
{
	struct huamobile_ts_device *ts = (struct huamobile_ts_device *)dev_id;

	complete(&ts->event_completion);

	return IRQ_HANDLED;
}

static const char *huamobile_irq_type_tostring(unsigned long flags)
{
	switch (flags & IRQF_TRIGGER_MASK)
	{
	case IRQF_TRIGGER_NONE:
		return "None";
	case IRQF_TRIGGER_FALLING:
		return "Falling Edge";
	case IRQF_TRIGGER_RISING:
		return "Rising Edge";
	case IRQF_TRIGGER_HIGH:
		return "High Level";
	case IRQF_TRIGGER_LOW:
		return "Low Level";
	case IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING:
		return "Both Edge";
	default:
		return "Invalid";
	}
}

static int huamobile_request_irq(struct huamobile_ts_device *ts)
{
	int ret;
	irq_handler_t handler;

	pr_bold_info("%s IRQ Trigger Type = %s", ts->name, huamobile_irq_type_tostring(ts->irq_flags));

	if (ts->irq_flags & (IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING))
	{
		handler = huamobile_isr_edge;
	}
	else
	{
		handler = huamobile_isr_level;
	}

	ret = request_irq(ts->client->irq, handler, ts->irq_flags, ts->name, ts);
	if (ret < 0)
	{
		pr_red_info("request_irq %d", ts->client->irq);
		return ret;
	}

	disable_irq(ts->client->irq);

	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void huamobile_ts_suspend(struct early_suspend *h)
{
	struct huamobile_ts_device *ts = container_of(h, struct huamobile_ts_device, early_suspend);
	huamobile_ts_suspend_enable_lock(ts, true);
}

static void huamobile_ts_resume(struct early_suspend *h)
{
	struct huamobile_ts_device *ts = container_of(h, struct huamobile_ts_device, early_suspend);
	huamobile_ts_suspend_enable_lock(ts, false);
}
#else
static int huamobile_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct huamobile_ts_device *ts = i2c_get_clientdata(client);
	huamobile_ts_suspend_enable_lock(ts, true);
}

static int huamobile_ts_resume(struct i2c_client *client)
{
	struct huamobile_ts_device *ts = i2c_get_clientdata(client);
	huamobile_ts_suspend_enable_lock(ts, false);
}
#endif

static int huamobile_misc_open(struct inode *inode, struct file *file)
{
	struct huamobile_ts_miscdevice *misc = (struct huamobile_ts_miscdevice *)file->f_op;
	struct huamobile_ts_misc_data *data;

	data = kmalloc(sizeof(struct huamobile_ts_misc_data), GFP_KERNEL);
	if (data == NULL)
	{
		pr_red_info("kmalloc");
		return -ENOMEM;
	}

	data->panel = misc->panel;
	data->fw = NULL;
	file->private_data = data;

	return 0;
}

static int huamobile_misc_release(struct inode *inode, struct file *file)
{
	int ret;
	struct huamobile_ts_misc_data *data = file->private_data;
	struct huamobile_ts_firmware *fw = data->fw;

	pr_pos_info();

	if (fw)
	{
		struct huamobile_ts_device *ts = data->panel;

		mutex_lock(&ts->lock);
		wake_lock(&ts->wake_lock);
		huamobile_ts_suspend_enable(ts, false);
		disable_irq_nosync(ts->client->irq);
		ret = ts->firmware_upgrade(ts, fw->data, fw->size);
		enable_irq(ts->client->irq);
		wake_unlock(&ts->wake_lock);
		mutex_unlock(&ts->lock);

		huamobile_ts_firmware_free(fw);

		if (ret < 0)
		{
			pr_red_info("ts->firmware_upgrade");
		}

		return ret;
	}

	kfree(data);

	return 0;
}

static ssize_t huamobile_misc_read(struct file *file, char __user *buff, size_t size, loff_t *offset)
{
	pr_pos_info();

	return 0;
}

static ssize_t huamobile_misc_write(struct file *file, const char __user *buff, size_t size, loff_t *offset)
{
	struct huamobile_ts_misc_data *data = file->private_data;
	struct huamobile_ts_firmware *fw;

	pr_bold_info("size = %d", size);

	if (data->fw == NULL)
	{
		struct huamobile_ts_device *ts = data->panel;

		if (ts->firmware_size == 0 || ts->firmware_upgrade == NULL)
		{
			pr_red_info("ts->firmware_size == 0 || ts->firmware_upgrade == NULL");
			return -EINVAL;
		}

		fw = huamobile_ts_firmware_alloc(ts->firmware_size);
		if (fw == NULL)
		{
			pr_red_info("huamobile_ts_firmware_alloc");
			return -ENOMEM;
		}

		data->fw = fw;
	}
	else
	{
		fw = data->fw;
	}

	if (fw->size + size > fw->max_size)
	{
		pr_red_info("fw->size + size > fw->max_size");
		return -ENOMEDIUM;
	}

	if (copy_from_user(fw->data + fw->size, buff, size))
	{
		pr_red_info("copy_from_user");
		return -EIO;
	}

	fw->size += size;

	return size;
}

static int huamobile_ts_calibration(struct huamobile_ts_device *ts)
{
	int ret;

	pr_pos_info();

	if (ts->calibration == NULL)
	{
		pr_red_info("ts->calibration == NULL");
		return -EIO;
	}

	mutex_lock(&ts->lock);
	wake_lock(&ts->wake_lock);
	huamobile_ts_suspend_enable(ts, false);
	ret = ts->calibration(ts);
	wake_unlock(&ts->wake_lock);
	mutex_unlock(&ts->lock);

	return ret;
}

static int huamobile_misc_ioctl_i2c_transfer(struct huamobile_ts_device *ts, void __user *data)
{
	int ret;
	struct i2c_client *client;
	struct huamobile_i2c_request req;

	if (copy_from_user(&req, data, sizeof(req)))
	{
		pr_red_info("copy_from_user");
		return -EFAULT;
	}

	client = ts->client;

	if (req.read)
	{
		char buff[req.size];

		if (req.addr < 0)
		{
			ret = i2c_master_recv(client, buff, req.size);
		}
		else
		{
			ret = huamobile_i2c_read_data(client, req.addr, buff, req.size);
		}

		if (ret < 0)
		{
			pr_red_info("i2c read data");
			return ret;
		}

		if (copy_to_user(req.data, buff, req.size))
		{
			pr_red_info("copy_to_user");
			return -EFAULT;
		}
	}
	else
	{
		char buff[req.size];

		if (copy_from_user(buff, req.data, req.size))
		{
			pr_red_info("copy_from_user");
			return -EFAULT;
		}

		if (req.addr < 0)
		{
			ret = i2c_master_send(client, buff, req.size);
		}
		else
		{
			ret = huamobile_i2c_write_data(client, req.addr, buff, req.size);
		}

		if (ret < 0)
		{
			pr_red_info("i2c write data");
			return ret;
		}
	}

	return 0;
}

static int huamobile_misc_ioctl(struct inode *inode, struct file *file, unsigned int command, unsigned long args)
{
	int value, length;
	struct huamobile_ts_misc_data *data = file->private_data;
	struct huamobile_ts_device *ts = data->panel;
	int __user *int_data = (int __user *)args;

	pr_pos_info();

	switch (command)
	{
	case HUA_TS_IOCTL_CALIBRATION:
		return huamobile_ts_calibration(ts);

	case HUA_TS_IOCTL_LOCK:
		if (get_user(value, int_data))
		{
			return -EFAULT;
		}

		if (value)
		{
			mutex_lock(&ts->lock);
			wake_lock(&ts->wake_lock);
		}
		else
		{
			wake_unlock(&ts->wake_lock);
			mutex_unlock(&ts->lock);
		}

		return 0;

	case HUA_TS_IOCTL_IRQ_ENABLE:
		if (get_user(value, int_data))
		{
			return -EFAULT;
		}

		if (value)
		{
			enable_irq(ts->client->irq);
		}
		else
		{
			disable_irq_nosync(ts->client->irq);
		}

		return 0;

	case HUA_TS_IOCTL_POWER_ENABLE:
		if (get_user(value, int_data))
		{
			return -EFAULT;
		}

		return huamobile_ts_power_enable(ts, value);

	case HUA_TS_IOCTL_SUSPEND_ENABLE:
		if (get_user(value, int_data))
		{
			return -EFAULT;
		}

		return huamobile_ts_suspend_enable(ts, value);

	case HUA_TS_IOCTL_SET_FW_SIZE:
		if (get_user(ts->firmware_size, int_data))
		{
			return -EFAULT;
		}

		return 0;

	case HUA_TS_IOCTL_I2C_TRANFER:
		return huamobile_misc_ioctl_i2c_transfer(ts, int_data);

	case HUA_TS_IOCTL_RESET_ENABLE:
		if (get_user(value, int_data))
		{
			return -EFAULT;
		}

		sprd_ts_reset_enable(value);
		return 0;

	case HUA_TS_IOCTL_IRQ_OUTPUT:
		if (get_user(value, int_data))
		{
			return -EFAULT;
		}

		sprd_ts_irq_output(value);
		return 0;

	default:
		switch (_IOC_NR(command))
		{
		case HUA_TS_IOCTL_GET_NAME(0):
			value = _IOC_SIZE(command);
			length = strlen(ts->name) + 1;
			if (length > value)
			{
				length = value;
			}

			if (copy_to_user(int_data, ts->name, length))
			{
				pr_red_info("copy_to_user");
				return -EFAULT;
			}

			return 0;
		}
	}

	return ts->ioctl ? ts->ioctl(ts, command, args) : -EINVAL;
}

static struct huamobile_ts_miscdevice huamobile_misc =
{
	.fops =
	{
		.open = huamobile_misc_open,
		.release = huamobile_misc_release,
		.read = huamobile_misc_read,
		.write = huamobile_misc_write,
		.ioctl = huamobile_misc_ioctl
	},
	.misc =
	{
		.minor = MISC_DYNAMIC_MINOR,
		.mode = S_IRUGO | S_IWUGO
	}
};

static int huamobile_ts_init(struct huamobile_ts_device *ts)
{
	if (ts->name == NULL)
	{
		pr_red_info("ts->name == NULL");
		return -EINVAL;
	}

	if (ts->event_loop == NULL)
	{
		pr_red_info("ts->event_loop == NULL");
		return -EINVAL;
	}

	if (ts->client == NULL)
	{
		pr_red_info("ts->client == NULL");
		return -EINVAL;
	}

	i2c_set_clientdata(ts->client, ts);
	mutex_init(&ts->lock);
	init_completion(&ts->event_completion);

	ts->is_poweron = false;
	ts->is_suspend = true;
	ts->state = HUAMOBILE_TS_STATE_STOPED;

	return 0;
}

static ssize_t huamobile_sysfs_show(struct kobject *kobj, struct attribute *attr, char *buff)
{
	struct huamobile_ts_kobject *hua_kobj = (struct huamobile_ts_kobject *)kobj;
	struct huamobile_ts_attribute *hua_attr = (struct huamobile_ts_attribute *)attr;

	if (hua_attr->show == NULL || hua_kobj->panel == NULL)
	{
		pr_red_info("hua_attr->show == NULL || hua_kobj->panel == NULL");
		return -EINVAL;
	}

	return hua_attr->show(hua_kobj->panel, hua_attr, buff);
}

static ssize_t huamobile_sysfs_store(struct kobject *kobj,struct attribute *attr, const char *buff, size_t size)
{
	struct huamobile_ts_kobject *hua_kobj = (struct huamobile_ts_kobject *)kobj;
	struct huamobile_ts_attribute *hua_attr = (struct huamobile_ts_attribute *)attr;

	if (hua_attr->store == NULL || hua_kobj->panel == NULL)
	{
		pr_red_info("hua_attr->store == NULL || hua_attr->panel == NULL");
		return -EINVAL;
	}

	return hua_attr->store(hua_kobj->panel, hua_attr, buff, size);
}

static struct sysfs_ops huamobile_sysfs_ops =
{
	.show = huamobile_sysfs_show,
	.store = huamobile_sysfs_store
};

static struct kobj_type huamobile_kobj_type =
{
	.sysfs_ops = &huamobile_sysfs_ops
};

static int huamobile_add_kobject(struct huamobile_ts_device *ts, struct huamobile_ts_kobject *kobj)
{
	int ret;
	struct huamobile_ts_attribute *p, *p_end;

	if (kobj->kobj.state_initialized == 0)
	{
		kobject_init(&kobj->kobj, &huamobile_kobj_type);
	}

	ret = kobject_add(&kobj->kobj, NULL, kobj->name);
	if (ret < 0)
	{
		pr_red_info("kobject_init_and_add");
		return ret;
	}

	for (p = kobj->attrs, p_end = p + kobj->attr_count; p < p_end; p++)
	{
		if (p->attr.name == NULL)
		{
			if (p->name[0] == 0)
			{
				sprintf(p->name, "virtualkeys.%s", ts->name);
			}

			p->attr.name = p->name;
		}

		ret = sysfs_create_file(&kobj->kobj, &p->attr);
		if (ret < 0)
		{
			pr_red_info("sysfs_create_file %s failed", p->attr.name);

			for (p--; p >= kobj->attrs; p--)
			{
				sysfs_remove_file(&kobj->kobj, &p->attr);
			}

			kobject_del(&kobj->kobj);
			kobject_put(&kobj->kobj);
			return ret;
		}
	}

	kobj->panel = ts;

	return 0;
}

static void huamobile_remove_kobject(struct huamobile_ts_kobject *kobj)
{
	struct huamobile_ts_attribute *p;

	for (p = kobj->attrs + kobj->attr_count - 1; p >= kobj->attrs; p--)
	{
		sysfs_remove_file(&kobj->kobj, &p->attr);
	}

	kobject_del(&kobj->kobj);
	kobject_put(&kobj->kobj);
}

static ssize_t huamobile_board_properties_show(struct huamobile_ts_device *ts, struct huamobile_ts_attribute *attr, char *buff)
{
	const struct huamobile_touch_key *key, *key_end;
	char *p;

	if (ts->keys == NULL || ts->key_count == 0)
	{
		return 0;
	}

	for (p = buff, key = ts->keys, key_end = key + ts->key_count; key < key_end; key++)
	{
		p += sprintf(p, "0x%02x:%d:%d:%d:%d:%d:", EV_KEY, key->code, key->x, key->y, key->width, key->height);
	}

	return p - buff;
}

static struct huamobile_ts_attribute huamobile_board_properties_attr =
{
	.attr =
	{
		.owner = THIS_MODULE,
		.mode = S_IRUGO
	},

	.show = huamobile_board_properties_show,
};

static struct huamobile_ts_kobject huamobile_board_properties_kobj =
{
	.name = "board_properties",
	.attr_count = 1,
	.attrs = &huamobile_board_properties_attr
};

static ssize_t huamobile_touchscreen_info_show(struct huamobile_ts_device *ts, struct huamobile_ts_attribute *attr, char *buff)
{
	char *p;

	pr_pos_info();

	mutex_lock(&ts->lock);

	p = buff;
	p += sprintf(p, "Name = %s\n", ts->name);
	p += sprintf(p, "Address = 0x%02x\n", ts->client->addr);
	p += sprintf(p, "PointCount = %d\n", ts->point_count);
	p += sprintf(p, "IRQ = %d, Type = %s\n", ts->client->irq, huamobile_irq_type_tostring(ts->irq_flags));
	p += sprintf(p, "X-Axis: min = %d, max = %d\n", ts->xmin, ts->xmax);
	p += sprintf(p, "Y-Axis: min = %d, max = %d\n", ts->ymin, ts->ymax);

	if (ts->keys && ts->key_count)
	{
		const struct huamobile_touch_key *key, *key_end;

		for (key = ts->keys, key_end = key + ts->key_count; key < key_end; key++)
		{
			p += sprintf(p, "KEY[%d]: x = %d, y = %d, width = %d, height = %d\n", \
				key->code, key->x, key->y, key->width, key->height);
		}
	}

	mutex_unlock(&ts->lock);

	return p - buff;
}

static ssize_t huamobile_touchscreen_command_show(struct huamobile_ts_device *ts, struct huamobile_ts_attribute *attr, char *buff)
{
	pr_pos_info();

	return 0;
}

static ssize_t huamobile_touchscreen_command_store(struct huamobile_ts_device *ts, struct huamobile_ts_attribute *attr, const char *buff, size_t count)
{
	pr_pos_info();

	return count;
}

static ssize_t huamobile_touchscreen_calibration_show(struct huamobile_ts_device *ts, struct huamobile_ts_attribute *attr, char *buff)
{
	int ret;

	pr_pos_info();

	ret = huamobile_ts_calibration(ts);
	if (ret < 0)
	{
		pr_red_info("huamobile_ts_calibration");
		return ret;
	}

	return 0;
}

static ssize_t huamobile_touchscreen_calibration_store(struct huamobile_ts_device *ts, struct huamobile_ts_attribute *attr, const char *buff, size_t count)
{
	int ret;

	pr_pos_info();

	ret = huamobile_ts_calibration(ts);
	if (ret < 0)
	{
		pr_red_info("huamobile_ts_calibration");
		return ret;
	}

	return count;
}

static struct huamobile_ts_attribute huamobile_touchscreen_attrs[] =
{
	{
		.attr =
		{
			.name = "info",
			.owner = THIS_MODULE,
			.mode = S_IRUGO | S_IWUGO
		},

		.show = huamobile_touchscreen_info_show,
	},
	{
		.attr =
		{
			.name = "command",
			.owner = THIS_MODULE,
			.mode = S_IRUGO | S_IWUGO
		},

		.show = huamobile_touchscreen_command_show,
		.store = huamobile_touchscreen_command_store
	},
	{
		.attr =
		{
			.name = "calibration",
			.owner = THIS_MODULE,
			.mode = S_IRUGO | S_IWUGO
		},

		.show = huamobile_touchscreen_calibration_show,
		.store = huamobile_touchscreen_calibration_store
	}
};

static struct huamobile_ts_kobject huamobile_touchscreen_kobj =
{
	.attr_count = ARRAY_SIZE(huamobile_touchscreen_attrs),
	.attrs = huamobile_touchscreen_attrs
};

static int huamobile_ts_probe(struct huamobile_ts_core *core, struct huamobile_ts_device *ts)
{
	int ret;
	struct input_dev *input;
	struct i2c_client *client = ts->client;
	const struct huamobile_touch_key *key, *key_end;

	pr_pos_info();

	wake_lock_init(&ts->wake_lock, WAKE_LOCK_SUSPEND, ts->name);

	input = input_allocate_device();
	if (input == NULL)
	{
		ret = -ENOMEM;
		pr_red_info("input_allocate_device");
		goto out_wake_lock_destroy;
	}

	if (ts->keys && ts->key_count)
	{
		set_bit(EV_KEY, input->evbit);

		for (key = ts->keys, key_end = key + ts->key_count; key < key_end; key++)
		{
			set_bit(key->code, input->keybit);
		}
	}

	set_bit(EV_ABS, input->evbit);
	input_set_abs_params(input, ABS_MT_POSITION_X, ts->xmin, ts->xmax, 0, 0);
	input_set_abs_params(input, ABS_MT_POSITION_Y, ts->ymin, ts->ymax, 0, 0);
	input_set_abs_params(input, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(input, ABS_MT_WIDTH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(input, ABS_MT_TRACKING_ID, 0, ts->point_count - 1, 0, 0);

	ts->input = input;
	input_set_drvdata(input, ts);
	input->name = ts->name;

	ret = input_register_device(input);
	if (ret < 0)
	{
		pr_red_info("input_register_device");
		goto out_input_free_device;
	}

	ret = huamobile_request_irq(ts);
	if (ret < 0)
	{
		pr_red_info("huamobile_request_irq");
		goto out_input_unregister_device;
	}

	ret = huamobile_add_kobject(ts, &huamobile_board_properties_kobj);
	if (ret < 0)
	{
		pr_red_info("huamobile_add_attribute");
		goto out_free_irq;
	}

	huamobile_touchscreen_kobj.name = core->name;
	ret = huamobile_add_kobject(ts, &huamobile_touchscreen_kobj);
	if (ret < 0)
	{
		pr_red_info("huamobile_add_attribute");
		goto out_huamobile_remove_kobject_board_properties;
	}

	huamobile_misc.misc.name = core->name;
	huamobile_misc.misc.fops = &huamobile_misc.fops;
	huamobile_misc.panel = ts;
	ret = misc_register(&huamobile_misc.misc);
	if (ret < 0)
	{
		pr_red_info("misc_register");
		goto out_huamobile_remove_kobject_touchscreen;
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
	client->driver->suspend = NULL;
	client->driver->resume = NULL;
	ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	ts->early_suspend.suspend = huamobile_ts_suspend;
	ts->early_suspend.resume = huamobile_ts_resume;
	register_early_suspend(&ts->early_suspend);
#else
	client->driver->suspend = huamobile_ts_suspend;
	client->driver->resume = huamobile_ts_resume;
#endif

	pr_green_info("Touch screen `%s' init complete", ts->name);
	return 0;

out_huamobile_remove_kobject_touchscreen:
	huamobile_remove_kobject(&huamobile_touchscreen_kobj);
out_huamobile_remove_kobject_board_properties:
	huamobile_remove_kobject(&huamobile_board_properties_kobj);
out_free_irq:
	free_irq(client->irq, ts);
out_input_unregister_device:
	input_unregister_device(input);
out_input_free_device:
	input_free_device(input);
out_wake_lock_destroy:
	wake_lock_destroy(&ts->wake_lock);
	return ret;
}

static void huamobile_ts_remove(struct huamobile_ts_device *ts)
{
	struct input_dev *input = ts->input;
	struct i2c_client *client = ts->client;

	pr_pos_info();

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&ts->early_suspend);
#endif

	misc_deregister(&huamobile_misc.misc);
	huamobile_remove_kobject(&huamobile_touchscreen_kobj);
	huamobile_remove_kobject(&huamobile_board_properties_kobj);
	free_irq(client->irq, ts);
	input_unregister_device(input);
	input_free_device(input);
	wake_lock_destroy(&ts->wake_lock);
}

static int huamobile_find_device(struct huamobile_ts_device **panels, size_t count, struct huamobile_ts_device *ts)
{
	int i;

	for (i = 0; i < count; i++)
	{
		if (panels[i] == ts)
		{
			return i;
		}
	}

	return -ENOMEDIUM;
}

int huamobile_ts_register(struct huamobile_ts_device *ts)
{
	int ret;
	int index;

	pr_pos_info();

	ret = huamobile_ts_init(ts);
	if (ret < 0)
	{
		pr_red_info("huamobile_ts_init");
		return ret;
	}

	mutex_lock(&ts_core.lock);

	index = huamobile_find_device(ts_core.panels, ARRAY_SIZE(ts_core.panels), NULL);
	if (index < 0)
	{
		pr_red_info("huamobile_get_free_desc");
		mutex_unlock(&ts_core.lock);
		return index;
	}

	ts_core.panels[index] = ts;
	ts_core.panel_count++;
	wake_up_process(ts_core.event_task);

	mutex_unlock(&ts_core.lock);

	pr_green_info("Touch screen %s register complete", ts->name);

	return 0;
}
EXPORT_SYMBOL_GPL(huamobile_ts_register);

static int huamobile_ts_stop(struct huamobile_ts_device *ts)
{
	pr_pos_info();

	mutex_lock(&ts->lock);

	if (ts->state != HUAMOBILE_TS_STATE_RUNNING)
	{
		mutex_unlock(&ts->lock);
		return 0;
	}

	ts->state = HUAMOBILE_TS_STATE_STOPPING;

	while (1)
	{
		mutex_unlock(&ts->lock);
		complete(&ts->event_completion);
		msleep(1);
		mutex_lock(&ts->lock);

		if (ts->state == HUAMOBILE_TS_STATE_STOPED)
		{
			break;
		}
	}

	mutex_unlock(&ts->lock);

	return 0;
}

int huamobile_ts_unregister(struct huamobile_ts_device *ts)
{
	int index;

	pr_pos_info();

	mutex_lock(&ts_core.lock);

	index = huamobile_find_device(ts_core.panels, ARRAY_SIZE(ts_core.panels), ts);
	if (index < 0)
	{
		pr_red_info("huamobile_find_device");
		mutex_unlock(&ts_core.lock);
		return index;
	}

	ts_core.panels[index] = NULL;
	ts_core.panel_count--;
	huamobile_ts_stop(ts);

	mutex_unlock(&ts_core.lock);

	pr_green_info("Touch screen %s unregister complete", ts->name);

	return 0;
}
EXPORT_SYMBOL_GPL(huamobile_ts_unregister);

static void huamobile_ts_set_state_lock(struct huamobile_ts_device *ts, enum huamobile_ts_state state)
{
	mutex_lock(&ts->lock);
	ts->state = state;
	mutex_unlock(&ts->lock);
}

static void huamobile_ts_show_device_id(struct huamobile_ts_device *ts)
{
	char buff[1024], *p;
	const u8 *id, *id_end;

	if (ts->id_size == 0)
	{
		return;
	}

	for (p = buff, id = ts->dev_id, id_end = id + ts->id_size; id < id_end; id++)
	{
		p += sprintf(p, "%02x", *id);
	}

	pr_green_info("%s Device ID = 0x%s", ts->name, buff);
}

static int huamobile_ts_run(struct huamobile_ts_core *core, struct huamobile_ts_device *ts)
{
	int ret;

	pr_green_info("Try Touch Screen `%s'", ts->name);

	ret = huamobile_ts_power_enable_lock(ts, true);
	if (ret < 0)
	{
		pr_red_info("huamobile_ts_power_enable");
		return ret;
	}

	ret = huamobile_ts_readid(ts);
	if (ret < 0)
	{
		pr_red_info("This touch screen is not %s", ts->name);
		goto out_power_disable;
	}

	huamobile_ts_show_device_id(ts);

	ret = huamobile_ts_probe(core, ts);
	if (ret < 0)
	{
		pr_red_info("huamobile_ts_probe");
		goto out_power_disable;
	}

	ret = huamobile_ts_suspend_enable_lock(ts, false);
	if (ret < 0)
	{
		pr_red_info("huamobile_ts_suspend_enable_lock");
		goto out_huamobile_ts_remove;
	}

	pr_green_info("%s Enter Event Loop", ts->name);
	ts->event_loop(ts);
	pr_red_info("%s Exit Event Loop", ts->name);

	huamobile_ts_suspend_enable_lock(ts, true);
out_huamobile_ts_remove:
	huamobile_ts_remove(ts);
out_power_disable:
	huamobile_ts_power_enable_lock(ts, false);
	return ret;
}

static int huamobile_event_thread_func(void *data)
{
	int i;
	int ret;
	unsigned int timeout;
	struct huamobile_ts_core *core;
	struct huamobile_ts_device *ts;

	core = (struct huamobile_ts_core *)data;
	timeout = 0;
	mutex_lock(&core->lock);

	while (1)
	{
		while (core->panel_count == 0)
		{
			if (kthread_should_stop())
			{
				goto out_mutex_unlock;
			}

			mutex_unlock(&core->lock);
			pr_green_info("`%s' enter sleep", core->name);
			set_current_state(TASK_UNINTERRUPTIBLE);
			schedule();
			pr_green_info("`%s' exit sleep", core->name);
			mutex_lock(&core->lock);
		}

		pr_bold_info("panel count = %d, timeout = %dms", core->panel_count, timeout);

		for (i = 0; i < ARRAY_SIZE(core->panels); i++)
		{
			if (kthread_should_stop())
			{
				goto out_mutex_unlock;
			}

			ts = core->panels[i];
			if (ts == NULL)
			{
				continue;
			}

			mutex_unlock(&core->lock);
			huamobile_ts_set_state_lock(ts, HUAMOBILE_TS_STATE_RUNNING);

			ret = huamobile_ts_run(core, ts);
			if (ret >= 0)
			{
				timeout = 0;
			}

			huamobile_ts_set_state_lock(ts, HUAMOBILE_TS_STATE_STOPED);
			mutex_lock(&core->lock);
		}

		if (core->panel_count)
		{
			mutex_unlock(&core->lock);
			msleep_interruptible(timeout);
			mutex_lock(&core->lock);

			if (timeout < 10 * 1000)
			{
				timeout += 100;
			}
		}
		else
		{
			timeout = 0;
		}
	}

out_mutex_unlock:
	core->event_task = NULL;
	mutex_unlock(&core->lock);
	pr_red_info("%s event thread exit", core->name);

	return 0;
}

static int huamobile_create_event_thread(struct huamobile_ts_core *core)
{
	int ret;
	struct sched_param param;

	mutex_lock(&core->lock);

	if (core->event_task)
	{
		ret = 0;
		pr_green_info("Thread already run");
		goto out_mutex_unlock;
	}

	core->event_task = kthread_create(huamobile_event_thread_func, core, core->name);
	if (core->event_task == NULL)
	{
		ret = -EFAULT;
		pr_red_info("kthread_create");
		goto out_mutex_unlock;
	}

	param.sched_priority = MAX_RT_PRIO - 1;
	sched_setscheduler(core->event_task, SCHED_FIFO, &param);

	ret = 0;
out_mutex_unlock:
	mutex_unlock(&core->lock);
	return ret;
}

static void huamobile_stop_event_thread(struct huamobile_ts_core *core)
{
	mutex_lock(&core->lock);

	if (core->event_task)
	{
		mutex_unlock(&core->lock);
		kthread_stop(core->event_task);
		mutex_lock(&core->lock);
		core->event_task = NULL;
	}

	mutex_unlock(&core->lock);
}

static int __init huamobile_ts_core_init(void)
{
	int ret;

	pr_pos_info();

	ret = huamobile_create_event_thread(&ts_core);
	if (ret < 0)
	{
		pr_red_info("huamobile_event_thread_start");
		return ret;
	}

	pr_green_info("HuaMobile Touch Screen Subsystem Init Complete");

	return 0;
}

static void __exit huamobile_ts_core_exit(void)
{
	huamobile_stop_event_thread(&ts_core);
}

module_init(huamobile_ts_core_init);
module_exit(huamobile_ts_core_exit);

MODULE_AUTHOR("<Fuang.Cao cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Huamobile TouchScreen Subsystem");
MODULE_LICENSE("GPL");
