#include <cavan/cavan_sysfs.h>
#include <linux/gpio.h>

static struct kobject *root_kobj;

static ssize_t cavan_input_sysfs_show(struct kobject *kobj, struct attribute *attr, char *buff)
{
	struct cavan_input_attribute *cavan_attr = (struct cavan_input_attribute *) attr;

	if (cavan_attr->show == NULL || cavan_attr->dev == NULL) {
		pr_red_info("cavan_attr->show == NULL || root_kobj->dev == NULL");
		return -EINVAL;
	}

	return cavan_attr->show(cavan_attr->dev, cavan_attr, buff);
}

static ssize_t cavan_input_sysfs_store(struct kobject *kobj,struct attribute *attr, const char *buff, size_t size)
{
	struct cavan_input_attribute *cavan_attr = (struct cavan_input_attribute *) attr;

	if (cavan_attr->store == NULL || cavan_attr->dev == NULL) {
		pr_red_info("cavan_attr->store == NULL || root_kobj->dev == NULL");
		return -EINVAL;
	}

	return cavan_attr->store(cavan_attr->dev, cavan_attr, buff, size);
}

static struct sysfs_ops cavan_input_sysfs_ops = {
	.show = cavan_input_sysfs_show,
	.store = cavan_input_sysfs_store
};

static struct kobj_type cavan_input_kobj_type = {
	.sysfs_ops = &cavan_input_sysfs_ops
};

int cavan_input_add_kobject(struct kobject *kobj, const char *name)
{
	int ret;

	if (kobj->state_initialized == 0) {
		kobject_init(kobj, &cavan_input_kobj_type);
	}

	ret = kobject_add(kobj, NULL, name);
	if (ret < 0) {
		pr_red_info("kobject_add");
		return ret;
	}

	return 0;
}

EXPORT_SYMBOL_GPL(cavan_input_add_kobject);

void cavan_input_remove_kobject(struct kobject *kobj)
{
	kobject_del(kobj);
	kobject_put(kobj);
}

EXPORT_SYMBOL_GPL(cavan_input_remove_kobject);

int cavan_input_create_sysfs_files(struct cavan_input_device *dev, struct kobject *kobj, struct cavan_input_attribute *attrs, size_t count)
{
	struct cavan_input_attribute *p, *p_end;

	for (p = attrs, p_end = p + count; p < p_end; p++) {
		int ret;

		p->dev = dev;

		ret = sysfs_create_file(kobj, &p->attr);
		if (ret < 0) {
			pr_red_info("sysfs_create_file %s failed", p->attr.name);

			for (p--; p >= attrs; p--) {
				sysfs_remove_file(kobj, &p->attr);
			}

			return ret;
		}
	}

	return 0;
}

EXPORT_SYMBOL_GPL(cavan_input_create_sysfs_files);

void cavan_input_remove_sysfs_files(struct kobject *kobj, struct cavan_input_attribute *attrs, size_t count)
{
	struct cavan_input_attribute *p, *p_end;

	for (p = attrs, p_end = p + count; p < p_end; p++) {
		sysfs_remove_file(kobj, &p->attr);
	}
}

EXPORT_SYMBOL_GPL(cavan_input_remove_sysfs_files);

int cavan_sysfs_create_file(const struct attribute *attr)
{
	if (root_kobj == NULL) {
		return -EFAULT;
	}

	return sysfs_create_file(root_kobj, attr);
}

EXPORT_SYMBOL_GPL(cavan_sysfs_create_file);

static ssize_t cavan_sysfs_show(struct kobject *kobj, struct attribute *attr, char *buff)
{
	struct cavan_sysfs_attribute *cavan_attr = (struct cavan_sysfs_attribute *) attr;

	if (cavan_attr->show == NULL) {
		pr_red_info("cavan_attr->show == NULL");
		return -EINVAL;
	}

	return cavan_attr->show(cavan_attr, buff);
}

static ssize_t cavan_sysfs_store(struct kobject *kobj,struct attribute *attr, const char *buff, size_t size)
{
	struct cavan_sysfs_attribute *cavan_attr = (struct cavan_sysfs_attribute *) attr;

	if (cavan_attr->store == NULL) {
		pr_red_info("cavan_attr->store == NULL");
		return -EINVAL;
	}

	return cavan_attr->store(cavan_attr, buff, size);
}

static struct sysfs_ops cavan_sysfs_ops = {
	.show = cavan_sysfs_show,
	.store = cavan_sysfs_store
};

static struct kobj_type cavan_kobj_type = {
	.sysfs_ops = &cavan_sysfs_ops
};

int cavan_sysfs_create_files(const struct attribute **attr)
{
	if (root_kobj == NULL) {
		return -EFAULT;
	}

	return sysfs_create_files(root_kobj, attr);
}

EXPORT_SYMBOL_GPL(cavan_sysfs_create_files);

static int cavan_current_gpio;

static ssize_t cavan_sysfs_gpio_value_show(struct cavan_sysfs_attribute *attr, char *buff)
{
	return sprintf(buff, "gpio = %d, value = %d\n", cavan_current_gpio, gpio_get_value_cansleep(cavan_current_gpio));
}

static ssize_t cavan_sysfs_gpio_value_store(struct cavan_sysfs_attribute *attr, const char *buff, size_t count)
{
	int gpio, value;

	switch (sscanf(buff, "%d %d", &gpio, &value)) {
	case 1:
		break;

	case 2:
		gpio_set_value_cansleep(gpio, value);
		break;

	default:
		return -EINVAL;
	}

	cavan_current_gpio = gpio;

	return count;
}

static ssize_t cavan_sysfs_gpio_direction_store(struct cavan_sysfs_attribute *attr, const char *buff, size_t count)
{
	int gpio, value;

	if (sscanf(buff, "input %d", &gpio) == 1) {
		gpio_direction_input(gpio);
	} else if (sscanf(buff, "output %d %d", &gpio, &value) == 2) {
		gpio_direction_output(gpio, value);
	} else {
		return -EINVAL;
	}

	cavan_current_gpio = gpio;

	return count;
}

static struct cavan_sysfs_attribute cavan_sysfs_gpio_value =
	__ATTR(gpio_value, S_IWUSR | S_IRUGO, cavan_sysfs_gpio_value_show, cavan_sysfs_gpio_value_store);

static struct cavan_sysfs_attribute cavan_sysfs_gpio_direction =
	__ATTR(gpio_direction, S_IWUSR, NULL, cavan_sysfs_gpio_direction_store);

static const struct attribute *cavan_sysfs_attrs[] = {
	&cavan_sysfs_gpio_value.attr,
	&cavan_sysfs_gpio_direction.attr,
	NULL,
};

static __init int cavan_sysfs_init(void)
{
	root_kobj = kobject_create_and_add("cavan", NULL);
	if (root_kobj == NULL) {
		pr_err("kobject_create_and_add cavan failed\n");
		return -EFAULT;
	}

	root_kobj->ktype = &cavan_kobj_type;

	cavan_sysfs_create_files(cavan_sysfs_attrs);

	return 0;
}

static __exit void cavan_sysfs_exit(void)
{
	if (root_kobj) {
		kobject_del(root_kobj);
	}
}

subsys_initcall(cavan_sysfs_init);
module_exit(cavan_sysfs_exit);

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Cavan Sysfs");
MODULE_LICENSE("GPL");
