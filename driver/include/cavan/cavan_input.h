#pragma once

#include <asm/uaccess.h>
#include <linux/device.h>
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

#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 8, 0)
#include <linux/sched/rt.h>
#endif

#ifdef CONFIG_ARCH_SC8810
#include <mach/eic.h>
#endif

#include <cavan/cavan_thread.h>
#include <cavan/cavan_firmware.h>

#define CAVAN_INPUT_DEBUG					1

#define CAVAN_INPUT_MAJOR					280
#define CAVAN_INPUT_MINORS				32
#define CAVAN_INPUT_CLASS_NAME			"cavan_input"

#ifdef CONFIG_ARCH_MSM
#define CAVAN_INPUT_CAL_DATA_DIR			"/persist"
#elif defined(CONFIG_ARCH_SC8810)
#define CAVAN_INPUT_CAL_DATA_DIR			"/productinfo"
#else
#define CAVAN_INPUT_CAL_DATA_DIR			"/NVM"
#endif

#define CAVAN_INPUT_ONLINE_DATA_DIR		"/data"

#define CAVAN_INPUT_CHIP_MAX_PROBE_COUNT	20

#define KB(size)	((size) << 10)
#define MB(size)	((size) << 20)
#define GB(size)	((size) << 30)

#define CAVAN_INPUT_CHIP_FLAG_NO_WAIT			(1 << 0)
#define CAVAN_INPUT_CHIP_FLAG_POWERON_INIT	(1 << 1)

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

#define CAVAN_INPUT_IOC_LENGTH_TO_MASK(len) \
	((1 << (len)) - 1)

#define CAVAN_INPUT_IOC_GET_VALUE(cmd, shift, mask) \
	(((cmd) >> (shift)) & (mask))

#define CAVAN_INPUT_IOC_TYPE_LEN		8
#define CAVAN_INPUT_IOC_NR_LEN		8
#define CAVAN_INPUT_IOC_SIZE_LEN		16

#define CAVAN_INPUT_IOC_TYPE_MASK		CAVAN_INPUT_IOC_LENGTH_TO_MASK(CAVAN_INPUT_IOC_TYPE_LEN)
#define CAVAN_INPUT_IOC_NR_MASK		CAVAN_INPUT_IOC_LENGTH_TO_MASK(CAVAN_INPUT_IOC_NR_LEN)
#define CAVAN_INPUT_IOC_SIZE_MASK		CAVAN_INPUT_IOC_LENGTH_TO_MASK(CAVAN_INPUT_IOC_SIZE_LEN)

#define CAVAN_INPUT_IOC_TYPE_SHIFT	0
#define CAVAN_INPUT_IOC_NR_SHIFT		(CAVAN_INPUT_IOC_TYPE_SHIFT + CAVAN_INPUT_IOC_TYPE_LEN)
#define CAVAN_INPUT_IOC_SIZE_SHIFT	(CAVAN_INPUT_IOC_NR_SHIFT + CAVAN_INPUT_IOC_NR_LEN)

#define CAVAN_INPUT_IOC_GET_TYPE(cmd) \
	CAVAN_INPUT_IOC_GET_VALUE(cmd, CAVAN_INPUT_IOC_TYPE_SHIFT, CAVAN_INPUT_IOC_TYPE_MASK)

#define CAVAN_INPUT_IOC_GET_NR(cmd) \
	CAVAN_INPUT_IOC_GET_VALUE(cmd, CAVAN_INPUT_IOC_NR_SHIFT, CAVAN_INPUT_IOC_NR_MASK)

#define CAVAN_INPUT_IOC_GET_SIZE(cmd) \
	CAVAN_INPUT_IOC_GET_VALUE(cmd, CAVAN_INPUT_IOC_SIZE_SHIFT, CAVAN_INPUT_IOC_SIZE_MASK)

#define CAVAN_INPUT_IOC_GET_CMD_RAW(cmd) \
	((cmd) & CAVAN_INPUT_IOC_LENGTH_TO_MASK(CAVAN_INPUT_IOC_TYPE_LEN + CAVAN_INPUT_IOC_NR_LEN))

#define CAVAN_INPUT_IOC(type, nr, size) \
	(((type) & CAVAN_INPUT_IOC_TYPE_MASK) << CAVAN_INPUT_IOC_TYPE_SHIFT | \
	((nr) & CAVAN_INPUT_IOC_NR_MASK) << CAVAN_INPUT_IOC_NR_SHIFT | \
	((size) & CAVAN_INPUT_IOC_SIZE_MASK) << CAVAN_INPUT_IOC_SIZE_SHIFT)

#define CAVAN_INPUT_CHIP_IOC_GET_NAME(len)		CAVAN_INPUT_IOC('I', 0x00, len)
#define CAVAN_INPUT_CHIP_IOC_GET_VENDOR(len)		CAVAN_INPUT_IOC('I', 0x01, len)

#define CAVAN_INPUT_DEVICE_IOC_GET_TYPE			CAVAN_INPUT_IOC('I', 0x10, 0)
#define CAVAN_INPUT_DEVICE_IOC_GET_NAME(len)		CAVAN_INPUT_IOC('I', 0x11, len)
#define CAVAN_INPUT_DEVICE_IOC_SET_DELAY			CAVAN_INPUT_IOC('I', 0x12, 0)
#define CAVAN_INPUT_DEVICE_IOC_SET_ENABLE			CAVAN_INPUT_IOC('I', 0x13, 0)
#define CAVAN_INPUT_DEVICE_IOC_GET_OFFSET(len)	CAVAN_INPUT_IOC('I', 0x14, len)
#define CAVAN_INPUT_DEVICE_IOC_SET_OFFSET(len)	CAVAN_INPUT_IOC('I', 0x15, len)

#define pr_color_info(color, fmt, args ...) \
	pr_info("\033[" color "m%s[%d]: " fmt "\033[0m\n", __FUNCTION__, __LINE__, ##args)

#define pr_red_info(fmt, args ...) \
	pr_color_info("31", fmt, ##args)

#define pr_pos_info() \
	do { \
		if (cavan_input_debug_enable) \
			pr_info("%s => %s[%d]\n", __FILE__, __FUNCTION__, __LINE__); \
	} while (0)

#define pr_func_info(fmt, args ...) \
	do { \
		if (cavan_input_debug_enable) \
			pr_info("%s[%d]: " fmt "\n", __FUNCTION__, __LINE__, ##args); \
	} while (0)

#define pr_green_info(fmt, args ...) \
	do { \
		if (cavan_input_debug_enable) \
			pr_color_info("32", fmt, ##args); \
	} while (0)

#define pr_bold_info(fmt, args ...) \
	do { \
		if (cavan_input_debug_enable) \
			pr_color_info("1", fmt, ##args); \
	} while (0)

struct cavan_input_core;
struct cavan_input_chip;
struct cavan_input_device;

enum cavan_input_device_type
{
	CAVAN_INPUT_DEVICE_TYPE_NONE,
	CAVAN_INPUT_DEVICE_TYPE_TOUCHSCREEN,
	CAVAN_INPUT_DEVICE_TYPE_ACCELEROMETER,
	CAVAN_INPUT_DEVICE_TYPE_MAGNETIC_FIELD,
	CAVAN_INPUT_DEVICE_TYPE_ORIENTATION,
	CAVAN_INPUT_DEVICE_TYPE_GYROSCOPE,
	CAVAN_INPUT_DEVICE_TYPE_LIGHT,
	CAVAN_INPUT_DEVICE_TYPE_PRESSURE,
	CAVAN_INPUT_DEVICE_TYPE_TEMPERATURE,
	CAVAN_INPUT_DEVICE_TYPE_PROXIMITY,
	CAVAN_INPUT_DEVICE_TYPE_GRAVITY,
	CAVAN_INPUT_DEVICE_TYPE_LINEAR_ACCELERATION,
	CAVAN_INPUT_DEVICE_TYPE_ROTATION_VECTOR
};

enum cavan_input_irq_type
{
	CAVAN_INPUT_IRQ_TYPE_NONE,
	CAVAN_INPUT_IRQ_TYPE_EDGE,
	CAVAN_INPUT_IRQ_TYPE_LEVEL
};

struct cavan_input_list
{
	struct mutex lock;
	struct list_head head;
};

struct cavan_input_init_data
{
	u8 addr;
	u8 value;
	u32 delay;
};

struct cavan_misc_device
{
	void *private_data;

	int minor;
	struct device *dev;

	int (*open)(struct cavan_misc_device *dev);
	int (*release)(struct cavan_misc_device *dev);
	ssize_t (*read)(struct cavan_misc_device *dev, char __user *buff, size_t size, loff_t *offset);
	ssize_t (*write)(struct cavan_misc_device *dev, const char __user *buff, size_t size, loff_t *offset);
	int (*ioctl)(struct cavan_misc_device *dev, unsigned int command, unsigned long args);
};

struct cavan_input_device
{
	enum cavan_input_device_type type;
	const char *name;
	const char *misc_name;

	int fuzz, flat;
	bool use_irq;
	u32 min_delay;
	u32 poll_delay;

	bool enabled;

	struct mutex lock;
	struct input_dev *input;
	struct cavan_input_chip *chip;

	void *private_data;
	struct cavan_misc_device misc_dev;
	struct work_struct resume_work;
	struct work_struct set_delay_work;

	struct list_head node;

	int (*event_handler)(struct cavan_input_chip *chip, struct cavan_input_device *dev);
	int (*set_enable)(struct cavan_input_device *dev, bool enable);
	int (*set_delay)(struct cavan_input_device *dev, unsigned int delay);
	ssize_t (*calibration)(struct cavan_input_device *dev, char *buff, size_t size, bool store);

	void (*remove)(struct cavan_input_device *dev);
	int (*ioctl)(struct cavan_input_device *dev, unsigned int command, unsigned long args);
};

struct cavan_input_chip
{
	struct device *dev;
	const char *vendor;
	const char *name;
	const char *misc_name;
	u32 devid;
	u32 devmask;
	unsigned long flags;
	u32 i2c_rate;

	int irq;
	u32 irq_state;
	unsigned long irq_flags;
	enum cavan_input_irq_type irq_type;
	struct completion event_completion;

	long poll_jiffies;
	size_t init_data_size;
	const struct cavan_input_init_data *init_data;

	void *bus_data;
	void *dev_data;
	void *misc_data;

	bool dead;
	bool powered;
	bool actived;
	int probe_count;
	int recovery_count;

	struct mutex lock;
	struct wake_lock wake_lock;
	struct cavan_input_core *core;

	struct kobject kobj;
	struct cavan_misc_device misc_dev;

	struct cavan_input_thread isr_thread;
	struct cavan_input_thread poll_thread;

	struct cavan_input_list dev_list;
	struct cavan_input_list isr_list;
	struct cavan_input_list poll_list;

	struct list_head node;

	int gpio_reset;
	int gpio_irq;
	int gpio_power;
	struct regulator *vdd;
	struct regulator *vio;

	int vdd_vol_min, vdd_vol_max;
	int vio_vol_min, vio_vol_max;

	int (*readid)(struct cavan_input_chip *chip);
	int (*set_power)(struct cavan_input_chip *chip, bool enable);
	int (*set_active)(struct cavan_input_chip *chip, bool enable);
	int (*probe)(struct cavan_input_chip *chip);
	void (*remove)(struct cavan_input_chip *chip);
	int (*event_handler_isr)(struct cavan_input_chip *chip);
	int (*event_handler_poll)(struct cavan_input_chip *chip);

	ssize_t (*read_data)(struct cavan_input_chip *chip, u8 addr, void *buff, size_t size);
	ssize_t (*write_data)(struct cavan_input_chip *chip, u8 addr, const void *buff, size_t size);

	ssize_t (*master_recv)(struct cavan_input_chip *chip, void *buff, size_t size);
	ssize_t (*master_send)(struct cavan_input_chip *chip, const void *buff, size_t size);

	int (*read_register)(struct cavan_input_chip *chip, u8 addr, u8 *value);
	int (*write_register)(struct cavan_input_chip *chip, u8 addr, u8 value);
	int (*read_register16)(struct cavan_input_chip *chip, u8 addr, u16 *value);
	int (*write_register16)(struct cavan_input_chip *chip, u8 addr, u16 value);

	ssize_t (*read_firmware_id)(struct cavan_input_chip *chip, char *buff, size_t size);
	int (*firmware_upgrade)(struct cavan_input_chip *chip, struct cavan_firmware *fw);
};

struct cavan_input_core
{
	const char *name;
	const char *chip_online[CAVAN_INPUT_MINORS];
	u32 devmask;
	u32 poll_jiffies;

	struct mutex lock;
	struct cavan_input_thread detect_thread;
	struct kobject prop_kobj;
	struct delayed_work write_online_work;
	struct workqueue_struct *workqueue;

	struct cavan_input_list chip_list;
	struct cavan_input_list work_list;
	struct cavan_input_list exclude_list;
};

extern int cavan_input_debug_enable;

extern ssize_t cavan_io_read_write_file(const char *pathname, const char *buff, size_t size, bool store);
extern int cavan_io_gpio_set_value(int gpio, int value);
extern int cavan_io_set_power_regulator(struct cavan_input_chip *chip, bool enable);
extern int cavan_input_chip_io_init(struct cavan_input_chip *chip);
extern void cavan_input_chip_io_deinit(struct cavan_input_chip *chip);

char *cavan_input_print_memory(const void *mem, size_t size);
const char *cavan_input_irq_trigger_type_tostring(unsigned long irq_flags);

int cavan_input_copy_to_user_text(unsigned int command, unsigned long args, const char *text);
int cavan_input_copy_to_user_uint(unsigned long args, unsigned int value);

int cavan_input_chip_set_power(struct cavan_input_chip *chip, bool enable);
int cavan_input_chip_set_power_lock(struct cavan_input_chip *chip, bool enable);
int cavan_input_chip_set_active(struct cavan_input_chip *chip, bool enable);
int cavan_input_chip_set_active_lock(struct cavan_input_chip *chip, bool enable);
void cavan_input_chip_recovery(struct cavan_input_chip *chip, bool force);
ssize_t cavan_input_chip_write_online(const char *chip_name, bool online);
ssize_t cavan_input_chip_read_online(const char *chip_name);
int cavan_input_chip_firmware_upgrade(struct cavan_input_chip *chip, void *buff, size_t size, int flags);
int cavan_input_device_set_enable_lock(struct cavan_input_device *dev, bool enable);
int cavan_input_device_set_enable_no_sync(struct cavan_input_device *dev, bool enable);
int cavan_input_device_calibration(struct cavan_input_device *dev, struct cavan_input_chip *chip, char *buff, size_t size, bool write);
int cavan_input_device_calibration_lock(struct cavan_input_device *dev, char *buff, size_t size, bool write);
ssize_t cavan_input_device_read_write_offset(struct cavan_input_device *dev, char *buff, size_t size, bool store);

int cavan_input_device_register(struct cavan_input_chip *chip, struct cavan_input_device *dev);
void cavan_input_device_unregister(struct cavan_input_chip *chip, struct cavan_input_device *dev);

int cavan_input_chip_read_firmware_id(struct cavan_input_chip *chip, char *buff, size_t size);
int cavan_input_chip_read_firmware_id_lock(struct cavan_input_chip *chip, char *buff, size_t size);

int cavan_input_chip_register(struct cavan_input_chip *chip, struct device *dev);
void cavan_input_chip_unregister(struct cavan_input_chip *chip);
int cavan_input_chip_report_events(struct cavan_input_chip *chip, struct cavan_input_list *list);

static inline void cavan_input_chip_set_bus_data(struct cavan_input_chip *chip, void *data)
{
	chip->bus_data = data;
}

static inline void *cavan_input_chip_get_bus_data(struct cavan_input_chip *chip)
{
	return chip->bus_data;
}

static inline void cavan_input_chip_set_dev_data(struct cavan_input_chip *chip, void *data)
{
	chip->dev_data = data;
}

static inline void *cavan_input_chip_get_dev_data(struct cavan_input_chip *chip)
{
	return chip->dev_data;
}

static inline void cavan_input_chip_set_misc_data(struct cavan_input_chip *chip, void *data)
{
	chip->misc_data = data;
}

static inline void *cavan_input_chip_get_misc_data(struct cavan_input_chip *chip)
{
	return chip->misc_data;
}

static inline int cavan_input_chip_get_online_pathname(const char *chip_name, char *pathname, size_t size)
{
	return snprintf(pathname, size, "%s/Chip-%s-Online", CAVAN_INPUT_ONLINE_DATA_DIR, chip_name);
}

static inline void cavan_input_device_set_data(struct cavan_input_device *dev, void *data)
{
	dev->private_data = data;
}

static inline void *cavan_input_device_get_data(struct cavan_input_device *dev)
{
	return dev->private_data;
}

static inline int cavan_input_device_get_offset_pathname(struct cavan_input_device *dev, char *pathname, size_t size)
{
	return snprintf(pathname, size, "%s/%s-Offset", CAVAN_INPUT_CAL_DATA_DIR, dev->misc_name);
}

static inline int cavan_input_read_register_dummy(struct cavan_input_chip *chip, u8 addr, u8 *value)
{
	return chip->read_data(chip, addr, value, 1);
}

static inline int cavan_input_write_register_dummy(struct cavan_input_chip *chip, u8 addr, u8 value)
{
	return chip->write_data(chip, addr, &value, 1);
}

static inline int cavan_input_read_register16_dummy(struct cavan_input_chip *chip, u8 addr, u16 *value)
{
	return chip->read_data(chip, addr, value, 2);
}

static inline int cavan_input_write_register16_dummy(struct cavan_input_chip *chip, u8 addr, u16 value)
{
	return chip->write_data(chip, addr, &value, 2);
}

static inline bool cavan_input_thread_running(struct cavan_input_thread *thread)
{
	bool res;

	mutex_lock(&thread->lock);
	res = thread->state == CAVAN_INPUT_THREAD_STATE_RUNNING;
	mutex_unlock(&thread->lock);

	return res;
}

static inline bool cavan_input_thread_wait_for_event(struct cavan_input_thread *thread)
{
	thread->wait_for_event(thread->private_data);

	return cavan_input_thread_running(thread);
}

static inline void cavan_input_thread_set_data(struct cavan_input_thread *thread, void *data)
{
	thread->private_data = data;
}

static inline void *cavan_input_thread_get_data(struct cavan_input_thread *thread)
{
	return thread->private_data;
}

static inline void cavan_misc_device_set_data(struct cavan_misc_device *dev, void *data)
{
	dev->private_data = data;
}

static inline void *cavan_misc_device_get_data(struct cavan_misc_device *dev)
{
	return dev->private_data;
}
