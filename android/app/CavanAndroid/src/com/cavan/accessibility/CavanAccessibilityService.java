package com.cavan.accessibility;

import java.util.HashMap;
import java.util.Iterator;

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

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanKeyguardLock;
import com.cavan.android.CavanWakeLock;
import com.cavan.android.SystemProperties;

public class CavanAccessibilityService extends AccessibilityService {

	public static final int POLL_DELAY = 500;

	private static final int MSG_SCREEN_ON = 1;

	private HashMap<String, CavanAccessibilityPackage<?>> mPackages = new HashMap<String, CavanAccessibilityPackage<?>>();
	private CavanAccessibilityPackage<?> mPackage;
	private boolean mUserPresent = true;
	private boolean mScreenOn = true;
	private CavanKeyguardLock mKeyguardLock = new CavanKeyguardLock();
	private CavanWakeLock mWakeLock = new CavanWakeLock(true);

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

		public int poll(CavanAccessibilityPackage<?> pkg) {
			int retry = 0;

			CavanAndroid.dLog("PollThread polling: " + pkg.getPackageName());

			try {
				pkg.launch();
			} catch (Exception e) {
				e.printStackTrace();
			}

			while (mScreenOn) {
				long delay;

				CavanAndroid.dLog("PollThread running");

				AccessibilityNodeInfo root = getRootInActiveWindow();
				if (root == null) {
					CavanAndroid.dLog("Failed to getRootInActiveWindow: " + retry);

					if (++retry > 5) {
						return -1;
					}

					delay = POLL_DELAY;
				} else {
					try {
						if (pkg.getPackageName().equals(root.getPackageName())) {
							delay = pkg.poll(root);
							if (delay > 0) {
								retry = 0;
							} else if (pkg.isGotoIdleEnabled()) {
								return 1;
							} else {
								return 0;
							}
						} else {
							CavanAndroid.dLog("Package not mach: " + retry);

							if (++retry > 5) {
								return -1;
							}

							delay = POLL_DELAY;
						}
					} catch (Exception e) {
						e.printStackTrace();
						return -1;
					} finally {
						root.recycle();
					}
				}

				CavanAndroid.dLog("PollThread waitting " + delay);

				synchronized (this) {
					try {
						wait(delay);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
			}

			return 0;
		}

		@Override
		public void run() {
			while (true) {
				boolean idle = false;

				while (mScreenOn) {
					CavanAccessibilityPackage<?> pkg = getPendingPackage();
					if (pkg == null) {
						break;
					}

					int retry = 0;

					while (true) {
						int ret = poll(pkg);
						if (ret < 0) {
							CavanAndroid.dLog("Failed to poll: " + retry);

							if (++retry > 3) {
								pkg.clearPackets();
								break;
							}
						} else {
							idle = (ret > 0);
							break;
						}
					}
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

				CavanAccessibilityPackage<?> pkg = getPendingPackage();
				if (pkg != null) {
					acquireScreenLock();
					Message message = mHandler.obtainMessage(MSG_SCREEN_ON, pkg);
					mHandler.sendMessageDelayed(message, 1000);
				}
				break;

			case Intent.ACTION_SCREEN_OFF:
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

	public synchronized CavanAccessibilityPackage<?> getPackage() {
		return mPackage;
	}

	public synchronized boolean setPackage(CavanAccessibilityPackage<?> pkg) {
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

	public synchronized String getCurrntPacketName() {
		AccessibilityNodeInfo root = getRootInActiveWindow();
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

	public synchronized CavanAccessibilityPackage<?> getPendingPackage() {
		Iterator<CavanAccessibilityPackage<?>> iterator = mPackages.values().iterator();

		while (iterator.hasNext()) {
			CavanAccessibilityPackage<?> pkg = iterator.next();
			if (pkg.isPending()) {
				while (iterator.hasNext()) {
					CavanAccessibilityPackage<?> node = iterator.next();
					if (node.isPending() && node.getUnpackTime() < pkg.getUnpackTime()) {
						pkg = node;
					}
				}

				return pkg;
			}
		}

		return null;
	}

	public synchronized void addPackage(CavanAccessibilityPackage<?> pkg) {
		mPackages.put(pkg.getPackageName(), pkg);
	}

	public synchronized CavanAccessibilityPackage<?> getPackage(CharSequence name) {
		if (name == null) {
			return null;
		}

		return mPackages.get(name.toString());
	}

	public boolean performActionBack() {
		CavanAndroid.dLog("performActionBack");
		return performGlobalAction(AccessibilityService.GLOBAL_ACTION_BACK);
	}

	public boolean performActionHome() {
		CavanAndroid.dLog("performActionHome");
		return performGlobalAction(AccessibilityService.GLOBAL_ACTION_HOME);
	}

	public synchronized void post() {
		mPollThread.start();
	}

	public int getEventTypes() {
		return 0;
	}

	public void initServiceInfo(AccessibilityServiceInfo info) {
		String[] packages = new String[mPackages.size()];
		int i = 0;

		for (CavanAccessibilityPackage<?> pkg : mPackages.values()) {
			packages[i++] = pkg.getPackageName();
		}

		info.packageNames = packages;

		info.flags = AccessibilityServiceInfo.DEFAULT |
				AccessibilityServiceInfo.FLAG_REPORT_VIEW_IDS |
				AccessibilityServiceInfo.FLAG_REQUEST_FILTER_KEY_EVENTS |
				AccessibilityServiceInfo.FLAG_RETRIEVE_INTERACTIVE_WINDOWS;

		info.eventTypes = AccessibilityEvent.TYPE_WINDOW_STATE_CHANGED | getEventTypes();
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
			CavanAccessibilityPackage<?> pkg = getPackage(event.getPackageName());
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

		for (CavanAccessibilityPackage<?> pkg : mPackages.values()) {
			pkg.onCreate();
		}

		IntentFilter filter = new IntentFilter();
		filter.addAction(Intent.ACTION_SCREEN_OFF);
		filter.addAction(Intent.ACTION_SCREEN_ON);
		filter.addAction(Intent.ACTION_USER_PRESENT);
		filter.addAction(Intent.ACTION_CLOSE_SYSTEM_DIALOGS);
		registerReceiver(mBroadcastReceiver, filter);
	}

	@Override
	public void onDestroy() {
		unregisterReceiver(mBroadcastReceiver);

		for (CavanAccessibilityPackage<?> pkg : mPackages.values()) {
			pkg.onDestroy();
		}

		super.onDestroy();
	}

}
