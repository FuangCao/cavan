package com.cavan.cavanmain;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.DatagramPacket;
import java.net.InetAddress;
import java.net.MulticastSocket;
import java.net.Socket;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.HashMap;
import java.util.List;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Color;
import android.os.Handler;
import android.os.Handler.Callback;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager.LayoutParams;
import android.view.inputmethod.InputMethodManager;
import android.widget.TextView;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanPackageName;
import com.cavan.android.FloatWidowService;
import com.cavan.cavanjni.CavanJni;
import com.cavan.java.CavanString;
import com.cavan.java.CavanTimedArray;

public class FloatMessageService extends FloatWidowService {

	public static final int LAN_SHARE_PORT = 9898;
	public static final String LAN_SHARE_ADDR = "224.0.0.1";
	public static final String LAN_SHARE_PREFIX = "RedPacketCode: ";
	public static final String NETWORK_TEST_CODE = "CavanNetworkTest";

	public static final int TEXT_PADDING = 8;
	public static final float TEXT_SIZE_TIME = 16;
	public static final float TEXT_SIZE_MESSAGE = 12;
	public static final int TEXT_COLOR_TIME = Color.WHITE;
	public static final int TEXT_COLOR_MESSAGE = Color.YELLOW;

	private static final int MSG_SHOW_TOAST = 1;
	private static final int MSG_WAN_SERVICE_STATE_CHANGED = 2;
	private static final int MSG_WAN_SERVICE_UPDATED = 3;
	private static final int MSG_TCP_BRIDGE_STATE_CHANGED = 4;
	private static final int MSG_TCP_BRIDGE_UPDATED = 5;

	private int mLastSecond;
	private boolean mUserPresent;
	private TextView mTextViewTime;
	private TextView mTextViewAutoUnlock;
	private CavanTimedArray<String> mCodes = new CavanTimedArray<String>(600000);
	private HashMap<CharSequence, RedPacketCode> mMessageCodeMap = new HashMap<CharSequence, RedPacketCode>();

	private NetworkShareThread mNetShareThread;
	private UdpReceiveThread mUdpServiceThread;
	private TcpReceiveThread mTcpReceiveThread;
	private TcpBridgeThread mTcpBridgeThread;

	private Handler mHandler = new Handler() {

		@SuppressWarnings("deprecation")
		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case MSG_WAN_SERVICE_STATE_CHANGED:
				Intent intent = new Intent(MainActivity.ACTION_WAN_UPDATED);
				intent.putExtra("state", msg.arg1);
				sendStickyBroadcast(intent);

				CavanAndroid.showToast(getApplicationContext(), msg.arg1);
				break;

			case MSG_WAN_SERVICE_UPDATED:
				if (MainActivity.isWanShareEnabled(FloatMessageService.this)) {
					if (mTcpReceiveThread == null) {
						mTcpReceiveThread = new TcpReceiveThread();
						mTcpReceiveThread.start();
					} else {
						synchronized (mTcpReceiveThread) {
							mTcpReceiveThread.setActive(true);
							mTcpReceiveThread.closeSocket();
							mTcpReceiveThread.notify();
						}
					}
				} else if (mTcpReceiveThread != null) {
					mTcpReceiveThread.setActive(false);
				}
				break;

			case MSG_TCP_BRIDGE_STATE_CHANGED:
				intent = new Intent(MainActivity.ACTION_BRIDGE_UPDATED);
				intent.putExtra("state", msg.arg1);
				sendStickyBroadcast(intent);

				CavanAndroid.showToast(getApplicationContext(), msg.arg1);
				break;

			case MSG_TCP_BRIDGE_UPDATED:
				if (MainActivity.isTcpBridgeEnabled(FloatMessageService.this)) {
					if (mTcpBridgeThread == null) {
						mTcpBridgeThread = new TcpBridgeThread();
						mTcpBridgeThread.start();
					} else {
						synchronized (mTcpBridgeThread) {
							mTcpBridgeThread.setActive(true);
							mTcpBridgeThread.killCommand();
							mTcpBridgeThread.notify();
						}
					}
				} else if (mTcpBridgeThread != null) {
					mTcpBridgeThread.setActive(false);
				}
				break;

			case MSG_SHOW_TOAST:
				if (msg.obj instanceof String) {
					CavanAndroid.showToast(getApplicationContext(), (String) msg.obj);
				} else {
					CavanAndroid.showToast(getApplicationContext(), (int) msg.obj);
				}
				break;
			}
		}
	};

	private InputMethodPickerRunnable mInputMethodPickerRunnable = new InputMethodPickerRunnable();

	private BroadcastReceiver mReceiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {
			String action = intent.getAction();

			CavanAndroid.eLog("action = " + action);

			switch (action) {
			case Intent.ACTION_SCREEN_OFF:
				CavanAndroid.setLockScreenEnable(FloatMessageService.this, true);
				mUserPresent = false;
				break;

			case Intent.ACTION_SCREEN_ON:
				if (getTextCount() > 0 || CavanAndroid.isPreferenceEnabled(FloatMessageService.this, MainActivity.KEY_AUTO_UNLOCK)) {
					unlockScreen();
				}
				break;

			case Intent.ACTION_USER_PRESENT:
				mTextViewAutoUnlock.setVisibility(View.INVISIBLE);
				mUserPresent = true;
				break;
			}
		}
	};

	private IFloatMessageService.Stub mBinder = new IFloatMessageService.Stub() {

		private void sendCodeUpdateBroadcast(String action, RedPacketCode code) {
			Intent intent = new Intent(action);
			intent.putExtra("code", code);
			sendBroadcast(intent);
		}

		@Override
		public boolean setTimerEnable(boolean enable) throws RemoteException {
			return FloatMessageService.this.setTimerEnable(enable);
		}

		@Override
		public boolean getTimerState() throws RemoteException {
			return mTextViewTime != null && mTextViewTime.getVisibility() != View.INVISIBLE;
		}

		@Override
		public int addMessage(CharSequence message, RedPacketCode code) throws RemoteException {
			TextView view = (TextView) FloatMessageService.this.addText(message, -1);
			if (view == null) {
				return -1;
			}

			unlockScreen();

			if (code != null) {
				mMessageCodeMap.put(message, code);
				mInputMethodPickerRunnable.post();

				sendCodeUpdateBroadcast(MainActivity.ACTION_CODE_ADD, code);

				if (code.isNetShared() == false && mNetShareThread != null) {
					long delay = code.getTime() - System.currentTimeMillis();
					mNetShareThread.sendCode(code.getCode(), delay);
				}

				mCodes.addTimedValue(code.getCode());
			}

			return view.getId();
		}

		@Override
		public boolean hasMessage(CharSequence message) throws RemoteException {
			return FloatMessageService.this.hasText(message);
		}

		@Override
		public void removeMessage(CharSequence message) throws RemoteException {
			FloatMessageService.this.removeText(message);

			RedPacketCode code = mMessageCodeMap.get(message);
			if (code != null) {
				mMessageCodeMap.remove(message);
				sendCodeUpdateBroadcast(MainActivity.ACTION_CODE_REMOVE, code);
			}
		}

		@Override
		public List<String> getMessages() throws RemoteException {
			List<String> list = new ArrayList<String>();
			for (CharSequence text : getTextSet()) {
				list.add(text.toString());
			}

			return list;
		}

		@Override
		public int getMessageCount() throws RemoteException {
			return FloatMessageService.this.getTextCount();
		}

		@Override
		public List<RedPacketCode> getCodes() throws RemoteException {
			List<RedPacketCode> codes = new ArrayList<RedPacketCode>();

			for (RedPacketCode code : mMessageCodeMap.values()) {
				codes.add(code);
			}

			return codes;
		}

		@Override
		public int getCodeCount() throws RemoteException {
			return mMessageCodeMap.size();
		}

		@Override
		public boolean sendSharedCode(String code) throws RemoteException {
			if (mNetShareThread != null) {
				return mNetShareThread.sendCode(code, 0);
			}

			return false;
		}

		@Override
		public void updateTcpService() throws RemoteException {
			mHandler.sendEmptyMessageDelayed(MSG_WAN_SERVICE_UPDATED, 500);
		}

		@Override
		public void updateTcpBridge() throws RemoteException {
			mHandler.sendEmptyMessageDelayed(MSG_TCP_BRIDGE_UPDATED, 500);
		}
	};

	private Runnable mRunnableTime = new Runnable() {

		@Override
		public void run() {
			Calendar calendar = Calendar.getInstance();
			int second = calendar.get(Calendar.SECOND);
			if (second == mLastSecond) {
				mTextViewTime.postDelayed(this, 100);
			} else {
				mLastSecond = second;
				mTextViewTime.postDelayed(this, 1000);
				mTextViewTime.setText(getTimeText(calendar, second));
			}
		}
	};

	public static Intent buildIntent(Context context) {
		return new Intent(context, FloatMessageService.class);
	}

	public static Intent startService(Context context) {
		Intent intent = buildIntent(context);
		context.startService(intent);
		return intent;
	}

	public boolean unlockScreen() {
		if (mUserPresent) {
			return false;
		}

		mTextViewAutoUnlock.setVisibility(View.VISIBLE);
		CavanAndroid.setLockScreenEnable(FloatMessageService.this, false);

		return true;
	}

	public String getTimeText(Calendar calendar, int second) {
		int hour = calendar.get(Calendar.HOUR_OF_DAY);
		int minute = calendar.get(Calendar.MINUTE);

		return String.format("%02d:%02d:%02d", hour, minute, second);
	}

	public String getTimeText(Calendar calendar) {
		return getTimeText(calendar, calendar.get(Calendar.SECOND));
	}

	public String getTimeText() {
		return getTimeText(Calendar.getInstance());
	}

	public boolean setTimerEnable(boolean enable) {
		if (mTextViewTime == null) {
			return false;
		}

		if (enable) {
			mLastSecond = -1;
			mTextViewTime.setVisibility(View.VISIBLE);
			mTextViewTime.post(mRunnableTime);
		} else {
			mTextViewTime.removeCallbacks(mRunnableTime);
			mTextViewTime.setVisibility(View.INVISIBLE);
		}

		return true;
	}

	public void initTextView(TextView view, CharSequence text) {
		if (text != null) {
			view.setText(text);
		}

		view.setMaxLines(1);
		view.setPadding(TEXT_PADDING, 0, TEXT_PADDING, 0);
		view.setBackgroundResource(R.drawable.desktop_timer_bg);
	}

	private void sendCodeReceivedBroadcast(String type, String text) {
		CavanAndroid.eLog("receive = " + text);

		if (text.startsWith(LAN_SHARE_PREFIX)) {
			String code = CavanString.deleteSpace(text.substring(LAN_SHARE_PREFIX.length()));

			CavanAndroid.eLog("code = " + code);

			if (code.equals(NETWORK_TEST_CODE)) {
				text = getResources().getString(R.string.text_network_test_success, type);
				mHandler.obtainMessage(MSG_SHOW_TOAST, text).sendToTarget();
			} else if (!mCodes.hasTimedValue(code)) {
				Intent intent = new Intent(MainActivity.ACTION_CODE_RECEIVED);
				intent.putExtra("type", type);
				intent.putExtra("code", code);
				sendBroadcast(intent);
			}
		}
	}

	@Override
	protected View createRootView() {
		return View.inflate(this, R.layout.float_message, null);
	}

	@Override
	protected LayoutParams createRootViewLayoutParams() {
		LayoutParams params = super.createRootViewLayoutParams();
		if (params == null) {
			return null;
		}

		params.width = LayoutParams.MATCH_PARENT;

		return params;
	}

	@Override
	public void onCreate() {
		IntentFilter filter = new IntentFilter();
		filter.addAction(Intent.ACTION_SCREEN_OFF);
		filter.addAction(Intent.ACTION_SCREEN_ON);
		filter.addAction(Intent.ACTION_USER_PRESENT);
		registerReceiver(mReceiver, filter );

		mNetShareThread = new NetworkShareThread();
		mNetShareThread.start();

		mUdpServiceThread = new UdpReceiveThread();
		mUdpServiceThread.start();

		mHandler.sendEmptyMessage(MSG_WAN_SERVICE_UPDATED);
		mHandler.sendEmptyMessage(MSG_TCP_BRIDGE_UPDATED);

		super.onCreate();
	}

	@Override
	public void onDestroy() {
		if (mTcpReceiveThread != null) {
			mTcpReceiveThread.setActive(false);
		}

		if (mUdpServiceThread != null) {
			mUdpServiceThread.setActive(false);
		}

		if (mNetShareThread != null) {
			mNetShareThread.quit();
		}

		unregisterReceiver(mReceiver);
		setTimerEnable(false);

		super.onDestroy();
	}

	@Override
	public IBinder onBind(Intent intent) {
		return mBinder;
	}

	@Override
	protected View createView(CharSequence text) {
		TextView view = new TextView(getApplicationContext());

		initTextView(view, text);

		view.setTextSize(TEXT_SIZE_MESSAGE);
		view.setTextColor(TEXT_COLOR_MESSAGE);
		view.setGravity(Gravity.LEFT | Gravity.CENTER_VERTICAL);

		return view;
	}

	@Override
	protected CharSequence getViewText(View arg0) {
		TextView view = (TextView) arg0;
		return view.getText();
	}

	@Override
	protected ViewGroup findViewGroup() {
		return (ViewGroup) findViewById(R.id.layoutMessage);
	}

	@Override
	protected boolean doInitialize() {
		mTextViewTime = (TextView) findViewById(R.id.textViewTime);

		initTextView(mTextViewTime, getTimeText());
		mTextViewTime.setTextSize(TEXT_SIZE_TIME);
		mTextViewTime.setTextColor(TEXT_COLOR_TIME);

		if (MainActivity.isFloatTimerEnabled(this)) {
			setTimerEnable(true);
		} else {
			setTimerEnable(false);
		}

		mTextViewAutoUnlock = (TextView) findViewById(R.id.textViewAutoUnlock);

		initTextView(mTextViewAutoUnlock, null);
		mTextViewAutoUnlock.setTextSize(TEXT_SIZE_TIME);
		mTextViewAutoUnlock.setTextColor(TEXT_COLOR_TIME);

		return super.doInitialize();
	}

	public class InputMethodPickerRunnable implements Runnable {

		private boolean mAlipayRunning;

		public void post() {
			mAlipayRunning = false;
			mHandler.post(this);
		}

		@Override
		public void run() {
			if (getTextCount() <= 0) {
				return;
			}

			if (CavanInputMethod.isDefaultInputMethod(getApplicationContext())) {
				return;
			}

			if (mAlipayRunning) {
				InputMethodManager manager = (InputMethodManager) getSystemService(INPUT_METHOD_SERVICE);
				manager.showInputMethodPicker();
			} else {
				ComponentName info = CavanAndroid.getTopActivityInfo(getApplicationContext());
				if (info != null && CavanPackageName.ALIPAY.equals(info.getPackageName())) {
					mAlipayRunning = true;
				}

				mHandler.removeCallbacks(this);
				mHandler.postDelayed(this, 500);
			}
		}
	}

	public class NetworkShareThread extends HandlerThread implements Callback {

		private Handler mUdpHandler;
		private MulticastSocket mUdpSocket;

		public NetworkShareThread() {
			super("NetworkShareThread");
		}

		public boolean sendCode(String code, long delay) {
			if (mUdpHandler == null) {
				return false;
			}

			Message message = mUdpHandler.obtainMessage(0, code);

			if (delay > 0) {
				mUdpHandler.sendMessageDelayed(message, delay);
			} else {
				mUdpHandler.sendMessage(message);
			}

			return false;
		}

		@Override
		public boolean quit() {
			boolean result = super.quit();

			if (mUdpSocket != null) {
				mUdpSocket.close();
				mUdpSocket = null;
			}

			return result;
		}

		@Override
		protected void onLooperPrepared() {
			mUdpHandler = new Handler(getLooper(), this);
		}

		@Override
		public boolean handleMessage(Message msg) {
			String code = (String) msg.obj;
			byte[] bytes = (LAN_SHARE_PREFIX + code).getBytes();

			CavanAndroid.eLog("send" + msg.what + " = " + code);

			if (msg.what == 0 && mTcpReceiveThread != null) {
				mTcpReceiveThread.sendCodeBytes(bytes);
			}

			if (MainActivity.isLanShareEnabled(FloatMessageService.this)) {
				try {
					if (mUdpSocket == null) {
						mUdpSocket = new MulticastSocket();
					}

					DatagramPacket pack = new DatagramPacket(bytes, bytes.length,
							InetAddress.getByName(LAN_SHARE_ADDR), LAN_SHARE_PORT);

					mUdpSocket.send(pack);
				} catch (Exception e) {
					e.printStackTrace();

					if (mUdpSocket != null) {
						mUdpSocket.close();
						mUdpSocket = null;
					}
				}

				if (msg.what < 10) {
					int index = msg.what + 1;

					Message message = mUdpHandler.obtainMessage(index, code);
					mUdpHandler.sendMessageDelayed(message, index * 100);
				}
			}

			return true;
		}
	}

	public class UdpReceiveThread extends Thread {

		private boolean mActive;
		private MulticastSocket mSocket;

		public void setActive(boolean enable) {
			if (enable) {
				mActive = true;
			} else {
				mActive = false;

				if (mSocket != null) {
					MulticastSocket socket = mSocket;

					mSocket = null;
					socket.close();
				}
			}
		}

		@Override
		public void run() {
			mActive = true;

			while (mActive) {
				if (mSocket != null) {
					mSocket.close();
				}

				try {
					mSocket = new MulticastSocket(LAN_SHARE_PORT);
					mSocket.joinGroup(InetAddress.getByName(LAN_SHARE_ADDR));
				} catch (IOException e) {
					e.printStackTrace();

					synchronized (this) {
						try {
							wait(2000);
						} catch (InterruptedException e1) {
							e1.printStackTrace();
						}
					}

					continue;
				}

				CavanAndroid.eLog("UdpServiceThread running");

				byte[] bytes = new byte[128];
				DatagramPacket pack = new DatagramPacket(bytes, bytes.length);

				while (mActive) {
					try {
						mSocket.receive(pack);

						if (MainActivity.isLanShareEnabled(FloatMessageService.this)) {
							String text = new String(pack.getData(), 0, pack.getLength());
							sendCodeReceivedBroadcast("内网分享", text);
						}
					} catch (IOException e) {
						e.printStackTrace();
						break;
					}
				}

				CavanAndroid.eLog("UdpServiceThread stopping");
			}

			mUdpServiceThread = null;

			if (mSocket != null) {
				mSocket.close();
				mSocket = null;
			}

			mActive = false;
		}
	}

	public class TcpReceiveThread extends Thread {

		private boolean mActive;

		private Socket mSocket;
		private InputStream mInputStream;
		private OutputStream mOutputStream;

		synchronized public boolean sendCodeBytes(byte[] bytes) {
			if (mOutputStream == null) {
				return false;
			}

			try {
				mOutputStream.write(bytes);
				mOutputStream.write('\n');
				return true;
			} catch (IOException e) {
				e.printStackTrace();
			}

			return false;
		}

		synchronized public void closeSocket() {
			if (mOutputStream != null) {
				try {
					mOutputStream.close();
				} catch (IOException e1) {
					e1.printStackTrace();
				}

				mOutputStream = null;
			}

			if (mInputStream != null) {
				try {
					mInputStream.close();
				} catch (IOException e1) {
					e1.printStackTrace();
				}

				mInputStream = null;
			}

			if (mSocket != null) {
				try {
					mSocket.close();
				} catch (IOException e1) {
					e1.printStackTrace();
				}

				mSocket = null;
			}
		}

		synchronized public void setActive(boolean active) {
			if (active) {
				mActive = true;
			} else {
				mActive = false;
				closeSocket();
			}
		}

		@Override
		public void run() {
			mActive = true;

			while (mActive && MainActivity.isWanShareEnabled(getApplicationContext())) {
				String host = MainActivity.getWanShareIpAddress(getApplicationContext());
				if (host == null) {
					break;
				}

				int port = MainActivity.getWanSharePort(getApplicationContext());
				if (port < 0) {
					break;
				}

				try {
					mHandler.obtainMessage(MSG_WAN_SERVICE_STATE_CHANGED, R.string.text_wan_connecting, 0).sendToTarget();

					mSocket = new Socket(InetAddress.getByName(host), port);
					mInputStream = mSocket.getInputStream();
					mOutputStream = mSocket.getOutputStream();

					mHandler.obtainMessage(MSG_WAN_SERVICE_STATE_CHANGED, R.string.text_wan_connected, 0).sendToTarget();

					BufferedReader reader = new BufferedReader(new InputStreamReader(mInputStream));

					while (true) {
						try {
							String line = reader.readLine();
							CavanAndroid.eLog("line = " + line);

							sendCodeReceivedBroadcast("外网分享", line);
						} catch (IOException e) {
							e.printStackTrace();
							break;
						}
					}

					mHandler.obtainMessage(MSG_WAN_SERVICE_STATE_CHANGED, R.string.text_wan_disconnected, 0).sendToTarget();
				} catch (Exception e) {
					e.printStackTrace();

					if (mActive) {
						synchronized (this) {
							try {
								wait(10000);
							} catch (InterruptedException e1) {
								e1.printStackTrace();
							}
						}
					}
				} finally {
					closeSocket();
				}
			}

			mTcpReceiveThread = null;
		}
	}

	public class TcpBridgeThread extends Thread {

		private boolean mActive;

		public void killCommand() {
			CavanJni.kill("tcp_bridge");
		}

		public void setActive(boolean active) {
			if (active) {
				mActive = true;
			} else {
				mActive = false;
				killCommand();
			}
		}

		@Override
		public void run() {
			mActive = true;

			while (mActive && MainActivity.isTcpBridgeEnabled(getApplicationContext())) {
				String setting = MainActivity.getTcpBridgeSetting(getApplicationContext());
				if (setting == null) {
					break;
				}

				String[] settings = setting.split("\\n");
				if (settings.length != 2) {
					break;
				}

				String url1 = "tcp://" + CavanString.deleteSpace(settings[0]);
				String url2 = "tcp://" + CavanString.deleteSpace(settings[1]);

				mHandler.obtainMessage(MSG_TCP_BRIDGE_STATE_CHANGED, R.string.text_tcp_bridge_running, 0).sendToTarget();
				CavanJni.doTcpBridge(url1, url2);
				mHandler.obtainMessage(MSG_TCP_BRIDGE_STATE_CHANGED, R.string.text_tcp_bridge_exit, 0).sendToTarget();

				if (mActive) {
					synchronized (this) {
						try {
							wait(10000);
						} catch (InterruptedException e) {
							e.printStackTrace();
						}
					}
				}
			}

			mTcpBridgeThread = null;
			mHandler.obtainMessage(MSG_TCP_BRIDGE_STATE_CHANGED, R.string.text_tcp_bridge_stopped, 0).sendToTarget();
		}
	}
}
