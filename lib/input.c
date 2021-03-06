/*
 * File:		input.c
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

#include <cavan/input.h>
#include <cavan/mouse.h>
#include <cavan/keypad.h>
#include <cavan/android.h>
#include <cavan/gsensor.h>
#include <cavan/touchpad.h>
#include <cavan/touchscreen.h>

#define CAVAN_INPUT_SUPPORT_GSENSOR	0

static const char *cavan_uinput_path_list[] = {
	"/dev/uinput",
	"/dev/input/uinput",
	"/dev/misc/uinput"
};

static struct cavan_input_key cavan_input_type_map[] = {
	{ "SYN", EV_SYN },
	{ "KEY", EV_KEY },
	{ "REL", EV_REL },
	{ "ABS", EV_ABS },
	{ "MSC", EV_MSC },
	{ "SW", EV_SW },
	{ "LED", EV_LED },
	{ "SND", EV_SND },
	{ "REP", EV_REP },
	{ "FF", EV_FF },
	{ "PWR", EV_PWR },
	{ "FF_STATUS", EV_FF_STATUS },
};

static struct cavan_input_key cavan_input_keymap[] = {
	{ "E", KEY_ENTER },
	{ "L", KEY_LEFT },
	{ "R", KEY_RIGHT },
	{ "U", KEY_UP },
	{ "D", KEY_DOWN },
	{ "B", KEY_BACK },
	{ "H", KEY_HOMEPAGE },
	{ "M", KEY_MENU },
	{ "P", KEY_POWER },
	{ "S", KEY_SEARCH },
	{ "VU", KEY_VOLUMEUP },
	{ "VD", KEY_VOLUMEDOWN },
	{ "v+", KEY_VOLUMEUP },
	{ "v-", KEY_VOLUMEDOWN },
	{ "BU", KEY_BRIGHTNESSUP },
	{ "BD", KEY_BRIGHTNESSDOWN },
	{ "b+", KEY_BRIGHTNESSUP },
	{ "b-", KEY_BRIGHTNESSDOWN },
	{ "esc", KEY_ESC },
	{ "key1", KEY_1 },
	{ "key2", KEY_2 },
	{ "key3", KEY_3 },
	{ "key4", KEY_4 },
	{ "key5", KEY_5 },
	{ "key6", KEY_6 },
	{ "key7", KEY_7 },
	{ "key8", KEY_8 },
	{ "key9", KEY_9 },
	{ "key0", KEY_0 },
	{ "minus", KEY_MINUS },
	{ "equal", KEY_EQUAL },
	{ "backspace", KEY_BACKSPACE },
	{ "tab", KEY_TAB },
	{ "q", KEY_Q },
	{ "w", KEY_W },
	{ "e", KEY_E },
	{ "r", KEY_R },
	{ "t", KEY_T },
	{ "y", KEY_Y },
	{ "u", KEY_U },
	{ "i", KEY_I },
	{ "o", KEY_O },
	{ "p", KEY_P },
	{ "leftbrace", KEY_LEFTBRACE },
	{ "rightbrace", KEY_RIGHTBRACE },
	{ "enter", KEY_ENTER },
	{ "leftctrl", KEY_LEFTCTRL },
	{ "a", KEY_A },
	{ "s", KEY_S },
	{ "d", KEY_D },
	{ "f", KEY_F },
	{ "g", KEY_G },
	{ "h", KEY_H },
	{ "j", KEY_J },
	{ "k", KEY_K },
	{ "l", KEY_L },
	{ "semicolon", KEY_SEMICOLON },
	{ "apostrophe", KEY_APOSTROPHE },
	{ "grave", KEY_GRAVE },
	{ "leftshift", KEY_LEFTSHIFT },
	{ "backslash", KEY_BACKSLASH },
	{ "z", KEY_Z },
	{ "x", KEY_X },
	{ "c", KEY_C },
	{ "v", KEY_V },
	{ "b", KEY_B },
	{ "n", KEY_N },
	{ "m", KEY_M },
	{ "comma", KEY_COMMA },
	{ "dot", KEY_DOT },
	{ "slash", KEY_SLASH },
	{ "rightshift", KEY_RIGHTSHIFT },
	{ "kpasterisk", KEY_KPASTERISK },
	{ "leftalt", KEY_LEFTALT },
	{ "space", KEY_SPACE },
	{ "capslock", KEY_CAPSLOCK },
	{ "f1", KEY_F1 },
	{ "f2", KEY_F2 },
	{ "f3", KEY_F3 },
	{ "f4", KEY_F4 },
	{ "f5", KEY_F5 },
	{ "f6", KEY_F6 },
	{ "f7", KEY_F7 },
	{ "f8", KEY_F8 },
	{ "f9", KEY_F9 },
	{ "f10", KEY_F10 },
	{ "numlock", KEY_NUMLOCK },
	{ "scrolllock", KEY_SCROLLLOCK },
	{ "kp7", KEY_KP7 },
	{ "kp8", KEY_KP8 },
	{ "kp9", KEY_KP9 },
	{ "kpminus", KEY_KPMINUS },
	{ "kp4", KEY_KP4 },
	{ "kp5", KEY_KP5 },
	{ "kp6", KEY_KP6 },
	{ "kpplus", KEY_KPPLUS },
	{ "kp1", KEY_KP1 },
	{ "kp2", KEY_KP2 },
	{ "kp3", KEY_KP3 },
	{ "kp0", KEY_KP0 },
	{ "kpdot", KEY_KPDOT },
	{ "zenkakuhankaku", KEY_ZENKAKUHANKAKU },
	{ "102nd", KEY_102ND },
	{ "f11", KEY_F11 },
	{ "f12", KEY_F12 },
	{ "ro", KEY_RO },
	{ "katakana", KEY_KATAKANA },
	{ "hiragana", KEY_HIRAGANA },
	{ "henkan", KEY_HENKAN },
	{ "katakanahiragana", KEY_KATAKANAHIRAGANA },
	{ "muhenkan", KEY_MUHENKAN },
	{ "kpjpcomma", KEY_KPJPCOMMA },
	{ "kpenter", KEY_KPENTER },
	{ "rightctrl", KEY_RIGHTCTRL },
	{ "kpslash", KEY_KPSLASH },
	{ "sysrq", KEY_SYSRQ },
	{ "rightalt", KEY_RIGHTALT },
	{ "linefeed", KEY_LINEFEED },
	{ "home", KEY_HOME },
	{ "up", KEY_UP },
	{ "pageup", KEY_PAGEUP },
	{ "left", KEY_LEFT },
	{ "right", KEY_RIGHT },
	{ "end", KEY_END },
	{ "down", KEY_DOWN },
	{ "pagedown", KEY_PAGEDOWN },
	{ "insert", KEY_INSERT },
	{ "delete", KEY_DELETE },
	{ "macro", KEY_MACRO },
	{ "mute", KEY_MUTE },
	{ "volumedown", KEY_VOLUMEDOWN },
	{ "volumeup", KEY_VOLUMEUP },
	{ "power", KEY_POWER },
	{ "kpequal", KEY_KPEQUAL },
	{ "kpplusminus", KEY_KPPLUSMINUS },
	{ "pause", KEY_PAUSE },
	{ "scale", KEY_SCALE },
	{ "kpcomma", KEY_KPCOMMA },
	{ "hangeul", KEY_HANGEUL },
	{ "hanguel", KEY_HANGUEL },
	{ "hanja", KEY_HANJA },
	{ "yen", KEY_YEN },
	{ "leftmeta", KEY_LEFTMETA },
	{ "rightmeta", KEY_RIGHTMETA },
	{ "compose", KEY_COMPOSE },
	{ "stop", KEY_STOP },
	{ "again", KEY_AGAIN },
	{ "props", KEY_PROPS },
	{ "undo", KEY_UNDO },
	{ "front", KEY_FRONT },
	{ "copy", KEY_COPY },
	{ "open", KEY_OPEN },
	{ "paste", KEY_PASTE },
	{ "find", KEY_FIND },
	{ "cut", KEY_CUT },
	{ "help", KEY_HELP },
	{ "menu", KEY_MENU },
	{ "calc", KEY_CALC },
	{ "setup", KEY_SETUP },
	{ "sleep", KEY_SLEEP },
	{ "wakeup", KEY_WAKEUP },
	{ "file", KEY_FILE },
	{ "sendfile", KEY_SENDFILE },
	{ "deletefile", KEY_DELETEFILE },
	{ "xfer", KEY_XFER },
	{ "prog1", KEY_PROG1 },
	{ "prog2", KEY_PROG2 },
	{ "www", KEY_WWW },
	{ "msdos", KEY_MSDOS },
	{ "coffee", KEY_COFFEE },
	{ "screenlock", KEY_SCREENLOCK },
	// { "rotate_display", KEY_ROTATE_DISPLAY },
	{ "direction", KEY_DIRECTION },
	{ "cyclewindows", KEY_CYCLEWINDOWS },
	{ "mail", KEY_MAIL },
	{ "bookmarks", KEY_BOOKMARKS },
	{ "computer", KEY_COMPUTER },
	{ "back", KEY_BACK },
	{ "forward", KEY_FORWARD },
	{ "closecd", KEY_CLOSECD },
	{ "ejectcd", KEY_EJECTCD },
	{ "ejectclosecd", KEY_EJECTCLOSECD },
	{ "nextsong", KEY_NEXTSONG },
	{ "playpause", KEY_PLAYPAUSE },
	{ "previoussong", KEY_PREVIOUSSONG },
	{ "stopcd", KEY_STOPCD },
	{ "record", KEY_RECORD },
	{ "rewind", KEY_REWIND },
	{ "phone", KEY_PHONE },
	{ "iso", KEY_ISO },
	{ "config", KEY_CONFIG },
	{ "homepage", KEY_HOMEPAGE },
	{ "refresh", KEY_REFRESH },
	{ "exit", KEY_EXIT },
	{ "move", KEY_MOVE },
	{ "edit", KEY_EDIT },
	{ "scrollup", KEY_SCROLLUP },
	{ "scrolldown", KEY_SCROLLDOWN },
	{ "kpleftparen", KEY_KPLEFTPAREN },
	{ "kprightparen", KEY_KPRIGHTPAREN },
	{ "new", KEY_NEW },
	{ "redo", KEY_REDO },
	{ "f13", KEY_F13 },
	{ "f14", KEY_F14 },
	{ "f15", KEY_F15 },
	{ "f16", KEY_F16 },
	{ "f17", KEY_F17 },
	{ "f18", KEY_F18 },
	{ "f19", KEY_F19 },
	{ "f20", KEY_F20 },
	{ "f21", KEY_F21 },
	{ "f22", KEY_F22 },
	{ "f23", KEY_F23 },
	{ "f24", KEY_F24 },
	{ "playcd", KEY_PLAYCD },
	{ "pausecd", KEY_PAUSECD },
	{ "prog3", KEY_PROG3 },
	{ "prog4", KEY_PROG4 },
	{ "dashboard", KEY_DASHBOARD },
	{ "suspend", KEY_SUSPEND },
	{ "close", KEY_CLOSE },
	{ "play", KEY_PLAY },
	{ "fastforward", KEY_FASTFORWARD },
	{ "bassboost", KEY_BASSBOOST },
	{ "print", KEY_PRINT },
	{ "hp", KEY_HP },
	{ "camera", KEY_CAMERA },
	{ "sound", KEY_SOUND },
	{ "question", KEY_QUESTION },
	{ "email", KEY_EMAIL },
	{ "chat", KEY_CHAT },
	{ "search", KEY_SEARCH },
	{ "connect", KEY_CONNECT },
	{ "finance", KEY_FINANCE },
	{ "sport", KEY_SPORT },
	{ "shop", KEY_SHOP },
	{ "alterase", KEY_ALTERASE },
	{ "cancel", KEY_CANCEL },
	{ "brightnessdown", KEY_BRIGHTNESSDOWN },
	{ "brightnessup", KEY_BRIGHTNESSUP },
	{ "media", KEY_MEDIA },
	{ "switchvideomode", KEY_SWITCHVIDEOMODE },
	{ "kbdillumtoggle", KEY_KBDILLUMTOGGLE },
	{ "kbdillumdown", KEY_KBDILLUMDOWN },
	{ "kbdillumup", KEY_KBDILLUMUP },
	{ "send", KEY_SEND },
	{ "reply", KEY_REPLY },
	{ "forwardmail", KEY_FORWARDMAIL },
	{ "save", KEY_SAVE },
	{ "documents", KEY_DOCUMENTS },
	{ "battery", KEY_BATTERY },
	{ "bluetooth", KEY_BLUETOOTH },
	{ "wlan", KEY_WLAN },
	{ "uwb", KEY_UWB },
	{ "unknown", KEY_UNKNOWN },
	{ "video_next", KEY_VIDEO_NEXT },
	{ "video_prev", KEY_VIDEO_PREV },
	{ "brightness_cycle", KEY_BRIGHTNESS_CYCLE },
	// { "brightness_auto", KEY_BRIGHTNESS_AUTO },
	{ "brightness_zero", KEY_BRIGHTNESS_ZERO },
	{ "display_off", KEY_DISPLAY_OFF },
#ifdef KEY_WWAN
	{ "wwan", KEY_WWAN },
#endif
	{ "wimax", KEY_WIMAX },
	{ "rfkill", KEY_RFKILL },
	{ "micmute", KEY_MICMUTE },
};

static struct cavan_input_key cavan_input_keymap_android[] = {
	// { "SOFT_LEFT", KEY_SOFT_LEFT },
	// { "SOFT_RIGHT", KEY_SOFT_RIGHT },
	{ "HOME", KEY_HOME },
	{ "BACK", KEY_BACK },
#if 0
	{ "CALL", KEY_CALL },
	{ "ENDCALL", KEY_ENDCALL },
#endif
	{ "KEYCODE_0", KEY_0 },
	{ "KEYCODE_1", KEY_1 },
	{ "KEYCODE_2", KEY_2 },
	{ "KEYCODE_3", KEY_3 },
	{ "KEYCODE_4", KEY_4 },
	{ "KEYCODE_5", KEY_5 },
	{ "KEYCODE_6", KEY_6 },
	{ "KEYCODE_7", KEY_7 },
	{ "KEYCODE_8", KEY_8 },
	{ "KEYCODE_9", KEY_9 },
#if 0
	{ "STAR", KEY_STAR },
	{ "POUND", KEY_POUND },
#endif
	{ "DPAD_UP", KEY_UP },
	{ "DPAD_DOWN", KEY_DOWN },
	{ "DPAD_LEFT", KEY_LEFT },
	{ "DPAD_RIGHT", KEY_RIGHT },
	{ "DPAD_CENTER", KEY_OK },
	{ "VOLUME_UP", KEY_VOLUMEUP },
	{ "VOLUME_DOWN", KEY_VOLUMEDOWN },
	{ "POWER", KEY_POWER },
	{ "CAMERA", KEY_CAMERA },
	{ "CLEAR", KEY_CLEAR },
	{ "A", KEY_A },
	{ "B", KEY_B },
	{ "C", KEY_C },
	{ "D", KEY_D },
	{ "E", KEY_E },
	{ "F", KEY_F },
	{ "G", KEY_G },
	{ "H", KEY_H },
	{ "I", KEY_I },
	{ "J", KEY_J },
	{ "K", KEY_K },
	{ "L", KEY_L },
	{ "M", KEY_M },
	{ "N", KEY_N },
	{ "O", KEY_O },
	{ "P", KEY_P },
	{ "Q", KEY_Q },
	{ "R", KEY_R },
	{ "S", KEY_S },
	{ "T", KEY_T },
	{ "U", KEY_U },
	{ "V", KEY_V },
	{ "W", KEY_W },
	{ "X", KEY_X },
	{ "Y", KEY_Y },
	{ "Z", KEY_Z },
	{ "COMMA", KEY_COMMA },
	{ "PERIOD", KEY_DOT },
	{ "ALT_LEFT", KEY_LEFTALT},
	{ "ALT_RIGHT", KEY_RIGHTALT},
	{ "SHIFT_LEFT", KEY_LEFTSHIFT},
	{ "SHIFT_RIGHT", KEY_RIGHTSHIFT },
	{ "TAB", KEY_TAB },
	{ "SPACE", KEY_SPACE },
#if 0
	{ "SYM", KEY_SYM },
	{ "EXPLORER", KEY_EXPLORER },
	{ "ENVELOPE", KEY_ENVELOPE },
#endif
	{ "ENTER", KEY_ENTER },
	{ "DEL", KEY_DELETE},
	{ "GRAVE", KEY_GRAVE },
	{ "MINUS", KEY_MINUS },
	{ "EQUALS", KEY_EQUAL },
	{ "LEFT_BRACKET", KEY_LEFTBRACE },
	{ "RIGHT_BRACKET", KEY_RIGHTBRACE },
	{ "BACKSLASH", KEY_BACKSLASH },
	{ "SEMICOLON", KEY_SEMICOLON },
	{ "APOSTROPHE", KEY_APOSTROPHE },
	{ "SLASH", KEY_SLASH },
#if 0
	{ "AT", KEY_AT },
	{ "NUM", KEY_NUM },
	{ "HEADSETHOOK", KEY_HEADSETHOOK },
	{ "FOCUS", KEY_FOCUS },
	{ "PLUS", KEY_PLUS },
#endif
	{ "MENU", KEY_MENU },
	// { "NOTIFICATION", KEY_NOTIFICATION },
	{ "SEARCH", KEY_SEARCH },
	{ "MEDIA_PLAY_PAUSE", KEY_PLAYPAUSE},
	{ "MEDIA_STOP", KEY_STOPCD },
	{ "MEDIA_NEXT", KEY_NEXTSONG},
	{ "MEDIA_PREVIOUS", KEY_PREVIOUSSONG},
	{ "MEDIA_REWIND", KEY_REWIND },
	{ "MEDIA_FAST_FORWARD", KEY_FASTFORWARD },
	{ "MUTE", KEY_MUTE },
	{ "PAGE_UP", KEY_PAGEUP },
	{ "PAGE_DOWN", KEY_PAGEDOWN },
	// { "PICTSYMBOLS", KEY_PICTSYMBOLS },
	// { "SWITCH_CHARSET", KEY_SWITCH_CHARSET },
	{ "BUTTON_A", BTN_A },
	{ "BUTTON_B", BTN_B },
	{ "BUTTON_C", BTN_C },
	{ "BUTTON_X", BTN_X },
	{ "BUTTON_Y", BTN_Y },
	{ "BUTTON_Z", BTN_Z },
	{ "BUTTON_L1", BTN_TL },
	{ "BUTTON_R1", BTN_TR },
	{ "BUTTON_L2", BTN_TL2 },
	{ "BUTTON_R2", BTN_TR2 },
	{ "BUTTON_THUMBL", BTN_THUMBL },
	{ "BUTTON_THUMBR", BTN_THUMBR },
	{ "BUTTON_START", BTN_START },
	{ "BUTTON_SELECT", BTN_SELECT },
	{ "BUTTON_MODE", BTN_MODE },
	{ "ESCAPE", KEY_ESC },
	{ "FORWARD_DEL", KEY_BACKSPACE },
	{ "CTRL_LEFT", KEY_LEFTCTRL },
	{ "CTRL_RIGHT", KEY_RIGHTCTRL },
	{ "CAPS_LOCK", KEY_CAPSLOCK },
	{ "SCROLL_LOCK", KEY_SCROLLLOCK },
	{ "META_LEFT", KEY_LEFTMETA },
	{ "META_RIGHT", KEY_RIGHTMETA },
	// { "FUNCTION", KEY_FUNCTION },
	{ "SYSRQ", KEY_SYSRQ },
	{ "BREAK", KEY_BREAK },
	// { "MOVE_HOME", KEY_MOVE_HOME },
	// { "MOVE_END", KEY_MOVE_END },
	{ "INSERT", KEY_INSERT },
	{ "FORWARD", KEY_FORWARD },
	{ "MEDIA_PLAY", KEY_PLAYCD },
	{ "MEDIA_PAUSE", KEY_PAUSECD },
	{ "MEDIA_CLOSE", KEY_EJECTCLOSECD },
	{ "MEDIA_EJECT", KEY_EJECTCD},
	{ "MEDIA_RECORD", KEY_RECORD },
	{ "F1", KEY_F1 },
	{ "F2", KEY_F2 },
	{ "F3", KEY_F3 },
	{ "F4", KEY_F4 },
	{ "F5", KEY_F5 },
	{ "F6", KEY_F6 },
	{ "F7", KEY_F7 },
	{ "F8", KEY_F8 },
	{ "F9", KEY_F9 },
	{ "F10", KEY_F10 },
	{ "F11", KEY_F11 },
	{ "F12", KEY_F12 },
	{ "NUM_LOCK", KEY_NUMLOCK },
	{ "NUMPAD_0", KEY_KP0 },
	{ "NUMPAD_1", KEY_KP1 },
	{ "NUMPAD_2", KEY_KP2 },
	{ "NUMPAD_3", KEY_KP3 },
	{ "NUMPAD_4", KEY_KP4 },
	{ "NUMPAD_5", KEY_KP5 },
	{ "NUMPAD_6", KEY_KP6 },
	{ "NUMPAD_7", KEY_KP7 },
	{ "NUMPAD_8", KEY_KP8 },
	{ "NUMPAD_9", KEY_KP9 },
	{ "NUMPAD_DIVIDE", KEY_KPSLASH },
	{ "NUMPAD_MULTIPLY", KEY_KPASTERISK },
	{ "NUMPAD_SUBTRACT", KEY_KPMINUS },
	{ "NUMPAD_ADD", KEY_KPPLUS },
	{ "NUMPAD_DOT", KEY_KPDOT },
	{ "NUMPAD_COMMA", KEY_KPCOMMA },
	{ "NUMPAD_ENTER", KEY_KPENTER },
	{ "NUMPAD_EQUALS", KEY_KPEQUAL },
	{ "NUMPAD_LEFT_PAREN", KEY_KPLEFTPAREN },
	{ "NUMPAD_RIGHT_PAREN", KEY_KPRIGHTPAREN },
	{ "VOLUME_MUTE", KEY_MUTE },
	{ "INFO", KEY_INFO },
	{ "CHANNEL_UP", KEY_CHANNELUP },
	{ "CHANNEL_DOWN", KEY_CHANNELDOWN },
	{ "ZOOM_IN", KEY_ZOOMIN },
	{ "ZOOM_OUT", KEY_ZOOMOUT },
	{ "TV", KEY_TV },
	{ "WINDOW", KEY_LEFTMETA },
#if 0
	{ "GUIDE", KEY_GUIDE },
	{ "DVR", KEY_DVR },
	{ "BOOKMARK", KEY_BOOKMARK },
	{ "CAPTIONS", KEY_CAPTIONS },
	{ "SETTINGS", KEY_SETTINGS },
	{ "TV_POWER", KEY_TV_POWER },
	{ "TV_INPUT", KEY_TV_INPUT },
	{ "STB_INPUT", KEY_STB_INPUT },
	{ "STB_POWER", KEY_STB_POWER },
	{ "AVR_POWER", KEY_AVR_POWER },
	{ "AVR_INPUT", KEY_AVR_INPUT },
	{ "PROG_RED", KEY_PROG_RED },
	{ "PROG_GREEN", KEY_PROG_GREEN },
	{ "PROG_YELLOW", KEY_PROG_YELLOW },
	{ "PROG_BLUE", KEY_PROG_BLUE },
	{ "APP_SWITCH", KEY_APP_SWITCH },
#endif
	{ "BUTTON_1", BTN_1 },
	{ "BUTTON_2", BTN_2 },
	{ "BUTTON_3", BTN_3 },
	{ "BUTTON_4", BTN_4 },
	{ "BUTTON_5", BTN_5 },
	{ "BUTTON_6", BTN_6 },
	{ "BUTTON_7", BTN_7 },
	{ "BUTTON_8", BTN_8 },
	{ "BUTTON_9", BTN_9 },
#if 0
	{ "BUTTON_10", KEY_BUTTON_10 },
	{ "BUTTON_11", KEY_BUTTON_11 },
	{ "BUTTON_12", KEY_BUTTON_12 },
	{ "BUTTON_13", KEY_BUTTON_13 },
	{ "BUTTON_14", KEY_BUTTON_14 },
	{ "BUTTON_15", KEY_BUTTON_15 },
	{ "BUTTON_16", KEY_BUTTON_16 },
	{ "LANGUAGE_SWITCH", KEY_LANGUAGE_SWITCH },
	{ "MANNER_MODE", KEY_MANNER_MODE },
	{ "3D_MODE", KEY_3D_MODE },
#endif
	// { "CONTACTS", KEY_CONTACTS },
	{ "CALENDAR", KEY_CALENDAR },
	{ "MUSIC", KEY_AUDIO },
	{ "CALCULATOR", KEY_CALC },
	{ "ZENKAKU_HANKAKU", KEY_ZENKAKUHANKAKU },
	// { "EISU", KEY_EISU },
	{ "MUHENKAN", KEY_MUHENKAN },
	{ "HENKAN", KEY_HENKAN },
	{ "KATAKANA_HIRAGANA", KEY_KATAKANAHIRAGANA },
	{ "YEN", KEY_YEN },
	{ "RO", KEY_RO },
	// { "KANA", KEY_KANA },
	// { "ASSIST", KEY_ASSIST },
	{ "BRIGHTNESS_DOWN", KEY_BRIGHTNESSDOWN },
	{ "BRIGHTNESS_UP", KEY_BRIGHTNESSUP },
	// { "MEDIA_AUDIO_TRACK", KEY_MEDIA_AUDIO_TRACK },
};

static struct cavan_input_device *cavan_input_device_create(uint8_t *key_bitmask, uint8_t *abs_bitmask, uint8_t *rel_bitmask)
{
#if CAVAN_INPUT_SUPPORT_GSENSOR
	if (cavan_gsensor_device_match(abs_bitmask)) {
		pr_green_info("G-Sensor Matched");
		return cavan_gsensor_create();
	}
#endif

	if (cavan_touchpad_device_match(key_bitmask, abs_bitmask)) {
		pr_green_info("Touch Pad Matched");
		return cavan_touchpad_device_create();
	}

	if (cavan_multi_touch_device_match(abs_bitmask)) {
		pr_green_info("Muti Touch Panel Matched");
		return cavan_multi_touch_device_create();
	}

	if (cavan_single_touch_device_match(abs_bitmask, key_bitmask)) {
		pr_green_info("Single Touch Panel Matched");
		return cavan_single_touch_device_create();
	}

	if (cavan_mouse_device_match(key_bitmask, rel_bitmask)) {
		pr_green_info("Mouse Matched");
		return cavan_mouse_create();
	}

	if (cavan_keypad_device_match(key_bitmask)) {
		pr_green_info("Keypad Matched");
		return cavan_keypad_create();
	}

	return NULL;
}

static int cavan_input_device_probe(struct cavan_event_device *event_dev, void *data)
{
	int ret;
	int fd = event_dev->fd;
	uint8_t key_bitmask[KEY_BITMASK_SIZE];
	uint8_t abs_bitmask[ABS_BITMASK_SIZE];
	uint8_t rel_bitmask[REL_BITMASK_SIZE];
	struct cavan_input_device *dev, *head, *tail;

	ret = cavan_event_get_abs_bitmask(fd, abs_bitmask);
	if (ret < 0) {
		pr_error_info("cavan_event_get_abs_bitmask");
		return ret;
	}

	ret = cavan_event_get_key_bitmask(fd, key_bitmask);
	if (ret < 0) {
		pr_error_info("cavan_event_get_key_bitmask");
		return ret;
	}

	ret = cavan_event_get_rel_bitmask(fd, rel_bitmask);
	if (ret < 0) {
		pr_error_info("cavan_event_get_rel_bitmask");
		return ret;
	}

	head = tail = NULL;

	while (1) {
		dev = cavan_input_device_create(key_bitmask, abs_bitmask, rel_bitmask);
		if (dev == NULL) {
			break;
		}

		dev->event_dev = event_dev;

		if (dev->probe && dev->probe(dev, data) < 0) {
			free(dev);
			continue;
		}

		if (head) {
			tail->next = dev;
			tail = dev;
		} else {
			head = tail = dev;
		}
	}

	if (head == NULL) {
		pr_red_info("can't recognize device");
		return -EINVAL;
	}

	tail->next = NULL;
	event_dev->private_data = head;

	return 0;
}

static void cavan_input_device_remove(struct cavan_event_device *event_dev, void *data)
{
	struct cavan_input_device *dev = event_dev->private_data, *next;

	while (dev) {
		if (dev->remove) {
			dev->remove(dev, data);
		}

		next = dev->next;
		free(dev);
		dev = next;
	}
}

static bool cavan_input_device_event_handler(struct cavan_event_device *event_dev, struct input_event *event, void *data)
{
	struct cavan_input_device *dev = event_dev->private_data;

	switch(event->type) {
	case EV_SYN:
		while (dev) {
			dev->event_handler(dev, event, data);
			dev = dev->next;
		}
		return true;

	case EV_MSC:
		return true;

	default:
		while (dev) {
			if (dev->event_handler(dev, event, data)) {
				return true;
			}

			dev = dev->next;
		}
	}

	return false;
}

static bool cavan_input_device_matcher(struct cavan_event_matcher *matcher, void *data)
{
	struct cavan_input_service *service = data;

	if (service->matcher) {
		return service->matcher(matcher, service->private_data);
	}

	return true;
}

void cavan_input_service_init(struct cavan_input_service *service, bool (*matcher)(struct cavan_event_matcher *, void *))
{
	cavan_event_service_init(&service->event_service, cavan_input_device_matcher);

	service->lcd_width = -1;
	service->lcd_height = -1;

	service->matcher = matcher;
	service->handler = NULL;
}

static void cavan_input_message_queue_handler(void *addr, void *data)
{
	struct cavan_input_service *service = data;

	service->handler(addr, service->private_data);
}

int cavan_input_message_tostring(cavan_input_message_t *message, char *buff, size_t size)
{
	struct cavan_input_message_key *key;
	struct cavan_input_message_point *point;
	struct cavan_input_message_vector *vector;

	switch (message->type) {
	case CAVAN_INPUT_MESSAGE_KEY:
		key = &message->key;
		return snprintf(buff, size, "key: name = %s, code = %d, value = %d", key->name, key->code, key->value);

	case CAVAN_INPUT_MESSAGE_MOVE:
		point = &message->point;
		return snprintf(buff, size, "move[%d] = [%d, %d]", point->id, point->x, point->y);

	case CAVAN_INPUT_MESSAGE_TOUCH:
		point = &message->point;
		return snprintf(buff, size, "touch[%d] = [%d, %d]", point->id, point->x, point->y);

	case CAVAN_INPUT_MESSAGE_WHEEL:
		key = &message->key;
		return snprintf(buff, size, "wheel[%d] = %d", key->code, key->value);

	case CAVAN_INPUT_MESSAGE_MOUSE_MOVE:
		vector = &message->vector;
		return snprintf(buff, size, "mouse_move = [%d, %d]", vector->x, vector->y);

	case CAVAN_INPUT_MESSAGE_MOUSE_TOUCH:
		key = &message->key;
		return snprintf(buff, size, "mouse_touch[%d] = %d", key->code, key->value);

	case CAVAN_INPUT_MESSAGE_ACCELEROMETER:
		vector = &message->vector;
		return snprintf(buff, size, "Accelerometer = [%d, %d, %d]", vector->x, vector->y, vector->z);

	case CAVAN_INPUT_MESSAGE_MAGNETIC_FIELD:
		vector = &message->vector;
		return snprintf(buff, size, "Magnetic_Field = [%d, %d, %d]", vector->x, vector->y, vector->z);

	case CAVAN_INPUT_MESSAGE_ORIENTATION:
		vector = &message->vector;
		return snprintf(buff, size, "Orientation = [%d, %d, %d]", vector->x, vector->y, vector->z);

	case CAVAN_INPUT_MESSAGE_GYROSCOPE:
		vector = &message->vector;
		return snprintf(buff, size, "Gyroscope = [%d, %d, %d]", vector->x, vector->y, vector->z);

	case CAVAN_INPUT_MESSAGE_GRAVITY:
		vector = &message->vector;
		return snprintf(buff, size, "Gravity = [%d, %d, %d]", vector->x, vector->y, vector->z);

	case CAVAN_INPUT_MESSAGE_LINEAR_ACCELERATION:
		vector = &message->vector;
		return snprintf(buff, size, "Linear_Acceleration = [%d, %d, %d]", vector->x, vector->y, vector->z);

	case CAVAN_INPUT_MESSAGE_ROTATION_VECTOR:
		vector = &message->vector;
		return snprintf(buff, size, "Rotation_Vector = [%d, %d, %d]", vector->x, vector->y, vector->z);

	case CAVAN_INPUT_MESSAGE_LIGHT:
		return snprintf(buff, size, "Light = [%d]", message->value);

	case CAVAN_INPUT_MESSAGE_PRESSURE:
		return snprintf(buff, size, "Pressure = [%d]", message->value);

	case CAVAN_INPUT_MESSAGE_TEMPERATURE:
		return snprintf(buff, size, "Temperature = [%d]", message->value);

	case CAVAN_INPUT_MESSAGE_PROXIMITY:
		return snprintf(buff, size, "Proximity = [%d]", message->value);

	default:
		return snprintf(buff, size, "Invalid message type %d", message->type);
	}
}

static void cavan_input_message_queue_handler_dummy(void *addr, void *data)
{
	int length;
	char buff[1024];

	length = cavan_input_message_tostring(addr, buff, sizeof(buff));
	if (length < (int) sizeof(buff)) {
		buff[length++] = '\n';
	}

	print_ntext(buff, length);
}

int cavan_input_service_start(struct cavan_input_service *service, void *data)
{
	int ret;
	struct cavan_event_service *event_service;

	if (service == NULL) {
		pr_red_info("service == NULL");
		ERROR_RETURN(EINVAL);
	}

	pthread_mutex_init(&service->lock, NULL);
	service->private_data = data;

	if (service->handler) {
		service->queue.handler = cavan_input_message_queue_handler;
	} else {
		service->queue.handler = cavan_input_message_queue_handler_dummy;
	}

	ret = cavan_data_queue_run(&service->queue, MOFS(cavan_input_message_t, node), \
			sizeof(cavan_input_message_t), CAVAN_INPUT_MESSAGE_POOL_SIZE, service);
	if (ret < 0) {
		pr_red_info("cavan_data_queue_run");
		return ret;
	}

	event_service = &service->event_service;
	event_service->matcher = cavan_input_device_matcher;
	event_service->probe = cavan_input_device_probe;
	event_service->remove = cavan_input_device_remove;
	event_service->event_handler = cavan_input_device_event_handler;

	ret = cavan_timer_service_start(&service->timer_service);
	if (ret < 0) {
		pr_red_info("cavan_timer_service_start");
		goto out_cavan_data_queue_stop;
	}

	ret = cavan_event_service_start(event_service, service);
	if (ret < 0) {
		pr_red_info("cavan_event_service_start");
		goto out_timer_service_stop;
	}

	return 0;

out_timer_service_stop:
	cavan_timer_service_stop(&service->timer_service);
out_cavan_data_queue_stop:
	cavan_data_queue_stop(&service->queue);
	cavan_data_queue_deinit(&service->queue);
	return ret;
}

void cavan_input_service_stop(struct cavan_input_service *service)
{
	cavan_event_service_stop(&service->event_service);
	cavan_timer_service_stop(&service->timer_service);
	cavan_data_queue_stop(&service->queue);
	cavan_data_queue_deinit(&service->queue);
	pthread_mutex_destroy(&service->lock);
}

bool cavan_input_service_append_key_message(struct cavan_input_service *service, int type, const char *name, int code, int value)
{
	cavan_input_message_t *message;
	struct cavan_input_message_key *key;

	message = cavan_input_service_get_message(service, type);
	if (message == NULL) {
		return false;
	}

	key = &message->key;
	key->name = name;
	key->code = code;
	key->value = value;

	cavan_input_service_append_message(service, message);

	return true;
}

bool cavan_input_service_append_vector_message(struct cavan_input_service *service, int type, int x, int y, int z)
{
	cavan_input_message_t *message;
	struct cavan_input_message_vector *vector;

	message = cavan_input_service_get_message(service, type);
	if (message == NULL) {
		return false;
	}

	vector = &message->vector;
	vector->x = x;
	vector->y = y;
	vector->z = z;

	cavan_input_service_append_message(service, message);

	return true;
}

bool cavan_input_service_append_point_message(struct cavan_input_service *service, int type, struct cavan_input_message_point *point)
{
	cavan_input_message_t *message;

	message = cavan_input_service_get_message(service, type);
	if (message == NULL) {
		return false;
	}

	message->point = *point;
	cavan_input_service_append_message(service, message);

	return true;
}

char cavan_keycode2ascii(int code, bool shift_down)
{
	const char *ascii_map =
		"**1234567890-=*\t"
		"qwertyuiop[]**"
		"asdfghjkl;'`*\\"
		"zxcvbnm,./*** ";
	const char *ascii_map_shift =
		"**!@#$%^&*()_+*\t"
		"QWERTYUIOP{}**"
		"ASDFGHJKL:\"~*|"
		"ZXCVBNM<>?*** ";

	if (code < 0 || code > KEY_SPACE) {
		return '*';
	}

	return shift_down ? ascii_map_shift[code] : ascii_map[code];
}

int cavan_uinput_open(int flags)
{
	int i;

	for (i = 0; i < NELEM(cavan_uinput_path_list); i++) {
		int fd;

		fd = open(cavan_uinput_path_list[i], flags);
		if (fd < 0) {
			if (errno != ENOENT) {
				return fd;
			}

			continue;
		}

		pd_info("uinput = %s", cavan_uinput_path_list[i]);

		return fd;
	}

	return -ENOENT;
}

int cavan_uinput_create(const char *name, int (*init)(struct uinput_user_dev *dev, int fd, void *data), void *data)
{
	int fd;
	int ret;
	struct uinput_user_dev dev;

	fd = cavan_uinput_open(O_WRONLY);
	if (fd < 0) {
		pr_err_info("cavan_uinput_open: %d", fd);
		return fd;
	}

	memset(&dev, 0, sizeof(dev));

	if (name) {
		strncpy(dev.name, name, UINPUT_MAX_NAME_SIZE);
	}

	dev.id.bustype = BUS_HOST;
	dev.id.vendor  = 0x0000;
	dev.id.product = 0x0000;
	dev.id.version = 0x0000;

	if (init) {
		ret = init(&dev, fd, data);
		if (ret < 0) {
			goto out_close_fd;
		}
	}

	ret = write(fd, &dev, sizeof(dev));
	if (ret < 0) {
		pr_err_info("write: %d", ret);
		goto out_close_fd;
	}

	ret = ioctl(fd, UI_DEV_CREATE, NULL);
	if (ret < 0) {
		pr_err_info("ioctl UI_DEV_CREATE: %d", ret);
		goto out_close_fd;
	}

	return fd;

out_close_fd:
	close(fd);
	return ret;
}

int cavan_input_event(int fd, const struct input_event *events, size_t count)
{
	return ffile_write(fd, events, sizeof(struct input_event) * count);
}

int cavan_input_event2(int fd, int type, int code, int value)
{
	struct input_event event = {
		.time = { 0, 0 },
		.type = type,
		.code = code,
		.value = value,
	};

	return cavan_input_event(fd, &event, 1);
}

struct cavan_input_key *cavan_input_find_key_by_name_base(struct cavan_input_key *keys, int count, const char *name)
{
	struct cavan_input_key *key_end;

	for (key_end = keys + count; keys < key_end; keys++) {
		if (strcmp(keys->name, name) == 0) {
			return keys;
		}
	}

	return NULL;
}

struct cavan_input_key *cavan_input_find_key_by_code_base(struct cavan_input_key *keys, int count, int code)
{
	struct cavan_input_key *key_end;

	for (key_end = keys + count; keys < key_end; keys++) {
		if (keys->code == code) {
			return keys;
		}
	}

	return NULL;
}

struct cavan_input_key *cavan_input_find_key_by_name(const char *name)
{
	return cavan_input_find_key_by_name_base(ARRAY_PAIR(cavan_input_keymap), name);
}

struct cavan_input_key *cavan_input_find_key_by_code(int code)
{
	return cavan_input_find_key_by_code_base(ARRAY_PAIR(cavan_input_keymap), code);
}

struct cavan_input_key *cavan_input_find_key_by_name_android(const char *name)
{
	return cavan_input_find_key_by_name_base(ARRAY_PAIR(cavan_input_keymap_android), name);
}

struct cavan_input_key *cavan_input_find_key_by_code_android(int code)
{
	return cavan_input_find_key_by_code_base(ARRAY_PAIR(cavan_input_keymap_android), code);
}

int cavan_input_find_key_code(const char *name)
{
	struct cavan_input_key *key = cavan_input_find_key_by_name(name);

	if (key == NULL) {
		return -1;
	}

	return key->code;
}

const char *cavan_input_find_key_name(int code)
{
	struct cavan_input_key *key = cavan_input_find_key_by_code(code);

	if (key == NULL) {
		return NULL;
	}

	return key->name;
}

int cavan_input_type2value(const char *name)
{
	struct cavan_input_key *p, *p_end;

	if (text_is_number(name)) {
		return text2value_unsigned(name, NULL, 10);
	}

	for (p = cavan_input_type_map, p_end = p + NELEM(cavan_input_type_map); p < p_end; p++) {
		if (text_cmp_nocase(p->name, name) == 0) {
			return p->code;
		}
	}

	return 0;
}

// ================================================================================

static struct cavan_input_proxy_device *cavan_input_proxy_device_open(const char *pathname, const char *filename)
{
	struct cavan_input_proxy_device *device;
	int ret;
	int fd;

	pr_info("pathname = %s", pathname);

	fd = open(pathname, O_WRONLY);
	if (fd < 0) {
		pr_err_info("open: %s", pathname);
		return NULL;
	}

	device = malloc(sizeof(struct cavan_input_proxy_device));
	if (device == NULL) {
		pr_err_info("malloc");
		goto out_close_fd;
	}

	ret = cavan_event_get_devname(fd, device->name, sizeof(device->name));
	if (ret < 0) {
		pr_error_info("cavan_event_get_devname");
		goto out_free_device;
	}

	pr_info("name = %s", device->name);

	ret = cavan_event_get_abs_bitmask(fd, device->abs_bitmask);
	if (ret < 0) {
		pr_error_info("cavan_event_get_abs_bitmask");
		goto out_free_device;
	}

	ret = cavan_event_get_key_bitmask(fd, device->key_bitmask);
	if (ret < 0) {
		pr_error_info("cavan_event_get_key_bitmask");
		goto out_free_device;
	}

	ret = cavan_event_get_rel_bitmask(fd, device->rel_bitmask);
	if (ret < 0) {
		pr_error_info("cavan_event_get_rel_bitmask");
		goto out_free_device;
	}

	ret = cavan_event_get_absinfo(fd, ABS_MT_POSITION_X, &device->xmin, &device->xres);
	if (ret < 0) {
		device->xmin = device->xres = 0;
	} else {
		device->xres = device->xres - device->xmin + 1;
	}

	pr_info("xmin = %d, xres = %d", device->xmin, device->xres);

	ret = cavan_event_get_absinfo(fd, ABS_MT_POSITION_Y, &device->ymin, &device->yres);
	if (ret < 0) {
		device->ymin = device->yres = 0;
	} else {
		device->yres = device->yres - device->ymin + 1;
	}

	pr_info("ymin = %d, yres = %d", device->ymin, device->yres);

	strcpy(device->filename, filename);
	device->fd = fd;

	pr_info("%s <= %s", pathname, device->name);

	return device;

out_free_device:
	free(device);
out_close_fd:
	close(fd);
	return NULL;
}

static void cavan_input_proxy_device_close(struct cavan_input_proxy_device *device)
{
	close(device->fd);
	free(device);
}

static ssize_t cavan_input_proxy_device_write(struct cavan_input_proxy_device *device, const struct input_event *events, int count)
{
	return ffile_write(device->fd, events, sizeof(struct input_event) * count);
}

static bool cavan_input_proxy_device_support_key(struct cavan_input_proxy_device *device, int code)
{
	return test_bit(code, device->key_bitmask);
}

static bool cavan_input_proxy_device_support_abs(struct cavan_input_proxy_device *device, int code)
{
	return test_bit(code, device->abs_bitmask);
}

static bool cavan_input_proxy_device_contains(struct cavan_input_proxy_device *head, const char *filename)
{
	struct cavan_input_proxy_device *device = head;

	if (device == NULL) {
		return false;
	}

	while (1) {
		if (strcmp(device->filename, filename) == 0) {
			return true;
		}

		device = device->next;

		if (device == head) {
			break;
		}
	}

	return false;
}

static struct cavan_input_proxy_device *cavan_input_proxy_device_add(struct cavan_input_proxy_device *head, struct cavan_input_proxy_device *device)
{
	if (head == NULL) {
		device->prev = device->next = device;
		return device;
	}

	device->prev = head->prev;
	head->prev->next = device;
	device->next = head;
	head->prev = device;

	return head;
}

static struct cavan_input_proxy_device *cavan_input_proxy_device_remove(struct cavan_input_proxy_device *head, struct cavan_input_proxy_device *device)
{
	struct cavan_input_proxy_device *next = device->next;

	if (next == device) {
		return NULL;
	}

	next->prev = device->prev;
	device->prev->next = next;

	if (device == head) {
		return next;
	}

	return head;
}

static struct cavan_input_proxy_device *cavan_input_proxy_device_find_keypad(struct cavan_input_proxy_device *head, int code)
{
	struct cavan_input_proxy_device *device = head;

	if (device == NULL) {
		return NULL;
	}

	while (1) {
		if (cavan_input_proxy_device_support_key(device, code)) {
			return device;
		}

		device = device->next;
		if (device == head) {
			break;
		}
	}

	return NULL;
}

static struct cavan_input_proxy_device *cavan_input_proxy_device_find_touchscreen(struct cavan_input_proxy_device *head)
{
	struct cavan_input_proxy_device *device = head;

	if (device == NULL) {
		return NULL;
	}

	while (1) {
		if (cavan_input_proxy_device_support_abs(device, ABS_MT_POSITION_X) && cavan_input_proxy_device_support_abs(device, ABS_MT_POSITION_Y)) {
			return device;
		}

		device = device->next;
		if (device == head) {
			break;
		}
	}

	return NULL;
}

static int cavan_input_proxy_device_scan(struct cavan_input_proxy *proxy)
{
	DIR *dp;
	char *filename;
	char pathname[1024];
	struct dirent *entry;

	filename = text_copy(pathname, "/dev/input/");

	dp = opendir(pathname);
	if (dp == NULL) {
		pr_err_info("opendir");
		return -ENOENT;
	}

	while ((entry = readdir(dp))) {
		const char *name = entry->d_name;
		struct cavan_input_proxy_device *device;

		if (cavan_path_is_dot_name(name)) {
			continue;
		}

		if (cavan_input_proxy_device_contains(proxy->devices, name)) {
			continue;
		}

		strcpy(filename, name);

		device = cavan_input_proxy_device_open(pathname, filename);
		if (device != NULL) {
			proxy->devices = cavan_input_proxy_device_add(proxy->devices, device);
		}
	}

	closedir(dp);

	return 0;
}

static int input_proxy_start_handler(struct cavan_dynamic_service *service)
{
	struct cavan_input_proxy *proxy = cavan_dynamic_service_get_data(service);

	return network_service_open(&proxy->service, &proxy->url, 0);
}

static void input_proxy_stop_handler(struct cavan_dynamic_service *service)
{
	struct cavan_input_proxy *proxy = cavan_dynamic_service_get_data(service);

	network_service_close(&proxy->service);
}

static int input_proxy_open_connect(struct cavan_dynamic_service *service, void *conn)
{
	struct cavan_input_proxy *proxy = cavan_dynamic_service_get_data(service);

	return network_service_accept(&proxy->service, conn, CAVAN_NET_FLAG_NODELAY);
}

static bool input_proxy_close_connect(struct cavan_dynamic_service *service, void *conn)
{
	network_client_close(conn);
	return false;
}

static bool input_proxy_send_events(struct cavan_input_proxy *proxy, struct cavan_input_proxy_device *device, const struct input_event *events, int count)
{
	ssize_t wrlen;

	wrlen = cavan_input_proxy_device_write(device, events, count);
	if (wrlen < 0) {
		proxy->devices = cavan_input_proxy_device_remove(proxy->devices, device);
		cavan_input_proxy_device_close(device);
		return false;
	}

	return true;
}

static bool input_proxy_send_tap(struct cavan_input_proxy *proxy, int argc, char *argv[])
{
	struct cavan_input_proxy_device *device;
	struct input_event events[10];
	struct input_event *event;
	bool support_tracking_id;
	bool support_touch;
	int x, y;

	if (argc < 3) {
		return false;
	}

	x = atoi(argv[1]);
	y = atoi(argv[2]);

	device = cavan_input_proxy_device_find_touchscreen(proxy->devices);
	if (device == NULL) {
		pr_red_info("cavan_input_proxy_device_find_touchscreen");
		return android_input_tap(x, y);
	}

	if (argc > 4) {
		int width = atoi(argv[3]);
		int height = atoi(argv[4]);

		x = x * device->xres / width + device->xmin;
		y = y * device->yres / height + device->ymin;
	}

	pr_info("TOUCH: (%d, %d)", x, y);

	support_tracking_id = cavan_input_proxy_device_support_abs(device, ABS_MT_TRACKING_ID);
	support_touch = cavan_input_proxy_device_support_key(device, BTN_TOUCH);
	event = events;

	if (cavan_input_proxy_device_support_abs(device, ABS_MT_SLOT)) {
		cavan_input_event_setup_abs(event++, ABS_MT_SLOT, 0);
	}

	if (cavan_input_proxy_device_support_abs(device, ABS_MT_PRESSURE)) {
		cavan_input_event_setup_abs(event++, ABS_MT_PRESSURE, 1);
	}

	cavan_input_event_setup_abs(event++, ABS_MT_POSITION_X, x);
	cavan_input_event_setup_abs(event++, ABS_MT_POSITION_Y, y);

	if (support_tracking_id) {
		cavan_input_event_setup_abs(event++, ABS_MT_TRACKING_ID, 0);
	}

	// cavan_input_event_setup_syn_mt_report(event++);

	if (support_touch) {
		cavan_input_event_setup_key(event++, BTN_TOUCH, 1);
	}

	cavan_input_event_setup_syn_report(event++);

	if (support_tracking_id) {
		cavan_input_event_setup_abs(event++, ABS_MT_TRACKING_ID, -1);
	}

	// cavan_input_event_setup_syn_mt_report(event++);

	if (support_touch) {
		cavan_input_event_setup_key(event++, BTN_TOUCH, 0);
	}

	cavan_input_event_setup_syn_report(event++);

	return input_proxy_send_events(proxy, device, events, event - events);
}

static bool input_proxy_send_key(struct cavan_input_proxy *proxy, int argc, char *argv[])
{
	struct cavan_input_proxy_device *device;
	struct input_event events[4];
	int code;

	if (argc < 2) {
		return false;
	}

	code = text2value_unsigned(argv[1], NULL, 10);
	if (code == 0) {
		struct cavan_input_key *key = cavan_input_find_key_by_name(argv[1]);

		if (key == NULL) {
			return android_input_key(argv[1]);
		}

		code = key->code;
	}

	pr_info("KEY: %d", code);

	device = cavan_input_proxy_device_find_keypad(proxy->devices, code);
	if (device == NULL) {
		struct cavan_input_key *key = cavan_input_find_key_by_code_android(code);

		if (key == NULL) {
			pr_red_info("Invalid key: %s", argv[1]);
			return false;
		}

		return android_input_key(key->name);
	}

	cavan_input_event_setup_key_report(events, code, 1);
	cavan_input_event_setup_key_report(events + 2, code, 0);

	return input_proxy_send_events(proxy, device, events, NELEM(events));
}

static int input_proxy_run_handler(struct cavan_dynamic_service *service, void *conn)
{
	struct cavan_input_proxy *proxy = cavan_dynamic_service_get_data(service);
	struct network_client *client = (struct network_client *) conn;

	cavan_dynamic_service_lock(service);
	cavan_input_proxy_device_scan(proxy);
	cavan_dynamic_service_unlock(service);

	while (1) {
		const char *command;
		char cmdline[1024];
		ssize_t rdlen;
		char *argv[20];
		int argc;

		rdlen = network_client_recv_packet(client, cmdline, sizeof(cmdline));
		if (rdlen < 0) {
			break;
		}

		cmdline[rdlen] = 0;
		pr_info("cmdline = %s", cmdline);

		argc = text_split_by_space(cmdline, argv, NELEM(argv));
		if (argc < 1) {
			continue;
		}

		command = argv[0];

		cavan_dynamic_service_lock(service);

		if (strcmp(command, "TAP") == 0) {
			input_proxy_send_tap(proxy, argc, argv);
		} else if (strcmp(command, "KEY") == 0) {
			input_proxy_send_key(proxy, argc, argv);
		}

		cavan_dynamic_service_unlock(service);
	}

	return 0;
}

int cavan_input_proxy_run(struct cavan_dynamic_service *service)
{
	service->name = "INPUT_PROXY";
	service->conn_size = sizeof(struct network_client);
	service->start = input_proxy_start_handler;
	service->stop = input_proxy_stop_handler;
	service->run = input_proxy_run_handler;
	service->open_connect = input_proxy_open_connect;
	service->close_connect = input_proxy_close_connect;

	return cavan_dynamic_service_run(service);
}
