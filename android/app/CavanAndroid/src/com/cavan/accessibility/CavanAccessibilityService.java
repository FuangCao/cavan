package com.cavan.accessibility;

import java.util.HashMap;

import android.accessibilityservice.AccessibilityService;
import android.accessibilityservice.AccessibilityServiceInfo;
import android.os.Build;
import android.os.Handler;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;

import com.cavan.android.CavanAndroid;
import com.cavan.java.CavanString;

public class CavanAccessibilityService extends AccessibilityService implements Runnable {

	private HashMap<String, CavanAccessibilityPackage<?>> mPackages = new HashMap<String, CavanAccessibilityPackage<?>>();
	private CavanAccessibilityPackage<?> mPackage;

	private Handler mHandler = new Handler();

	public void addPackage(CavanAccessibilityPackage<?> pkg) {
		mPackages.put(pkg.getPackageName(), pkg);
	}

	public boolean performActionBack() {
		return performGlobalAction(AccessibilityService.GLOBAL_ACTION_BACK);
	}

	public boolean performActionHome() {
		return performGlobalAction(AccessibilityService.GLOBAL_ACTION_HOME);
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
		String pkgName = CavanString.fromCharSequence(event.getPackageName(), null);
		if (pkgName != null) {
			mPackage = mPackages.get(pkgName);
			if (mPackage != null) {
				mPackage.onAccessibilityEvent(event);;
			}
		}
	}

	@Override
	public void onInterrupt() {
		CavanAndroid.pLog();
	}

	@Override
	public void run() {
		if (mPackage != null) {
			AccessibilityNodeInfo root = getRootInActiveWindow();
			if (root != null) {
				String pkgName = CavanString.fromCharSequence(root.getPackageName(), null);
				if (pkgName != null && mPackage.getPackageName().equals(pkgName)) {
					long delay = mPackage.run(root);
					if (delay > 0) {
						mHandler.postDelayed(this, delay);
					} else {
						for (CavanAccessibilityPackage<?> pkg : mPackages.values()) {
							if (pkg.isPending()) {
								pkg.launch();
								break;
							}
						}
					}
				} else {
					mPackage = null;
				}

				root.recycle();
			}
		}
	}

	@Override
	public void onCreate() {
		super.onCreate();

		for (CavanAccessibilityPackage<?> pkg : mPackages.values()) {
			pkg.onCreate();
		}
	}

	@Override
	public void onDestroy() {
		for (CavanAccessibilityPackage<?> pkg : mPackages.values()) {
			pkg.onDestroy();
		}

		super.onDestroy();
	}

}
