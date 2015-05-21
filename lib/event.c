// Fuang.Cao <cavan.cfa@gmail.com> Wed May 25 10:12:17 CST 2011

#include <cavan.h>
#include <cavan/event.h>
#include <cavan/input.h>

const char *cavan_event_key_code_tostring(int code)
{
	switch (code)
	{
	case KEY_ESC:
		return "KEY_ESC";
	case KEY_1:
		return "KEY_1";
	case KEY_2:
		return "KEY_2";
	case KEY_3:
		return "KEY_3";
	case KEY_4:
		return "KEY_4";
	case KEY_5:
		return "KEY_5";
	case KEY_6:
		return "KEY_6";
	case KEY_7:
		return "KEY_7";
	case KEY_8:
		return "KEY_8";
	case KEY_9:
		return "KEY_9";
	case KEY_0:
		return "KEY_0";
	case KEY_MINUS:
		return "KEY_MINUS";
	case KEY_EQUAL:
		return "KEY_EQUAL";
	case KEY_BACKSPACE:
		return "KEY_BACKSPACE";
	case KEY_TAB:
		return "KEY_TAB";
	case KEY_Q:
		return "KEY_Q";
	case KEY_W:
		return "KEY_W";
	case KEY_E:
		return "KEY_E";
	case KEY_R:
		return "KEY_R";
	case KEY_T:
		return "KEY_T";
	case KEY_Y:
		return "KEY_Y";
	case KEY_U:
		return "KEY_U";
	case KEY_I:
		return "KEY_I";
	case KEY_O:
		return "KEY_O";
	case KEY_P:
		return "KEY_P";
	case KEY_LEFTBRACE:
		return "KEY_LEFTBRACE";
	case KEY_RIGHTBRACE:
		return "KEY_RIGHTBRACE";
	case KEY_ENTER:
		return "KEY_ENTER";
	case KEY_LEFTCTRL:
		return "KEY_LEFTCTRL";
	case KEY_A:
		return "KEY_A";
	case KEY_S:
		return "KEY_S";
	case KEY_D:
		return "KEY_D";
	case KEY_F:
		return "KEY_F";
	case KEY_G:
		return "KEY_G";
	case KEY_H:
		return "KEY_H";
	case KEY_J:
		return "KEY_J";
	case KEY_K:
		return "KEY_K";
	case KEY_L:
		return "KEY_L";
	case KEY_SEMICOLON:
		return "KEY_SEMICOLON";
	case KEY_APOSTROPHE:
		return "KEY_APOSTROPHE";
	case KEY_GRAVE:
		return "KEY_GRAVE";
	case KEY_LEFTSHIFT:
		return "KEY_LEFTSHIFT";
	case KEY_BACKSLASH:
		return "KEY_BACKSLASH";
	case KEY_Z:
		return "KEY_Z";
	case KEY_X:
		return "KEY_X";
	case KEY_C:
		return "KEY_C";
	case KEY_V:
		return "KEY_V";
	case KEY_B:
		return "KEY_B";
	case KEY_N:
		return "KEY_N";
	case KEY_M:
		return "KEY_M";
	case KEY_COMMA:
		return "KEY_COMMA";
	case KEY_DOT:
		return "KEY_DOT";
	case KEY_SLASH:
		return "KEY_SLASH";
	case KEY_RIGHTSHIFT:
		return "KEY_RIGHTSHIFT";
	case KEY_KPASTERISK:
		return "KEY_KPASTERISK";
	case KEY_LEFTALT:
		return "KEY_LEFTALT";
	case KEY_SPACE:
		return "KEY_SPACE";
	case KEY_CAPSLOCK:
		return "KEY_CAPSLOCK";
	case KEY_F1:
		return "KEY_F1";
	case KEY_F2:
		return "KEY_F2";
	case KEY_F3:
		return "KEY_F3";
	case KEY_F4:
		return "KEY_F4";
	case KEY_F5:
		return "KEY_F5";
	case KEY_F6:
		return "KEY_F6";
	case KEY_F7:
		return "KEY_F7";
	case KEY_F8:
		return "KEY_F8";
	case KEY_F9:
		return "KEY_F9";
	case KEY_F10:
		return "KEY_F10";
	case KEY_NUMLOCK:
		return "KEY_NUMLOCK";
	case KEY_SCROLLLOCK:
		return "KEY_SCROLLLOCK";
	case KEY_KP7:
		return "KEY_KP7";
	case KEY_KP8:
		return "KEY_KP8";
	case KEY_KP9:
		return "KEY_KP9";
	case KEY_KPMINUS:
		return "KEY_KPMINUS";
	case KEY_KP4:
		return "KEY_KP4";
	case KEY_KP5:
		return "KEY_KP5";
	case KEY_KP6:
		return "KEY_KP6";
	case KEY_KPPLUS:
		return "KEY_KPPLUS";
	case KEY_KP1:
		return "KEY_KP1";
	case KEY_KP2:
		return "KEY_KP2";
	case KEY_KP3:
		return "KEY_KP3";
	case KEY_KP0:
		return "KEY_KP0";
	case KEY_KPDOT:
		return "KEY_KPDOT";
	case KEY_ZENKAKUHANKAKU:
		return "KEY_ZENKAKUHANKAKU";
	case KEY_102ND:
		return "KEY_102ND";
	case KEY_F11:
		return "KEY_F11";
	case KEY_F12:
		return "KEY_F12";
	case KEY_RO:
		return "KEY_RO";
	case KEY_KATAKANA:
		return "KEY_KATAKANA";
	case KEY_HIRAGANA:
		return "KEY_HIRAGANA";
	case KEY_HENKAN:
		return "KEY_HENKAN";
	case KEY_KATAKANAHIRAGANA:
		return "KEY_KATAKANAHIRAGANA";
	case KEY_MUHENKAN:
		return "KEY_MUHENKAN";
	case KEY_KPJPCOMMA:
		return "KEY_KPJPCOMMA";
	case KEY_KPENTER:
		return "KEY_KPENTER";
	case KEY_RIGHTCTRL:
		return "KEY_RIGHTCTRL";
	case KEY_KPSLASH:
		return "KEY_KPSLASH";
	case KEY_SYSRQ:
		return "KEY_SYSRQ";
	case KEY_RIGHTALT:
		return "KEY_RIGHTALT";
	case KEY_LINEFEED:
		return "KEY_LINEFEED";
	case KEY_HOME:
		return "KEY_HOME";
	case KEY_UP:
		return "KEY_UP";
	case KEY_PAGEUP:
		return "KEY_PAGEUP";
	case KEY_LEFT:
		return "KEY_LEFT";
	case KEY_RIGHT:
		return "KEY_RIGHT";
	case KEY_END:
		return "KEY_END";
	case KEY_DOWN:
		return "KEY_DOWN";
	case KEY_PAGEDOWN:
		return "KEY_PAGEDOWN";
	case KEY_INSERT:
		return "KEY_INSERT";
	case KEY_DELETE:
		return "KEY_DELETE";
	case KEY_MACRO:
		return "KEY_MACRO";
	case KEY_MUTE:
		return "KEY_MUTE";
	case KEY_VOLUMEDOWN:
		return "KEY_VOLUMEDOWN";
	case KEY_VOLUMEUP:
		return "KEY_VOLUMEUP";
	case KEY_POWER:
		return "KEY_POWER";
	case KEY_KPEQUAL:
		return "KEY_KPEQUAL";
	case KEY_KPPLUSMINUS:
		return "KEY_KPPLUSMINUS";
	case KEY_PAUSE:
		return "KEY_PAUSE";
	case KEY_SCALE:
		return "KEY_SCALE";
	case KEY_KPCOMMA:
		return "KEY_KPCOMMA";
	case KEY_HANGEUL:
		return "KEY_HANGEUL";
#if 0
	case KEY_HANGUEL:
		return "KEY_HANGUEL";
#endif
	case KEY_HANJA:
		return "KEY_HANJA";
	case KEY_YEN:
		return "KEY_YEN";
	case KEY_LEFTMETA:
		return "KEY_LEFTMETA";
	case KEY_RIGHTMETA:
		return "KEY_RIGHTMETA";
	case KEY_COMPOSE:
		return "KEY_COMPOSE";
	case KEY_STOP:
		return "KEY_STOP";
	case KEY_AGAIN:
		return "KEY_AGAIN";
	case KEY_PROPS:
		return "KEY_PROPS";
	case KEY_UNDO:
		return "KEY_UNDO";
	case KEY_FRONT:
		return "KEY_FRONT";
	case KEY_COPY:
		return "KEY_COPY";
	case KEY_OPEN:
		return "KEY_OPEN";
	case KEY_PASTE:
		return "KEY_PASTE";
	case KEY_FIND:
		return "KEY_FIND";
	case KEY_CUT:
		return "KEY_CUT";
	case KEY_HELP:
		return "KEY_HELP";
	case KEY_MENU:
		return "KEY_MENU";
	case KEY_CALC:
		return "KEY_CALC";
	case KEY_SETUP:
		return "KEY_SETUP";
	case KEY_SLEEP:
		return "KEY_SLEEP";
	case KEY_WAKEUP:
		return "KEY_WAKEUP";
	case KEY_FILE:
		return "KEY_FILE";
	case KEY_SENDFILE:
		return "KEY_SENDFILE";
	case KEY_DELETEFILE:
		return "KEY_DELETEFILE";
	case KEY_XFER:
		return "KEY_XFER";
	case KEY_PROG1:
		return "KEY_PROG1";
	case KEY_PROG2:
		return "KEY_PROG2";
	case KEY_WWW:
		return "KEY_WWW";
	case KEY_MSDOS:
		return "KEY_MSDOS";
#if 0
	case KEY_COFFEE:
		return "KEY_COFFEE";
#endif
	case KEY_SCREENLOCK:
		return "KEY_SCREENLOCK";
	case KEY_DIRECTION:
		return "KEY_DIRECTION";
	case KEY_CYCLEWINDOWS:
		return "KEY_CYCLEWINDOWS";
	case KEY_MAIL:
		return "KEY_MAIL";
	case KEY_BOOKMARKS:
		return "KEY_BOOKMARKS";
	case KEY_COMPUTER:
		return "KEY_COMPUTER";
	case KEY_BACK:
		return "KEY_BACK";
	case KEY_FORWARD:
		return "KEY_FORWARD";
	case KEY_CLOSECD:
		return "KEY_CLOSECD";
	case KEY_EJECTCD:
		return "KEY_EJECTCD";
	case KEY_EJECTCLOSECD:
		return "KEY_EJECTCLOSECD";
	case KEY_NEXTSONG:
		return "KEY_NEXTSONG";
	case KEY_PLAYPAUSE:
		return "KEY_PLAYPAUSE";
	case KEY_PREVIOUSSONG:
		return "KEY_PREVIOUSSONG";
	case KEY_STOPCD:
		return "KEY_STOPCD";
	case KEY_RECORD:
		return "KEY_RECORD";
	case KEY_REWIND:
		return "KEY_REWIND";
	case KEY_PHONE:
		return "KEY_PHONE";
	case KEY_ISO:
		return "KEY_ISO";
	case KEY_CONFIG:
		return "KEY_CONFIG";
	case KEY_HOMEPAGE:
		return "KEY_HOMEPAGE";
	case KEY_REFRESH:
		return "KEY_REFRESH";
	case KEY_EXIT:
		return "KEY_EXIT";
	case KEY_MOVE:
		return "KEY_MOVE";
	case KEY_EDIT:
		return "KEY_EDIT";
	case KEY_SCROLLUP:
		return "KEY_SCROLLUP";
	case KEY_SCROLLDOWN:
		return "KEY_SCROLLDOWN";
	case KEY_KPLEFTPAREN:
		return "KEY_KPLEFTPAREN";
	case KEY_KPRIGHTPAREN:
		return "KEY_KPRIGHTPAREN";
	case KEY_NEW:
		return "KEY_NEW";
	case KEY_REDO:
		return "KEY_REDO";
	case KEY_F13:
		return "KEY_F13";
	case KEY_F14:
		return "KEY_F14";
	case KEY_F15:
		return "KEY_F15";
	case KEY_F16:
		return "KEY_F16";
	case KEY_F17:
		return "KEY_F17";
	case KEY_F18:
		return "KEY_F18";
	case KEY_F19:
		return "KEY_F19";
	case KEY_F20:
		return "KEY_F20";
	case KEY_F21:
		return "KEY_F21";
	case KEY_F22:
		return "KEY_F22";
	case KEY_F23:
		return "KEY_F23";
	case KEY_F24:
		return "KEY_F24";
	case KEY_PLAYCD:
		return "KEY_PLAYCD";
	case KEY_PAUSECD:
		return "KEY_PAUSECD";
	case KEY_PROG3:
		return "KEY_PROG3";
	case KEY_PROG4:
		return "KEY_PROG4";
	case KEY_DASHBOARD:
		return "KEY_DASHBOARD";
	case KEY_SUSPEND:
		return "KEY_SUSPEND";
	case KEY_CLOSE:
		return "KEY_CLOSE";
	case KEY_PLAY:
		return "KEY_PLAY";
	case KEY_FASTFORWARD:
		return "KEY_FASTFORWARD";
	case KEY_BASSBOOST:
		return "KEY_BASSBOOST";
	case KEY_PRINT:
		return "KEY_PRINT";
	case KEY_HP:
		return "KEY_HP";
	case KEY_CAMERA:
		return "KEY_CAMERA";
	case KEY_SOUND:
		return "KEY_SOUND";
	case KEY_QUESTION:
		return "KEY_QUESTION";
	case KEY_EMAIL:
		return "KEY_EMAIL";
	case KEY_CHAT:
		return "KEY_CHAT";
	case KEY_SEARCH:
		return "KEY_SEARCH";
	case KEY_CONNECT:
		return "KEY_CONNECT";
	case KEY_FINANCE:
		return "KEY_FINANCE";
	case KEY_SPORT:
		return "KEY_SPORT";
	case KEY_SHOP:
		return "KEY_SHOP";
	case KEY_ALTERASE:
		return "KEY_ALTERASE";
	case KEY_CANCEL:
		return "KEY_CANCEL";
	case KEY_BRIGHTNESSDOWN:
		return "KEY_BRIGHTNESSDOWN";
	case KEY_BRIGHTNESSUP:
		return "KEY_BRIGHTNESSUP";
	case KEY_MEDIA:
		return "KEY_MEDIA";
	case KEY_SWITCHVIDEOMODE:
		return "KEY_SWITCHVIDEOMODE";
	case KEY_KBDILLUMTOGGLE:
		return "KEY_KBDILLUMTOGGLE";
	case KEY_KBDILLUMDOWN:
		return "KEY_KBDILLUMDOWN";
	case KEY_KBDILLUMUP:
		return "KEY_KBDILLUMUP";
	case KEY_SEND:
		return "KEY_SEND";
	case KEY_REPLY:
		return "KEY_REPLY";
	case KEY_FORWARDMAIL:
		return "KEY_FORWARDMAIL";
	case KEY_SAVE:
		return "KEY_SAVE";
	case KEY_DOCUMENTS:
		return "KEY_DOCUMENTS";
	case KEY_BATTERY:
		return "KEY_BATTERY";
	case KEY_BLUETOOTH:
		return "KEY_BLUETOOTH";
	case KEY_WLAN:
		return "KEY_WLAN";
	case KEY_UWB:
		return "KEY_UWB";
	case KEY_UNKNOWN:
		return "KEY_UNKNOWN";
	case KEY_VIDEO_NEXT:
		return "KEY_VIDEO_NEXT";
	case KEY_VIDEO_PREV:
		return "KEY_VIDEO_PREV";
	case KEY_BRIGHTNESS_CYCLE:
		return "KEY_BRIGHTNESS_CYCLE";
	case KEY_BRIGHTNESS_ZERO:
		return "KEY_BRIGHTNESS_ZERO";
	case KEY_DISPLAY_OFF:
		return "KEY_DISPLAY_OFF";
	case KEY_WIMAX:
		return "KEY_WIMAX";
	case KEY_RFKILL:
		return "KEY_RFKILL";
	case KEY_MICMUTE:
		return "KEY_MICMUTE";
#if 0
	case BTN_MISC:
		return "BTN_MISC";
#endif
	case BTN_0:
		return "BTN_0";
	case BTN_1:
		return "BTN_1";
	case BTN_2:
		return "BTN_2";
	case BTN_3:
		return "BTN_3";
	case BTN_4:
		return "BTN_4";
	case BTN_5:
		return "BTN_5";
	case BTN_6:
		return "BTN_6";
	case BTN_7:
		return "BTN_7";
	case BTN_8:
		return "BTN_8";
	case BTN_9:
		return "BTN_9";
#if 0
	case BTN_MOUSE:
		return "BTN_MOUSE";
#endif
	case BTN_LEFT:
		return "BTN_LEFT";
	case BTN_RIGHT:
		return "BTN_RIGHT";
	case BTN_MIDDLE:
		return "BTN_MIDDLE";
	case BTN_SIDE:
		return "BTN_SIDE";
	case BTN_EXTRA:
		return "BTN_EXTRA";
	case BTN_FORWARD:
		return "BTN_FORWARD";
	case BTN_BACK:
		return "BTN_BACK";
	case BTN_TASK:
		return "BTN_TASK";
#if 0
	case BTN_JOYSTICK:
		return "BTN_JOYSTICK";
#endif
	case BTN_TRIGGER:
		return "BTN_TRIGGER";
	case BTN_THUMB:
		return "BTN_THUMB";
	case BTN_THUMB2:
		return "BTN_THUMB2";
	case BTN_TOP:
		return "BTN_TOP";
	case BTN_TOP2:
		return "BTN_TOP2";
	case BTN_PINKIE:
		return "BTN_PINKIE";
	case BTN_BASE:
		return "BTN_BASE";
	case BTN_BASE2:
		return "BTN_BASE2";
	case BTN_BASE3:
		return "BTN_BASE3";
	case BTN_BASE4:
		return "BTN_BASE4";
	case BTN_BASE5:
		return "BTN_BASE5";
	case BTN_BASE6:
		return "BTN_BASE6";
#if 0
	case BTN_DEAD:
		return "BTN_DEAD";
	case BTN_GAMEPAD:
		return "BTN_GAMEPAD";
	case BTN_SOUTH:
		return "BTN_SOUTH";
#endif
	case BTN_A:
		return "BTN_A";
#if 0
	case BTN_EAST:
		return "BTN_EAST";
#endif
	case BTN_B:
		return "BTN_B";
	case BTN_C:
		return "BTN_C";
#if 0
	case BTN_NORTH:
		return "BTN_NORTH";
#endif
	case BTN_X:
		return "BTN_X";
#if 0
	case BTN_WEST:
		return "BTN_WEST";
#endif
	case BTN_Y:
		return "BTN_Y";
	case BTN_Z:
		return "BTN_Z";
	case BTN_TL:
		return "BTN_TL";
	case BTN_TR:
		return "BTN_TR";
	case BTN_TL2:
		return "BTN_TL2";
	case BTN_TR2:
		return "BTN_TR2";
	case BTN_SELECT:
		return "BTN_SELECT";
	case BTN_START:
		return "BTN_START";
	case BTN_MODE:
		return "BTN_MODE";
	case BTN_THUMBL:
		return "BTN_THUMBL";
	case BTN_THUMBR:
		return "BTN_THUMBR";
#if 0
	case BTN_DIGI:
		return "BTN_DIGI";
#endif
	case BTN_TOOL_PEN:
		return "BTN_TOOL_PEN";
	case BTN_TOOL_RUBBER:
		return "BTN_TOOL_RUBBER";
	case BTN_TOOL_BRUSH:
		return "BTN_TOOL_BRUSH";
	case BTN_TOOL_PENCIL:
		return "BTN_TOOL_PENCIL";
	case BTN_TOOL_AIRBRUSH:
		return "BTN_TOOL_AIRBRUSH";
	case BTN_TOOL_FINGER:
		return "BTN_TOOL_FINGER";
	case BTN_TOOL_MOUSE:
		return "BTN_TOOL_MOUSE";
	case BTN_TOOL_LENS:
		return "BTN_TOOL_LENS";
	case BTN_TOOL_QUINTTAP:
		return "BTN_TOOL_QUINTTAP";
	case BTN_TOUCH:
		return "BTN_TOUCH";
	case BTN_STYLUS:
		return "BTN_STYLUS";
	case BTN_STYLUS2:
		return "BTN_STYLUS2";
	case BTN_TOOL_DOUBLETAP:
		return "BTN_TOOL_DOUBLETAP";
	case BTN_TOOL_TRIPLETAP:
		return "BTN_TOOL_TRIPLETAP";
	case BTN_TOOL_QUADTAP:
		return "BTN_TOOL_QUADTAP";
	case BTN_WHEEL:
		return "BTN_WHEEL";
#if 0
	case BTN_GEAR_DOWN:
		return "BTN_GEAR_DOWN";
#endif
	case BTN_GEAR_UP:
		return "BTN_GEAR_UP";
	case KEY_OK:
		return "KEY_OK";
	case KEY_SELECT:
		return "KEY_SELECT";
	case KEY_GOTO:
		return "KEY_GOTO";
	case KEY_CLEAR:
		return "KEY_CLEAR";
	case KEY_POWER2:
		return "KEY_POWER2";
	case KEY_OPTION:
		return "KEY_OPTION";
	case KEY_INFO:
		return "KEY_INFO";
#if 0
	case KEY_TIME:
		return "KEY_TIME";
#endif
	case KEY_VENDOR:
		return "KEY_VENDOR";
	case KEY_ARCHIVE:
		return "KEY_ARCHIVE";
	case KEY_PROGRAM:
		return "KEY_PROGRAM";
	case KEY_CHANNEL:
		return "KEY_CHANNEL";
	case KEY_FAVORITES:
		return "KEY_FAVORITES";
	case KEY_EPG:
		return "KEY_EPG";
	case KEY_PVR:
		return "KEY_PVR";
	case KEY_MHP:
		return "KEY_MHP";
	case KEY_LANGUAGE:
		return "KEY_LANGUAGE";
	case KEY_TITLE:
		return "KEY_TITLE";
	case KEY_SUBTITLE:
		return "KEY_SUBTITLE";
	case KEY_ANGLE:
		return "KEY_ANGLE";
	case KEY_ZOOM:
		return "KEY_ZOOM";
	case KEY_MODE:
		return "KEY_MODE";
	case KEY_KEYBOARD:
		return "KEY_KEYBOARD";
	case KEY_SCREEN:
		return "KEY_SCREEN";
	case KEY_PC:
		return "KEY_PC";
	case KEY_TV:
		return "KEY_TV";
	case KEY_TV2:
		return "KEY_TV2";
	case KEY_VCR:
		return "KEY_VCR";
	case KEY_VCR2:
		return "KEY_VCR2";
	case KEY_SAT:
		return "KEY_SAT";
	case KEY_SAT2:
		return "KEY_SAT2";
	case KEY_CD:
		return "KEY_CD";
	case KEY_TAPE:
		return "KEY_TAPE";
	case KEY_RADIO:
		return "KEY_RADIO";
	case KEY_TUNER:
		return "KEY_TUNER";
	case KEY_PLAYER:
		return "KEY_PLAYER";
	case KEY_TEXT:
		return "KEY_TEXT";
	case KEY_DVD:
		return "KEY_DVD";
	case KEY_AUX:
		return "KEY_AUX";
	case KEY_MP3:
		return "KEY_MP3";
	case KEY_AUDIO:
		return "KEY_AUDIO";
	case KEY_VIDEO:
		return "KEY_VIDEO";
	case KEY_DIRECTORY:
		return "KEY_DIRECTORY";
	case KEY_LIST:
		return "KEY_LIST";
	case KEY_MEMO:
		return "KEY_MEMO";
	case KEY_CALENDAR:
		return "KEY_CALENDAR";
	case KEY_RED:
		return "KEY_RED";
	case KEY_GREEN:
		return "KEY_GREEN";
	case KEY_YELLOW:
		return "KEY_YELLOW";
	case KEY_BLUE:
		return "KEY_BLUE";
	case KEY_CHANNELUP:
		return "KEY_CHANNELUP";
	case KEY_CHANNELDOWN:
		return "KEY_CHANNELDOWN";
	case KEY_FIRST:
		return "KEY_FIRST";
	case KEY_LAST:
		return "KEY_LAST";
	case KEY_AB:
		return "KEY_AB";
	case KEY_NEXT:
		return "KEY_NEXT";
	case KEY_RESTART:
		return "KEY_RESTART";
	case KEY_SLOW:
		return "KEY_SLOW";
	case KEY_SHUFFLE:
		return "KEY_SHUFFLE";
	case KEY_BREAK:
		return "KEY_BREAK";
	case KEY_PREVIOUS:
		return "KEY_PREVIOUS";
	case KEY_DIGITS:
		return "KEY_DIGITS";
	case KEY_TEEN:
		return "KEY_TEEN";
	case KEY_TWEN:
		return "KEY_TWEN";
	case KEY_VIDEOPHONE:
		return "KEY_VIDEOPHONE";
	case KEY_GAMES:
		return "KEY_GAMES";
	case KEY_ZOOMIN:
		return "KEY_ZOOMIN";
	case KEY_ZOOMOUT:
		return "KEY_ZOOMOUT";
	case KEY_ZOOMRESET:
		return "KEY_ZOOMRESET";
	case KEY_WORDPROCESSOR:
		return "KEY_WORDPROCESSOR";
	case KEY_EDITOR:
		return "KEY_EDITOR";
	case KEY_SPREADSHEET:
		return "KEY_SPREADSHEET";
	case KEY_GRAPHICSEDITOR:
		return "KEY_GRAPHICSEDITOR";
	case KEY_PRESENTATION:
		return "KEY_PRESENTATION";
	case KEY_DATABASE:
		return "KEY_DATABASE";
	case KEY_NEWS:
		return "KEY_NEWS";
	case KEY_VOICEMAIL:
		return "KEY_VOICEMAIL";
	case KEY_ADDRESSBOOK:
		return "KEY_ADDRESSBOOK";
	case KEY_MESSENGER:
		return "KEY_MESSENGER";
	case KEY_DISPLAYTOGGLE:
		return "KEY_DISPLAYTOGGLE";
	case KEY_SPELLCHECK:
		return "KEY_SPELLCHECK";
	case KEY_LOGOFF:
		return "KEY_LOGOFF";
	case KEY_DOLLAR:
		return "KEY_DOLLAR";
	case KEY_EURO:
		return "KEY_EURO";
	case KEY_FRAMEBACK:
		return "KEY_FRAMEBACK";
	case KEY_FRAMEFORWARD:
		return "KEY_FRAMEFORWARD";
	case KEY_CONTEXT_MENU:
		return "KEY_CONTEXT_MENU";
	case KEY_MEDIA_REPEAT:
		return "KEY_MEDIA_REPEAT";
	case KEY_10CHANNELSUP:
		return "KEY_10CHANNELSUP";
	case KEY_10CHANNELSDOWN:
		return "KEY_10CHANNELSDOWN";
	case KEY_IMAGES:
		return "KEY_IMAGES";
	case KEY_DEL_EOL:
		return "KEY_DEL_EOL";
	case KEY_DEL_EOS:
		return "KEY_DEL_EOS";
	case KEY_INS_LINE:
		return "KEY_INS_LINE";
	case KEY_DEL_LINE:
		return "KEY_DEL_LINE";
	case KEY_FN:
		return "KEY_FN";
	case KEY_FN_ESC:
		return "KEY_FN_ESC";
	case KEY_FN_F1:
		return "KEY_FN_F1";
	case KEY_FN_F2:
		return "KEY_FN_F2";
	case KEY_FN_F3:
		return "KEY_FN_F3";
	case KEY_FN_F4:
		return "KEY_FN_F4";
	case KEY_FN_F5:
		return "KEY_FN_F5";
	case KEY_FN_F6:
		return "KEY_FN_F6";
	case KEY_FN_F7:
		return "KEY_FN_F7";
	case KEY_FN_F8:
		return "KEY_FN_F8";
	case KEY_FN_F9:
		return "KEY_FN_F9";
	case KEY_FN_F10:
		return "KEY_FN_F10";
	case KEY_FN_F11:
		return "KEY_FN_F11";
	case KEY_FN_F12:
		return "KEY_FN_F12";
	case KEY_FN_1:
		return "KEY_FN_1";
	case KEY_FN_2:
		return "KEY_FN_2";
	case KEY_FN_D:
		return "KEY_FN_D";
	case KEY_FN_E:
		return "KEY_FN_E";
	case KEY_FN_F:
		return "KEY_FN_F";
	case KEY_FN_S:
		return "KEY_FN_S";
	case KEY_FN_B:
		return "KEY_FN_B";
	case KEY_BRL_DOT1:
		return "KEY_BRL_DOT1";
	case KEY_BRL_DOT2:
		return "KEY_BRL_DOT2";
	case KEY_BRL_DOT3:
		return "KEY_BRL_DOT3";
	case KEY_BRL_DOT4:
		return "KEY_BRL_DOT4";
	case KEY_BRL_DOT5:
		return "KEY_BRL_DOT5";
	case KEY_BRL_DOT6:
		return "KEY_BRL_DOT6";
	case KEY_BRL_DOT7:
		return "KEY_BRL_DOT7";
	case KEY_BRL_DOT8:
		return "KEY_BRL_DOT8";
	case KEY_BRL_DOT9:
		return "KEY_BRL_DOT9";
	case KEY_BRL_DOT10:
		return "KEY_BRL_DOT10";
	case KEY_NUMERIC_0:
		return "KEY_NUMERIC_0";
	case KEY_NUMERIC_1:
		return "KEY_NUMERIC_1";
	case KEY_NUMERIC_2:
		return "KEY_NUMERIC_2";
	case KEY_NUMERIC_3:
		return "KEY_NUMERIC_3";
	case KEY_NUMERIC_4:
		return "KEY_NUMERIC_4";
	case KEY_NUMERIC_5:
		return "KEY_NUMERIC_5";
	case KEY_NUMERIC_6:
		return "KEY_NUMERIC_6";
	case KEY_NUMERIC_7:
		return "KEY_NUMERIC_7";
	case KEY_NUMERIC_8:
		return "KEY_NUMERIC_8";
	case KEY_NUMERIC_9:
		return "KEY_NUMERIC_9";
	case KEY_NUMERIC_STAR:
		return "KEY_NUMERIC_STAR";
	case KEY_NUMERIC_POUND:
		return "KEY_NUMERIC_POUND";
	case KEY_CAMERA_FOCUS:
		return "KEY_CAMERA_FOCUS";
	case KEY_WPS_BUTTON:
		return "KEY_WPS_BUTTON";
	case KEY_TOUCHPAD_TOGGLE:
		return "KEY_TOUCHPAD_TOGGLE";
	case KEY_TOUCHPAD_ON:
		return "KEY_TOUCHPAD_ON";
	case KEY_TOUCHPAD_OFF:
		return "KEY_TOUCHPAD_OFF";
	case KEY_CAMERA_ZOOMIN:
		return "KEY_CAMERA_ZOOMIN";
	case KEY_CAMERA_ZOOMOUT:
		return "KEY_CAMERA_ZOOMOUT";
	case KEY_CAMERA_UP:
		return "KEY_CAMERA_UP";
	case KEY_CAMERA_DOWN:
		return "KEY_CAMERA_DOWN";
	case KEY_CAMERA_LEFT:
		return "KEY_CAMERA_LEFT";
	case KEY_CAMERA_RIGHT:
		return "KEY_CAMERA_RIGHT";
	case KEY_ATTENDANT_ON:
		return "KEY_ATTENDANT_ON";
	case KEY_ATTENDANT_OFF:
		return "KEY_ATTENDANT_OFF";
	case KEY_ATTENDANT_TOGGLE:
		return "KEY_ATTENDANT_TOGGLE";
	case KEY_LIGHTS_TOGGLE:
		return "KEY_LIGHTS_TOGGLE";
	case BTN_DPAD_UP:
		return "BTN_DPAD_UP";
	case BTN_DPAD_DOWN:
		return "BTN_DPAD_DOWN";
	case BTN_DPAD_LEFT:
		return "BTN_DPAD_LEFT";
	case BTN_DPAD_RIGHT:
		return "BTN_DPAD_RIGHT";
#if 0
	case BTN_TRIGGER_HAPPY:
		return "BTN_TRIGGER_HAPPY";
#endif
	case BTN_TRIGGER_HAPPY1:
		return "BTN_TRIGGER_HAPPY1";
	case BTN_TRIGGER_HAPPY2:
		return "BTN_TRIGGER_HAPPY2";
	case BTN_TRIGGER_HAPPY3:
		return "BTN_TRIGGER_HAPPY3";
	case BTN_TRIGGER_HAPPY4:
		return "BTN_TRIGGER_HAPPY4";
	case BTN_TRIGGER_HAPPY5:
		return "BTN_TRIGGER_HAPPY5";
	case BTN_TRIGGER_HAPPY6:
		return "BTN_TRIGGER_HAPPY6";
	case BTN_TRIGGER_HAPPY7:
		return "BTN_TRIGGER_HAPPY7";
	case BTN_TRIGGER_HAPPY8:
		return "BTN_TRIGGER_HAPPY8";
	case BTN_TRIGGER_HAPPY9:
		return "BTN_TRIGGER_HAPPY9";
	case BTN_TRIGGER_HAPPY10:
		return "BTN_TRIGGER_HAPPY10";
	case BTN_TRIGGER_HAPPY11:
		return "BTN_TRIGGER_HAPPY11";
	case BTN_TRIGGER_HAPPY12:
		return "BTN_TRIGGER_HAPPY12";
	case BTN_TRIGGER_HAPPY13:
		return "BTN_TRIGGER_HAPPY13";
	case BTN_TRIGGER_HAPPY14:
		return "BTN_TRIGGER_HAPPY14";
	case BTN_TRIGGER_HAPPY15:
		return "BTN_TRIGGER_HAPPY15";
	case BTN_TRIGGER_HAPPY16:
		return "BTN_TRIGGER_HAPPY16";
	case BTN_TRIGGER_HAPPY17:
		return "BTN_TRIGGER_HAPPY17";
	case BTN_TRIGGER_HAPPY18:
		return "BTN_TRIGGER_HAPPY18";
	case BTN_TRIGGER_HAPPY19:
		return "BTN_TRIGGER_HAPPY19";
	case BTN_TRIGGER_HAPPY20:
		return "BTN_TRIGGER_HAPPY20";
	case BTN_TRIGGER_HAPPY21:
		return "BTN_TRIGGER_HAPPY21";
	case BTN_TRIGGER_HAPPY22:
		return "BTN_TRIGGER_HAPPY22";
	case BTN_TRIGGER_HAPPY23:
		return "BTN_TRIGGER_HAPPY23";
	case BTN_TRIGGER_HAPPY24:
		return "BTN_TRIGGER_HAPPY24";
	case BTN_TRIGGER_HAPPY25:
		return "BTN_TRIGGER_HAPPY25";
	case BTN_TRIGGER_HAPPY26:
		return "BTN_TRIGGER_HAPPY26";
	case BTN_TRIGGER_HAPPY27:
		return "BTN_TRIGGER_HAPPY27";
	case BTN_TRIGGER_HAPPY28:
		return "BTN_TRIGGER_HAPPY28";
	case BTN_TRIGGER_HAPPY29:
		return "BTN_TRIGGER_HAPPY29";
	case BTN_TRIGGER_HAPPY30:
		return "BTN_TRIGGER_HAPPY30";
	case BTN_TRIGGER_HAPPY31:
		return "BTN_TRIGGER_HAPPY31";
	case BTN_TRIGGER_HAPPY32:
		return "BTN_TRIGGER_HAPPY32";
	case BTN_TRIGGER_HAPPY33:
		return "BTN_TRIGGER_HAPPY33";
	case BTN_TRIGGER_HAPPY34:
		return "BTN_TRIGGER_HAPPY34";
	case BTN_TRIGGER_HAPPY35:
		return "BTN_TRIGGER_HAPPY35";
	case BTN_TRIGGER_HAPPY36:
		return "BTN_TRIGGER_HAPPY36";
	case BTN_TRIGGER_HAPPY37:
		return "BTN_TRIGGER_HAPPY37";
	case BTN_TRIGGER_HAPPY38:
		return "BTN_TRIGGER_HAPPY38";
	case BTN_TRIGGER_HAPPY39:
		return "BTN_TRIGGER_HAPPY39";
	case BTN_TRIGGER_HAPPY40:
		return "BTN_TRIGGER_HAPPY40";
	default:
		return NULL;
	}
}

char *cavan_event_tostring(struct cavan_event_device *dev, struct input_event *event, char *text)
{
	const char *event_code = "UNKNOWN";

	switch (event->type)
	{
	case EV_SYN:
		switch (event->code)
		{
		case SYN_REPORT:
			text_copy(text, "SYN_REPORT");
			return text;

		case SYN_CONFIG:
			text_copy(text, "SYN_CONFIG");
			return text;

		case SYN_MT_REPORT:
			text_copy(text, "SYN_MT_REPORT");
			return text;

		default:
			sprintf(text, "EV_SYN[%d] = %d", event->code, event->value);
			return text;
		}
		break;
	case EV_KEY:
		event_code = cavan_event_find_key_name_by_keylayout(dev, event->code);
		if (event_code)
		{
			sprintf(text, "KEY_%s[%d] = %d", event_code, event->code, event->value);
			return text;
		}

		event_code = cavan_event_key_code_tostring(event->code);
		if (event_code == NULL)
		{
			sprintf(text, "EV_KEY[%d] = %d", event->code, event->value);
			return text;
		}
		break;
	case EV_REL:
		switch (event->code)
		{
		case REL_X:
			event_code = "REL_X";
			break;
		case REL_Y:
			event_code = "REL_Y";
			break;
		case REL_Z:
			event_code = "REL_Z";
			break;
		case REL_RX:
			event_code = "REL_RX";
			break;
		case REL_RY:
			event_code = "REL_RY";
			break;
		case REL_RZ:
			event_code = "REL_RZ";
			break;
		case REL_HWHEEL:
			event_code = "REL_HWHEEL";
			break;
		case REL_DIAL:
			event_code = "REL_DIAL";
			break;
		case REL_WHEEL:
			event_code = "REL_WHEEL";
			break;
		case REL_MISC:
			event_code = "REL_MISC";
			break;
		case REL_MAX:
			event_code = "REL_MAX";
			break;
		default:
			sprintf(text, "EV_REL[%d] = %d", event->code, event->value);
			return text;
		}
		break;
	case EV_ABS:
		switch (event->code)
		{
		case ABS_X:
			event_code = "ABS_X";
			break;
		case ABS_Y:
			event_code = "ABS_Y";
			break;
		case ABS_Z:
			event_code = "ABS_Z";
			break;
		case ABS_RX:
			event_code = "ABS_RX";
			break;
		case ABS_RY:
			event_code = "ABS_RY";
			break;
		case ABS_RZ:
			event_code = "ABS_RZ";
			break;
		case ABS_THROTTLE:
			event_code = "ABS_THROTTLE";
			break;
		case ABS_RUDDER:
			event_code = "ABS_RUDDER";
			break;
		case ABS_WHEEL:
			event_code = "ABS_WHEEL";
			break;
		case ABS_GAS:
			event_code = "ABS_GAS";
			break;
		case ABS_BRAKE:
			event_code = "ABS_BRAKE";
			break;
		case ABS_HAT0X:
			event_code = "ABS_HAT0X";
			break;
		case ABS_HAT0Y:
			event_code = "ABS_HAT0Y";
			break;
		case ABS_HAT1X:
			event_code = "ABS_HAT1X";
			break;
		case ABS_HAT1Y:
			event_code = "ABS_HAT1Y";
			break;
		case ABS_HAT2X:
			event_code = "ABS_HAT2X";
			break;
		case ABS_HAT2Y:
			event_code = "ABS_HAT2Y";
			break;
		case ABS_HAT3X:
			event_code = "ABS_HAT3X";
			break;
		case ABS_HAT3Y:
			event_code = "ABS_HAT3Y";
			break;
		case ABS_PRESSURE:
			event_code = "ABS_PRESSURE";
			break;
		case ABS_DISTANCE:
			event_code = "ABS_DISTANCE";
			break;
		case ABS_TILT_X:
			event_code = "ABS_TILT_X";
			break;
		case ABS_TILT_Y:
			event_code = "ABS_TILT_Y";
			break;
		case ABS_TOOL_WIDTH:
			event_code = "ABS_TOOL_WIDTH";
			break;
		case ABS_VOLUME:
			event_code = "ABS_VOLUME";
			break;
		case ABS_MISC:
			event_code = "ABS_MISC";
			break;
		case ABS_MT_SLOT:
			event_code = "ABS_MT_SLOT";
			break;
		case ABS_MT_TOUCH_MAJOR:
			event_code = "ABS_MT_TOUCH_MAJOR";
			break;
		case ABS_MT_TOUCH_MINOR:
			event_code = "ABS_MT_TOUCH_MINOR";
			break;
		case ABS_MT_WIDTH_MAJOR:
			event_code = "ABS_MT_WIDTH_MAJOR";
			break;
		case ABS_MT_WIDTH_MINOR:
			event_code = "ABS_MT_WIDTH_MINOR";
			break;
		case ABS_MT_ORIENTATION:
			event_code = "ABS_MT_ORIENTATION";
			break;
		case ABS_MT_POSITION_X:
			event_code = "ABS_MT_POSITION_X";
			break;
		case ABS_MT_POSITION_Y:
			event_code = "ABS_MT_POSITION_Y";
			break;
		case ABS_MT_TOOL_TYPE:
			event_code = "ABS_MT_TOOL_TYPE";
			break;
		case ABS_MT_BLOB_ID:
			event_code = "ABS_MT_BLOB_ID";
			break;
		case ABS_MT_TRACKING_ID:
			event_code = "ABS_MT_TRACKING_ID";
			break;
		case ABS_MT_PRESSURE:
			event_code = "ABS_MT_PRESSURE";
			break;
		case ABS_MAX:
			event_code = "ABS_MAX";
			break;
		default:
			sprintf(text, "EV_ABS[%d] = %d", event->code, event->value);
			return text;
		}
		break;

	case EV_MSC:
		sprintf(text, "EV_MSC[%d] = %d", event->code, event->value);
		return text;

	case EV_SW:
		sprintf(text, "EV_SW[%d] = %d", event->code, event->value);
		return text;

	case EV_LED:
		sprintf(text, "EV_LED[%d] = %d", event->code, event->value);
		return text;

	case EV_SND:
		sprintf(text, "EV_SND[%d] = %d", event->code, event->value);
		return text;

	case EV_REP:
		sprintf(text, "EV_REP[%d] = %d", event->code, event->value);
		return text;

	case EV_FF:
		sprintf(text, "EV_FF[%d] = %d", event->code, event->value);
		return text;

	case EV_PWR:
		sprintf(text, "EV_PWR[%d] = %d", event->code, event->value);
		return text;

	case EV_FF_STATUS:
		sprintf(text, "EV_FF_STATUS[%d] = %d", event->code, event->value);
		return text;

	default:
		return cavan_event_tostring_simple(event, text);
	}

	sprintf(text, "%s[%d] = %d", event_code, event->code, event->value);

	return text;
}

// ================================================================================

static void cavan_event_virtual_build_bitmask(struct single_link *link, uint8_t *bitmask)
{
	struct cavan_virtual_key *key;

	single_link_foreach(link, key)
	{
		set_bit(key->code, bitmask);
	}
	end_link_foreach(link);
}

static int cavan_event_parse_virtual_keymap(struct cavan_event_device *dev)
{
	size_t size;
	char pathname[1024];
	const char *mem, *p, *file_end;
	struct cavan_virtual_key *key;
	int x, y, width, height, code;

	sprintf(pathname, "/sys/board_properties/virtualkeys.%s", dev->name);
	mem = file_read_all(pathname, 0, &size);
	if (mem == NULL)
	{
		// pr_red_info("file_read_all %s", pathname);
		return -ENOMEM;
	}

	pr_bold_info("Parse virtual key file %s", pathname);

	file_end = mem + size;
	p = text_skip_space(mem, file_end);

	while (p < file_end)
	{
		if (text_lhcmp("0x01", p) || sscanf(p, "0x01:%d:%d:%d:%d:%d", &code, &x, &y, &width, &height) != 5)
		{
			p++;
			continue;
		}

		key = malloc(sizeof(*key));
		if (key == NULL)
		{
			pr_error_info("malloc");
			goto out_free_mem;
		}

		pr_bold_info("code = %d, x = %d, y = %d, width = %d, height = %d", code, x, y, width, height);

		width >>= 1;
		height >>= 1;

		key->left = x - width;
		key->right = x + width - 1;
		key->top = y - height;
		key->bottom = y + height - 1;
		key->code = code;
		key->value = 0;

		single_link_push(&dev->vk_link, &key->node);

		p = text_nfind2(p, file_end, ':', 5);
	}

out_free_mem:
	free((char *) mem);

	return 0;
}

static char *cavan_event_get_keylayout_pathname(struct cavan_event_device *dev, char *pathname)
{
	int i;
	char *filename;
	char dev_name[2][512];
	const char *filenames[] =
	{dev_name[0], dev_name[1], "Generic", "qwerty"};

	text_replace_char2(dev->name, dev_name[0], ' ', '_');
	text_replace_char2(dev->name, dev_name[1], ' ', '-');

	filename = text_copy(pathname, "/system/usr/keylayout/");
	for (i = 0; i < (int) NELEM(filenames); i++)
	{
		sprintf(filename, "%s.kl", filenames[i]);
		if (access(pathname, R_OK) == 0)
		{
			return pathname;
		}
	}

	return NULL;
}

const char *cavan_event_find_key_name_by_keylayout_base(struct single_link *link, int code)
{
	struct cavan_keylayout_node *key;

	single_link_foreach(link, key)
	{
		if (key->code == code)
		{
			link_foreach_return(link, key->name);
		}
	}
	end_link_foreach(link);

	return NULL;
}

const char *cavan_event_find_key_name_base(struct single_link *link, int code)
{
	const char *name;

	name = cavan_event_find_key_name_by_keylayout_base(link, code);
	if (name == NULL)
	{
		name = cavan_event_key_code_tostring(code);
		if (name)
		{
			name += 4;
		}
		else
		{
			name = "NONE";
		}
	}

	return name;
}

static void cavan_event_virtual_key_set_name(struct single_link *vk_link, struct single_link *kl_link)
{
	struct cavan_virtual_key *key;

	single_link_foreach(vk_link, key)
	{
		key->name = cavan_event_find_key_name_base(kl_link, key->code);
	}
	end_link_foreach(vk_link);
}

static int cavan_event_parse_keylayout(struct cavan_event_device *dev)
{
	int ret;
	char *mem;
	size_t size;
	char pathname[1024];
	uint8_t key_bitmask[KEY_BITMASK_SIZE];
	const char *p, *line_end, *file_end;
	struct cavan_keylayout_node *node;

	if (cavan_event_get_keylayout_pathname(dev, pathname) == NULL)
	{
		return -ENOENT;
	}

	memset(key_bitmask, 0, sizeof(key_bitmask));

	ret = ioctl(dev->fd, EVIOCGBIT(EV_KEY, sizeof(key_bitmask)), key_bitmask);
	if (ret < 0)
	{
		pr_error_info("ioctl EVIOCGBIT EV_KEY");
		return ret;
	}

	mem = file_read_all(pathname, 1, &size);
	if (mem == NULL)
	{
		pr_red_info("file_read_all %s", pathname);
		return -EFAULT;
	}

	mem[size] = 0;

	pr_bold_info("Parse keylayout file %s", pathname);

	p = mem;
	file_end = p + size;

	node = malloc(sizeof(*node));
	if (node == NULL)
	{
		pr_error_info("malloc");
		goto out_free_memory;
	}

	cavan_event_virtual_build_bitmask(&dev->vk_link, key_bitmask);

	while (p < file_end)
	{
		line_end = text_find_lf(p, file_end);
		if (line_end == NULL)
		{
			line_end = file_end;
		}

		p = text_skip_space(p, line_end);
		if (p == line_end || *p == '#')
		{
			goto label_goto_next_line;
		}

		ret = sscanf(p, "key %d %s", &node->code, node->name);
		if (ret != 2 || test_bit(node->code, key_bitmask) == 0)
		{
			goto label_goto_next_line;
		}

		pr_bold_info("name = %s, code = %d", node->name, node->code);

		single_link_push(&dev->kl_link, &node->node);

		node = malloc(sizeof(*node));
		if (node == NULL)
		{
			pr_error_info("malloc");
			goto label_goto_next_line;
		}

label_goto_next_line:
		p = text_skip_lf(line_end, file_end);
	}

	free(node);

	cavan_event_virtual_key_set_name(&dev->vk_link, &dev->kl_link);

out_free_memory:
	free(mem);

	return 0;
}

static void cavan_event_close_device_handler(struct double_link *link, struct double_link_node *node, void *data)
{
	struct cavan_event_service *service = data;
	struct cavan_event_device *dev = double_link_get_container(link, node);

	if (service->remove)
	{
		service->remove(dev, service->private_data);
	}

	close(dev->fd);

	single_link_free_all(&dev->kl_link);
	single_link_free_all(&dev->vk_link);

	free(dev);
}

static void cavan_event_close_devices(struct cavan_event_service *service)
{
	pthread_mutex_lock(&service->lock);
	double_link_destroy(&service->link, service, cavan_event_close_device_handler);
	pthread_mutex_unlock(&service->lock);
}

ssize_t cavan_event_scan_devices(struct cavan_event_matcher *matcher, void *data)
{
	int fd;
	int ret;
	DIR *dp;
	struct dirent *entry;
	char *filename;
	size_t count;

	if (matcher == NULL || matcher->handler == NULL)
	{
		pr_red_info("matcher == NULL || matcher->handler == NULL");
		return -EINVAL;
	}

	filename = text_copy(matcher->pathname, "/dev/input/");
	dp = opendir(matcher->pathname);
	if (dp == NULL)
	{
		pr_error_info("open directory `%s'", matcher->pathname);
		return -ENOENT;
	}

	count = 0;

	while ((entry = readdir(dp)))
	{
		if (text_lhcmp("event", entry->d_name))
		{
			continue;
		}

		pr_info("============================================================");

		text_copy(filename, entry->d_name);
		fd = open(matcher->pathname, O_RDONLY);
		if (fd < 0)
		{
			pr_error_info("open file `%s'", matcher->pathname);
			continue;
		}

		ret = cavan_event_get_devname(fd, matcher->devname, sizeof(matcher->devname));
		if (ret < 0)
		{
			pr_error_info("cavan_event_get_devname");
			close(fd);
			continue;
		}

		matcher->fd = fd;
		if (matcher->match && matcher->match(matcher, data) == false)
		{
			pr_red_info("Can't match device `%s', path = %s", matcher->devname, matcher->pathname);
			close(fd);
			continue;
		}

		ret = matcher->handler(matcher, data);
		if (ret < 0)
		{
			pr_red_info("Handler device `%s', path = %s", matcher->devname, matcher->pathname);
			close(fd);
			continue;
		}

		count++;
	}

	closedir(dp);

	return count;
}

static bool cavan_event_service_match(struct cavan_event_matcher *matcher, void *data)
{
	struct cavan_event_service *service = data;

	if (service->matcher)
	{
		return service->matcher(matcher, service->private_data);
	}

	return true;
}

static int cavan_event_service_match_handler(struct cavan_event_matcher *matcher, void *data)
{
	int ret;
	struct cavan_event_device *dev;
	struct cavan_event_service *service = data;

	dev = malloc(sizeof(*dev));
	if (dev == NULL)
	{
		pr_error_info("malloc");
		return -ENOMEM;
	}

	ret = single_link_init(&dev->vk_link, MOFS(struct cavan_virtual_key, node));
	if (ret < 0)
	{
		pr_red_info("single_link_init");
		goto out_free_dev;
	}

	ret = single_link_init(&dev->kl_link, MOFS(struct cavan_keylayout_node, node));
	if (ret < 0)
	{
		pr_red_info("single_link_init");
		goto out_single_link_deinit_vk;
	}

	dev->fd = matcher->fd;
	text_ncopy(dev->name, matcher->devname, sizeof(dev->name));
	text_ncopy(dev->pathname, matcher->pathname, sizeof(dev->pathname));

	if (service->probe && service->probe(dev, service->private_data) < 0)
	{
		ret = -EFAULT;
		pr_red_info("Faile to probe device %s, name = %s", matcher->pathname, matcher->devname);
		goto out_single_link_deinit_kl;
	}

	pr_green_info("Add device %s, name = %s", matcher->pathname, matcher->devname);

	cavan_event_parse_virtual_keymap(dev);
	cavan_event_parse_keylayout(dev);

	double_link_append(&service->link, &dev->node);

	return 0;

out_single_link_deinit_kl:
	single_link_deinit(&dev->kl_link);
out_single_link_deinit_vk:
	single_link_deinit(&dev->vk_link);
out_free_dev:
	free(dev);
	return ret;
}

static int cavan_event_open_devices(struct cavan_event_service *service)
{
	ssize_t count;
	struct cavan_event_matcher matcher =
	{
		.match = cavan_event_service_match,
		.handler = cavan_event_service_match_handler
	};

	count = cavan_event_scan_devices(&matcher, service);
	if (count < 0)
	{
		pr_red_info("cavan_event_scan_devices");
	}

	return count;
}

static int cavan_event_service_handler(struct cavan_thread *thread, void *data)
{
	int ret;
	ssize_t rdlen;
	struct cavan_event_service *service = data;
	struct pollfd *pfds = service->pfds;
	struct cavan_event_device *pdev;
	struct double_link *link;
	struct double_link_node *node, *head;
	struct input_event events[32], *ep, *ep_end;

	ret = poll(pfds, service->pfd_count, -1);
	if (ret < 0)
	{
		pr_error_info("poll");
		return ret;
	}

	if (pfds[0].revents)
	{
		return 0;
	}

	link = &service->link;

	pthread_mutex_lock(&link->lock);

	for (head = &link->head_node, node = head->next; node != head; node = node->next)
	{
		pdev = double_link_get_container(link, node);
		if (pdev->pfd->revents == 0)
		{
			continue;
		}

		rdlen = read(pdev->fd, events, sizeof(events));
		if (rdlen < 0)
		{
			pr_error_info("read");
			pthread_mutex_unlock(&link->lock);
			return rdlen;
		}

		for (ep = events, ep_end = ep + (rdlen / sizeof(events[0])); ep < ep_end; ep++)
		{
			if (service->event_handler(pdev, ep, service->private_data) == false)
			{
				char print_buff[1024];

				pr_red_info("%s", cavan_event_tostring(pdev, ep, print_buff));
			}
		}
	}

	pthread_mutex_unlock(&link->lock);

	return 0;
}

struct cavan_virtual_key *cavan_event_find_virtual_key(struct cavan_event_device *dev, int x, int y)
{
	struct cavan_virtual_key *key;

	single_link_foreach(&dev->vk_link, key)
	{
		if (y >= key->top && y <= key->bottom && x >= key->left && x <= key->right)
		{
			link_foreach_return(&dev->vk_link, key);
		}
	}
	end_link_foreach(&dev->vk_link);

	return NULL;
}

void cavan_event_service_init(struct cavan_event_service *service, bool (*matcher)(struct cavan_event_matcher *, void *))
{
	service->matcher = matcher;
	service->probe = NULL;
	service->remove = NULL;
	service->event_handler = NULL;
}

static bool cavan_event_handler_dummy(struct cavan_event_device *dev, struct input_event *event, void *data)
{
	char buff[1024];

	print_string(cavan_event_tostring(dev, event, buff));

	return true;
}

int cavan_event_service_start(struct cavan_event_service *service, void *data)
{
	int ret;
	ssize_t count;
	struct double_link *link;
	struct double_link_node *node;
	struct cavan_event_device *dev;
	struct pollfd *pfd, *pfd_end;
	struct cavan_thread *thread;

	if (service == NULL)
	{
		pr_red_info("service == NULL");
		return -EINVAL;
	}

	if (service->event_handler == NULL)
	{
		service->event_handler = cavan_event_handler_dummy;
	}

	service->private_data = data;
	pthread_mutex_init(&service->lock, NULL);

	ret = double_link_init(&service->link, MOFS(struct cavan_event_device, node));
	if (ret < 0)
	{
		pr_red_info("double_link_init");
		return ret;
	}

	count = cavan_event_open_devices(service);
	if (count < 0)
	{
		ret = count;
		pr_red_info("cavan_event_open_devices");
		goto out_double_link_deinit;
	}

	if (count == 0)
	{
		ret = -ENOENT;
		pr_red_info("No input device found!");
		goto out_double_link_deinit;
	}

	thread = &service->thread;
	thread->name = "EVENT";
	thread->wake_handker = NULL;
	thread->handler = cavan_event_service_handler;
	ret = cavan_thread_init(thread, service);
	if (ret < 0)
	{
		pr_red_info("cavan_thread_init");
		goto out_cavan_event_close_devices;
	}

	service->pfd_count = count + 1;
	pfd = malloc(service->pfd_count * sizeof(*pfd));
	if (pfd == NULL)
	{
		ret = -ENOMEM;
		pr_error_info("malloc");
		goto out_cavan_thread_deinit;
	}

	service->pfds = pfd;

	*pfd = service->thread.pfd;
	link = &service->link;

	pthread_mutex_lock(&link->lock);

	for (pfd++, pfd_end = pfd + count, node = link->head_node.next; pfd < pfd_end; pfd++, node = node->next)
	{
		dev = double_link_get_container(link, node);

		dev->pfd = pfd;
		pfd->fd = dev->fd;
		pfd->events = POLLIN;
		pfd->revents = 0;
	}

	pthread_mutex_unlock(&link->lock);

	ret = cavan_event_start_poll_thread(service);
	if (ret < 0)
	{
		pr_red_info("cavan_event_start_poll_thread");
		goto out_free_pfd;
	}

	return 0;

out_free_pfd:
	free(pfd);
out_cavan_thread_deinit:
	cavan_thread_deinit(thread);
out_cavan_event_close_devices:
	cavan_event_close_devices(service);
out_double_link_deinit:
	double_link_deinit(&service->link);
	return ret;
}

void cavan_event_service_stop(struct cavan_event_service *service)
{
	cavan_event_stop_poll_thread(service);
	free(service->pfds);
	cavan_event_close_devices(service);
	cavan_thread_deinit(&service->thread);
	pthread_mutex_destroy(&service->lock);
}

bool cavan_event_simple_matcher(struct cavan_event_matcher *matcher, void *data)
{
	if (data)
	{
		return text_cmp(matcher->devname, data) == 0 || text_cmp(matcher->pathname, data) == 0;
	}

	return true;
}

bool cavan_event_name_matcher(const char *devname, ...)
{
	va_list ap;
	const char *name;

	va_start(ap, devname);

	while (1)
	{
		name = va_arg(ap, const char *);
		if (name == NULL)
		{
			break;
		}

		if (strcmp(devname, name) == 0)
		{
			break;
		}
	}

	va_end(ap);

	return name ? true : false;
}

int cavan_event_get_absinfo(int fd, int axis, int *min, int *max)
{
	int ret;
	struct input_absinfo info;

	ret = ioctl(fd, EVIOCGABS(axis), &info);
	if (ret < 0)
	{
		pr_error_info("ioctl EVIOCGABS");
		return ret;
	}

	*min = info.minimum;
	*max = info.maximum;

	return 0;
}
