#pragma once

#include <stdio.h>
#include <poll.h>
#include <sys/types.h>
#include <linux/limits.h>
#include <linux/input.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>

#define test_bit(bit, array) \
	((array)[(bit) >> 3] & (1 << ((bit) & 0x07)))

#define sizeof_bit_array(bits) \
	(((bits) + 7) >> 3)

#define NELEM(a) \
	(sizeof(a) / sizeof((a)[0]))

#define pr_pos_info() \
	printf("%s => %s[%d]\n", __FILE__, __FUNCTION__, __LINE__)

#define pr_red_info(fmt, args ...) \
	printf("\033[31m" fmt "\033[0m\n", ##args)

#define pr_green_info(fmt, args ...) \
	printf("\033[32m" fmt "\033[0m\n", ##args)

#define pr_bold_info(fmt, args ...) \
	printf("\033[1m" fmt "\033[0m\n", ##args)

#define pr_error_info(fmt, args ...) \
	if (errno) { \
		pr_red_info("%s[%d](" fmt "): %s", __FUNCTION__, __LINE__, ##args, strerror(errno)); \
	} else { \
		pr_red_info("%s[%d]:" fmt, __FUNCTION__, __LINE__, ##args); \
	}

#ifndef SYN_MT_REPORT
#define SYN_MT_REPORT		2
#define ABS_MT_POSITION_X	0x35	/* Center X ellipse position */
#define ABS_MT_POSITION_Y	0x36	/* Center Y ellipse position */
#define ABS_MT_TOUCH_MAJOR	0x30	/* Major axis of touching ellipse */
#define ABS_MT_TRACKING_ID	0x39	/* Unique ID of initiated contact */
#endif

enum huamobile_input_command
{
	HUA_INPUT_COMMAND_STOP
};

enum huamobile_input_thread_state
{
	HUA_INPUT_THREAD_STATE_RUNNING,
	HUA_INPUT_THREAD_STATE_STOPPING,
	HUA_INPUT_THREAD_STATE_STOPPED
};

struct huamobile_virtual_key
{
	int left;
	int right;
	int top;
	int bottom;
	int code;
	int value;
};

struct huamobile_input_device
{
	int fd;
	struct pollfd *pfd;
	char name[512];
	struct huamobile_virtual_key vkeys[4];
	size_t vkey_count;
};

struct huamobile_input_thread
{
	pthread_t thread;
	enum huamobile_input_thread_state state;
	struct huamobile_input_device input_devs[8];
	size_t dev_count;
	int pipefd[2];
	pthread_mutex_t lock;
	void *private_data;

	int (*matcher)(struct huamobile_input_device *dev, void *data);
	int (*event_handler)(struct huamobile_input_device *dev, struct input_event *event, void *data);
};

struct huamobile_touch_point
{
	int id;
	int x;
	int y;
	int pressure;
};

struct huamobile_ts_device
{
	int pressed;
	int released;
	int point_count;
	struct huamobile_touch_point points[10];
	void *private_data;
	struct huamobile_input_thread thread;

	int (*matcher)(struct huamobile_input_device *dev, void *data);
	int (*key_handler)(struct huamobile_input_device *dev, int code, int value, void *data);
	int (*point_handler)(struct huamobile_input_device *dev, struct huamobile_touch_point *point, void *data);
};

int huamobile_input_msleep(useconds_t ms);
int huamobile_input_ssleep(useconds_t ss);
int huamobile_input_text_lhcmp(const char *text1, const char *text2);
char *huamobile_input_text_copy(char *dest, const char *src);

int huamobile_input_parse_virtual_keymap(const char *devname, struct huamobile_virtual_key *keys, size_t count);
ssize_t huamobile_input_open_devices(struct huamobile_input_device *devs, size_t count, int (*matcher)(struct huamobile_input_device *dev, void *data), void *data);
void huamobile_input_close_devices(struct huamobile_input_device *devs, size_t count);
void *huamobile_input_thread_handler(void *data);

int huamobile_input_start_poll_thread(struct huamobile_input_thread *thread);
int huamobile_input_stop_poll_thread(struct huamobile_input_thread *thread);
int huamobile_input_thread_start(struct huamobile_input_thread *thread, void *data);
int huamobile_input_thread_stop(struct huamobile_input_thread *thread);

int huamobile_ts_start(struct huamobile_ts_device *ts, void *data);

int huamobile_touch_screen_matcher_multi(struct huamobile_input_device *dev, void *data);
int huamobile_touch_screen_matcher_single(struct huamobile_input_device *dev, void *data);
int huamobile_touch_screen_matcher(struct huamobile_input_device *dev, void *data);
int huamobile_gsensor_matcher(struct huamobile_input_device *dev, void *data);
int huamobile_input_name_matcher(const char *devname, ...);

static inline int huamobile_input_send_command(struct huamobile_input_thread *thread, enum huamobile_input_command cmd)
{
	return write(thread->pipefd[1], &cmd, sizeof(cmd));
}

static inline int huamobile_input_thread_join(struct huamobile_input_thread *thread)
{
	return pthread_join(thread->thread, NULL);
}

static inline int huamobile_ts_stop(struct huamobile_ts_device *ts)
{
	return huamobile_input_thread_stop(&ts->thread);
}
