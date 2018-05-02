/*
 * File:		input.h
 * Author:		Fuang Cao <cavan.cfa@gmail.com>
 * Created:		2012-11-14
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#pragma once

#include <cavan.h>
#include <cavan/network.h>
#include <cavan/service.h>
#include <cavan/event.h>
#include <cavan/timer.h>
#include <cavan/queue.h>
#include <linux/input.h>

#ifdef CONFIG_ANDROID_NDK
#include <android-ndk/uinput.h>
#else
#include <linux/uinput.h>
#endif

#define CAVAN_INPUT_MESSAGE_POOL_SIZE	50

#ifndef SYN_MT_REPORT
#define SYN_MT_REPORT		2
#define ABS_MT_SLOT			0x2f	/* MT slot being modified */
#define ABS_MT_TOUCH_MAJOR	0x30	/* Major axis of touching ellipse */
#define ABS_MT_TOUCH_MINOR	0x31	/* Minor axis (omit if circular) */
#define ABS_MT_WIDTH_MAJOR	0x32	/* Major axis of approaching ellipse */
#define ABS_MT_WIDTH_MINOR	0x33	/* Minor axis (omit if circular) */
#define ABS_MT_ORIENTATION	0x34	/* Ellipse orientation */
#define ABS_MT_POSITION_X	0x35	/* Center X ellipse position */
#define ABS_MT_POSITION_Y	0x36	/* Center Y ellipse position */
#define ABS_MT_TOOL_TYPE	0x37	/* Type of touching device */
#define ABS_MT_BLOB_ID		0x38	/* Group a set of packets as a blob */
#define ABS_MT_TRACKING_ID	0x39	/* Unique ID of initiated contact */
#define ABS_MT_PRESSURE		0x3a	/* Pressure on contact area */
#define ABS_MT_DISTANCE		0x3b	/* Contact hover distance */
#endif

#ifndef KEY_MICMUTE
#define KEY_MICMUTE		248	/* Mute / unmute the microphone */
#endif

#ifndef BTN_TOOL_QUINTTAP
#define BTN_TOOL_QUINTTAP	0x148	/* Five fingers on trackpad */
#endif

#ifndef KEY_IMAGES
#define KEY_IMAGES		0x1ba	/* AL Image Browser */
#endif

#ifndef KEY_CAMERA_ZOOMIN
#define KEY_CAMERA_ZOOMIN	0x215
#endif

#ifndef KEY_CAMERA_ZOOMOUT
#define KEY_CAMERA_ZOOMOUT	0x216
#endif

#ifndef KEY_CAMERA_UP
#define KEY_CAMERA_UP		0x217
#endif

#ifndef KEY_CAMERA_DOWN
#define KEY_CAMERA_DOWN		0x218
#endif

#ifndef KEY_CAMERA_LEFT
#define KEY_CAMERA_LEFT		0x219
#endif

#ifndef KEY_CAMERA_RIGHT
#define KEY_CAMERA_RIGHT	0x21a
#endif

#ifndef KEY_ATTENDANT_ON
#define KEY_ATTENDANT_ON	0x21b
#endif

#ifndef KEY_ATTENDANT_OFF
#define KEY_ATTENDANT_OFF	0x21c
#endif

#ifndef KEY_ATTENDANT_TOGGLE
#define KEY_ATTENDANT_TOGGLE	0x21d	/* Attendant call on or off */
#endif

#ifndef KEY_LIGHTS_TOGGLE
#define KEY_LIGHTS_TOGGLE	0x21e	/* Reading light on or off */
#endif

#ifndef BTN_DPAD_UP
#define BTN_DPAD_UP		0x220
#endif

#ifndef BTN_DPAD_DOWN
#define BTN_DPAD_DOWN		0x221
#endif

#ifndef BTN_DPAD_LEFT
#define BTN_DPAD_LEFT		0x222
#endif

#ifndef BTN_DPAD_RIGHT
#define BTN_DPAD_RIGHT		0x223
#endif

#ifndef KEY_SCALE
#define KEY_SCALE		120	/* AL Compiz Scale (Expose) */
#endif

#ifndef KEY_SCREENLOCK
#define KEY_SCREENLOCK		KEY_COFFEE
#endif

#ifndef KEY_DASHBOARD
#define KEY_DASHBOARD		204	/* AL Dashboard */
#endif

#ifndef KEY_BLUETOOTH
#define KEY_BLUETOOTH		237
#endif

#ifndef KEY_WLAN
#define KEY_WLAN		238
#endif

#ifndef KEY_UWB
#define KEY_UWB			239
#endif

#ifndef KEY_VIDEO_NEXT
#define KEY_VIDEO_NEXT		241	/* drive next video source */
#endif

#ifndef KEY_VIDEO_PREV
#define KEY_VIDEO_PREV		242	/* drive previous video source */
#endif

#ifndef KEY_BRIGHTNESS_CYCLE
#define KEY_BRIGHTNESS_CYCLE	243	/* brightness up, after max is min */
#endif

#ifndef KEY_BRIGHTNESS_AUTO
#define KEY_BRIGHTNESS_AUTO	244	/* Set Auto Brightness: manual
					  brightness control is off,
					  rely on ambient */
#endif

#ifndef KEY_BRIGHTNESS_ZERO
#define KEY_BRIGHTNESS_ZERO	KEY_BRIGHTNESS_AUTO
#endif

#ifndef KEY_DISPLAY_OFF
#define KEY_DISPLAY_OFF		245	/* display device to off state */
#endif

#ifndef KEY_WWAN
#define KEY_WWAN		246	/* Wireless WAN (LTE, UMTS, GSM, etc.) */
#endif

#ifndef KEY_WIMAX
#define KEY_WIMAX		KEY_WWAN
#endif

#ifndef KEY_RFKILL
#define KEY_RFKILL		247	/* Key that controls all radios */
#endif

#ifndef BTN_TOOL_QUADTAP
#define BTN_TOOL_QUADTAP	0x14f	/* Four fingers on trackpad */
#endif

#ifndef KEY_VIDEOPHONE
#define KEY_VIDEOPHONE		0x1a0	/* Media Select Video Phone */
#endif

#ifndef KEY_GAMES
#define KEY_GAMES		0x1a1	/* Media Select Games */
#endif

#ifndef KEY_ZOOMIN
#define KEY_ZOOMIN		0x1a2	/* AC Zoom In */
#endif

#ifndef KEY_ZOOMOUT
#define KEY_ZOOMOUT		0x1a3	/* AC Zoom Out */
#endif

#ifndef KEY_ZOOMRESET
#define KEY_ZOOMRESET		0x1a4	/* AC Zoom */
#endif

#ifndef KEY_WORDPROCESSOR
#define KEY_WORDPROCESSOR	0x1a5	/* AL Word Processor */
#endif

#ifndef KEY_EDITOR
#define KEY_EDITOR		0x1a6	/* AL Text Editor */
#endif

#ifndef KEY_SPREADSHEET
#define KEY_SPREADSHEET		0x1a7	/* AL Spreadsheet */
#endif

#ifndef KEY_GRAPHICSEDITOR
#define KEY_GRAPHICSEDITOR	0x1a8	/* AL Graphics Editor */
#endif

#ifndef KEY_PRESENTATION
#define KEY_PRESENTATION	0x1a9	/* AL Presentation App */
#endif

#ifndef KEY_DATABASE
#define KEY_DATABASE		0x1aa	/* AL Database App */
#endif

#ifndef KEY_NEWS
#define KEY_NEWS		0x1ab	/* AL Newsreader */
#endif

#ifndef KEY_VOICEMAIL
#define KEY_VOICEMAIL		0x1ac	/* AL Voicemail */
#endif

#ifndef KEY_ADDRESSBOOK
#define KEY_ADDRESSBOOK		0x1ad	/* AL Contacts/Address Book */
#endif

#ifndef KEY_MESSENGER
#define KEY_MESSENGER		0x1ae	/* AL Instant Messaging */
#endif

#ifndef KEY_DISPLAYTOGGLE
#define KEY_DISPLAYTOGGLE	0x1af	/* Turn display (LCD) on and off */
#endif

#ifndef KEY_SPELLCHECK
#define KEY_SPELLCHECK		0x1b0   /* AL Spell Check */
#endif

#ifndef KEY_LOGOFF
#define KEY_LOGOFF		0x1b1   /* AL Logoff */
#endif

#ifndef KEY_DOLLAR
#define KEY_DOLLAR		0x1b2
#endif

#ifndef KEY_EURO
#define KEY_EURO		0x1b3
#endif

#ifndef KEY_FRAMEBACK
#define KEY_FRAMEBACK		0x1b4	/* Consumer - transport controls */
#endif

#ifndef KEY_FRAMEFORWARD
#define KEY_FRAMEFORWARD	0x1b5
#endif

#ifndef KEY_CONTEXT_MENU
#define KEY_CONTEXT_MENU	0x1b6	/* GenDesc - system context menu */
#endif

#ifndef KEY_MEDIA_REPEAT
#define KEY_MEDIA_REPEAT	0x1b7	/* Consumer - transport control */
#endif

#ifndef KEY_10CHANNELSUP
#define KEY_10CHANNELSUP	0x1b8	/* 10 channels up (10+) */
#endif

#ifndef KEY_10CHANNELSDOWN
#define KEY_10CHANNELSDOWN	0x1b9	/* 10 channels down (10-) */
#endif

#ifndef KEY_BRL_DOT9
#define KEY_BRL_DOT9		0x1f9
#endif

#ifndef KEY_BRL_DOT10
#define KEY_BRL_DOT10		0x1fa
#endif

#ifndef KEY_NUMERIC_0
#define KEY_NUMERIC_0		0x200	/* used by phones, remote controls, */
#endif

#ifndef KEY_NUMERIC_1
#define KEY_NUMERIC_1		0x201	/* and other keypads */
#endif

#ifndef KEY_NUMERIC_2
#define KEY_NUMERIC_2		0x202
#endif

#ifndef KEY_NUMERIC_3
#define KEY_NUMERIC_3		0x203
#endif

#ifndef KEY_NUMERIC_4
#define KEY_NUMERIC_4		0x204
#endif

#ifndef KEY_NUMERIC_5
#define KEY_NUMERIC_5		0x205
#endif

#ifndef KEY_NUMERIC_6
#define KEY_NUMERIC_6		0x206
#endif

#ifndef KEY_NUMERIC_7
#define KEY_NUMERIC_7		0x207
#endif

#ifndef KEY_NUMERIC_8
#define KEY_NUMERIC_8		0x208
#endif

#ifndef KEY_NUMERIC_9
#define KEY_NUMERIC_9		0x209
#endif

#ifndef KEY_NUMERIC_STAR
#define KEY_NUMERIC_STAR	0x20a
#endif

#ifndef KEY_NUMERIC_POUND
#define KEY_NUMERIC_POUND	0x20b
#endif

#ifndef KEY_CAMERA_FOCUS
#define KEY_CAMERA_FOCUS	0x210
#endif

#ifndef KEY_WPS_BUTTON
#define KEY_WPS_BUTTON		0x211	/* WiFi Protected Setup key */
#endif

#ifndef KEY_TOUCHPAD_TOGGLE
#define KEY_TOUCHPAD_TOGGLE	0x212	/* Request switch touchpad on or off */
#endif

#ifndef KEY_TOUCHPAD_ON
#define KEY_TOUCHPAD_ON		0x213
#endif

#ifndef KEY_TOUCHPAD_OFF
#define KEY_TOUCHPAD_OFF	0x214
#endif

#ifndef BTN_TRIGGER_HAPPY1
#define BTN_TRIGGER_HAPPY1		0x2c0
#endif

#ifndef BTN_TRIGGER_HAPPY2
#define BTN_TRIGGER_HAPPY2		0x2c1
#endif

#ifndef BTN_TRIGGER_HAPPY3
#define BTN_TRIGGER_HAPPY3		0x2c2
#endif

#ifndef BTN_TRIGGER_HAPPY4
#define BTN_TRIGGER_HAPPY4		0x2c3
#endif

#ifndef BTN_TRIGGER_HAPPY5
#define BTN_TRIGGER_HAPPY5		0x2c4
#endif

#ifndef BTN_TRIGGER_HAPPY6
#define BTN_TRIGGER_HAPPY6		0x2c5
#endif

#ifndef BTN_TRIGGER_HAPPY7
#define BTN_TRIGGER_HAPPY7		0x2c6
#endif

#ifndef BTN_TRIGGER_HAPPY8
#define BTN_TRIGGER_HAPPY8		0x2c7
#endif

#ifndef BTN_TRIGGER_HAPPY9
#define BTN_TRIGGER_HAPPY9		0x2c8
#endif

#ifndef BTN_TRIGGER_HAPPY10
#define BTN_TRIGGER_HAPPY10		0x2c9
#endif

#ifndef BTN_TRIGGER_HAPPY11
#define BTN_TRIGGER_HAPPY11		0x2ca
#endif

#ifndef BTN_TRIGGER_HAPPY12
#define BTN_TRIGGER_HAPPY12		0x2cb
#endif

#ifndef BTN_TRIGGER_HAPPY13
#define BTN_TRIGGER_HAPPY13		0x2cc
#endif

#ifndef BTN_TRIGGER_HAPPY14
#define BTN_TRIGGER_HAPPY14		0x2cd
#endif

#ifndef BTN_TRIGGER_HAPPY15
#define BTN_TRIGGER_HAPPY15		0x2ce
#endif

#ifndef BTN_TRIGGER_HAPPY16
#define BTN_TRIGGER_HAPPY16		0x2cf
#endif

#ifndef BTN_TRIGGER_HAPPY17
#define BTN_TRIGGER_HAPPY17		0x2d0
#endif

#ifndef BTN_TRIGGER_HAPPY18
#define BTN_TRIGGER_HAPPY18		0x2d1
#endif

#ifndef BTN_TRIGGER_HAPPY19
#define BTN_TRIGGER_HAPPY19		0x2d2
#endif

#ifndef BTN_TRIGGER_HAPPY20
#define BTN_TRIGGER_HAPPY20		0x2d3
#endif

#ifndef BTN_TRIGGER_HAPPY21
#define BTN_TRIGGER_HAPPY21		0x2d4
#endif

#ifndef BTN_TRIGGER_HAPPY22
#define BTN_TRIGGER_HAPPY22		0x2d5
#endif

#ifndef BTN_TRIGGER_HAPPY23
#define BTN_TRIGGER_HAPPY23		0x2d6
#endif

#ifndef BTN_TRIGGER_HAPPY24
#define BTN_TRIGGER_HAPPY24		0x2d7
#endif

#ifndef BTN_TRIGGER_HAPPY25
#define BTN_TRIGGER_HAPPY25		0x2d8
#endif

#ifndef BTN_TRIGGER_HAPPY26
#define BTN_TRIGGER_HAPPY26		0x2d9
#endif

#ifndef BTN_TRIGGER_HAPPY27
#define BTN_TRIGGER_HAPPY27		0x2da
#endif

#ifndef BTN_TRIGGER_HAPPY28
#define BTN_TRIGGER_HAPPY28		0x2db
#endif

#ifndef BTN_TRIGGER_HAPPY29
#define BTN_TRIGGER_HAPPY29		0x2dc
#endif

#ifndef BTN_TRIGGER_HAPPY30
#define BTN_TRIGGER_HAPPY30		0x2dd
#endif

#ifndef BTN_TRIGGER_HAPPY31
#define BTN_TRIGGER_HAPPY31		0x2de
#endif

#ifndef BTN_TRIGGER_HAPPY32
#define BTN_TRIGGER_HAPPY32		0x2df
#endif

#ifndef BTN_TRIGGER_HAPPY33
#define BTN_TRIGGER_HAPPY33		0x2e0
#endif

#ifndef BTN_TRIGGER_HAPPY34
#define BTN_TRIGGER_HAPPY34		0x2e1
#endif

#ifndef BTN_TRIGGER_HAPPY35
#define BTN_TRIGGER_HAPPY35		0x2e2
#endif

#ifndef BTN_TRIGGER_HAPPY36
#define BTN_TRIGGER_HAPPY36		0x2e3
#endif

#ifndef BTN_TRIGGER_HAPPY37
#define BTN_TRIGGER_HAPPY37		0x2e4
#endif

#ifndef BTN_TRIGGER_HAPPY38
#define BTN_TRIGGER_HAPPY38		0x2e5
#endif

#ifndef BTN_TRIGGER_HAPPY39
#define BTN_TRIGGER_HAPPY39		0x2e6
#endif

#ifndef BTN_TRIGGER_HAPPY40
#define BTN_TRIGGER_HAPPY40		0x2e7
#endif

typedef enum cavan_input_message_type {
	CAVAN_INPUT_MESSAGE_KEY,
	CAVAN_INPUT_MESSAGE_MOVE,
	CAVAN_INPUT_MESSAGE_TOUCH,
	CAVAN_INPUT_MESSAGE_WHEEL,
	CAVAN_INPUT_MESSAGE_MOUSE_MOVE,
	CAVAN_INPUT_MESSAGE_MOUSE_TOUCH,
	CAVAN_INPUT_MESSAGE_ACCELEROMETER,
	CAVAN_INPUT_MESSAGE_MAGNETIC_FIELD,
	CAVAN_INPUT_MESSAGE_ORIENTATION,
	CAVAN_INPUT_MESSAGE_GYROSCOPE,
	CAVAN_INPUT_MESSAGE_LIGHT,
	CAVAN_INPUT_MESSAGE_PRESSURE,
	CAVAN_INPUT_MESSAGE_TEMPERATURE,
	CAVAN_INPUT_MESSAGE_PROXIMITY,
	CAVAN_INPUT_MESSAGE_GRAVITY,
	CAVAN_INPUT_MESSAGE_LINEAR_ACCELERATION,
	CAVAN_INPUT_MESSAGE_ROTATION_VECTOR
} cavan_input_message_type_t;

struct cavan_input_key {
	const char *name;
	int code;
};

struct cavan_input_message_key {
	const char *name;
	int code;
	int value;
};

struct cavan_input_message_point {
	int id;
	int x;
	int y;
	int pressure;
	bool pressed;
};

struct cavan_input_message_vector {
	int x;
	int y;
	int z;
};

typedef struct cavan_input_message {
	cavan_input_message_type_t type;

	union {
		int value;
		struct cavan_input_message_key key;
		struct cavan_input_message_point point;
		struct cavan_input_message_vector vector;
	};

	struct cavan_data_pool_node node;
} cavan_input_message_t;

struct cavan_input_message_queue {
	void *private_data;
	struct cavan_data_queue queue;

	void (*handler)(struct cavan_input_message_queue *queue, cavan_input_message_t *message, void *data);
};

struct cavan_input_device {
	struct cavan_event_device *event_dev;
	struct cavan_input_device *next;

	int (*probe)(struct cavan_input_device *dev, void *data);
	void (*remove)(struct cavan_input_device *dev, void *data);
	bool (*event_handler)(struct cavan_input_device *dev, struct input_event *event, void *data);
};

struct cavan_input_service {
	struct cavan_data_queue queue;
	struct cavan_event_service event_service;
	struct cavan_timer_service timer_service;

	void *private_data;
	int lcd_width, lcd_height;
	pthread_mutex_t lock;

	bool (*matcher)(struct cavan_event_matcher *matcher, void *data);
	void (*handler)(cavan_input_message_t *message, void *data);
};

struct cavan_input_proxy_device {
	int fd;
	char name[128];
	char filename[256];
	uint8_t abs_bitmask[ABS_BITMASK_SIZE];
	uint8_t key_bitmask[KEY_BITMASK_SIZE];
	uint8_t rel_bitmask[REL_BITMASK_SIZE];

	struct cavan_input_proxy_device *prev;
	struct cavan_input_proxy_device *next;
};

struct cavan_input_proxy {
	struct network_service service;
	struct network_url url;
	struct cavan_input_proxy_device *devices;
};

void cavan_input_service_init(struct cavan_input_service *service, bool (*matcher)(struct cavan_event_matcher *, void *));
int cavan_input_service_start(struct cavan_input_service *service, void *data);
void cavan_input_service_stop(struct cavan_input_service *service);
bool cavan_input_service_append_key_message(struct cavan_input_service *service, int type, const char *name, int code, int value);
bool cavan_input_service_append_vector_message(struct cavan_input_service *service, int type, int x, int y, int z);
bool cavan_input_service_append_point_message(struct cavan_input_service *service, int type, struct cavan_input_message_point *point);
char cavan_keycode2ascii(int code, bool shift_down);
int cavan_input_message_tostring(cavan_input_message_t *message, char *buff, size_t size);

int cavan_uinput_open(int flags);
int cavan_uinput_create(const char *name, int (*init)(struct uinput_user_dev *dev, int fd, void *data), void *data);
int cavan_input_event(int fd, const struct input_event *events, size_t count);
int cavan_input_event2(int fd, int type, int code, int value);
struct cavan_input_key *cavan_input_find_key(const char *name);
int cavan_input_type2value(const char *name);

int cavan_input_proxy_run(struct cavan_dynamic_service *service);

static inline int cavan_input_service_join(struct cavan_input_service *service)
{
	return cavan_event_service_join(&service->event_service);
}

static inline u32 timeval2msec(struct timeval *time)
{
	return time->tv_sec * 1000 + time->tv_usec / 1000;
}

static inline cavan_input_message_t *cavan_input_service_get_message(struct cavan_input_service *service, int type)
{
	cavan_input_message_t *message;

	message = cavan_data_queue_get_node(&service->queue);
	if (message) {
		message->type = type;
	}

	return message;
}

static inline void cavan_input_service_append_message(struct cavan_input_service *service, cavan_input_message_t *message)
{
	cavan_data_queue_append(&service->queue, &message->node);
}

static inline int cavan_input_report_key(int fd, int code, int value)
{
	return cavan_input_event2(fd, EV_KEY, code, value);
}

static inline int cavan_input_report_rel(int fd, int code, int value)
{
	return cavan_input_event2(fd, EV_REL, code, value);
}

static inline int cavan_input_report_abs(int fd, int code, int value)
{
	return cavan_input_event2(fd, EV_ABS, code, value);
}

static inline int cavan_input_sync(int fd)
{
	return cavan_input_event2(fd, EV_SYN, SYN_REPORT, 0);
}
