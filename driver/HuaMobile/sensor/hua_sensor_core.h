#pragma once

#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/earlysuspend.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/wakelock.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/init.h>
#include <linux/delay.h>

#define HUA_SENSOR_ENABLE_ALL		1

#define HUA_SENSOR_IOC_LENGTH_TO_MASK(len) \
	((1 << (len)) - 1)

#define HUA_SENSOR_IOC_GET_VALUE(cmd, shift, mask) \
	(((cmd) >> (shift)) & (mask))

#define HUA_SENSOR_IOC_TYPE_LEN		8
#define HUA_SENSOR_IOC_NR_LEN		8
#define HUA_SENSOR_IOC_INDEX_LEN	4
#define HUA_SENSOR_IOC_SIZE_LEN		12

#define HUA_SENSOR_IOC_TYPE_MASK	HUA_SENSOR_IOC_LENGTH_TO_MASK(HUA_SENSOR_IOC_TYPE_LEN)
#define HUA_SENSOR_IOC_NR_MASK		HUA_SENSOR_IOC_LENGTH_TO_MASK(HUA_SENSOR_IOC_NR_LEN)
#define HUA_SENSOR_IOC_INDEX_MASK	HUA_SENSOR_IOC_LENGTH_TO_MASK(HUA_SENSOR_IOC_INDEX_LEN)
#define HUA_SENSOR_IOC_SIZE_MASK	HUA_SENSOR_IOC_LENGTH_TO_MASK(HUA_SENSOR_IOC_SIZE_LEN)

#define HUA_SENSOR_IOC_TYPE_SHIFT	0
#define HUA_SENSOR_IOC_NR_SHIFT		(HUA_SENSOR_IOC_TYPE_SHIFT + HUA_SENSOR_IOC_TYPE_LEN)
#define HUA_SENSOR_IOC_INDEX_SHIFT	(HUA_SENSOR_IOC_NR_SHIFT + HUA_SENSOR_IOC_NR_LEN)
#define HUA_SENSOR_IOC_SIZE_SHIFT	(HUA_SENSOR_IOC_INDEX_SHIFT + HUA_SENSOR_IOC_INDEX_LEN)

#define HUA_SENSOR_IOC_GET_TYPE(cmd) \
	HUA_SENSOR_IOC_GET_VALUE(cmd, HUA_SENSOR_IOC_TYPE_SHIFT, HUA_SENSOR_IOC_TYPE_MASK)

#define HUA_SENSOR_IOC_GET_NR(cmd) \
	HUA_SENSOR_IOC_GET_VALUE(cmd, HUA_SENSOR_IOC_NR_SHIFT, HUA_SENSOR_IOC_NR_MASK)

#define HUA_SENSOR_IOC_GET_INDEX(cmd) \
	HUA_SENSOR_IOC_GET_VALUE(cmd, HUA_SENSOR_IOC_INDEX_SHIFT, HUA_SENSOR_IOC_INDEX_MASK)

#define HUA_SENSOR_IOC_GET_SIZE(cmd) \
	HUA_SENSOR_IOC_GET_VALUE(cmd, HUA_SENSOR_IOC_SIZE_SHIFT, HUA_SENSOR_IOC_SIZE_MASK)

#define HUA_SENSOR_IOC_GET_CMD_RAW(cmd) \
	((cmd) & HUA_SENSOR_IOC_LENGTH_TO_MASK(HUA_SENSOR_IOC_TYPE_LEN + HUA_SENSOR_IOC_NR_LEN))

#define HUA_SENSOR_IOC(type, nr, index, size) \
	(((type) & HUA_SENSOR_IOC_TYPE_MASK) << HUA_SENSOR_IOC_TYPE_SHIFT | \
	((nr) & HUA_SENSOR_IOC_NR_MASK) << HUA_SENSOR_IOC_NR_SHIFT | \
	((index) & HUA_SENSOR_IOC_INDEX_MASK) << HUA_SENSOR_IOC_INDEX_SHIFT | \
	((size) & HUA_SENSOR_IOC_SIZE_MASK) << HUA_SENSOR_IOC_SIZE_SHIFT)

#define HUA_SENSOR_IOC_SET_DETECT					HUA_SENSOR_IOC('H', 0x00, 0, 0)
#define HUA_SENSOR_IOC_GET_CHIP_NAME(len)			HUA_SENSOR_IOC('H', 0x01, 0, len)
#define HUA_SENSOR_IOC_GET_CHIP_VENDOR(len)			HUA_SENSOR_IOC('H', 0x02, 0, len)
#define HUA_SENSOR_IOC_GET_SENSOR_COUNT				HUA_SENSOR_IOC('H', 0x03, 0, 0)
#define HUA_SENSOR_IOC_GET_MIN_DELAY				HUA_SENSOR_IOC('H', 0x04, 0, 0)
#define HUA_SENSOR_IOC_GET_SENSOR_TYPE(index)		HUA_SENSOR_IOC('H', 0x05, index, 0)
#define HUA_SENSOR_IOC_GET_SENSOR_NAME(index, len)	HUA_SENSOR_IOC('H', 0x06, index, len)
#define HUA_SENSOR_IOC_GET_MAX_RANGE(index)			HUA_SENSOR_IOC('H', 0x07, index, 0)
#define HUA_SENSOR_IOC_GET_RESOLUTION(index)		HUA_SENSOR_IOC('H', 0x08, index, 0)
#define HUA_SENSOR_IOC_GET_POWER_CONSUME(index)		HUA_SENSOR_IOC('H', 0x09, index, 0)
#define HUA_SENSOR_IOC_SET_DELAY(index)				HUA_SENSOR_IOC('H', 0x0A, index, 0)
#define HUA_SENSOR_IOC_SET_ENABLE(index)			HUA_SENSOR_IOC('H', 0x0B, index, 0)

#define pr_pos_info() \
	printk(KERN_INFO "%s => %s[%d]\n", __FILE__, __FUNCTION__, __LINE__)

#define pr_func_info(fmt, args ...) \
	printk(KERN_INFO "%s[%d]: " fmt "\n", __FUNCTION__, __LINE__, ##args)

#define pr_color_info(color, fmt, args ...) \
	printk(KERN_INFO "\033[" color "m%s[%d]: " fmt "\033[0m\n", __FUNCTION__, __LINE__, ##args)

#define pr_red_info(fmt, args ...) \
	pr_color_info("31", fmt, ##args)

#define pr_green_info(fmt, args ...) \
	pr_color_info("32", fmt, ##args)

#define pr_bold_info(fmt, args ...) \
	pr_color_info("1", fmt, ##args)

struct hua_sensor_chip;
struct hua_sensor_core;

enum hua_sensor_type
{
	HUA_SENSOR_TYPE_NONE,
	HUA_SENSOR_TYPE_ACCELEROMETER,
	HUA_SENSOR_TYPE_MAGNETIC_FIELD,
	HUA_SENSOR_TYPE_ORIENTATION,
	HUA_SENSOR_TYPE_GYROSCOPE,
	HUA_SENSOR_TYPE_LIGHT,
	HUA_SENSOR_TYPE_PRESSURE,
	HUA_SENSOR_TYPE_TEMPERATURE,
	HUA_SENSOR_TYPE_PROXIMITY,
	HUA_SENSOR_TYPE_GRAVITY,
	HUA_SENSOR_TYPE_LINEAR_ACCELERATION,
	HUA_SENSOR_TYPE_ROTATION_VECTOR
};

enum hua_sensor_thread_state
{
	HUA_SENSOR_THREAD_STATE_RUNNING,
	HUA_SENSOR_THREAD_STATE_SUSPEND,
	HUA_SENSOR_THREAD_STATE_STOPPING,
	HUA_SENSOR_THREAD_STATE_STOPPED
};

struct hua_sensor_init_data
{
	u8 addr;
	u8 value;
	u32 delay;
};

struct hua_sensor_device
{
	const char *name;
	enum hua_sensor_type type;

	int fuzz, flat;
	unsigned int poll_delay;
	unsigned int max_range;
	unsigned int resolution;
	unsigned int power_consume;

	void *private_data;
	struct hua_sensor_chip *chip;

	bool enabled;

	struct mutex lock;

	int (*set_enable)(struct hua_sensor_device *sensor, bool enable);
	bool (*event_handler)(struct hua_sensor_device *sensor, struct hua_sensor_chip *chip, u32 mask);
	void (*report_event)(struct input_dev *input, int value);
	void (*report_vector_event)(struct input_dev *input, int x, int y, int z);
};

struct hua_sensor_chip
{
	const char *name;
	const char *vendor;

	unsigned int min_delay;
	unsigned int poll_delay;

	int irq;
	unsigned long irq_flags;

	struct hua_sensor_device *sensor_list;
	size_t sensor_count;

	const struct hua_sensor_init_data *init_data;
	size_t init_data_size;
	bool power_on_init;

	void *private_data;
	u32 devid;
	enum hua_sensor_thread_state state;

	bool powered;
	bool enabled;
	bool irq_ctrl;
	size_t use_count;

	struct mutex lock;
	struct input_dev *input;
	struct hua_sensor_core *core;
	struct completion event_completion;

	char misc_name[32];
	struct file_operations misc_fops;
	struct miscdevice misc_dev;

	struct task_struct *event_task;

	struct hua_sensor_chip *next;
	struct hua_sensor_chip *prev;

	int (*readid)(struct hua_sensor_chip *chip);
	void (*main_loop)(struct hua_sensor_chip *chip);
	int (*set_delay)(struct hua_sensor_chip *chip, unsigned int delay);
	int (*set_power)(struct hua_sensor_chip *chip, bool enable);
	int (*probe)(struct hua_sensor_chip *chip);
	void (*remove)(struct hua_sensor_chip *chip);
	int (*ioctl)(struct hua_sensor_chip *chip, unsigned int command, unsigned long arg);
	ssize_t (*write_data)(struct hua_sensor_chip *chip, u8 addr, const void *buff, size_t size);
	ssize_t (*read_data)(struct hua_sensor_chip *chip, u8 addr, void *buff, size_t size);
	int (*write_register)(struct hua_sensor_chip *chip, u8 addr, u8 value);
	int (*read_register)(struct hua_sensor_chip *chip, u8 addr, u8 *value);
};

struct hua_sensor_core
{
	const char *name;

	struct mutex lock;

	u32 detect_delay;
	struct task_struct *detect_task;

	struct file_operations misc_fops;
	struct miscdevice misc_dev;

	struct hua_sensor_chip *chip_head;
};

ssize_t hua_i2c_read_data(struct i2c_client *client, u8 addr, void *buff, size_t size);
ssize_t hua_i2c_write_data(struct i2c_client *client, u8 addr, const void *buff, size_t size);

int hua_sensor_read_register_i2c_smbus(struct hua_sensor_chip *chip, u8 addr, u8 *value);
int hua_sensor_write_register_i2c_smbus(struct hua_sensor_chip *chip, u8 addr, u8 value);

void hua_sensor_chip_report_event(struct hua_sensor_chip *chip, u32 mask);
int hua_sensor_register_chip(struct hua_sensor_chip *chip);
void hua_sensor_unregister_chip(struct hua_sensor_chip *chip);

static inline ssize_t hua_sensor_read_data_i2c(struct hua_sensor_chip *chip, u8 addr, void *buff, size_t size)
{
	return hua_i2c_read_data(chip->private_data, addr, buff, size);
}

static inline ssize_t hua_sensor_write_data_i2c(struct hua_sensor_chip *chip, u8 addr, const void *buff, size_t size)
{
	return hua_i2c_write_data(chip->private_data, addr, buff, size);
}

static inline ssize_t hua_sensor_read_register_dummy(struct hua_sensor_chip *chip, u8 addr, u8 *value)
{
	return chip->read_data(chip, addr, value, 1);
}

static inline ssize_t hua_sensor_write_register_dummy(struct hua_sensor_chip *chip, u8 addr, u8 value)
{
	return chip->write_data(chip, addr, &value, 1);
}

static inline void hua_sensor_device_set_data(struct hua_sensor_device *sensor, void *data)
{
	sensor->private_data = data;
}

static inline void *hua_sensor_device_get_data(struct hua_sensor_device *sensor)
{
	return sensor->private_data;
}

static inline void hua_sensor_chip_set_data(struct hua_sensor_chip *chip, void *data)
{
	chip->private_data = data;
}

static inline void *hua_sensor_chip_get_data(struct hua_sensor_chip *chip)
{
	return chip->private_data;
}
