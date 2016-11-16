package com.cavan.cavanmain;

import android.os.Handler;
import android.view.KeyEvent;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;

public abstract class CavanAccessibilityBase extends Handler {

	protected String mClassName;
	private long mWindowStartTime;
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

	public void dispatchAccessibilityEvent(AccessibilityEvent event) {
		switch (event.getEventType()) {
		case AccessibilityEvent.TYPE_WINDOW_STATE_CHANGED:
			mWindowStartTime = System.currentTimeMillis();
			mClassName = event.getClassName().toString();
			onWindowStateChanged(event);
			break;

		case AccessibilityEvent.TYPE_WINDOW_CONTENT_CHANGED:
			onWindowContentChanged(event);
			break;

		case AccessibilityEvent.TYPE_VIEW_CLICKED:
			if (MainActivity.isListenClickEnabled(mService)) {
				onViewClicked(event);
			}
			break;

		case AccessibilityEvent.TYPE_VIEW_TEXT_CHANGED:
			onViewTextChanged(event);
			break;
		}
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