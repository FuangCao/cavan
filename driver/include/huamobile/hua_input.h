#pragma once

#include <asm/uaccess.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/wakelock.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/version.h>
#include <huamobile/hua_thread.h>
#include <huamobile/hua_firmware.h>

#define KB(size)	((size) << 10)
#define MB(size)	((size) << 20)
#define GB(size)	((size) << 30)

#define BUILD_WORD(hb, lb) \
	((short)((hb) << 8 | (lb)))

#define BUILD_DWORD(hw, lw) \
	((int)((hw) << 16 | (lw)))

#define BYTE_IS_LF(b) \
	((b) == '\n' || (b) == '\r')

#define BYTE_IS_SPACE(b) \
	((b) == ' ' || (b) == '\t' || BYTE_IS_LF(b))

#define BYTE_IS_NUMBER(b) \
	((b) >= '0' && (b) <= '9')

#define BYTE_IS_NUMBER16(b) \
	(BYTE_IS_NUMBER(b) || ((b) >= 'a' && (b) <= 'f') || ((b) >= 'A' && (b) <= 'F'))

#define HUA_INPUT_IOC_LENGTH_TO_MASK(len) \
	((1 << (len)) - 1)

#define HUA_INPUT_IOC_GET_VALUE(cmd, shift, mask) \
	(((cmd) >> (shift)) & (mask))

#define HUA_INPUT_IOC_TYPE_LEN		8
#define HUA_INPUT_IOC_NR_LEN		8
#define HUA_INPUT_IOC_SIZE_LEN		16

#define HUA_INPUT_IOC_TYPE_MASK		HUA_INPUT_IOC_LENGTH_TO_MASK(HUA_INPUT_IOC_TYPE_LEN)
#define HUA_INPUT_IOC_NR_MASK		HUA_INPUT_IOC_LENGTH_TO_MASK(HUA_INPUT_IOC_NR_LEN)
#define HUA_INPUT_IOC_SIZE_MASK		HUA_INPUT_IOC_LENGTH_TO_MASK(HUA_INPUT_IOC_SIZE_LEN)

#define HUA_INPUT_IOC_TYPE_SHIFT	0
#define HUA_INPUT_IOC_NR_SHIFT		(HUA_INPUT_IOC_TYPE_SHIFT + HUA_INPUT_IOC_TYPE_LEN)
#define HUA_INPUT_IOC_SIZE_SHIFT	(HUA_INPUT_IOC_NR_SHIFT + HUA_INPUT_IOC_NR_LEN)

#define HUA_INPUT_IOC_GET_TYPE(cmd) \
	HUA_INPUT_IOC_GET_VALUE(cmd, HUA_INPUT_IOC_TYPE_SHIFT, HUA_INPUT_IOC_TYPE_MASK)

#define HUA_INPUT_IOC_GET_NR(cmd) \
	HUA_INPUT_IOC_GET_VALUE(cmd, HUA_INPUT_IOC_NR_SHIFT, HUA_INPUT_IOC_NR_MASK)

#define HUA_INPUT_IOC_GET_SIZE(cmd) \
	HUA_INPUT_IOC_GET_VALUE(cmd, HUA_INPUT_IOC_SIZE_SHIFT, HUA_INPUT_IOC_SIZE_MASK)

#define HUA_INPUT_IOC_GET_CMD_RAW(cmd) \
	((cmd) & HUA_INPUT_IOC_LENGTH_TO_MASK(HUA_INPUT_IOC_TYPE_LEN + HUA_INPUT_IOC_NR_LEN))

#define HUA_INPUT_IOC(type, nr, size) \
	(((type) & HUA_INPUT_IOC_TYPE_MASK) << HUA_INPUT_IOC_TYPE_SHIFT | \
	((nr) & HUA_INPUT_IOC_NR_MASK) << HUA_INPUT_IOC_NR_SHIFT | \
	((size) & HUA_INPUT_IOC_SIZE_MASK) << HUA_INPUT_IOC_SIZE_SHIFT)

#define HUA_INPUT_CHIP_IOC_GET_NAME(len)	HUA_INPUT_IOC('I', 0x00, len)
#define HUA_INPUT_CHIP_IOC_GET_VENDOR(len)	HUA_INPUT_IOC('I', 0x01, len)

#define HUA_INPUT_DEVICE_IOC_GET_TYPE		HUA_INPUT_IOC('I', 0x10, 0)
#define HUA_INPUT_DEVICE_IOC_GET_NAME(len)	HUA_INPUT_IOC('I', 0x11, len)
#define HUA_INPUT_DEVICE_IOC_SET_DELAY		HUA_INPUT_IOC('I', 0x12, 0)
#define HUA_INPUT_DEVICE_IOC_SET_ENABLE		HUA_INPUT_IOC('I', 0x13, 0)

#define pr_pos_info() \
	pr_info("%s => %s[%d]\n", __FILE__, __FUNCTION__, __LINE__)

#define pr_func_info(fmt, args ...) \
	pr_info("%s[%d]: " fmt "\n", __FUNCTION__, __LINE__, ##args)

#define pr_color_info(color, fmt, args ...) \
	pr_info("\033[" color "m%s[%d]: " fmt "\033[0m\n", __FUNCTION__, __LINE__, ##args)

#define pr_red_info(fmt, args ...) \
	pr_color_info("31", fmt, ##args)

#define pr_green_info(fmt, args ...) \
	pr_color_info("32", fmt, ##args)

#define pr_bold_info(fmt, args ...) \
	pr_color_info("1", fmt, ##args)

struct hua_input_core;
struct hua_input_chip;
struct hua_input_device;

enum hua_input_device_type
{
	HUA_INPUT_DEVICE_TYPE_NONE,
	HUA_INPUT_DEVICE_TYPE_TOUCHSCREEN,
	HUA_INPUT_DEVICE_TYPE_ACCELEROMETER,
	HUA_INPUT_DEVICE_TYPE_MAGNETIC_FIELD,
	HUA_INPUT_DEVICE_TYPE_ORIENTATION,
	HUA_INPUT_DEVICE_TYPE_GYROSCOPE,
	HUA_INPUT_DEVICE_TYPE_LIGHT,
	HUA_INPUT_DEVICE_TYPE_PRESSURE,
	HUA_INPUT_DEVICE_TYPE_TEMPERATURE,
	HUA_INPUT_DEVICE_TYPE_PROXIMITY,
	HUA_INPUT_DEVICE_TYPE_GRAVITY,
	HUA_INPUT_DEVICE_TYPE_LINEAR_ACCELERATION,
	HUA_INPUT_DEVICE_TYPE_ROTATION_VECTOR
};

enum hua_input_irq_type
{
	HUA_INPUT_IRQ_TYPE_NONE,
	HUA_INPUT_IRQ_TYPE_EDGE,
	HUA_INPUT_IRQ_TYPE_LEVEL
};

struct hua_input_list
{
	struct mutex lock;
	struct list_head head;
};

struct hua_input_init_data
{
	u8 addr;
	u8 value;
	u32 delay;
};

struct hua_misc_device
{
	void *private_data;
	struct miscdevice dev;
	struct file_operations fops;

	int (*open)(struct hua_misc_device *dev);
	int (*release)(struct hua_misc_device *dev);
	ssize_t (*read)(struct hua_misc_device *dev, char __user *buff, size_t size, loff_t *offset);
	ssize_t (*write)(struct hua_misc_device *dev, const char __user *buff, size_t size, loff_t *offset);
	int (*ioctl)(struct hua_misc_device *dev, unsigned int command, unsigned long args);
};

struct hua_input_device
{
	enum hua_input_device_type type;
	const char *name;
	const char *misc_name;

	int fuzz, flat;
	bool use_irq;
	u32 poll_delay;

	bool enabled;

	struct mutex lock;
	struct input_dev *input;
	struct hua_input_chip *chip;

	void *private_data;
	struct hua_misc_device misc_dev;
	struct work_struct resume_work;

	struct list_head node;

	int (*event_handler)(struct hua_input_chip *chip, struct hua_input_device *dev);
	int (*set_enable)(struct hua_input_device *dev, bool enable);
	int (*set_delay)(struct hua_input_device *dev, unsigned int delay);

	void (*remove)(struct hua_input_device *dev);
	int (*ioctl)(struct hua_input_device *dev, unsigned int command, unsigned long args);
};

struct hua_input_chip
{
	const char *vendor;
	const char *name;
	const char *misc_name;
	u32 devid;
	u32 devmask;

	int irq;
	u32 irq_state;
	unsigned long irq_flags;
	enum hua_input_irq_type irq_type;
	struct completion event_completion;

	long poll_jiffies;
	bool poweron_init;
	size_t init_data_size;
	const struct hua_input_init_data *init_data;

	void *bus_data;
	void *dev_data;
	void *misc_data;

	bool powered;
	bool actived;
	int recovery_count;

	struct mutex lock;
	struct wake_lock wake_lock;
	struct hua_input_core *core;

	struct kobject kobj;
	struct hua_misc_device misc_dev;

	struct hua_input_thread isr_thread;
	struct hua_input_thread poll_thread;

	struct hua_input_list dev_list;
	struct hua_input_list isr_list;
	struct hua_input_list poll_list;

	struct list_head node;

	int (*readid)(struct hua_input_chip *chip);
	int (*set_power)(struct hua_input_chip *chip, bool enable);
	int (*set_active)(struct hua_input_chip *chip, bool enable);
	int (*probe)(struct hua_input_chip *chip);
	void (*remove)(struct hua_input_chip *chip);
	void (*event_handler)(struct hua_input_chip *chip);

	ssize_t (*read_data)(struct hua_input_chip *chip, u8 addr, void *buff, size_t size);
	ssize_t (*write_data)(struct hua_input_chip *chip, u8 addr, const void *buff, size_t size);

	ssize_t (*master_recv)(struct hua_input_chip *chip, void *buff, size_t size);
	ssize_t (*master_send)(struct hua_input_chip *chip, const void *buff, size_t size);

	int (*read_register)(struct hua_input_chip *chip, u8 addr, u8 *value);
	int (*write_register)(struct hua_input_chip *chip, u8 addr, u8 value);

	int (*firmware_upgrade)(struct hua_input_chip *chip, struct hua_firmware *fw);
	int (*calibration)(struct hua_input_chip *chip, const void *buff, size_t size);
};

struct hua_input_core
{
	const char *name;
	u32 devmask;
	u32 poll_jiffies;

	struct mutex lock;
	struct hua_input_thread detect_thread;
	struct kobject prop_kobj;
	struct workqueue_struct *resume_wq;

	struct hua_input_list chip_list;
	struct hua_input_list work_list;
	struct hua_input_list exclude_list;
};

char *hua_input_print_memory(const void *mem, size_t size);

int hua_input_copy_to_user_text(unsigned int command, unsigned long args, const char *text);
int hua_input_copy_to_user_uint(unsigned long args, unsigned int value);

int hua_input_chip_set_power(struct hua_input_chip *chip, bool enable);
int hua_input_chip_set_power_lock(struct hua_input_chip *chip, bool enable);
int hua_input_chip_set_active(struct hua_input_chip *chip, bool enable);
int hua_input_chip_set_active_lock(struct hua_input_chip *chip, bool enable);
void hua_input_chip_recovery(struct hua_input_chip *chip, bool force);
int hua_input_chip_firmware_upgrade(struct hua_input_chip *chip, void *buff, size_t size, int flags);
int hua_input_device_set_enable_lock(struct hua_input_device *dev, bool enable);
int hua_input_device_set_enable_no_sync(struct hua_input_device *dev, bool enable);

int hua_input_device_register(struct hua_input_chip *chip, struct hua_input_device *dev);
void hua_input_device_unregister(struct hua_input_chip *chip, struct hua_input_device *dev);

int hua_input_chip_register(struct hua_input_chip *chip);
void hua_input_chip_unregister(struct hua_input_chip *chip);
void hua_input_chip_report_events(struct hua_input_chip *chip, struct hua_input_list *list);

static inline void hua_input_chip_set_bus_data(struct hua_input_chip *chip, void *data)
{
	chip->bus_data = data;
}

static inline void *hua_input_chip_get_bus_data(struct hua_input_chip *chip)
{
	return chip->bus_data;
}

static inline void hua_input_chip_set_dev_data(struct hua_input_chip *chip, void *data)
{
	chip->dev_data = data;
}

static inline void *hua_input_chip_get_dev_data(struct hua_input_chip *chip)
{
	return chip->dev_data;
}

static inline void hua_input_chip_set_misc_data(struct hua_input_chip *chip, void *data)
{
	chip->misc_data = data;
}

static inline void *hua_input_chip_get_misc_data(struct hua_input_chip *chip)
{
	return chip->misc_data;
}

static inline void hua_input_device_set_data(struct hua_input_device *dev, void *data)
{
	dev->private_data = data;
}

static inline void *hua_input_device_get_data(struct hua_input_device *dev)
{
	return dev->private_data;
}

static inline int hua_input_read_register_dummy(struct hua_input_chip *chip, u8 addr, u8 *value)
{
	return chip->read_data(chip, addr, value, 1);
}

static inline int hua_input_write_register_dummy(struct hua_input_chip *chip, u8 addr, u8 value)
{
	return chip->write_data(chip, addr, &value, 1);
}

static inline struct hua_misc_device *hua_input_file_to_misc_device(struct file *file)
{
	return container_of(file->f_op, struct hua_misc_device, fops);
}

static inline bool hua_input_thread_running(struct hua_input_thread *thread)
{
	bool res;

	mutex_lock(&thread->lock);
	res = thread->state == HUA_INPUT_THREAD_STATE_RUNNING;
	mutex_unlock(&thread->lock);

	return res;
}

static inline bool hua_input_thread_wait_for_event(struct hua_input_thread *thread)
{
	thread->wait_for_event(thread->private_data);

	return hua_input_thread_running(thread);
}

static inline void hua_input_thread_set_data(struct hua_input_thread *thread, void *data)
{
	thread->private_data = data;
}

static inline void *hua_input_thread_get_data(struct hua_input_thread *thread)
{
	return thread->private_data;
}

static inline void hua_misc_device_set_data(struct hua_misc_device *dev, void *data)
{
	dev->private_data = data;
}

static inline void *hua_misc_device_get_data(struct hua_misc_device *dev)
{
	return dev->private_data;
}
