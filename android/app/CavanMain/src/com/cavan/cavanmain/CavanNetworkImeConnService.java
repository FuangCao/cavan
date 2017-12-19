package com.cavan.cavanmain;

import java.net.Socket;
import java.util.ArrayList;

import android.accessibilityservice.AccessibilityService;
import android.content.Context;
import android.content.Intent;
import android.media.AudioManager;
import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import android.view.inputmethod.InputConnection;

import com.cavan.android.CavanAndroid;
import com.cavan.android.SystemProperties;
import com.cavan.java.CavanJava;
import com.cavan.java.CavanString;
import com.cavan.java.CavanTcpClient;
import com.cavan.java.CavanTcpPacketClient;
import com.cavan.service.CavanTcpConnService;

public class CavanNetworkImeConnService extends CavanTcpConnService {

	private static final int MSG_TCP_PACKET_RECEIVED = 1;

	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case MSG_TCP_PACKET_RECEIVED:
				onTcpPacketReceived((String) msg.obj);
				break;
			}
		}
	};

	private CavanTcpPacketClient mTcpPacketClient = new CavanTcpPacketClient() {

		@Override
		protected boolean onPacketReceived(byte[] bytes, int length) {
			Message message = mHandler.obtainMessage(MSG_TCP_PACKET_RECEIVED, new String(bytes, 0, length));
			message.sendToTarget();
			return super.onPacketReceived(bytes, length);
		}

		@Override
		protected boolean onTcpConnected(Socket socket) {
			String hostname = SystemProperties.get("net.hostname");
			if (hostname != null) {
				send("USER " + hostname);
			}

			return super.onTcpConnected(socket);
		}
	};

	public static Intent getIntent(Context context) {
		return new Intent(context, CavanNetworkImeConnService.class);
	}

	public static void startService(Context context) {
		context.startService(getIntent(context));
	}

	protected void onTcpPacketReceived(String command) {
		CavanAndroid.dLog("onTcpPacketReceived: " + command);

		CavanAccessibilityService accessibility = CavanAccessibilityService.getInstance();
		String[] args = command.split(" ", 2);
		boolean send = false;

		switch (args[0]) {
		case "OPEN":
			if (args.length > 0) {
				CavanAndroid.startActivity(getApplicationContext(), args[1]);
			}
			break;

		case "CLIPBOARD":
			if (args.length > 0) {
				String text = args[1];
				CavanAndroid.postClipboardText(getApplicationContext(), text);
				FloatMessageService fms = FloatMessageService.getInstance();
				if (fms != null) {
					fms.sendShowToastWithArgs(R.string.clipboard_updated, text);
				}
			}
			break;

		case "HOME":
			if (accessibility != null) {
				accessibility.performGlobalAction(AccessibilityService.GLOBAL_ACTION_HOME);
			}
			break;

		case "BACK":
			if (accessibility != null) {
				accessibility.performGlobalAction(AccessibilityService.GLOBAL_ACTION_BACK);
			}
			break;

		case "VOLUME":
			AudioManager audioManager = (AudioManager) CavanAndroid.getSystemServiceCached(this, AUDIO_SERVICE);
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
					if (action == '=') {
						int volume = CavanJava.parseInt(args[1].substring(1));
						audioManager.setStreamVolume(AudioManager.STREAM_MUSIC, volume, 0);
					}

					break;
				}

				audioManager.adjustStreamVolume(AudioManager.STREAM_MUSIC, direction, 0);
			}
			break;

		default:
			CavanInputMethod ime = CavanInputMethod.getInstance();
			if (ime == null) {
				CavanAndroid.dLog("ime is null");
				break;
			}

			InputConnection conn = ime.getCurrentInputConnection();
			if (conn == null) {
				CavanAndroid.dLog("conn is null");
				break;
			}

			switch (args[0]) {
			case "SEND":
				send = true;
			case "REPLACE":
				if (!conn.performContextMenuAction(android.R.id.selectAll)) {
					break;
				}
			case "INSERT":
				if (args.length > 1) {
					conn.commitText(args[1], 0);

					if (send) {
						ime.postAutoCommit();
					}
				} else {
					conn.commitText(CavanString.EMPTY_STRING, 0);
				}
				break;

			case "DELETE":
				CharSequence text = conn.getSelectedText(0);
				if (text != null && text.length() > 0) {
					conn.commitText(CavanString.EMPTY_STRING, 1);
				} else {
					conn.deleteSurroundingText(1, 0);
				}
				break;

			case "COMMIT":
				if (accessibility != null && accessibility.commitText(ime)) {
					break;
				}
			case "DONE":
				ime.sendGoAction(conn);
				break;

			case "KEY":
				if (args.length > 1) {
					try {
						args = args[1].split("\\s+");

						int code = Integer.parseInt(args[0].trim());

						if (args.length > 1) {
							int value = Integer.parseInt(args[1].trim());
							ime.sendKeyEvent(code, value);
						} else {
							ime.sendKeyDownUp(code);
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
						conn.performEditorAction(action);
					} catch (NumberFormatException e) {
						e.printStackTrace();
					}
				}
				break;

			case "MENU_ACTION":
				if (args.length > 0) {
					try {
						int action = Integer.parseInt(args[1].trim());
						conn.performContextMenuAction(action);
					} catch (NumberFormatException e) {
						e.printStackTrace();
					}
				}
				break;

			case "COPY":
				conn.performContextMenuAction(android.R.id.copy);
				break;

			case "PASTE":
				conn.performContextMenuAction(android.R.id.paste);
				break;

			case "SELECT_ALL":
				conn.performContextMenuAction(android.R.id.selectAll);
				break;
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

		try {
			ArrayList<String> addresses = CavanNetworkImeClientPreference.load(this, CavanServiceActivity.KEY_NETWORK_IME);
			mBinder.setAddresses(addresses);
		} catch (RemoteException e) {
			e.printStackTrace();
		}
	}

	@Override
	protected int getDefaultPort() {
		return 8865;
	}
}
