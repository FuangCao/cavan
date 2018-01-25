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
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;

import com.cavan.android.CavanAndroid;

public class CavanAccessibilityService extends AccessibilityService {

	private HashMap<String, CavanAccessibilityPackage<?>> mPackages = new HashMap<String, CavanAccessibilityPackage<?>>();
	private boolean mUserPresent = true;
	private boolean mScreenOn = true;

	private Thread mPollThread = new Thread() {

		@Override
		public synchronized void start() {
			if (isAlive()) {
				notify();
			} else {
				super.start();
			}
		}

		@Override
		public void run() {
			while (true) {
				boolean gotoIdle = false;

				while (true) {
					CavanAndroid.dLog("PollThread running");

					AccessibilityNodeInfo root = getRootInActiveWindow();
					if (root == null) {
						break;
					}

					try {
						CavanAccessibilityPackage<?> pkg = getPackage(root.getPackageName());
						if (pkg == null) {
							break;
						}

						long delay = pkg.poll(root);
						if (delay > 0) {
							CavanAndroid.dLog("PollThread waitting " + delay);

							synchronized (this) {
								wait(delay);
							}
						} else {
							gotoIdle = pkg.isGotoIdleEnabled();
							break;
						}
					} catch (Exception e) {
						e.printStackTrace();
					} finally {
						root.recycle();
					}
				}

				CavanAccessibilityPackage<?> pkg = getPendingPackage();
				if (pkg != null) {
					pkg.launch();
				} else if (gotoIdle) {
					performActionHome();
				}

				CavanAndroid.dLog("PollThread suspend");

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
					CavanAndroid.setLockScreenEnable(CavanAccessibilityService.this, false);
					pkg.launch();
				}
				break;

			case Intent.ACTION_SCREEN_OFF:
				CavanAndroid.setLockScreenEnable(CavanAccessibilityService.this, true);
				CavanAndroid.releaseWakeLock();
				mUserPresent = false;
				mScreenOn = false;
				onUserOffline();
				onScreenOff();
				break;

			case Intent.ACTION_USER_PRESENT:
				mUserPresent = true;
				onUserOnline();
				break;
			}
		}
	};

	protected void onUserOnline() {}
	protected void onUserOffline() {}
	protected void onScreenOn() {}
	protected void onScreenOff() {}

	public synchronized boolean isUserPresent() {
		return mUserPresent;
	}

	public synchronized boolean isScreenOn() {
		return mScreenOn;
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

	@Override
	protected void onServiceConnected() {
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2) {
			String[] packages = new String[mPackages.size()];
			int types = 0;
			int i = 0;

			for (CavanAccessibilityPackage<?> pkg : mPackages.values()) {
				packages[i++] = pkg.getPackageName();
				types |= pkg.getEventTypes();
			}

			AccessibilityServiceInfo info = getServiceInfo();

			info.packageNames = packages;

			info.flags |= AccessibilityServiceInfo.DEFAULT |
					AccessibilityServiceInfo.FLAG_REPORT_VIEW_IDS |
					AccessibilityServiceInfo.FLAG_REQUEST_FILTER_KEY_EVENTS |
					AccessibilityServiceInfo.FLAG_RETRIEVE_INTERACTIVE_WINDOWS;

			info.eventTypes = AccessibilityEvent.TYPE_WINDOW_STATE_CHANGED | types;

			setServiceInfo(info);

			CavanAndroid.dLog("info = " + getServiceInfo());
		}

		super.onServiceConnected();
	}

	@Override
	public void onAccessibilityEvent(AccessibilityEvent event) {
		CavanAccessibilityPackage<?> pkg = getPackage(event.getPackageName());
		if (pkg != null) {
			pkg.onAccessibilityEvent(event);
		}
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
