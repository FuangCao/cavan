package com.cavan.cavanmain;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Color;
import android.net.ConnectivityManager;
import android.os.Build;
import android.os.Environment;
import android.os.FileObserver;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager.LayoutParams;
import android.widget.TextView;

import com.cavan.accessibility.CavanAccessibilityAlipay;
import com.cavan.accessibility.CavanAccessibilityService;
import com.cavan.accessibility.CavanRedPacketAlipay;
import com.cavan.accessibility.CavanUnlockActivity;
import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanCursorView;
import com.cavan.android.CavanThreadedHandler;
import com.cavan.android.CavanWakeLock;
import com.cavan.cavanjni.CavanJni;
import com.cavan.java.CavanString;
import com.cavan.java.CavanTcpClient;
import com.cavan.service.FloatWindowService;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.DatagramPacket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.MulticastSocket;
import java.net.Socket;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.HashMap;
import java.util.List;

public class FloatMessageService extends FloatWindowService {

	public static final long KEEP_ALIVE_DELAY = 120000;
	public static final long UPDATE_LOCATION_DELAY = 2000;

	public static final String NET_CMD_RDPKG = "RedPacketCode: ";
	public static final String NET_CMD_UPDATE = "RedPacketUpdate: ";
	public static final String NET_CMD_DELETE = "RedPacketDelete: ";
	public static final String NET_CMD_TEST = "CavanNetworkTest";
	public static final String NET_CMD_KEEP_ALIVE = "CavanKeepAlive";
	public static final String NET_CMD_TM_CODE = "SecretOrder: ";
	public static final String NET_CMD_CLIPBOARD = "Clipboard: ";
	public static final String NET_CMD_NOTIFY = "Notify: ";

	public static final int AUTO_UNLOCK_ALWAYS = 0;
	public static final int AUTO_UNLOCK_RED_PACKET = 1;
	public static final int AUTO_UNLOCK_INFOMATION = 2;
	public static final int AUTO_UNLOCK_KEY_WORD = 3;
	public static final int AUTO_UNLOCK_SCREEN_ON = 4;

	public static final String PATH_QQ_IMAGES = Environment.getExternalStorageDirectory().getPath() + "/Tencent/QQ_Images";

	public static final int UDP_PORT = 9898;
	public static final String UDP_ADDR = "224.0.0.1";

	public static final int TEXT_PADDING = 8;
	public static final float TEXT_SIZE_TIME = 16;
	public static final float TEXT_SIZE_MESSAGE = 12;
	public static final int TEXT_COLOR_TIME = Color.WHITE;
	public static final int TEXT_COLOR_MESSAGE = Color.YELLOW;

	private static final int MSG_UPDATE_TIME = 0;
	private static final int MSG_SHOW_TOAST = 1;
	private static final int MSG_SHOW_NOTIFY = 2;
	private static final int MSG_TCP_SERVICE_STATE_CHANGED = 3;
	private static final int MSG_TCP_SERVICE_UPDATED = 4;
	private static final int MSG_TCP_BRIDGE_STATE_CHANGED = 5;
	private static final int MSG_TCP_BRIDGE_UPDATED = 6;
	private static final int MSG_START_OCR = 7;
	private static final int MSG_CLIPBOARD_RECEIVED = 8;
	private static final int MSG_CHECK_KEYGUARD = 9;
	private static final int MSG_RED_PACKET_UPDATED = 10;
	private static final int MSG_CHECK_SERVICE_STATE = 11;

	public static final int MSG_SEND_UDP_COMMAND = 1;
	public static final int MSG_SEND_TCP_COMMAND = 2;

	public static FloatMessageService instance;

	private long mCountDownTime;
	private boolean mScreenClosed;
	private int mAutoUnlockLevel;
	private TextView mTextViewTime;
	private TextView mTextViewToast;
	private TextView mTextViewNotify;
	private CavanWakeLock mWakeLock = new CavanWakeLock(FloatMessageService.class.getCanonicalName());
	private HashMap<CharSequence, CavanRedPacketAlipay> mMessageCodeMap = new HashMap<CharSequence, CavanRedPacketAlipay>();

	private CavanCursorView mCursorView;
	private UdpDaemonThread mUdpDaemon;
	private TcpBridgeThread mTcpBridge;

	private TcpSharedClient mTcpClient = new TcpSharedClient();

	private boolean mNetworkConnected;
	private NetworkSendHandler mNetworkSendHandler = new NetworkSendHandler();

	private int mWanState = R.string.wan_disconnected;
	private int mBridgeState = R.string.tcp_bridge_exit;

	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case MSG_UPDATE_TIME:
				removeMessages(MSG_UPDATE_TIME);

				synchronized (mHandler) {
					if (mScreenClosed || mTextViewTime.getVisibility() != View.VISIBLE) {
						break;
					}

					Calendar calendar = Calendar.getInstance();

					if (mCountDownTime > 0) {
						long time = System.currentTimeMillis();

						if (mCountDownTime > time) {
							long delay = mCountDownTime - time;

							if (delay > 100) {
								sendEmptyMessageDelayed(MSG_UPDATE_TIME, 100);
							} else {
								sendEmptyMessageDelayed(MSG_UPDATE_TIME, delay);
							}

							mTextViewTime.setText(getTimeText(calendar) + "-" + (delay / 100));
							break;
						}

						mCountDownTime = 0;
					}

					int msecond = calendar.get(Calendar.MILLISECOND);
					sendEmptyMessageDelayed(MSG_UPDATE_TIME, 1000 - msecond);
					mTextViewTime.setText(getTimeText(calendar));
				}
				break;

			case MSG_TCP_SERVICE_STATE_CHANGED: {
					CavanMessageActivity activity = CavanMessageActivity.instance;
					if (activity != null) {
						activity.updateWanState(mWanState, getWanSummary());
					}

					if (mWanState == R.string.wan_connected) {
						postShowToast(mWanState);
					}
				}
				break;

			case MSG_TCP_SERVICE_UPDATED:
				if (mNetworkConnected && CavanMessageActivity.isWanShareEnabled(FloatMessageService.this)) {
					mTcpClient.reconnect();
				} else {
					mTcpClient.disconnect();
				}
				break;

			case MSG_TCP_BRIDGE_STATE_CHANGED: {
					CavanMessageActivity activity = CavanMessageActivity.instance;
					if (activity != null) {
						activity.updateBridgeState(mBridgeState);
					}
				}
				break;

			case MSG_TCP_BRIDGE_UPDATED:
				if (mNetworkConnected && CavanMessageActivity.isTcpBridgeEnabled(FloatMessageService.this)) {
					if (mTcpBridge == null) {
						mTcpBridge = new TcpBridgeThread();
						mTcpBridge.start();
					} else {
						synchronized (mTcpBridge) {
							mTcpBridge.setActive(true);
							mTcpBridge.killCommand();
							mTcpBridge.notify();
						}
					}
				} else if (mTcpBridge != null) {
					mTcpBridge.setActive(false);
				}
				break;

			case MSG_SHOW_TOAST:
			case MSG_SHOW_NOTIFY:
				TextView view;

				if (msg.what == MSG_SHOW_TOAST) {
					CavanAndroid.dLog("MSG_SHOW_TOAST");
					view = mTextViewToast;
				} else {
					CavanAndroid.dLog("MSG_SHOW_NOTIFY");
					view = mTextViewNotify;
				}

				removeMessages(msg.what);

				if (view == null) {
					break;
				}

				if (msg.obj == null) {
					view.setVisibility(View.GONE);
				} else {
					CharSequence message;

					if (msg.what == MSG_SHOW_NOTIFY) {
						setAutoUnlockLevel(AUTO_UNLOCK_KEY_WORD);
					}

					if (msg.obj instanceof CharSequence) {
						message = (CharSequence) msg.obj;
					} else {
						message = getResources().getString((int) msg.obj);
					}

					CavanAndroid.dLog("message = " + message);

					view.setText(message);
					view.setVisibility(View.VISIBLE);
					sendEmptyMessageDelayed(msg.what, 10000);
				}
				break;

			case MSG_START_OCR:
				CavanMessageActivity.startSogouOcrActivity(getApplicationContext());
				break;

			case MSG_CLIPBOARD_RECEIVED:
				String code = (String) msg.obj;
				String text = getResources().getString(R.string.clipboard_updated, code);
				postShowToast(text);
				CavanAndroid.postClipboardTextTemp(getApplicationContext(), code);
				break;

			case MSG_CHECK_KEYGUARD:
				removeMessages(MSG_CHECK_KEYGUARD);

				synchronized (mHandler) {
					if (mScreenClosed) {
						break;
					}

					if (CavanAndroid.inKeyguardRestrictedInputMode(FloatMessageService.this)) {
						sendEmptyMessageDelayed(MSG_CHECK_KEYGUARD, 2000);
					} else {
						mTextViewTime.setBackgroundResource(R.drawable.desktop_timer_bg);
					}
				}
				break;

			case MSG_RED_PACKET_UPDATED:
				CavanRedPacketAlipay node = (CavanRedPacketAlipay) msg.obj;
				text = getResources().getString(R.string.red_packet_code_updated, node.getCode());
				postShowToast(text);
				break;

			case MSG_CHECK_SERVICE_STATE:
				removeMessages(MSG_CHECK_SERVICE_STATE);

				if (!CavanMessageActivity.isAutoCheckPermissionEnabled(FloatMessageService.this)) {
					break;
				}

				if (checkServiceState()) {
					break;
				}

				sendEmptyMessageDelayed(MSG_CHECK_SERVICE_STATE, 5000);
				break;
			}
		}
	};

	private BroadcastReceiver mReceiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {
			String action = intent.getAction();

			CavanAndroid.dLog("action = " + action);

			switch (action) {
			case Intent.ACTION_SCREEN_OFF:
				setLockScreenEnable(true);
				mAutoUnlockLevel = AUTO_UNLOCK_SCREEN_ON;

				hiddenCursorView();

				synchronized (mHandler) {
					mScreenClosed = true;
				}

				CavanAndroid.releaseWakeLock();
				break;

			case Intent.ACTION_SCREEN_ON:
				synchronized (mHandler) {
					mScreenClosed = false;
					mHandler.sendEmptyMessage(MSG_UPDATE_TIME);
				}

				if (isAutoUnlockEnabled()) {
					setLockScreenEnable(false);
				}

				mTextViewTime.setBackgroundResource(R.drawable.desktop_timer_unlock_bg);
				break;

			case ConnectivityManager.CONNECTIVITY_ACTION:
				updateNetworkConnState();
				break;

			case CavanMessageActivity.ACTION_SERVICE_EXIT:
				if (!CavanMessageActivity.isAutoCheckPermissionEnabled(FloatMessageService.this)) {
					break;
				}

				String service = intent.getStringExtra("service");
				if (service == null) {
					break;
				}

				if (service.equals(CavanMainAccessibilityService.class.getCanonicalName())) {
					CavanMainAccessibilityService.checkAndOpenSettingsActivity(getApplicationContext());
				} else if (service.equals(RedPacketListenerService.class.getCanonicalName())) {
					RedPacketListenerService.checkAndOpenSettingsActivity(getApplicationContext());
				}
				break;
			}
		}
	};

	private FileObserver mFileObserverQQ = new FileObserver(PATH_QQ_IMAGES, FileObserver.CLOSE_WRITE) {

		private String mPath;

		@Override
		public void onEvent(int event, String path) {
			if (path.equals(mPath)) {
				path = mPath;
			} else {
				mPath = path;
			}

			mHandler.removeMessages(MSG_START_OCR, path);
			Message message = mHandler.obtainMessage(MSG_START_OCR, path);
			mHandler.sendMessageDelayed(message, 200);
		}
	};

	private IFloatMessageService.Stub mBinder = new IFloatMessageService.Stub() {

		@Override
		public boolean setTimerEnable(boolean enable) throws RemoteException {
			return FloatMessageService.this.setTimerEnable(enable);
		}

		@Override
		public boolean getTimerState() throws RemoteException {
			return mTextViewTime != null && mTextViewTime.getVisibility() == View.VISIBLE;
		}

		@Override
		public int addMessage(CharSequence message, String code, int level) throws RemoteException {
			setAutoUnlockLevel(level);

			if (code != null) {
				CavanRedPacketAlipay packet = CavanRedPacketAlipay.get(code);
				if (packet != null) {
					mMessageCodeMap.put(message, packet);

					if (packet.isCompleted()) {
						postShowToast(R.string.ignore_completed_code, code);
					} else {
						CavanAccessibilityService service = CavanAccessibilityService.instance;
						if (service != null) {
							service.addPacket(CavanAccessibilityAlipay.instance, packet);
						}
					}

					if (packet.isSendEnabled()) {
						if (packet.isRepeatable()) {
							mNetworkSendHandler.sendCode(code, 0);
						} else {
							long delay = packet.getUnpackDelay(0);
							mNetworkSendHandler.sendCode(code, delay);
						}
					}
				}
			}

			FloatMessageService.this.removeText(message);
			TextView view = (TextView) FloatMessageService.this.addText(message, -1);
			if (view == null) {
				return -1;
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

			CavanRedPacketAlipay packet = mMessageCodeMap.get(message);
			if (packet != null) {
				packet.setNetworkEnable(true, true);
				mMessageCodeMap.remove(message);
				CavanAccessibilityService service = CavanAccessibilityService.instance;
				if (service != null) {
					service.removePacket(packet);
				}
			}
		}

		@Override
		public void removeMessageAll() throws RemoteException {
			FloatMessageService.this.removeTextAll();
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
		public List<String> getCodes() throws RemoteException {
			List<String> codes = new ArrayList<String>();

			for (CavanRedPacketAlipay packet : mMessageCodeMap.values()) {
				codes.add(packet.getCode());
			}

			return codes;
		}

		@Override
		public int getCodeCount() throws RemoteException {
			return mMessageCodeMap.size();
		}

		@Override
		public int getCodePending() throws RemoteException {
			int count = 0;

			for (CavanRedPacketAlipay packet : mMessageCodeMap.values()) {
				if (!packet.isCompleted()) {
					count++;
				}
			}

			return count;
		}

		@Override
		public void updateTcpService() throws RemoteException {
			mHandler.sendEmptyMessageDelayed(MSG_TCP_SERVICE_UPDATED, 500);
		}

		@Override
		public void updateTcpBridge() throws RemoteException {
			mHandler.sendEmptyMessageDelayed(MSG_TCP_BRIDGE_UPDATED, 500);
		}

		@Override
		public boolean sendRedPacketCode(String code) throws RemoteException {
			return mNetworkSendHandler.sendCode(code, 0);
		}

		@Override
		public boolean sendUdpCommand(String command) throws RemoteException {
			return mNetworkSendHandler.sendUdpCommand(command, 0, 0);
		}

		@Override
		public boolean sendTcpCommand(String command) throws RemoteException {
			return mNetworkSendHandler.sendTcpCommand(command, 0);
		}

		@Override
		public boolean isSuspendDisabled() throws RemoteException {
			return FloatMessageService.this.isSuspendDisabled();
		}

		@Override
		public void setSuspendDisable(boolean disable) throws RemoteException {
			FloatMessageService.this.setSuspendDisable(disable);
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

	public int getAutoUnlockLevel() {
		return mAutoUnlockLevel;
	}

	public void setAutoUnlockLevel(int level) {
		if (level < mAutoUnlockLevel) {
			mAutoUnlockLevel = level;
		}

		CavanAndroid.acquireWakeupLock(getApplicationContext(), 20000);
		CavanAndroid.dLog("mScreenClosed = " + mScreenClosed);

		if (mScreenClosed) {
			return;
		}

		if (isAutoUnlockEnabled()) {
			setLockScreenEnable(false);
		}
	}

	public boolean isAutoUnlockEnabled() {
		if (mAutoUnlockLevel == AUTO_UNLOCK_ALWAYS) {
			return true;
		}

		return CavanMessageActivity.isAutoUnlockEnabled(getApplicationContext(), mAutoUnlockLevel);
	}

	public void setCountDownTime(long time) {
		synchronized (mHandler) {
			mCountDownTime = time;
			mHandler.sendEmptyMessage(MSG_UPDATE_TIME);
		}
	}

	public void showOnTimeNotify() {
		if (CavanMessageActivity.isOnTimeNotifyEnabledNow(this)) {
			postShowNotify(R.string.on_time_notify);
		}
	}

	public void postShowToast(int what, Object messsage) {
		Message message = mHandler.obtainMessage(what, messsage);
		message.sendToTarget();
	}

	public void postShowToast(Object messsage) {
		postShowToast(MSG_SHOW_TOAST, messsage);
	}

	public void postShowNotify(Object messsage) {
		postShowToast(MSG_SHOW_NOTIFY, messsage);

	}

	public void postShowToastWithArgs(int id, Object... formatArgs) {
		String message = getResources().getString(id, formatArgs);
		postShowToast(MSG_SHOW_TOAST, message);
	}

	public static boolean showToast(int what, Object messsage) {
		FloatMessageService service = instance;
		if (service != null) {
			service.postShowToast(what, messsage);
			return true;
		}

		return false;
	}

	public static boolean showToast(Object messsage) {
		return showToast(MSG_SHOW_TOAST, messsage);
	}

	public static boolean showNotify(Object messsage) {
		return showToast(MSG_SHOW_NOTIFY, messsage);
	}

	private boolean checkServiceState() {
		if (!CavanMainAccessibilityService.checkAndOpenSettingsActivity(this)) {
			return false;
		}

		if (Build.VERSION.SDK_INT < Build.VERSION_CODES.ICE_CREAM_SANDWICH) {
			return true;
		}

		return RedPacketListenerService.checkAndOpenSettingsActivity(this);
	}

	public boolean isSuspendDisabled() {
		return mWakeLock.isHeld();
	}

	public void setSuspendDisable(boolean disable) {
		if (disable) {
			mWakeLock.acquire(this);
		} else {
			mWakeLock.release();
		}
	}

	public boolean setLockScreenEnable(boolean enable) {
		if (CavanMessageActivity.isDisableKeyguardEnabled(this)) {
			enable = true;
		}

		CavanUnlockActivity.setLockScreenEnable(this, enable);

		return true;
	}

	public String getTimeText(Calendar calendar) {
		int hour = calendar.get(Calendar.HOUR_OF_DAY);
		int minute = calendar.get(Calendar.MINUTE);
		int second = calendar.get(Calendar.SECOND);

		return String.format("%02d:%02d:%02d", hour, minute, second);
	}

	public String getTimeText() {
		return getTimeText(Calendar.getInstance());
	}

	public boolean setTimerEnable(boolean enable) {
		if (mTextViewTime == null) {
			return false;
		}

		if (enable) {
			mTextViewTime.setVisibility(View.VISIBLE);
			mHandler.sendEmptyMessage(MSG_UPDATE_TIME);
		} else {
			mTextViewTime.setVisibility(View.INVISIBLE);
		}

		return true;
	}

	public void initTextView(TextView view, float size, int color, CharSequence text) {
		if (text != null) {
			view.setText(text);
		}

		view.setMaxLines(3);
		view.setTextSize(size);
		view.setTextColor(color);
		view.setPadding(TEXT_PADDING, 0, TEXT_PADDING, 0);
		view.setBackgroundResource(R.drawable.desktop_timer_unlock_bg);
	}

	private void onNetworkCommandReceived(String type, String command) {
		CavanAndroid.dLog("receive = " + command);

		if (command.equals(NET_CMD_TEST)) {
			command = getResources().getString(R.string.network_test_success, type);
			postShowToast(command);
		} else if (command.startsWith(NET_CMD_RDPKG)) {
			String code = CavanString.deleteSpace(command.substring(NET_CMD_RDPKG.length()));

			CavanAndroid.dLog("code = " + code);

			if (code.equals(NET_CMD_TEST)) {
				command = getResources().getString(R.string.network_test_success, type);
				postShowToast(command);
			} else if (CavanMessageActivity.isRedPacketCodeReceiveEnabled()) {
				CavanRedPacketAlipay packet = CavanRedPacketAlipay.get(code);
				if (packet == null || packet.isRecvEnabled()) {
					RedPacketListenerService listener = RedPacketListenerService.instance;
					if (listener != null) {
						listener.addRedPacketCode(code, type, true);
					}
				}
			}
		} else if (command.startsWith(NET_CMD_UPDATE)) {
			String text = command.substring(NET_CMD_UPDATE.length());
			String[] texts = text.split("\\s*\\|\\s*");

			if (texts.length == 3) {
				try {
					String code = texts[0].trim();
					long time = Long.parseLong(texts[1]);
					boolean ignore = Boolean.parseBoolean(texts[2]);

					CavanRedPacketAlipay packet = CavanRedPacketAlipay.update(code, time, ignore);
					if (packet != null) {
						mHandler.obtainMessage(MSG_RED_PACKET_UPDATED, packet).sendToTarget();
					}
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		} else if (command.startsWith(NET_CMD_TM_CODE)) {
			String code = CavanString.deleteSpace(command.substring(NET_CMD_TM_CODE.length()));
			mHandler.obtainMessage(MSG_CLIPBOARD_RECEIVED, code).sendToTarget();
		} else if (command.startsWith(NET_CMD_CLIPBOARD)) {
			String code = command.substring(NET_CMD_CLIPBOARD.length());
			mHandler.obtainMessage(MSG_CLIPBOARD_RECEIVED, code).sendToTarget();
		} else if (command.startsWith(NET_CMD_NOTIFY) && CavanMessageActivity.isThanksShareEnabled(this)) {
			postShowNotify(command.substring(NET_CMD_NOTIFY.length()));
		}
	}

	public void updateNetworkConnState() {
		mNetworkConnected = CavanAndroid.isNetworkAvailable(this);
		mHandler.sendEmptyMessage(MSG_TCP_SERVICE_UPDATED);
		mHandler.sendEmptyMessage(MSG_TCP_BRIDGE_UPDATED);
	}

	public void showCursorView(int type) {
		if (mCursorView == null) {
			mCursorView = new CavanCursorView(this, createRootViewLayoutParams());
			addRootView(mCursorView);
		}

		if (mTextViewTime != null) {
			mTextViewTime.setVisibility(View.GONE);
		}

		mCursorView.enable(type);
	}

	public void hiddenCursorView() {
		if (mCursorView != null) {
			mCursorView.disable();
		}

		if (CavanMessageActivity.isFloatTimerEnabled(this) && mTextViewTime != null) {
			mTextViewTime.setVisibility(View.VISIBLE);
		}
	}

	public boolean setCursorPosition(int x, int y) {
		if (mCursorView != null) {
			return mCursorView.setPosition(x, y);
		}

		return false;
	}

	public boolean addCursorPosition(int x, int y) {
		if (mCursorView != null) {
			return mCursorView.addPosition(x, y);
		}

		return false;
	}

	public boolean saveCursorPosition() {
		if (mCursorView != null) {
			return mCursorView.save();
		}

		return false;
	}

	public boolean removeCursorPosition() {
		if (mCursorView != null) {
			return mCursorView.remove();
		}

		return false;
	}

	public boolean tapCursorPosition() {
		if (mCursorView != null) {
			return mCursorView.tap();
		}

		return false;
	}

	@Override
	public synchronized void removeTextAll() {
		CavanMainAccessibilityService service = CavanMainAccessibilityService.instance;
		if (service != null) {
			service.getPackets().clear();
		}

		super.removeTextAll();
		mMessageCodeMap.clear();
		CavanRedPacketAlipay.getRecentPackets().clear();
		CavanAndroid.showToast(FloatMessageService.this, R.string.already_clear);
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
		super.onCreate();

		IntentFilter filter = new IntentFilter();
		filter.addAction(Intent.ACTION_SCREEN_OFF);
		filter.addAction(Intent.ACTION_SCREEN_ON);
		filter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);
		filter.addAction(CavanMessageActivity.ACTION_SERVICE_EXIT);
		registerReceiver(mReceiver, filter );

		// mFileObserverQQ.startWatching();

		mUdpDaemon = new UdpDaemonThread();
		mUdpDaemon.start();

		updateNetworkConnState();
		setSuspendDisable(CavanMessageActivity.isDisableSuspendEnabled(this));

		if (CavanMessageActivity.isOnTimeNotifyEnabled(this)) {
			CavanBroadcastReceiver.setOnTimeNotifyAlarm(this);
		}

		if (CavanMessageActivity.isOnTimeMuteEnabled(this)) {
			CavanBroadcastReceiver.setOnTimeMuteAlarm(this, true);
		}

		mHandler.sendEmptyMessageDelayed(MSG_CHECK_SERVICE_STATE, 10000);

		instance = this;
	}

	@Override
	public void onDestroy() {
		instance = null;

		mTcpClient.disconnect();

		if (mUdpDaemon != null) {
			mUdpDaemon.setActive(false);
		}

		if (mNetworkSendHandler != null) {
			mNetworkSendHandler.quit();
		}

		// mFileObserverQQ.stopWatching();
		unregisterReceiver(mReceiver);
		setTimerEnable(false);

		super.onDestroy();
	}

	public IFloatMessageService getBind() {
		return mBinder;
	}

	@Override
	public IBinder onBind(Intent intent) {
		return mBinder;
	}

	@Override
	protected View createView(CharSequence text) {
		TextView view = new TextView(getApplicationContext());

		initTextView(view, TEXT_SIZE_MESSAGE, TEXT_COLOR_MESSAGE, text);
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
		mTextViewToast = (TextView) findViewById(R.id.textViewToast);
		mTextViewNotify = (TextView) findViewById(R.id.textViewNotify);

		initTextView(mTextViewTime, TEXT_SIZE_TIME, TEXT_COLOR_TIME, getTimeText());
		initTextView(mTextViewToast, TEXT_SIZE_TIME, TEXT_COLOR_MESSAGE, null);
		initTextView(mTextViewNotify, TEXT_SIZE_TIME, TEXT_COLOR_MESSAGE, null);

		mTextViewToast.setVisibility(View.GONE);
		mTextViewNotify.setVisibility(View.GONE);

		if (CavanMessageActivity.isFloatTimerEnabled(this)) {
			setTimerEnable(true);
		} else {
			setTimerEnable(false);
		}

		return super.doInitialize();
	}

	public class NetworkSendHandler extends CavanThreadedHandler {

		private MulticastSocket mUdpSocket;

		public NetworkSendHandler() {
			super(NetworkSendHandler.class);
		}

		public void sendCommand(int what, String command, long delay, int retry) {
			Message message = obtainMessage(what, retry, 0, command);

			if (delay > 0) {
				sendMessageDelayed(message, delay);
			} else {
				sendMessage(message);
			}
		}

		public boolean sendTcpCommand(String command, long delay) {
			if (CavanMessageActivity.isWanShareEnabled(getApplicationContext())) {
				sendCommand(MSG_SEND_TCP_COMMAND, command, delay, 0);
				return true;
			}

			return false;
		}

		public boolean sendUdpCommand(String command, long delay, int retry) {
			if (CavanMessageActivity.isLanShareEnabled(getApplicationContext())) {
				sendCommand(MSG_SEND_UDP_COMMAND, command, delay, retry);
				return true;
			}

			return false;
		}

		public boolean sendCode(String code, long delay) {
			String command = NET_CMD_RDPKG + code;
			return sendTcpCommand(command, delay) || sendUdpCommand(command, delay, 5);
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
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case MSG_SEND_TCP_COMMAND:
				mTcpClient.send((String) msg.obj);
				break;

			case MSG_SEND_UDP_COMMAND:
				try {
					if (mUdpSocket == null) {
						mUdpSocket = new MulticastSocket();
					}

					String command = (String) msg.obj;
					byte[] bytes = command.getBytes();
					DatagramPacket pack = new DatagramPacket(bytes, bytes.length, InetAddress.getByName(UDP_ADDR), UDP_PORT);

					mUdpSocket.send(pack);
					CavanAndroid.dLog("udp success send: " + command);
				} catch (Exception e) {
					e.printStackTrace();

					if (mUdpSocket != null) {
						mUdpSocket.close();
						mUdpSocket = null;
					}
				}

				if (msg.arg1 > 0) {
					Message message = obtainMessage(msg.what, msg.arg1 - 1, 0, msg.obj);
					sendMessageDelayed(message, 1000);
				}
				break;
			}
		}
	}

	public class UdpDaemonThread extends Thread {

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
			CavanAndroid.setMulticastEnabled(getApplicationContext(), true);

			while (mActive) {
				if (mSocket != null) {
					mSocket.close();
				}

				try {
					mSocket = new MulticastSocket(UDP_PORT);
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

				try {
					mSocket.joinGroup(InetAddress.getByName(UDP_ADDR));
				} catch (Exception e) {
					e.printStackTrace();
				}

				CavanAndroid.dLog("UdpServiceThread running");

				byte[] bytes = new byte[128];
				DatagramPacket pack = new DatagramPacket(bytes, bytes.length);

				while (mActive) {
					try {
						mSocket.receive(pack);

						if (CavanMessageActivity.isLanShareEnabled(FloatMessageService.this)) {
							String text = new String(pack.getData(), 0, pack.getLength());
							onNetworkCommandReceived("内网分享", text);
						}
					} catch (IOException e) {
						e.printStackTrace();
						break;
					}
				}

				CavanAndroid.dLog("UdpServiceThread stopping");
			}

			mUdpDaemon = null;

			if (mSocket != null) {
				mSocket.close();
				mSocket = null;
			}

			CavanAndroid.setMulticastEnabled(getApplicationContext(), false);
			mActive = false;
		}
	}

	public class TcpSharedClient extends CavanTcpClient {

		@Override
		protected int onGetConnOvertime() {
			return 5000;
		}

		@Override
		protected int onGetKeepAliveDelay() {
			return 20000;
		}

		@Override
		public synchronized boolean send(byte[] bytes, int offset, int length) {
			byte[] newLine = "\n".getBytes();
			return super.send(bytes, offset, length) && super.send(newLine, 0, newLine.length);
		}

		@Override
		protected int doTcpKeepAlive(int times) {
			if (send(NET_CMD_KEEP_ALIVE)) {
				return times;
			}

			return -1;
		}

		@Override
		public synchronized boolean isConnEnabled() {
			return super.isConnEnabled() && CavanMessageActivity.isWanShareEnabled(getApplicationContext());
		}

		@Override
		public synchronized List<InetSocketAddress> getAddresses() {
			ArrayList<String> lines = CavanMessageActivity.getWanShareServer(getApplicationContext());
			if (lines == null || lines.isEmpty()) {
				return null;
			}

			ArrayList<InetSocketAddress> addresses = new ArrayList<>();

			for (String line : lines) {
				addresses.add(newSocketAddressByUrl(line, 8864));
			}

			return addresses;
		}

		@Override
		protected void runRecvThread(InputStream stream) {
			BufferedReader reader = new BufferedReader(new InputStreamReader(stream));

			while (true) {
				String line;

				try {
					line = reader.readLine();
					if (line == null) {
						break;
					}

					touchKeepAlive();

					if (NET_CMD_KEEP_ALIVE.equals(line)) {
						CavanAndroid.dLog("Received: " + line);
					} else if (CavanMessageActivity.isWanReceiveEnabled(getApplicationContext())) {
						onNetworkCommandReceived("外网分享", line);
					}
				} catch (IOException e) {
					e.printStackTrace();
					break;
				}
			}

			try {
				reader.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}

		@Override
		protected void onTcpConnecting(InetSocketAddress address) {
			mWanState = R.string.wan_connecting;
			mHandler.sendEmptyMessage(MSG_TCP_SERVICE_STATE_CHANGED);
		}

		@Override
		protected boolean onTcpConnected(Socket socket) {
			mWanState = R.string.wan_connected;
			mHandler.sendEmptyMessage(MSG_TCP_SERVICE_STATE_CHANGED);
			return true;
		}

		@Override
		protected void onTcpDisconnected() {
			mWanState = R.string.wan_disconnected;
			mHandler.sendEmptyMessage(MSG_TCP_SERVICE_STATE_CHANGED);
		}

		@Override
		protected long onTcpConnFailed(int times) {
			return 0;
		}
	}

	public class TcpBridgeThread extends Thread {

		private boolean mActive;
		private int mPid;

		public void killCommand() {
			if (mPid != 0) {
				CavanJni.kill(mPid);
			}
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

			while (mActive && CavanMessageActivity.isTcpBridgeEnabled(getApplicationContext())) {
				String setting = CavanMessageActivity.getTcpBridgeSetting(getApplicationContext());
				if (setting == null) {
					break;
				}

				String[] settings = setting.split("\\n");
				if (settings.length != 2) {
					break;
				}

				String url1 = "tcp://" + CavanString.deleteSpace(settings[0]);
				String url2 = "tcp://" + CavanString.deleteSpace(settings[1]);

				mBridgeState = R.string.tcp_bridge_running;
				mHandler.sendEmptyMessage(MSG_TCP_BRIDGE_STATE_CHANGED);

				int pid = CavanJni.doTcpBridge(true, url1, url2);
				if (pid >= 0) {
					mPid = pid;
					CavanJni.waitpid(pid);
					mPid = 0;
				}

				mBridgeState = R.string.tcp_bridge_exit;
				mHandler.sendEmptyMessage(MSG_TCP_BRIDGE_STATE_CHANGED);

				if (mActive) {
					synchronized (this) {
						try {
							wait(5000);
						} catch (InterruptedException e) {
							e.printStackTrace();
						}
					}
				}
			}

			mTcpBridge = null;
			mHandler.obtainMessage(MSG_TCP_BRIDGE_STATE_CHANGED, R.string.tcp_bridge_stopped, 0).sendToTarget();
		}
	}

	public int getWanState() {
		return mWanState;
	}

	public CharSequence getWanSummary() {
		return mTcpClient.getCurrentAddressString();
	}

	public int getBridgeState() {
		return mBridgeState;
	}

	public void sendWanCommand(String command) {
		mNetworkSendHandler.sendTcpCommand(command, 0);
	}
}
