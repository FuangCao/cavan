package com.cavan.cavanmain;

import android.os.Handler;
import android.view.KeyEvent;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;

import com.cavan.android.CavanAndroid;
import com.cavan.java.CavanString;

public abstract class CavanAccessibilityBase extends Handler {

	protected String mClassName = CavanString.EMPTY_STRING;
	protected String mPackageName = CavanString.EMPTY_STRING;

	protected long mWindowStartTime;
	protected CavanAccessibilityService mService;

	public CavanAccessibilityBase(CavanAccessibilityService service) {
		mService = service;
	}

	public abstract String getPackageName();

	protected void onWindowStateChanged(AccessibilityEvent event) {}
	protected void onWindowContentChanged(AccessibilityEvent event) {}
	protected void onViewClicked(AccessibilityEvent event) {}
	protected void onViewTextChanged(AccessibilityEvent event) {}

	protected boolean onKeyEvent(KeyEvent event) {
		return false;
	}

	public void onWindowStateChanged(AccessibilityEvent event, String packageName, String className, long startTime) {
		mWindowStartTime = startTime;
		mPackageName = packageName;
		mClassName = className;

		CavanAndroid.dLog("package = " + mPackageName);
		CavanAndroid.dLog("class = " + mClassName);

		onWindowStateChanged(event);
	}

	public AccessibilityNodeInfo getRootInActiveWindow() {
		return mService.getRootInActiveWindow();
	}

	public void performGlobalAction(int action) {
		mService.performGlobalAction(action);
	}

	public long getWindowTimeConsume() {
		return System.currentTimeMillis() - mWindowStartTime;
	}

	public boolean isRootActivity(AccessibilityNodeInfo root) {
		if (root == null) {
			return false;
		}

		return getPackageName().equals(root.getPackageName());
	}

	public boolean isRootActivity() {
		return isRootActivity(getRootInActiveWindow());
	}
}