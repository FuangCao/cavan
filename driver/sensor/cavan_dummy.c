#include <cavan/cavan_sensor.h>
#include <cavan/cavan_i2c.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>

#define DUMMY_RESOLUTION			100
#define DUMMY_ACC_RANGE				1
#define DUMMY_GYR_RANGE				100

struct dummy_chip {
	struct cavan_sensor_device acceleration;
	struct cavan_sensor_device gyroscope;
};

static int acceleration_x;
module_param(acceleration_x, int, 0644);
static int acceleration_y;
module_param(acceleration_y, int, 0644);
static int acceleration_z = DUMMY_RESOLUTION;
module_param(acceleration_z, int, 0644);
static int acceleration_fuzz = 1;
module_param(acceleration_fuzz, int, 0644);

static int gyroscope_x;
module_param(gyroscope_x, int, 0644);
static int gyroscope_y;
module_param(gyroscope_y, int, 0644);
static int gyroscope_z;
module_param(gyroscope_z, int, 0644);
static int gyroscope_fuzz = 1;
module_param(gyroscope_fuzz, int, 0644);

// ================================================================================

static int dummy_sensor_chip_readid(struct cavan_input_chip *chip)
{
	pr_pos_info();

	return 0;
}

static int dummy_sensor_chip_set_active(struct cavan_input_chip *chip, bool enable)
{
	pr_pos_info();

	return 0;
}

// ================================================================================

static int dummy_acceleration_set_delay(struct cavan_input_device *dev, unsigned int delay)
{
	pr_pos_info();

	return 0;
}

static int dummy_acceleration_set_enable(struct cavan_input_device *dev, bool enable)
{
	pr_pos_info();

	return 0;
}

static int dummy_acceleration_event_handler(struct cavan_input_chip *chip, struct cavan_input_device *dev)
{
	static unsigned int count;

	if ((++count) & 1) {
		cavan_gsensor_report_vector(dev->input, acceleration_x + acceleration_fuzz, acceleration_y + acceleration_fuzz, acceleration_z + acceleration_fuzz);
	} else {
		cavan_gsensor_report_vector(dev->input, acceleration_x - acceleration_fuzz, acceleration_y - acceleration_fuzz, acceleration_z - acceleration_fuzz);
	}

	return 0;
}

// ================================================================================

static int dummy_gyroscope_set_delay(struct cavan_input_device *dev, unsigned int delay)
{
	pr_pos_info();

	return 0;
}

static int dummy_gyroscope_set_enable(struct cavan_input_device *dev, bool enable)
{
	pr_pos_info();

	return 0;
}

static int dummy_gyroscope_event_handler(struct cavan_input_chip *chip, struct cavan_input_device *dev)
{
	int x = gyroscope_x;
	int y = gyroscope_y;
	int z = gyroscope_z;
	static unsigned int count;

	if ((++count) & 1) {
		if (x) {
			x += gyroscope_fuzz;
		}

		if (y) {
			y += gyroscope_fuzz;
		}

		if (z) {
			z += gyroscope_fuzz;
		}
	} else {
		if (x) {
			x -= gyroscope_fuzz;
		}

		if (y) {
			y -= gyroscope_fuzz;
		}

		if (z) {
			z -= gyroscope_fuzz;
		}
	}

	cavan_gyroscope_report_vector(dev->input, x, y, z);

	return 0;
}

// ================================================================================

static int dummy_input_chip_probe(struct cavan_input_chip *chip)
{
	int ret;
	struct dummy_chip *dummy;
	struct cavan_sensor_device *sensor;
	struct cavan_input_device *dev;

	pr_pos_info();

	dummy = kzalloc(sizeof(struct dummy_chip), GFP_KERNEL);
	if (dummy == NULL) {
		pr_red_info("kzalloc");
		return -ENOMEM;
	}

	cavan_input_chip_set_dev_data(chip, dummy);

	sensor = &dummy->acceleration;
	sensor->max_range = DUMMY_ACC_RANGE;
	sensor->resolution = DUMMY_RESOLUTION;
	sensor->power_consume = 145;

	dev = &sensor->dev;
	dev->name = "DUMMY Three-Axis Digital Accelerometer";
	dev->use_irq = false;
	dev->type = CAVAN_INPUT_DEVICE_TYPE_ACCELEROMETER;
	dev->min_delay = 100;
	dev->poll_delay = 200;
	dev->set_enable = dummy_acceleration_set_enable;
	dev->set_delay = dummy_acceleration_set_delay;
	dev->event_handler = dummy_acceleration_event_handler;

	ret = cavan_input_device_register(chip, dev);
	if (ret < 0) {
		pr_red_info("cavan_input_device_register");
		goto out_kfree_dummy;
	}

	sensor = &dummy->gyroscope;
	sensor->max_range = DUMMY_GYR_RANGE;
	sensor->resolution = DUMMY_RESOLUTION;
	sensor->power_consume = 145;

	dev = &sensor->dev;
	dev->name = "DUMMY Three-Axis Digital Gyroscope";
	dev->use_irq = false;
	dev->type = CAVAN_INPUT_DEVICE_TYPE_GYROSCOPE;
	dev->min_delay = 100;
	dev->poll_delay = 200;
	dev->set_enable = dummy_gyroscope_set_enable;
	dev->set_delay = dummy_gyroscope_set_delay;
	dev->event_handler = dummy_gyroscope_event_handler;

	ret = cavan_input_device_register(chip, dev);
	if (ret < 0) {
		pr_red_info("cavan_input_device_register");
		goto out_unregister_acceleration;
	}

	return 0;

out_unregister_acceleration:
	cavan_input_device_unregister(chip, &dummy->acceleration.dev);
out_kfree_dummy:
	kfree(dummy);
	return ret;
}

static void dummy_input_chip_remove(struct cavan_input_chip *chip)
{
	struct dummy_chip *dummy = cavan_input_chip_get_dev_data(chip);

	pr_pos_info();

	cavan_input_device_unregister(chip, &dummy->gyroscope.dev);
	cavan_input_device_unregister(chip, &dummy->acceleration.dev);
	kfree(dummy);
}

// ================================================================================

static ssize_t cavan_input_read_data_dummy(struct cavan_input_chip *chip, u8 addr, void *buff, size_t size)
{
	pr_pos_info();

	return 0;
}

static ssize_t cavan_input_write_data_dummy(struct cavan_input_chip *chip, u8 addr, const void *buff, size_t size)
{
	pr_pos_info();

	return size;
}

static struct cavan_input_chip cavan_input_dummy_chip = {
	.name = "DUMMY",
	.irq = -1,
	.devmask = 1 << CAVAN_INPUT_DEVICE_TYPE_ACCELEROMETER | 1 << CAVAN_INPUT_DEVICE_TYPE_GYROSCOPE,
	.read_data = cavan_input_read_data_dummy,
	.write_data = cavan_input_write_data_dummy,
	.readid = dummy_sensor_chip_readid,
	.set_active = dummy_sensor_chip_set_active,
	.probe = dummy_input_chip_probe,
	.remove = dummy_input_chip_remove,
};

static int cavan_dummy_misc_open(struct cavan_misc_device *dev)
{
	pr_pos_info();

	return 0;
}

static int cavan_dummy_misc_release(struct cavan_misc_device *dev)
{
	pr_pos_info();

	return 0;
}

static ssize_t cavan_dummy_misc_read(struct cavan_misc_device *dev, char __user *buff, size_t size, loff_t *offset)
{
	pr_pos_info();

	return 0;
}

static ssize_t cavan_dummy_misc_write(struct cavan_misc_device *dev, const char __user *buff, size_t size, loff_t *offset)
{
	pr_pos_info();

	return size;
}

static int cavan_dummy_misc_ioctl(struct cavan_misc_device *dev, unsigned int command, unsigned long args)
{
	pr_pos_info();

	return 0;
}

struct cavan_misc_device cavan_input_dummy_misc_dev = {
	.open = cavan_dummy_misc_open,
	.release = cavan_dummy_misc_release,
	.read = cavan_dummy_misc_read,
	.write = cavan_dummy_misc_write,
	.ioctl = cavan_dummy_misc_ioctl,
};

static int __init cavan_dummy_init(void)
{
	int ret;

	pr_pos_info();

	ret = cavan_misc_device_register(&cavan_input_dummy_misc_dev, "cavan-dummy-sensor");
	if (ret < 0) {
		pr_red_info("cavan_misc_device_register: %d", ret);
		return ret;
	}

	ret = cavan_input_chip_register(&cavan_input_dummy_chip, cavan_input_dummy_misc_dev.dev);
	if (ret < 0) {
		pr_red_info("cavan_input_chip_register: %d", ret);
		goto out_cavan_misc_device_unregister;
	}

	return 0;

out_cavan_misc_device_unregister:
	cavan_misc_device_unregister(&cavan_input_dummy_misc_dev);
	return ret;
}

static void __exit cavan_dummy_exit(void)
{
	pr_pos_info();

	cavan_misc_device_unregister(&cavan_input_dummy_misc_dev);
	cavan_input_chip_unregister(&cavan_input_dummy_chip);
}

late_initcall(cavan_dummy_init);
module_exit(cavan_dummy_exit);

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Cavan Dummy Sensor Driver");
MODULE_LICENSE("GPL");
