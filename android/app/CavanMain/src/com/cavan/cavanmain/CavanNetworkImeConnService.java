package com.cavan.cavanmain;

import java.net.Socket;
import java.util.ArrayList;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import android.accessibilityservice.AccessibilityService;
import android.app.NotificationManager;
import android.content.Context;
import android.content.Intent;
import android.media.AudioManager;
import android.net.Uri;
import android.os.Build;
import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import android.view.accessibility.AccessibilityNodeInfo;
import android.view.inputmethod.InputConnection;
import android.view.inputmethod.InputMethodManager;

import com.cavan.accessibility.CavanAccessibilityHelper;
import com.cavan.accessibility.CavanAccessibilityService;
import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanWakeLock;
import com.cavan.java.CavanJava;
import com.cavan.java.CavanJava.ClosureVoid;
import com.cavan.java.CavanString;
import com.cavan.java.CavanTcpClient;
import com.cavan.java.CavanTcpPacketClient;
import com.cavan.service.CavanPowerStateListener;
import com.cavan.service.CavanTcpConnService;

public class CavanNetworkImeConnService extends CavanTcpConnService implements CavanPowerStateListener {

	public static final Pattern sUrlPattern = Pattern.compile("^\\w+://");

	private static final int MSG_TCP_PACKET_RECEIVED = 1;
	private static final int MSG_SHOW_MEDIA_VOLUME = 2;

	public static CavanNetworkImeConnService instance;

	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case MSG_TCP_PACKET_RECEIVED:
				try {
					onTcpPacketReceived((String[]) msg.obj);
				} catch (Exception e) {
					e.printStackTrace();
				}
				break;

			case MSG_SHOW_MEDIA_VOLUME:
				FloatMessageService service = FloatMessageService.instance;
				if (service != null) {
					int volume = mAudioManager.getStreamVolume(AudioManager.STREAM_MUSIC);
					service.postShowToastWithArgs(R.string.media_volume_changed, volume);
				}
				break;
			}
		}
	};

	private AudioManager mAudioManager;
	private InputMethodManager mInputMethodManager;
	private CavanWakeLock mFullWakeLock = new CavanWakeLock.FullLock();

	private CavanTcpPacketClient mTcpPacketClient = new CavanTcpPacketClient() {

		public boolean sendPing() {
			return mSendThread.send("PING");
		}

		public boolean sendPong() {
			return mSendThread.send("PONG");
		}

		@Override
		public int onGetConnOvertime() {
			return 3000;
		}

		@Override
		protected int onGetKeepAliveDelay() {
			return 10000;
		}

		@Override
		protected int doTcpKeepAlive(int times) {
			if (sendPing()) {
				return times;
			}

			return -1;
		}

		@Override
		protected boolean onPacketReceived(byte[] bytes, int length) {
			String command = new String(bytes, 0, length);
			CavanAndroid.dLog("onTcpPacketReceived: " + command);

			String[] args = command.split("\\s+", 2);
			args[0] = args[0].toUpperCase();

			switch (args[0]) {
			case "PING":
				if (!sendPong()) {
					return false;
				}
				break;

			case "PONG":
				break;

			default:
				Message message = mHandler.obtainMessage(MSG_TCP_PACKET_RECEIVED, args);
				message.sendToTarget();
			}

			return super.onPacketReceived(bytes, length);
		}

		@Override
		protected boolean onTcpConnected(Socket socket) {
			String hostname = Build.MODEL.replaceAll("\\s+", "_");
			CavanAndroid.dLog("hostname = " + hostname);

			if (hostname != null) {
				send("USER " + hostname);
			}

			return super.onTcpConnected(socket);
		}

		@Override
		protected void onTcpDisconnected() {
			super.onTcpDisconnected();
		}
	};

	private CavanTcpClient.SendThread mSendThread = mTcpPacketClient.newSendThread();

	public static Intent getIntent(Context context) {
		return new Intent(context, CavanNetworkImeConnService.class);
	}

	public static void startService(Context context) {
		context.startService(getIntent(context));
	}

	private boolean processCommand(CavanMainInputMethod ime, CavanAccessibilityService accessibility, String[] args) {
		InputConnection conn = ime.getCurrentInputConnection();
		if (conn == null) {
			CavanAndroid.dLog("conn is null");
			return false;
		}

		boolean send = false;

		switch (args[0]) {
		case "SEND":
			if (args.length < 2) {
				break;
			}

			if (accessibility != null && accessibility.sendText(args[1], true)) {
				break;
			}

			send = true;
		case "REPLACE":
			if (!conn.performContextMenuAction(android.R.id.selectAll)) {
				return false;
			}
		case "INSERT":
			if (args.length > 1) {
				conn.commitText(args[1], 0);

				if (send) {
					ime.postAutoCommit();
				}
			} else {
				return conn.commitText(CavanString.NONE, 0);
			}
			break;

		case "DELETE":
			CharSequence text = conn.getSelectedText(0);
			if (text != null && text.length() > 0) {
				return conn.commitText(CavanString.NONE, 1);
			} else {
				return conn.deleteSurroundingText(1, 0);
			}

		case "COMMIT":
			if (accessibility != null && accessibility.sendText(null, true)) {
				break;
			}
		case "DONE":
			return ime.sendGoAction(conn);

		case "KEY":
			if (args.length > 1) {
				try {
					args = args[1].split("\\s+");

					int code = Integer.parseInt(args[0].trim());

					if (args.length > 1) {
						int value = Integer.parseInt(args[1].trim());
						return ime.sendKeyEvent(code, value);
					} else {
						return ime.sendKeyDownUp(code);
					}
				} catch (NumberFormatException e) {
					e.printStackTrace();
				}
			}
			break;

		case "ACTION":
			if (args.length > 0) {
				try {
					int action = Integer.parseInt(args[1].trim());
					return conn.performEditorAction(action);
				} catch (NumberFormatException e) {
					e.printStackTrace();
				}
			}
			break;

		case "MENU_ACTION":
			if (args.length > 0) {
				try {
					int action = Integer.parseInt(args[1].trim());
					return conn.performContextMenuAction(action);
				} catch (NumberFormatException e) {
					e.printStackTrace();
				}
			}
			break;

		case "COPY":
			return conn.performContextMenuAction(android.R.id.copy);

		case "PASTE":
			return conn.performContextMenuAction(android.R.id.paste);

		case "SELECT_ALL":
			return conn.performContextMenuAction(android.R.id.selectAll);

		default:
			CavanAndroid.eLog("Invalid command: " + args[0]);
			return false;
		}

		return true;
	}

	private boolean processCommand(CavanAccessibilityService accessibility, String[] args) {
		switch (args[0]) {
		case "SEND":
			return accessibility.sendText(args[1], true);

		case "REPLACE":
			return accessibility.sendText(args[1], false);

		case "COMMIT":
		case "DONE":
			return accessibility.sendText(null, true);

		default:
			CavanAndroid.eLog("Invalid command: " + args[0]);
		}

		return false;
	}

	public boolean doUnlockScreen() {
		if (CavanMainApplication.isUserPresent()) {
			return true;
		}

		FloatMessageService fms = FloatMessageService.instance;
		if (fms == null) {
			return false;
		}

		fms.setAutoUnlockLevel(FloatMessageService.AUTO_UNLOCK_ALWAYS);

		return true;
	}

	public Object[] parseIndexs(String[] args) {
		if (args.length < 2) {
			return null;
		}

		String[] values = args[1].split("\\s+");
		Object[] indexs = new Object[values.length];

		for (int i = 0; i < values.length; i++) {
			String value = values[i];

			if (value.charAt(0) == '@') {
				indexs[i] = value.substring(1);
			} else {
				indexs[i] = CavanJava.parseInt(value);
			}
		}

		return indexs;
	}

	public boolean performAction(CavanAccessibilityService accessibility, int action, boolean touch, String[] args) {
		Object[] indexs = parseIndexs(args);
		if (indexs == null) {
			return false;
		}

		AccessibilityNodeInfo node = accessibility.getChildRecursive(indexs);
		if (node == null) {
			return false;
		}

		CavanAndroid.dLog("node = " + node);

		if (touch && accessibility.doInputTap(node)) {
			return true;
		}

		return CavanAccessibilityHelper.performActionAndRecycle(node, action);
	}

	protected void onTcpPacketReceived(String[] args) {
		CavanAccessibilityService accessibility = CavanAccessibilityService.instance;

		switch (args[0]) {
		case "OPEN":
			if (args.length > 1 && CavanMainApplication.isScreenOn()) {
				CavanAndroid.startActivityFuzzy(getApplicationContext(), args[1]);
			}
			break;

		case "VIEW":
			if (args.length > 1 && CavanMainApplication.isScreenOn()) {
				String url = args[1];

				Matcher matcher = sUrlPattern.matcher(url);
				if (!matcher.find()) {
					url = "http://" + url;
				}

				CavanAndroid.dLog("url = " + url);

				try {
					Uri uri = Uri.parse(url);
					Intent intent = new Intent(Intent.ACTION_VIEW, uri);
					CavanAndroid.startActivity(this, intent);
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
			break;

		case "UNLOCK":
			mFullWakeLock.acquire(this, 300000);

			if (args.length > 1) {
				CavanAndroid.startActivityFuzzy(getApplicationContext(), args[1]);
			}

			doUnlockScreen();
			break;

		case "WEB":
			if (accessibility != null && args.length > 1) {
				accessibility.sendCommandWeb(args[1]);
			}
			break;

		case "SHARE":
			if (accessibility != null) {
				accessibility.sendCommandShred(args.length > 1 && CavanJava.parseBoolean(args[1]));
			}
			break;

		case "CLEAR":
			NotificationManager manager = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
			if (manager != null) {
				manager.cancelAll();
			}

			FloatMessageService fms = FloatMessageService.instance;
			if (fms != null) {
				fms.removeTextAll();
			}
			break;

		case "DUMP":
			if (accessibility != null) {
				int level;

				if (args.length > 1) {
					level = CavanJava.parseInt(args[1]);
				} else {
					level = 0;
				}

				if (level > 1) {
					AccessibilityNodeInfo root = accessibility.getRootInActiveWindow(3);
					if (root != null) {
						ClosureVoid closure;

						if (level > 2) {
							closure = new CavanAccessibilityHelper.ClosureDumpNode();
						} else {
							closure = new CavanAccessibilityHelper.ClosureDumpNodeSimple();
						}

						StringBuilder builder = new StringBuilder("RESPONSE ");
						CavanAccessibilityHelper.dumpNodeTo(builder, root, closure);
						root.recycle();

						mSendThread.send(builder.toString());
					}
				} else if (level > 0) {
					accessibility.dump(false);
				} else {
					accessibility.dump(true);
				}
			}
			break;

		case "ALIPAY":
			if (args.length > 1) {
				for (String code : args[1].split("\\s+")) {
					RedPacketListenerService listener = RedPacketListenerService.instance;
					if (listener != null) {
						listener.addRedPacketCode(code, "网络输入法", false);
					}
				}
			}
			break;

		case "TEST":
			if (CavanMainApplication.isScreenOn() && CavanMainApplication.test(getApplicationContext())) {
				FloatMessageService.showToast("测试成功");
			}
			break;

		case "CLICK":
			if (accessibility != null && CavanMainApplication.isScreenOn()) {
				performAction(accessibility, AccessibilityNodeInfo.ACTION_CLICK, false, args);
			}
			break;

		case "TOUCH":
			if (accessibility != null && CavanMainApplication.isScreenOn()) {
				performAction(accessibility, AccessibilityNodeInfo.ACTION_CLICK, true, args);
			}
			break;

		case "LONG_CLICK":
			if (accessibility != null && CavanMainApplication.isScreenOn()) {
				performAction(accessibility, AccessibilityNodeInfo.ACTION_LONG_CLICK, false, args);
			}
			break;

		case "CLOCK":
			if (args.length > 1) {
				fms = FloatMessageService.instance;
				if (fms != null) {
					boolean enable = CavanJava.parseBoolean(args[1]);

					try {
						fms.getBind().setTimerEnable(enable);
					} catch (RemoteException e) {
						e.printStackTrace();
					}
				}
			}
			break;

		case "PROXY":
			if (args.length > 1) {
				boolean success = CavanAndroid.setHttpProxy(this, args[1]);
				CavanAndroid.dLog("setHttpProxy: " + success);
			}
			break;

		case "CLIPBOARD":
			if (args.length > 1) {
				String text = args[1];
				CavanAndroid.postClipboardTextTemp(getApplicationContext(), text);
				fms = FloatMessageService.instance;
				if (fms != null) {
					fms.postShowToastWithArgs(R.string.clipboard_updated, text);
				}
			}
			break;

		case "GLOBAL":
			if (accessibility != null && args.length > 1) {
				int action = CavanJava.parseInt(args[1]);
				accessibility.performGlobalAction(action);
			}
			break;

		case "HOME":
			if (accessibility != null && CavanMainApplication.isScreenOn()) {
				accessibility.sendCommandHome();
			}
			break;

		case "DESKTOP":
			if (accessibility != null && CavanMainApplication.isScreenOn()) {
				accessibility.performActionHome();
			}
			break;

		case "BACK":
			if (accessibility != null && CavanMainApplication.isScreenOn()) {
				accessibility.sendCommandBack();
			}
			break;

		case "RECENTS":
			if (accessibility != null && CavanMainApplication.isScreenOn()) {
				accessibility.performGlobalAction(AccessibilityService.GLOBAL_ACTION_RECENTS);
			}
			break;

		case "LOGIN":
			if (accessibility != null) {
				if (args.length > 1) {
					String[] params = args[1].split("\\s+", 2);

					if (params.length > 1) {
						accessibility.login(params[0], params[1]);
					} else {
						accessibility.login(args[1], null);
					}
				} else {
					accessibility.login(null, null);
				}
			}
			break;

		case "SIGNIN":
			if (accessibility != null) {
				accessibility.signin();
			}
			break;

		case "REFRESH":
			if (accessibility != null) {
				accessibility.refresh();
			}
			break;

		case "FOLLOW":
			if (accessibility != null) {
				accessibility.follow();
			}
			break;

		case "UNFOLLOW":
			if (accessibility != null) {
				accessibility.unfollow();
			}
			break;

		case "TAP":
			if (args.length > 1 && accessibility != null) {
				String[] params = args[1].split("\\s+");
				if (params.length > 1) {
					int x = CavanJava.parseInt(params[0]);
					int y = CavanJava.parseInt(params[1]);
					accessibility.doInputTap(x, y);
				}
			}
			break;

		case "IME":
			if (CavanMainApplication.isScreenOn()) {
				mInputMethodManager.showInputMethodPicker();
			}
			break;

		case "VOLUME":
			if (args.length > 1) {
				char action = args[1].charAt(0);
				int direction;

				if (action == '+') {
					direction = AudioManager.ADJUST_RAISE;
				} else if (action == '-') {
					direction = AudioManager.ADJUST_LOWER;
				} else if (action == 'x') {
					direction = AudioManager.ADJUST_MUTE;
				} else if (action == 'o') {
					direction = AudioManager.ADJUST_UNMUTE;
				} else {
					int volume;

					if (action == '=') {
						volume = CavanJava.parseInt(args[1].substring(1));
					} else {
						try {
							volume = Integer.parseInt(args[1]);
						} catch (Exception e) {
							break;
						}
					}

					mAudioManager.setStreamVolume(AudioManager.STREAM_MUSIC, volume, 0);
					mHandler.sendEmptyMessage(MSG_SHOW_MEDIA_VOLUME);
					break;
				}

				mAudioManager.adjustStreamVolume(AudioManager.STREAM_MUSIC, direction, 0);
				mHandler.sendEmptyMessage(MSG_SHOW_MEDIA_VOLUME);
			}
			break;

		default:
			CavanMainInputMethod ime = CavanMainInputMethod.instance;
			if (ime == null) {
				if (accessibility != null) {
					processCommand(accessibility, args);
				}
			} else {
				processCommand(ime, accessibility, args);
			}
		}
	}

	@Override
	protected CavanTcpClient doCreateTcpClient() {
		mTcpPacketClient.setTcpClientListener(this);
		return mTcpPacketClient;
	}

	@Override
	public void onCreate() {
		super.onCreate();

		mAudioManager = (AudioManager) getSystemService(AUDIO_SERVICE);
		mInputMethodManager = (InputMethodManager) getSystemService(INPUT_METHOD_SERVICE);

		try {
			ArrayList<String> addresses = CavanNetworkImeClientPreference.load(this, CavanServiceActivity.KEY_NETWORK_IME);
			mBinder.setAddresses(addresses);
		} catch (RemoteException e) {
			e.printStackTrace();
		}

		instance = this;
		CavanMainApplication.addPowerStateListener(this);
	}

	@Override
	public void onDestroy() {
		instance = null;
		super.onDestroy();
	}

	@Override
	protected int getDefaultPort() {
		return 8865;
	}

	@Override
	public long onTcpConnFailed(int times) {
		if (times < 3 || CavanMainApplication.isScreenOn()) {
			return super.onTcpConnFailed(times);
		}

		return -1;
	}

	@Override
	public void onScreenOn() {
		mTcpPacketClient.connect();
	}

	@Override
	public void onScreenOff() {
		mFullWakeLock.release();
	}

	@Override
	public void onUserPresent() {}

	@Override
	public void onCloseSystemDialogs(String reason) {}

	@Override
	public void onKeepAliveFailed() {}
}
