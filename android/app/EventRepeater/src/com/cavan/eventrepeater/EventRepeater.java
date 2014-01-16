package com.cavan.eventrepeater;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.HashMap;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;

import android.app.Instrumentation;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemClock;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Display;
import android.view.IWindowManager;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.WindowManager;

public class EventRepeater extends Service {
	private static final String TAG = "Cavan";
	private static final int MIN_DAEMON_COUNT = 10;
	public static final String ACTION_STATE_CHANGED = "com.cavan.eventrepeater.STATE_CHANGED";
	private static HashMap<String, Integer> mHashMapKeyName = new HashMap<String, Integer>();
	private static HashMap<Integer, Integer> mHashMapKeyCode = new HashMap<Integer, Integer>();

	private static final int CAVAN_INPUT_MESSAGE_KEY = 0;
	private static final int CAVAN_INPUT_MESSAGE_MOVE = 1;
	private static final int CAVAN_INPUT_MESSAGE_TOUCH = 2;
	private static final int CAVAN_INPUT_MESSAGE_WHEEL = 3;
	private static final int CAVAN_INPUT_MESSAGE_MOUSE_MOVE = 4;
	private static final int CAVAN_INPUT_MESSAGE_MOUSE_TOUCH = 5;
	private static final int CAVAN_INPUT_MESSAGE_ACCELEROMETER = 6;
	private static final int CAVAN_INPUT_MESSAGE_MAGNETIC_FIELD = 7;
	private static final int CAVAN_INPUT_MESSAGE_ORIENTATION = 8;
	private static final int CAVAN_INPUT_MESSAGE_GYROSCOPE = 9;
	private static final int CAVAN_INPUT_MESSAGE_LIGHT = 10;
	private static final int CAVAN_INPUT_MESSAGE_PRESSURE = 11;
	private static final int CAVAN_INPUT_MESSAGE_TEMPERATURE = 12;
	private static final int CAVAN_INPUT_MESSAGE_PROXIMITY = 13;
	private static final int CAVAN_INPUT_MESSAGE_GRAVITY = 14;
	private static final int CAVAN_INPUT_MESSAGE_LINEAR_ACCELERATION = 15;
	private static final int CAVAN_INPUT_MESSAGE_ROTATION_VECTOR = 16;

	public static final int STATE_ERROR = -1;
	public static final int STATE_STOPPED = 0;
	public static final int STATE_STOPPING = 1;
	public static final int STATE_START = 2;
	public static final int STATE_RUNNING = 3;

	private ServerSocket mServerSocket;
	private int mDaemonCount;
	private int mDaemonUsed;
	int x;
	int y;
	int xres;
	int yres;

	private IWindowManager mWindowManager;

	private static final Pattern mPatternKey = Pattern.compile("key: name = (.*), code = (.*), value = (.*)");
	private static final Pattern mPatternMouseMove = Pattern.compile("mouse_move = \\[(.*), (.*)\\]");
	private static final Pattern mPatternMouseTouch = Pattern.compile("mouse_touch\\[(.*)\\] = (.*)");

	static {
	    mHashMapKeyName.put("UNKNOWN", KeyEvent.KEYCODE_UNKNOWN);
	    mHashMapKeyName.put("SOFT_LEFT", KeyEvent.KEYCODE_SOFT_LEFT);
	    mHashMapKeyName.put("SOFT_RIGHT", KeyEvent.KEYCODE_SOFT_RIGHT);
	    mHashMapKeyName.put("HOME", KeyEvent.KEYCODE_HOME);
	    mHashMapKeyName.put("BACK", KeyEvent.KEYCODE_BACK);
	    mHashMapKeyName.put("CALL", KeyEvent.KEYCODE_CALL);
	    mHashMapKeyName.put("ENDCALL", KeyEvent.KEYCODE_ENDCALL);
	    mHashMapKeyName.put("0", KeyEvent.KEYCODE_0);
	    mHashMapKeyName.put("1", KeyEvent.KEYCODE_1);
	    mHashMapKeyName.put("2", KeyEvent.KEYCODE_2);
	    mHashMapKeyName.put("3", KeyEvent.KEYCODE_3);
	    mHashMapKeyName.put("4", KeyEvent.KEYCODE_4);
	    mHashMapKeyName.put("5", KeyEvent.KEYCODE_5);
	    mHashMapKeyName.put("6", KeyEvent.KEYCODE_6);
	    mHashMapKeyName.put("7", KeyEvent.KEYCODE_7);
	    mHashMapKeyName.put("8", KeyEvent.KEYCODE_8);
	    mHashMapKeyName.put("9", KeyEvent.KEYCODE_9);
	    mHashMapKeyName.put("STAR", KeyEvent.KEYCODE_STAR);
	    mHashMapKeyName.put("POUND", KeyEvent.KEYCODE_POUND);
	    mHashMapKeyName.put("DPAD_UP", KeyEvent.KEYCODE_DPAD_UP);
	    mHashMapKeyName.put("DPAD_DOWN", KeyEvent.KEYCODE_DPAD_DOWN);
	    mHashMapKeyName.put("DPAD_LEFT", KeyEvent.KEYCODE_DPAD_LEFT);
	    mHashMapKeyName.put("DPAD_RIGHT", KeyEvent.KEYCODE_DPAD_RIGHT);
	    mHashMapKeyName.put("DPAD_CENTER", KeyEvent.KEYCODE_DPAD_CENTER);
	    mHashMapKeyName.put("VOLUME_UP", KeyEvent.KEYCODE_VOLUME_UP);
	    mHashMapKeyName.put("VOLUME_DOWN", KeyEvent.KEYCODE_VOLUME_DOWN);
	    mHashMapKeyName.put("POWER", KeyEvent.KEYCODE_POWER);
	    mHashMapKeyName.put("CAMERA", KeyEvent.KEYCODE_CAMERA);
	    mHashMapKeyName.put("CLEAR", KeyEvent.KEYCODE_CLEAR);
	    mHashMapKeyName.put("A", KeyEvent.KEYCODE_A);
	    mHashMapKeyName.put("B", KeyEvent.KEYCODE_B);
	    mHashMapKeyName.put("C", KeyEvent.KEYCODE_C);
	    mHashMapKeyName.put("D", KeyEvent.KEYCODE_D);
	    mHashMapKeyName.put("E", KeyEvent.KEYCODE_E);
	    mHashMapKeyName.put("F", KeyEvent.KEYCODE_F);
	    mHashMapKeyName.put("G", KeyEvent.KEYCODE_G);
	    mHashMapKeyName.put("H", KeyEvent.KEYCODE_H);
	    mHashMapKeyName.put("I", KeyEvent.KEYCODE_I);
	    mHashMapKeyName.put("J", KeyEvent.KEYCODE_J);
	    mHashMapKeyName.put("K", KeyEvent.KEYCODE_K);
	    mHashMapKeyName.put("L", KeyEvent.KEYCODE_L);
	    mHashMapKeyName.put("M", KeyEvent.KEYCODE_M);
	    mHashMapKeyName.put("O", KeyEvent.KEYCODE_O);
	    mHashMapKeyName.put("P", KeyEvent.KEYCODE_P);
	    mHashMapKeyName.put("Q", KeyEvent.KEYCODE_Q);
	    mHashMapKeyName.put("R", KeyEvent.KEYCODE_R);
	    mHashMapKeyName.put("S", KeyEvent.KEYCODE_S);
	    mHashMapKeyName.put("T", KeyEvent.KEYCODE_T);
	    mHashMapKeyName.put("U", KeyEvent.KEYCODE_U);
	    mHashMapKeyName.put("V", KeyEvent.KEYCODE_V);
	    mHashMapKeyName.put("W", KeyEvent.KEYCODE_W);
	    mHashMapKeyName.put("X", KeyEvent.KEYCODE_X);
	    mHashMapKeyName.put("Y", KeyEvent.KEYCODE_Y);
	    mHashMapKeyName.put("Z", KeyEvent.KEYCODE_Z);
	    mHashMapKeyName.put("COMMA", KeyEvent.KEYCODE_COMMA);
	    mHashMapKeyName.put("PERIOD", KeyEvent.KEYCODE_PERIOD);
	    mHashMapKeyName.put("ALT_LEFT", KeyEvent.KEYCODE_ALT_LEFT);
	    mHashMapKeyName.put("ALT_RIGHT", KeyEvent.KEYCODE_ALT_RIGHT);
	    mHashMapKeyName.put("SHIFT_LEFT", KeyEvent.KEYCODE_SHIFT_LEFT);
	    mHashMapKeyName.put("SHIFT_RIGHT", KeyEvent.KEYCODE_SHIFT_RIGHT);
	    mHashMapKeyName.put("TAB", KeyEvent.KEYCODE_TAB);
	    mHashMapKeyName.put("SPACE", KeyEvent.KEYCODE_SPACE);
	    mHashMapKeyName.put("SYM", KeyEvent.KEYCODE_SYM);
	    mHashMapKeyName.put("EXPLORER", KeyEvent.KEYCODE_EXPLORER);
	    mHashMapKeyName.put("ENVELOPE", KeyEvent.KEYCODE_ENVELOPE);
	    mHashMapKeyName.put("ENTER", KeyEvent.KEYCODE_ENTER);
	    mHashMapKeyName.put("DEL", KeyEvent.KEYCODE_DEL);
	    mHashMapKeyName.put("GRAVE", KeyEvent.KEYCODE_GRAVE);
	    mHashMapKeyName.put("MINUS", KeyEvent.KEYCODE_MINUS);
	    mHashMapKeyName.put("EQUALS", KeyEvent.KEYCODE_EQUALS);
	    mHashMapKeyName.put("LEFT_BRACKET", KeyEvent.KEYCODE_LEFT_BRACKET);
	    mHashMapKeyName.put("RIGHT_BRACKET", KeyEvent.KEYCODE_RIGHT_BRACKET);
	    mHashMapKeyName.put("BACKSLASH", KeyEvent.KEYCODE_BACKSLASH);
	    mHashMapKeyName.put("SEMICOLON", KeyEvent.KEYCODE_SEMICOLON);
	    mHashMapKeyName.put("APOSTROPHE", KeyEvent.KEYCODE_APOSTROPHE);
	    mHashMapKeyName.put("SLASH", KeyEvent.KEYCODE_SLASH);
	    mHashMapKeyName.put("AT", KeyEvent.KEYCODE_AT);
	    mHashMapKeyName.put("NUM", KeyEvent.KEYCODE_NUM);
	    mHashMapKeyName.put("HEADSETHOOK", KeyEvent.KEYCODE_HEADSETHOOK);
	    mHashMapKeyName.put("FOCUS", KeyEvent.KEYCODE_FOCUS);
	    mHashMapKeyName.put("PLUS", KeyEvent.KEYCODE_PLUS);
	    mHashMapKeyName.put("MENU", KeyEvent.KEYCODE_MENU);
	    mHashMapKeyName.put("NOTIFICATION", KeyEvent.KEYCODE_NOTIFICATION);
	    mHashMapKeyName.put("SEARCH", KeyEvent.KEYCODE_SEARCH);
	    mHashMapKeyName.put("MEDIA_PLAY_PAUSE", KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE);
	    mHashMapKeyName.put("MEDIA_STOP", KeyEvent.KEYCODE_MEDIA_STOP);
	    mHashMapKeyName.put("MEDIA_NEXT", KeyEvent.KEYCODE_MEDIA_NEXT);
	    mHashMapKeyName.put("MEDIA_PREVIOUS", KeyEvent.KEYCODE_MEDIA_PREVIOUS);
	    mHashMapKeyName.put("MEDIA_REWIND", KeyEvent.KEYCODE_MEDIA_REWIND);
	    mHashMapKeyName.put("MEDIA_FAST_FORWARD", KeyEvent.KEYCODE_MEDIA_FAST_FORWARD);
	    mHashMapKeyName.put("MUTE", KeyEvent.KEYCODE_MUTE);
	    mHashMapKeyName.put("PAGE_UP", KeyEvent.KEYCODE_PAGE_UP);
	    mHashMapKeyName.put("PAGE_DOWN", KeyEvent.KEYCODE_PAGE_DOWN);
	    mHashMapKeyName.put("PICTSYMBOLS", KeyEvent.KEYCODE_PICTSYMBOLS);
	    mHashMapKeyName.put("SWITCH_CHARSET", KeyEvent.KEYCODE_SWITCH_CHARSET);
	    mHashMapKeyName.put("BUTTON_A", KeyEvent.KEYCODE_BUTTON_A);
	    mHashMapKeyName.put("BUTTON_B", KeyEvent.KEYCODE_BUTTON_B);
	    mHashMapKeyName.put("BUTTON_C", KeyEvent.KEYCODE_BUTTON_C);
	    mHashMapKeyName.put("BUTTON_X", KeyEvent.KEYCODE_BUTTON_X);
	    mHashMapKeyName.put("BUTTON_Y", KeyEvent.KEYCODE_BUTTON_Y);
	    mHashMapKeyName.put("BUTTON_Z", KeyEvent.KEYCODE_BUTTON_Z);
	    mHashMapKeyName.put("BUTTON_L1", KeyEvent.KEYCODE_BUTTON_L1);
	    mHashMapKeyName.put("BUTTON_R1", KeyEvent.KEYCODE_BUTTON_R1);
	    mHashMapKeyName.put("BUTTON_L2", KeyEvent.KEYCODE_BUTTON_L2);
	    mHashMapKeyName.put("BUTTON_R2", KeyEvent.KEYCODE_BUTTON_R2);
	    mHashMapKeyName.put("BUTTON_THUMBL", KeyEvent.KEYCODE_BUTTON_THUMBL);
	    mHashMapKeyName.put("BUTTON_THUMBR", KeyEvent.KEYCODE_BUTTON_THUMBR);
	    mHashMapKeyName.put("BUTTON_START", KeyEvent.KEYCODE_BUTTON_START);
	    mHashMapKeyName.put("BUTTON_SELECT", KeyEvent.KEYCODE_BUTTON_SELECT);
	    mHashMapKeyName.put("BUTTON_MODE", KeyEvent.KEYCODE_BUTTON_MODE);

	    parseKeyLayoutFile("/system/usr/keylayout/qwerty.kl");
	    parseKeyLayoutFile("/system/usr/keylayout/AVRCP.kl");
	    // android.R.attr.keycode

		// mHashMapKeyCode.put(1, KeyEvent.KEYCODE_ESCAPE);
		mHashMapKeyCode.put(2, KeyEvent.KEYCODE_1);
		mHashMapKeyCode.put(3, KeyEvent.KEYCODE_2);
		mHashMapKeyCode.put(4, KeyEvent.KEYCODE_3);
		mHashMapKeyCode.put(5, KeyEvent.KEYCODE_4);
		mHashMapKeyCode.put(6, KeyEvent.KEYCODE_5);
		mHashMapKeyCode.put(7, KeyEvent.KEYCODE_6);
		mHashMapKeyCode.put(8, KeyEvent.KEYCODE_7);
		mHashMapKeyCode.put(9, KeyEvent.KEYCODE_8);
		mHashMapKeyCode.put(10, KeyEvent.KEYCODE_9);
		mHashMapKeyCode.put(11, KeyEvent.KEYCODE_0);
		mHashMapKeyCode.put(12, KeyEvent.KEYCODE_MINUS);
		mHashMapKeyCode.put(13, KeyEvent.KEYCODE_EQUALS);
		// mHashMapKeyCode.put(14, KeyEvent.KEYCODE_BACKSPACE);
		mHashMapKeyCode.put(15, KeyEvent.KEYCODE_TAB);
		mHashMapKeyCode.put(16, KeyEvent.KEYCODE_Q);
		mHashMapKeyCode.put(17, KeyEvent.KEYCODE_W);
		mHashMapKeyCode.put(18, KeyEvent.KEYCODE_E);
		mHashMapKeyCode.put(19, KeyEvent.KEYCODE_R);
		mHashMapKeyCode.put(20, KeyEvent.KEYCODE_T);
		mHashMapKeyCode.put(21, KeyEvent.KEYCODE_Y);
		mHashMapKeyCode.put(22, KeyEvent.KEYCODE_U);
		mHashMapKeyCode.put(23, KeyEvent.KEYCODE_I);
		mHashMapKeyCode.put(24, KeyEvent.KEYCODE_O);
		mHashMapKeyCode.put(25, KeyEvent.KEYCODE_P);
		// mHashMapKeyCode.put(26, KeyEvent.KEYCODE_LEFTBRACE);
		// mHashMapKeyCode.put(27, KeyEvent.KEYCODE_RIGHTBRACE);
		mHashMapKeyCode.put(28, KeyEvent.KEYCODE_ENTER);
		// mHashMapKeyCode.put(29, KeyEvent.KEYCODE_LEFTCTRL);
		mHashMapKeyCode.put(30, KeyEvent.KEYCODE_A);
		mHashMapKeyCode.put(31, KeyEvent.KEYCODE_S);
		mHashMapKeyCode.put(32, KeyEvent.KEYCODE_D);
		mHashMapKeyCode.put(33, KeyEvent.KEYCODE_F);
		mHashMapKeyCode.put(34, KeyEvent.KEYCODE_G);
		mHashMapKeyCode.put(35, KeyEvent.KEYCODE_H);
		mHashMapKeyCode.put(36, KeyEvent.KEYCODE_J);
		mHashMapKeyCode.put(37, KeyEvent.KEYCODE_K);
		mHashMapKeyCode.put(38, KeyEvent.KEYCODE_L);
		mHashMapKeyCode.put(39, KeyEvent.KEYCODE_SEMICOLON);
		mHashMapKeyCode.put(40, KeyEvent.KEYCODE_APOSTROPHE);
		mHashMapKeyCode.put(41, KeyEvent.KEYCODE_GRAVE);
		// mHashMapKeyCode.put(42, KeyEvent.KEYCODE_LEFTSHIFT);
		mHashMapKeyCode.put(43, KeyEvent.KEYCODE_BACKSLASH);
		mHashMapKeyCode.put(44, KeyEvent.KEYCODE_Z);
		mHashMapKeyCode.put(45, KeyEvent.KEYCODE_X);
		mHashMapKeyCode.put(46, KeyEvent.KEYCODE_C);
		mHashMapKeyCode.put(47, KeyEvent.KEYCODE_V);
		mHashMapKeyCode.put(48, KeyEvent.KEYCODE_B);
		mHashMapKeyCode.put(49, KeyEvent.KEYCODE_N);
		mHashMapKeyCode.put(50, KeyEvent.KEYCODE_M);
		mHashMapKeyCode.put(51, KeyEvent.KEYCODE_COMMA);
		// mHashMapKeyCode.put(52, KeyEvent.KEYCODE_DOT);
		mHashMapKeyCode.put(53, KeyEvent.KEYCODE_SLASH);
		// mHashMapKeyCode.put(54, KeyEvent.KEYCODE_RIGHTSHIFT);
		// mHashMapKeyCode.put(55, KeyEvent.KEYCODE_KPASTERISK);
		// mHashMapKeyCode.put(56, KeyEvent.KEYCODE_LEFTALT);
		mHashMapKeyCode.put(57, KeyEvent.KEYCODE_SPACE);
		// mHashMapKeyCode.put(58, KeyEvent.KEYCODE_CAPSLOCK);
		// mHashMapKeyCode.put(59, KeyEvent.KEYCODE_F1);
		// mHashMapKeyCode.put(60, KeyEvent.KEYCODE_F2);
		// mHashMapKeyCode.put(61, KeyEvent.KEYCODE_F3);
		// mHashMapKeyCode.put(62, KeyEvent.KEYCODE_F4);
		// mHashMapKeyCode.put(63, KeyEvent.KEYCODE_F5);
		// mHashMapKeyCode.put(64, KeyEvent.KEYCODE_F6);
		// mHashMapKeyCode.put(65, KeyEvent.KEYCODE_F7);
		// mHashMapKeyCode.put(66, KeyEvent.KEYCODE_F8);
		mHashMapKeyCode.put(67, KeyEvent.KEYCODE_SEARCH); // KeyEvent.KEYCODE_F9);
		mHashMapKeyCode.put(68, KeyEvent.KEYCODE_MENU); // KeyEvent.KEYCODE_F10);
		// mHashMapKeyCode.put(69, KeyEvent.KEYCODE_NUMLOCK);
		// mHashMapKeyCode.put(70, KeyEvent.KEYCODE_SCROLLLOCK);
		// mHashMapKeyCode.put(71, KeyEvent.KEYCODE_KP7);
		// mHashMapKeyCode.put(72, KeyEvent.KEYCODE_KP8);
		// mHashMapKeyCode.put(73, KeyEvent.KEYCODE_KP9);
		// mHashMapKeyCode.put(74, KeyEvent.KEYCODE_KPMINUS);
		// mHashMapKeyCode.put(75, KeyEvent.KEYCODE_KP4);
		// mHashMapKeyCode.put(76, KeyEvent.KEYCODE_KP5);
		// mHashMapKeyCode.put(77, KeyEvent.KEYCODE_KP6);
		// mHashMapKeyCode.put(78, KeyEvent.KEYCODE_KPPLUS);
		// mHashMapKeyCode.put(79, KeyEvent.KEYCODE_KP1);
		// mHashMapKeyCode.put(80, KeyEvent.KEYCODE_KP2);
		// mHashMapKeyCode.put(81, KeyEvent.KEYCODE_KP3);
		// mHashMapKeyCode.put(82, KeyEvent.KEYCODE_KP0);
		// mHashMapKeyCode.put(83, KeyEvent.KEYCODE_KPDOT);
		// mHashMapKeyCode.put(85, KeyEvent.KEYCODE_ZENKAKUHANKAKU);
		// mHashMapKeyCode.put(86, KeyEvent.KEYCODE_102ND);
		mHashMapKeyCode.put(87, KeyEvent.KEYCODE_HOME); // KeyEvent.KEYCODE_F11);
		mHashMapKeyCode.put(88, KeyEvent.KEYCODE_BACK); // KeyEvent.KEYCODE_F12);
		// mHashMapKeyCode.put(89, KeyEvent.KEYCODE_RO);
		// mHashMapKeyCode.put(90, KeyEvent.KEYCODE_KATAKANA);
		// mHashMapKeyCode.put(91, KeyEvent.KEYCODE_HIRAGANA);
		// mHashMapKeyCode.put(92, KeyEvent.KEYCODE_HENKAN);
		// mHashMapKeyCode.put(93, KeyEvent.KEYCODE_KATAKANAHIRAGANA);
		// mHashMapKeyCode.put(94, KeyEvent.KEYCODE_MUHENKAN);
		// mHashMapKeyCode.put(95, KeyEvent.KEYCODE_KPJPCOMMA);
		// mHashMapKeyCode.put(96, KeyEvent.KEYCODE_KPENTER);
		// mHashMapKeyCode.put(97, KeyEvent.KEYCODE_RIGHTCTRL);
		// mHashMapKeyCode.put(98, KeyEvent.KEYCODE_KPSLASH);
		// mHashMapKeyCode.put(99, KeyEvent.KEYCODE_SYSRQ);
		// mHashMapKeyCode.put(100, KeyEvent.KEYCODE_RIGHTALT);
		// mHashMapKeyCode.put(101, KeyEvent.KEYCODE_LINEFEED);
		mHashMapKeyCode.put(102, KeyEvent.KEYCODE_HOME);
		// mHashMapKeyCode.put(103, KeyEvent.KEYCODE_UP);
		mHashMapKeyCode.put(104, KeyEvent.KEYCODE_VOLUME_UP); // KeyEvent.KEYCODE_PAGEUP);
		// mHashMapKeyCode.put(105, KeyEvent.KEYCODE_LEFT);
		// mHashMapKeyCode.put(106, KeyEvent.KEYCODE_RIGHT);
		// mHashMapKeyCode.put(107, KeyEvent.KEYCODE_END);
		// mHashMapKeyCode.put(108, KeyEvent.KEYCODE_DOWN);
		mHashMapKeyCode.put(109, KeyEvent.KEYCODE_VOLUME_DOWN); // KeyEvent.KEYCODE_PAGEDOWN);
		// mHashMapKeyCode.put(110, KeyEvent.KEYCODE_INSERT);
		// mHashMapKeyCode.put(111, KeyEvent.KEYCODE_DELETE);
		// mHashMapKeyCode.put(112, KeyEvent.KEYCODE_MACRO);
		mHashMapKeyCode.put(113, KeyEvent.KEYCODE_MUTE);
		// mHashMapKeyCode.put(114, KeyEvent.KEYCODE_VOLUMEDOWN);
		// mHashMapKeyCode.put(115, KeyEvent.KEYCODE_VOLUMEUP);
		mHashMapKeyCode.put(116, KeyEvent.KEYCODE_POWER);
		// mHashMapKeyCode.put(117, KeyEvent.KEYCODE_KPEQUAL);
		// mHashMapKeyCode.put(118, KeyEvent.KEYCODE_KPPLUSMINUS);
		mHashMapKeyCode.put(119, KeyEvent.KEYCODE_POWER); // KeyEvent.KEYCODE_PAUSE);
		// mHashMapKeyCode.put(120, KeyEvent.KEYCODE_SCALE);
		// mHashMapKeyCode.put(121, KeyEvent.KEYCODE_KPCOMMA);
		// mHashMapKeyCode.put(122, KeyEvent.KEYCODE_HANGEUL);
		// mHashMapKeyCode.put(123, KeyEvent.KEYCODE_HANJA);
		// mHashMapKeyCode.put(124, KeyEvent.KEYCODE_YEN);
		// mHashMapKeyCode.put(125, KeyEvent.KEYCODE_LEFTMETA);
		// mHashMapKeyCode.put(126, KeyEvent.KEYCODE_RIGHTMETA);
		// mHashMapKeyCode.put(127, KeyEvent.KEYCODE_COMPOSE);
		// mHashMapKeyCode.put(128, KeyEvent.KEYCODE_STOP);
		// mHashMapKeyCode.put(129, KeyEvent.KEYCODE_AGAIN);
		// mHashMapKeyCode.put(130, KeyEvent.KEYCODE_PROPS);
		// mHashMapKeyCode.put(131, KeyEvent.KEYCODE_UNDO);
		// mHashMapKeyCode.put(132, KeyEvent.KEYCODE_FRONT);
		// mHashMapKeyCode.put(133, KeyEvent.KEYCODE_COPY);
		// mHashMapKeyCode.put(134, KeyEvent.KEYCODE_OPEN);
		// mHashMapKeyCode.put(135, KeyEvent.KEYCODE_PASTE);
		// mHashMapKeyCode.put(136, KeyEvent.KEYCODE_FIND);
		// mHashMapKeyCode.put(137, KeyEvent.KEYCODE_CUT);
		// mHashMapKeyCode.put(138, KeyEvent.KEYCODE_HELP);
		mHashMapKeyCode.put(139, KeyEvent.KEYCODE_MENU);
		// mHashMapKeyCode.put(140, KeyEvent.KEYCODE_CALC);
		// mHashMapKeyCode.put(141, KeyEvent.KEYCODE_SETUP);
		// mHashMapKeyCode.put(142, KeyEvent.KEYCODE_SLEEP);
		// mHashMapKeyCode.put(143, KeyEvent.KEYCODE_WAKEUP);
		// mHashMapKeyCode.put(144, KeyEvent.KEYCODE_FILE);
		// mHashMapKeyCode.put(145, KeyEvent.KEYCODE_SENDFILE);
		// mHashMapKeyCode.put(146, KeyEvent.KEYCODE_DELETEFILE);
		// mHashMapKeyCode.put(147, KeyEvent.KEYCODE_XFER);
		// mHashMapKeyCode.put(148, KeyEvent.KEYCODE_PROG1);
		// mHashMapKeyCode.put(149, KeyEvent.KEYCODE_PROG2);
		// mHashMapKeyCode.put(150, KeyEvent.KEYCODE_WWW);
		// mHashMapKeyCode.put(151, KeyEvent.KEYCODE_MSDOS);
		// mHashMapKeyCode.put(152, KeyEvent.KEYCODE_COFFEE);
		// mHashMapKeyCode.put(153, KeyEvent.KEYCODE_DIRECTION);
		// mHashMapKeyCode.put(154, KeyEvent.KEYCODE_CYCLEWINDOWS);
		// mHashMapKeyCode.put(155, KeyEvent.KEYCODE_MAIL);
		// mHashMapKeyCode.put(156, KeyEvent.KEYCODE_BOOKMARKS);
		// mHashMapKeyCode.put(157, KeyEvent.KEYCODE_COMPUTER);
		mHashMapKeyCode.put(158, KeyEvent.KEYCODE_BACK);
		// mHashMapKeyCode.put(159, KeyEvent.KEYCODE_FORWARD);
		// mHashMapKeyCode.put(160, KeyEvent.KEYCODE_CLOSECD);
		// mHashMapKeyCode.put(161, KeyEvent.KEYCODE_EJECTCD);
		// mHashMapKeyCode.put(162, KeyEvent.KEYCODE_EJECTCLOSECD);
		// mHashMapKeyCode.put(163, KeyEvent.KEYCODE_NEXTSONG);
		// mHashMapKeyCode.put(164, KeyEvent.KEYCODE_PLAYPAUSE);
		// mHashMapKeyCode.put(165, KeyEvent.KEYCODE_PREVIOUSSONG);
		// mHashMapKeyCode.put(166, KeyEvent.KEYCODE_STOPCD);
		// mHashMapKeyCode.put(167, KeyEvent.KEYCODE_RECORD);
		// mHashMapKeyCode.put(168, KeyEvent.KEYCODE_REWIND);
		// mHashMapKeyCode.put(169, KeyEvent.KEYCODE_PHONE);
		// mHashMapKeyCode.put(170, KeyEvent.KEYCODE_ISO);
		// mHashMapKeyCode.put(171, KeyEvent.KEYCODE_CONFIG);
		// mHashMapKeyCode.put(172, KeyEvent.KEYCODE_HOMEPAGE);
		// mHashMapKeyCode.put(173, KeyEvent.KEYCODE_REFRESH);
		// mHashMapKeyCode.put(174, KeyEvent.KEYCODE_EXIT);
		// mHashMapKeyCode.put(175, KeyEvent.KEYCODE_MOVE);
		// mHashMapKeyCode.put(176, KeyEvent.KEYCODE_EDIT);
		// mHashMapKeyCode.put(177, KeyEvent.KEYCODE_SCROLLUP);
		// mHashMapKeyCode.put(178, KeyEvent.KEYCODE_SCROLLDOWN);
		// mHashMapKeyCode.put(179, KeyEvent.KEYCODE_KPLEFTPAREN);
		// mHashMapKeyCode.put(180, KeyEvent.KEYCODE_KPRIGHTPAREN);
		// mHashMapKeyCode.put(181, KeyEvent.KEYCODE_NEW);
		// mHashMapKeyCode.put(182, KeyEvent.KEYCODE_REDO);
		// mHashMapKeyCode.put(183, KeyEvent.KEYCODE_F13);
		// mHashMapKeyCode.put(184, KeyEvent.KEYCODE_F14);
		// mHashMapKeyCode.put(185, KeyEvent.KEYCODE_F15);
		// mHashMapKeyCode.put(186, KeyEvent.KEYCODE_F16);
		// mHashMapKeyCode.put(187, KeyEvent.KEYCODE_F17);
		// mHashMapKeyCode.put(188, KeyEvent.KEYCODE_F18);
		// mHashMapKeyCode.put(189, KeyEvent.KEYCODE_F19);
		// mHashMapKeyCode.put(190, KeyEvent.KEYCODE_F20);
		// mHashMapKeyCode.put(191, KeyEvent.KEYCODE_F21);
		// mHashMapKeyCode.put(192, KeyEvent.KEYCODE_F22);
		// mHashMapKeyCode.put(193, KeyEvent.KEYCODE_F23);
		// mHashMapKeyCode.put(194, KeyEvent.KEYCODE_F24);
		// mHashMapKeyCode.put(200, KeyEvent.KEYCODE_PLAYCD);
		// mHashMapKeyCode.put(201, KeyEvent.KEYCODE_PAUSECD);
		// mHashMapKeyCode.put(202, KeyEvent.KEYCODE_PROG3);
		// mHashMapKeyCode.put(203, KeyEvent.KEYCODE_PROG4);
		// mHashMapKeyCode.put(204, KeyEvent.KEYCODE_DASHBOARD);
		// mHashMapKeyCode.put(205, KeyEvent.KEYCODE_SUSPEND);
		// mHashMapKeyCode.put(206, KeyEvent.KEYCODE_CLOSE);
		// mHashMapKeyCode.put(207, KeyEvent.KEYCODE_PLAY);
		// mHashMapKeyCode.put(208, KeyEvent.KEYCODE_FASTFORWARD);
		// mHashMapKeyCode.put(209, KeyEvent.KEYCODE_BASSBOOST);
		// mHashMapKeyCode.put(210, KeyEvent.KEYCODE_PRINT);
		// mHashMapKeyCode.put(211, KeyEvent.KEYCODE_HP);
		mHashMapKeyCode.put(212, KeyEvent.KEYCODE_CAMERA);
		// mHashMapKeyCode.put(213, KeyEvent.KEYCODE_SOUND);
		// mHashMapKeyCode.put(214, KeyEvent.KEYCODE_QUESTION);
		// mHashMapKeyCode.put(215, KeyEvent.KEYCODE_EMAIL);
		// mHashMapKeyCode.put(216, KeyEvent.KEYCODE_CHAT);
		mHashMapKeyCode.put(217, KeyEvent.KEYCODE_SEARCH);
		// mHashMapKeyCode.put(218, KeyEvent.KEYCODE_CONNECT);
		// mHashMapKeyCode.put(219, KeyEvent.KEYCODE_FINANCE);
		// mHashMapKeyCode.put(220, KeyEvent.KEYCODE_SPORT);
		// mHashMapKeyCode.put(221, KeyEvent.KEYCODE_SHOP);
		// mHashMapKeyCode.put(222, KeyEvent.KEYCODE_ALTERASE);
		// mHashMapKeyCode.put(223, KeyEvent.KEYCODE_CANCEL);
		// mHashMapKeyCode.put(224, KeyEvent.KEYCODE_BRIGHTNESSDOWN);
		// mHashMapKeyCode.put(225, KeyEvent.KEYCODE_BRIGHTNESSUP);
		// mHashMapKeyCode.put(226, KeyEvent.KEYCODE_MEDIA);
		// mHashMapKeyCode.put(227, KeyEvent.KEYCODE_SWITCHVIDEOMODE);
		// mHashMapKeyCode.put(228, KeyEvent.KEYCODE_KBDILLUMTOGGLE);
		// mHashMapKeyCode.put(229, KeyEvent.KEYCODE_KBDILLUMDOWN);
		// mHashMapKeyCode.put(230, KeyEvent.KEYCODE_KBDILLUMUP);
		// mHashMapKeyCode.put(231, KeyEvent.KEYCODE_SEND);
		// mHashMapKeyCode.put(232, KeyEvent.KEYCODE_REPLY);
		// mHashMapKeyCode.put(233, KeyEvent.KEYCODE_FORWARDMAIL);
		// mHashMapKeyCode.put(234, KeyEvent.KEYCODE_SAVE);
		// mHashMapKeyCode.put(235, KeyEvent.KEYCODE_DOCUMENTS);
		// mHashMapKeyCode.put(236, KeyEvent.KEYCODE_BATTERY);
		// mHashMapKeyCode.put(237, KeyEvent.KEYCODE_BLUETOOTH);
		// mHashMapKeyCode.put(238, KeyEvent.KEYCODE_WLAN);
		// mHashMapKeyCode.put(239, KeyEvent.KEYCODE_UWB);
		mHashMapKeyCode.put(240, KeyEvent.KEYCODE_UNKNOWN);
		// mHashMapKeyCode.put(241, KeyEvent.KEYCODE_VIDEO_NEXT);
		// mHashMapKeyCode.put(242, KeyEvent.KEYCODE_VIDEO_PREV);
		// mHashMapKeyCode.put(243, KeyEvent.KEYCODE_BRIGHTNESS_CYCLE);
		// mHashMapKeyCode.put(244, KeyEvent.KEYCODE_BRIGHTNESS_ZERO);
		// mHashMapKeyCode.put(245, KeyEvent.KEYCODE_DISPLAY_OFF);
		// mHashMapKeyCode.put(246, KeyEvent.KEYCODE_WIMAX);
		// mHashMapKeyCode.put(247, KeyEvent.KEYCODE_RFKILL);
		// mHashMapKeyCode.put(248, KeyEvent.KEYCODE_MICMUTE);
	}

	private static final boolean parseKeyLayoutFile(String pathname) {
		BufferedReader reader;
		try {
			reader = new BufferedReader(new FileReader(pathname));
		} catch (FileNotFoundException e1) {
			e1.printStackTrace();
			return false;
		}

		boolean res = false;

		try {
			while (true) {
				String line = reader.readLine();
				if (line == null) {
					break;
				}

				String[] elements = line.split("\\s+");
				if (elements.length < 3 || elements[0].equals("key") == false) {
					continue;
				}

				Integer code = mHashMapKeyName.get(elements[2]);
				if (code == null) {
					continue;
				}

				int rawCode = Integer.decode(elements[1]);
				Log.d(TAG, rawCode + " => " + code);
				mHashMapKeyCode.put(rawCode, code);
			}

			res = true;
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}

		try {
			reader.close();
		} catch (IOException e) {
			e.printStackTrace();
		}

		return res;
	}

	private static final int findKeyCode(int code) {
		Integer keycode = mHashMapKeyCode.get(code);
		if (keycode == null) {
			Log.w(TAG, "unknown key = " + code);
			return KeyEvent.KEYCODE_UNKNOWN;
		}

		Log.d(TAG, code + " => " + keycode);

		return keycode;
	}

	@Override
	public IBinder onBind(Intent arg0) {
		return null;
	}

	@Override
	public void onCreate() {
		Log.d(TAG, "onCreate()");

		mWindowManager = IWindowManager.Stub.asInterface(ServiceManager.getService(Context.WINDOW_SERVICE));

		super.onCreate();
	}

	@Override
	public void onDestroy() {
		Log.d(TAG, "onDestroy()");

		if (mServerSocket != null) {
			try {
				mServerSocket.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}

		super.onDestroy();
	}

	private void notifyStateChanged(int state) {
		Intent intent = new Intent(ACTION_STATE_CHANGED);
		intent.putExtra("state", state);
		sendStickyBroadcast(intent);
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		Log.d(TAG, "onStartCommand()");

		notifyStateChanged(STATE_START);

		try {
			mServerSocket = new ServerSocket(3333);
		} catch (IOException e) {
			e.printStackTrace();
			notifyStateChanged(STATE_ERROR);
			return START_NOT_STICKY;
		}

		DaemonThread thread = new DaemonThread();
		thread.start();

		notifyStateChanged(STATE_RUNNING);

		return super.onStartCommand(intent, flags, startId);
	}

	private boolean run(InputStream inputStream, OutputStream outputStream) throws IOException {
		byte[] buff = new byte[512];
		while (true) {
			int length = inputStream.read(buff);
			if (length < 0) {
				break;
			}

			if (length < 2) {
				continue;
			}

			long now = SystemClock.uptimeMillis();
			int i = 0;

			Instrumentation instrumentation = new Instrumentation();

			switch (buff[i++]) {
			case CAVAN_INPUT_MESSAGE_KEY:
				if (length == 5) {
					int code = buff[i++] | buff[i++] << 8;
					int value = buff[i++] | buff[i++] << 8;
					int action = value > 0 ? KeyEvent.ACTION_DOWN : KeyEvent.ACTION_UP;
					KeyEvent keyEvent = new KeyEvent(now, now, action, findKeyCode(code), 0);
					instrumentation.sendKeySync(keyEvent);
				}
				break;

			case CAVAN_INPUT_MESSAGE_MOUSE_MOVE:
				if (length == 3) {
					int dx = buff[i++];
					int dy = buff[i++];
					MotionEvent motionEvent = MotionEvent.obtain(now, now, MotionEvent.ACTION_MOVE, dx, dy, 0);
					instrumentation.sendTrackballEventSync(motionEvent);
				}
				break;

			case CAVAN_INPUT_MESSAGE_MOUSE_TOUCH:
				if (length == 5) {
					int code = buff[i++] | buff[i++] << 8;
					int value = buff[i++] | buff[i++] << 8;
					int action = value > 0 ? MotionEvent.ACTION_DOWN : MotionEvent.ACTION_UP;
					MotionEvent motionEvent = MotionEvent.obtain(now, now, action, 0, 0, 0);
					instrumentation.sendTrackballEventSync(motionEvent);
				}
				break;
			}
		}

		return true;
	}

	private boolean run(Socket client) {
		InputStream inputStream;
		try {
			inputStream = client.getInputStream();
		} catch (IOException e) {
			e.printStackTrace();
			return false;
		}

		OutputStream outputStream;
		try {
			outputStream = client.getOutputStream();
		} catch (IOException e) {
			e.printStackTrace();
			try {
				inputStream.close();
			} catch (IOException e1) {
				e1.printStackTrace();
			}
			return false;
		}

		boolean res;
		try {
			res = run(inputStream, outputStream);
		} catch (IOException e1) {
			e1.printStackTrace();
			res = false;
		}

		try {
			outputStream.close();
		} catch (IOException e) {
			e.printStackTrace();
		}

		try {
			inputStream.close();
		} catch (IOException e) {
			e.printStackTrace();
		}

		return res;
	}

	private void mainLoop() {
		int index;

		synchronized (this) {
			index = mDaemonCount++;
			Log.d(TAG, "Daemon[" + index + "] Entry (" + mDaemonUsed + " / " + mDaemonCount + ")");
		}

		while (true) {
			Socket client;

			Log.d(TAG, "Daemon[" + index + "] Listening (" + mDaemonUsed + " / " + mDaemonCount + ")");
			try {
				client = mServerSocket.accept();
				if (client == null) {
					break;
				}
			} catch (IOException e) {
				e.printStackTrace();
				break;
			}

			synchronized (this) {
				mDaemonUsed++;
				Log.d(TAG, "Daemon[" + index + "] Running (" + mDaemonUsed + " / " + mDaemonCount + ")");

				if (mDaemonUsed >= mDaemonCount) {
					DaemonThread thread = new DaemonThread();
					thread.start();
				}
			}

			run(client);

			try {
				client.close();
			} catch (IOException e) {
				e.printStackTrace();
			}

			synchronized (this) {
				mDaemonUsed--;
				Log.d(TAG, "Daemon[" + index + "] Stopped (" + mDaemonUsed + " / " + mDaemonCount + ")");

				if (mDaemonCount - mDaemonUsed > MIN_DAEMON_COUNT) {
					break;
				}
			}
		}

		synchronized (this) {
			mDaemonCount--;
			if (mDaemonCount <= 0) {
				try {
					mServerSocket.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
				mServerSocket = null;
				notifyStateChanged(STATE_STOPPED);
			}
			Log.d(TAG, "Daemon[" + index + "] Exit (" + mDaemonUsed + " / " + mDaemonCount + ")");
		}
	}

	class DaemonThread extends Thread {
		@Override
		public void run() {
			mainLoop();
		}
	}
}
