package com.cavan.accessibility;

import java.util.HashMap;
import java.util.HashSet;
import java.util.List;

import android.accessibilityservice.AccessibilityService;
import android.accessibilityservice.AccessibilityServiceInfo;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Build;
import android.os.Handler;
import android.os.Message;
import android.view.KeyEvent;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;
import android.view.inputmethod.InputMethodManager;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanKeyguardLock;
import com.cavan.android.CavanWakeLock;
import com.cavan.android.SystemProperties;
import com.cavan.java.CavanJava;

public class CavanAccessibilityService extends AccessibilityService {

	public static final int POLL_DELAY = 500;
	public static final int LAUNCH_DELAY = 2000;

	private static final int MSG_SCREEN_ON = 1;
	private static final int MSG_SHOW_COUNT_DOWN = 2;

	public static CavanAccessibilityService instance;

	protected HashMap<String, CavanAccessibilityPackage> mPackages = new HashMap<String, CavanAccessibilityPackage>();
	protected HashSet<AccessibilityNodeInfo> mRecycleNodes = new HashSet<AccessibilityNodeInfo>();
	protected CavanRedPacketList mPackets = new CavanRedPacketList();
	protected CavanAccessibilityPackage mPackage;
	protected boolean mUserPresent = true;
	protected boolean mScreenOn = true;

	private CavanKeyguardLock mKeyguardLock = new CavanKeyguardLock();
	private CavanWakeLock mWakeLock = new CavanWakeLock(true);
	private CavanRedPacket mPacketDummy = new CavanRedPacket();

	private Thread mPollThread = new Thread() {

		@Override
		public synchronized void start() {
			if (mScreenOn) {
				if (isAlive()) {
					notify();
				} else {
					super.start();
				}
			}
		}

		public boolean poll(CavanAccessibilityPackage pkg, CavanRedPacket packet) {
			int retry = 0;

			CavanAndroid.dLog("PollThread polling: " + pkg.getName());

			while (mScreenOn && getPendingPacket() == packet) {
				long delay;

				CavanAndroid.dLog("PollThread running");

				AccessibilityNodeInfo root = getRootInActiveWindow();
				if (root == null) {
					CavanAndroid.dLog("Failed to getRootInActiveWindow: " + retry);

					if (++retry > 5) {
						return false;
					}

					delay = POLL_DELAY;
				} else {
					try {
						if (pkg.isCurrentPackage(root)) {
							delay = pkg.poll(packet, root);
							if (delay < 0) {
								pkg.setPending(false);
								return true;
							}

							retry = 0;
						} else {
							CavanAndroid.dLog("Invalid package(" + retry + "): " + root.getPackageName());

							if (++retry > 5) {
								return false;
							}

							packet.launch();
							delay = LAUNCH_DELAY;
						}
					} catch (Exception e) {
						e.printStackTrace();
						return false;
					} finally {
						removeRecycleNodes();
						root.recycle();
					}
				}

				if (getPendingPacket() != packet) {
					break;
				}

				CavanAndroid.dLog("PollThread waitting " + delay);

				if (delay > 0) {
					synchronized (this) {
						try {
							wait(delay);
						} catch (InterruptedException e) {
							e.printStackTrace();
						}
					}
				}
			}

			return true;
		}

		@Override
		public void run() {
			while (true) {
				boolean idle = false;

				while (mScreenOn) {
					CavanAccessibilityPackage pkg;
					CavanRedPacket packet;

					packet = getPendingPacket();
					if (packet == mPacketDummy) {
						pkg = getPendingPackage();
						if (pkg == null) {
							break;
						}

						packet.setPackage(pkg);
						packet.setGotoIdle(false);
						pkg.setCurrentPacket(null);
					} else {
						packet.setGotoIdle(true);
						pkg = packet.getPackage();
						pkg.setPending(true);
						pkg.setCurrentPacket(packet);
					}

					packet.setPending();
					pkg.onPollStarted();

					int retry = 0;

					while (true) {
						if (poll(pkg, packet)) {
							break;
						}

						CavanAndroid.dLog("Failed to poll: " + retry);

						if (++retry > 3) {
							mPackets.remove(packet);
							break;
						}
					}

					pkg.setCurrentPacket(null);
					pkg.setPending(false);
					pkg.onPollStopped();
					idle = packet.needGotoIdle();
				}

				CavanAndroid.dLog("PollThread suspend: idle = " + idle);
				onPollSuspend(idle);

				synchronized (this) {
					try {
						wait();
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
			}
		}
	};

	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case MSG_SCREEN_ON:
				mPollThread.start();
				break;

			case MSG_SHOW_COUNT_DOWN:
				CavanRedPacket packet = (CavanRedPacket) msg.obj;
				if (packet != null && packet == mPackets.get()) {
					long remain = packet.getUnpackDelay(0);
					if (remain > 0) {
						onCountDownUpdated(packet, remain);

						if (remain > 1000) {
							remain = 1000;
						}

						Message message = obtainMessage(MSG_SHOW_COUNT_DOWN, packet);
						sendMessageDelayed(message, remain);
						break;
					}
				}

				onCountDownCompleted();
				break;
			}
		}
	};

	private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {
			String action = intent.getAction();
			CavanAndroid.dLog("action = " + action);

			switch (action) {
			case Intent.ACTION_SCREEN_ON:
				mScreenOn = true;
				onScreenOn();

				if (mPackets.isEmpty()) {
					break;
				}

				mHandler.sendEmptyMessageDelayed(MSG_SCREEN_ON, 1000);
				acquireScreenLock();
				break;

			case Intent.ACTION_SCREEN_OFF:
				mPackets.clear();
				setPackage(null);
				mUserPresent = false;
				onUserOffline();
				mScreenOn = false;
				onScreenOff();
				releaseScreenLock();
				releaseWakeLock();
				break;

			case Intent.ACTION_USER_PRESENT:
				mUserPresent = true;
				onUserOnline();
				break;

			case Intent.ACTION_CLOSE_SYSTEM_DIALOGS:
				setPackage(null);
				break;
			}
		}
	};

	protected void onPollSuspend(boolean idle) {
		if (mScreenOn && idle) {
			CavanAndroid.startLauncher(this);
		}
	}

	public CavanAccessibilityPackage getPendingPackage() {
		synchronized (mPackages) {
			for (CavanAccessibilityPackage pkg : mPackages.values()) {
				if (pkg.isPending()) {
					return pkg;
				}
			}
		}

		return null;
	}

	public CavanRedPacket getPendingPacket() {
		CavanRedPacket packet = mPackets.get();
		if (packet != null) {
			return packet;
		}

		return mPacketDummy;
	}

	protected void onCountDownCompleted() {
		CavanAndroid.dLog("onCountDownCompleted");
	}

	protected void onCountDownUpdated(CavanRedPacket packet, long remain) {
		CavanAndroid.dLog("onCountDownUpdated: " + remain);
	}

	protected void onUserOnline() {}
	protected void onUserOffline() {}
	protected void onScreenOn() {}
	protected void onScreenOff() {}

	public Handler getHandler() {
		return mHandler;
	}

	public synchronized boolean isUserPresent() {
		return mUserPresent;
	}

	public synchronized boolean isScreenOn() {
		return mScreenOn;
	}

	public boolean addRecycleNode(AccessibilityNodeInfo node) {
		synchronized (mRecycleNodes) {
			return mRecycleNodes.add(node);
		}
	}

	public int addRecycleNodes(AccessibilityNodeInfo... nodes) {
		int count = 0;

		synchronized (mRecycleNodes) {
			for (AccessibilityNodeInfo node : nodes) {
				if (mRecycleNodes.add(node)) {
					count++;
				}
			}
		}

		return count;
	}

	public int addRecycleNodes(List<AccessibilityNodeInfo> nodes) {
		int count = 0;

		synchronized (mRecycleNodes) {
			for (AccessibilityNodeInfo node : nodes) {
				if (mRecycleNodes.add(node)) {
					count++;
				}
			}
		}

		return count;
	}

	public void removeRecycleNodes() {
		synchronized (mRecycleNodes) {
			try {
				for (AccessibilityNodeInfo node : mRecycleNodes) {
					node.recycle();
				}
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				mRecycleNodes.clear();
			}
		}
	}

	public synchronized CavanAccessibilityPackage getPackage() {
		return mPackage;
	}

	public synchronized boolean setPackage(CavanAccessibilityPackage pkg) {
		if (mPackage == pkg) {
			return false;
		}

		if (mPackage != null) {
			mPackage.onLeave();
		}

		mPackage = pkg;

		if (pkg != null) {
			pkg.onEnter();
		}

		return true;
	}

	public synchronized boolean setPackageByName(CharSequence name) {
		return setPackage(getPackage(name));
	}

	public String getCurrntPacketName() {
		AccessibilityNodeInfo root = getRootInActiveWindow(10);
		if (root == null) {
			return null;
		}

		try {
			CharSequence name = root.getPackageName();
			if (name != null) {
				return name.toString();
			}
		} catch (Exception e) {
			e.printStackTrace();
		} finally {
			root.recycle();
		}

		return null;
	}

	public CavanWakeLock getWakeLock() {
		return mWakeLock;
	}

	public CavanKeyguardLock getKeyguardLock() {
		return mKeyguardLock;
	}

	public void acquireWakeLock() {
		mWakeLock.acquire(this);
	}

	public void acquireWakeLock(long overtime) {
		mWakeLock.acquire(this, overtime);
	}

	public void releaseWakeLock() {
		mWakeLock.release();
	}

	public void acquireScreenLock() {
		mKeyguardLock.acquire(this);
	}

	public void releaseScreenLock() {
		mKeyguardLock.release();
	}

	public void addPackage(CavanAccessibilityPackage pkg) {
		synchronized (mPackages) {
			for (String name : pkg.getNames()) {
				CavanAndroid.dLog("addPackage: " + name);
				mPackages.put(name, pkg);
			}
		}
	}

	public CavanAccessibilityPackage getPackage(CharSequence name) {
		if (name == null) {
			return null;
		}

		synchronized (mPackages) {
			return mPackages.get(name.toString());
		}
	}

	public boolean performActionBack() {
		CavanAndroid.dLog("performActionBack");
		return performGlobalAction(AccessibilityService.GLOBAL_ACTION_BACK);
	}

	public boolean performActionHome() {
		CavanAndroid.dLog("performActionHome");
		return performGlobalAction(AccessibilityService.GLOBAL_ACTION_HOME);
	}

	public void startPollThread() {
		mPollThread.start();
	}

	public int getEventTypes() {
		return 0;
	}

	public AccessibilityNodeInfo getRootInActiveWindow(int retry) {
		while (true) {
			AccessibilityNodeInfo root = getRootInActiveWindow();
			if (root != null) {
				return root;
			}

			CavanAndroid.eLog("Failed to getRootInActiveWindow: " + retry);

			if (retry > 0) {
				synchronized (mPollThread) {
					try {
						mPollThread.wait(100);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}

				retry--;
			} else {
				break;
			}
		}

		return null;
	}

	public void showCountDownView(CavanRedPacket packet) {
		mHandler.obtainMessage(MSG_SHOW_COUNT_DOWN, packet).sendToTarget();
	}

	public boolean addPacket(CavanAccessibilityPackage pkg, CavanRedPacket packet) {
		acquireWakeLock(2000);

		if (mPackets.add(pkg, packet)) {
			startPollThread();
			return true;
		}

		return false;
	}

	public CavanRedPacketList getPackets() {
		return mPackets;
	}

	public void removePacket(CavanRedPacket packet) {
		mPackets.remove(packet);
	}

	public int removePackets(CavanAccessibilityPackage pkg) {
		return mPackets.remove(pkg);
	}

	public void initServiceInfo(AccessibilityServiceInfo info) {
		String[] packages = new String[mPackages.size()];
		mPackages.keySet().toArray(packages);

		info.packageNames = packages;

		info.flags = AccessibilityServiceInfo.DEFAULT |
				AccessibilityServiceInfo.FLAG_REPORT_VIEW_IDS |
				AccessibilityServiceInfo.FLAG_REQUEST_FILTER_KEY_EVENTS |
				AccessibilityServiceInfo.FLAG_RETRIEVE_INTERACTIVE_WINDOWS;

		info.eventTypes = AccessibilityEvent.TYPE_WINDOW_STATE_CHANGED | getEventTypes();
	}

	public boolean setInputMethod(String name) {
		InputMethodManager manager = (InputMethodManager) CavanAndroid.getSystemServiceCached(this, INPUT_METHOD_SERVICE);
		if (manager == null) {
			return false;
		}

		for (int i = 0; i < 10; i++) {
			AccessibilityNodeInfo root = getRootInActiveWindow();

			if (root != null) {
				try {
					if (CavanAccessibilityHelper.isNodePackgeEquals(root, "android") && "选择输入法".equals(CavanAccessibilityHelper.getChildText(root, 0))) {
						AccessibilityNodeInfo node = CavanAccessibilityHelper.findNodeByText(root, name);
						if (node != null) {
							AccessibilityNodeInfo parent = node.getParent();
							node.recycle();

							if (parent != null) {
								return CavanAccessibilityHelper.performClickAndRecycle(parent);
							}
						}
					} else {
						manager.showInputMethodPicker();
					}
				} catch (Exception e) {
					e.printStackTrace();
				} finally {
					root.recycle();
				}
			}

			CavanJava.msleep(500);
		}

		performActionBack();

		return false;
	}

	public CavanInputMethodService getInputMethodService() {
		CavanInputMethodService ime = CavanInputMethodService.instance;
		if (ime != null) {
			return ime;
		}

		String name = getInputMethodName();
		if (name == null) {
			return null;
		}

		CavanAndroid.dLog("ime = " + name);

		if (setInputMethod(name)) {
			for (int i = 0; i < 10; i++) {
				ime = CavanInputMethodService.instance;
				if (ime != null) {
					return ime;
				}

				CavanJava.msleep(200);
			}
		}

		return null;
	}

	protected String getInputMethodName() {
		return null;
	}

	@Override
	protected void onServiceConnected() {
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2) {
			AccessibilityServiceInfo info = getServiceInfo();
			initServiceInfo(info);
			setServiceInfo(info);
			CavanAndroid.dLog("info = " + getServiceInfo());
		}

		super.onServiceConnected();
	}

	@Override
	public void onAccessibilityEvent(AccessibilityEvent event) {
		try {
			CavanAccessibilityPackage pkg = getPackage(event.getPackageName());
			if (pkg != null) {
				pkg.onAccessibilityEvent(event);
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	@Override
	protected boolean onKeyEvent(KeyEvent event) {
		if (event.getKeyCode() == KeyEvent.KEYCODE_VOLUME_UP && event.getAction() == KeyEvent.ACTION_UP) {
			int dump = SystemProperties.getInt("debug.cavan.dump.node", 0);
			if (dump > 0) {
				AccessibilityNodeInfo root = getRootInActiveWindow();
				if (root != null) {
					if (dump > 1) {
						CavanAccessibilityHelper.dumpNode(root);
					} else {
						CavanAccessibilityHelper.dumpNodeSimple(root);
					}

					root.recycle();
				}
			}
		}

		return super.onKeyEvent(event);
	}

	@Override
	public void onInterrupt() {
		CavanAndroid.pLog();
	}

	@Override
	public void onCreate() {
		super.onCreate();

		for (CavanAccessibilityPackage pkg : mPackages.values()) {
			pkg.onCreate();
		}

		IntentFilter filter = new IntentFilter();
		filter.addAction(Intent.ACTION_SCREEN_OFF);
		filter.addAction(Intent.ACTION_SCREEN_ON);
		filter.addAction(Intent.ACTION_USER_PRESENT);
		filter.addAction(Intent.ACTION_CLOSE_SYSTEM_DIALOGS);
		registerReceiver(mBroadcastReceiver, filter);

		instance = this;
	}

	@Override
	public void onDestroy() {
		instance = null;

		unregisterReceiver(mBroadcastReceiver);

		for (CavanAccessibilityPackage pkg : mPackages.values()) {
			pkg.onDestroy();
		}

		super.onDestroy();
	}
}
