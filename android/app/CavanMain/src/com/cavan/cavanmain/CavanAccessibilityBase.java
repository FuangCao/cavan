package com.cavan.cavanmain;

import android.content.Intent;
import android.os.Handler;
import android.view.KeyEvent;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;

import com.cavan.android.CavanAccessibility;
import com.cavan.android.CavanAndroid;
import com.cavan.android.DelayedSwitch;
import com.cavan.java.CavanString;
import com.cavan.java.RedPacketFinder;

public abstract class CavanAccessibilityBase extends Handler {

	protected CavanAccessibilityService mService;
	protected String mClassName = CavanString.EMPTY_STRING;
	protected String mPackageName = CavanString.EMPTY_STRING;

	private DelayedSwitch mSwitchUnlock = new DelayedSwitch(this) {

		@Override
		protected void onSwitchStateChanged(boolean enabled) {
			onLockStateChanged(enabled);
		}

		@Override
		protected void handleMessage() {
			cancel();
		}
	};

	private DelayedSwitch mSwitchBack = new DelayedSwitch(this) {

		@Override
		protected void handleMessage() {
			if (!isLocked()) {
				performGlobalBack();
			}
		}
	};

	public CavanAccessibilityBase(CavanAccessibilityService service) {
		mService = service;
	}

	protected void onLockStateChanged(boolean locked) {
		CavanAndroid.dLog("onLockStateChanged: locked = " + locked);
	}

	public abstract String getPackageName();
	public abstract int getRedPacketCount();

	protected void onWindowStateChanged(AccessibilityEvent event) {}
	protected void onWindowContentChanged(AccessibilityEvent event) {}
	protected void onViewClicked(AccessibilityEvent event) {}
	protected void onViewTextChanged(AccessibilityEvent event) {}

	protected boolean onKeyEvent(KeyEvent event) {
		return false;
	}

	public void onWindowStateChanged(AccessibilityEvent event, String packageName, String className) {
		mPackageName = packageName;
		mClassName = className;
		onWindowStateChanged(event);
	}

	public AccessibilityNodeInfo getRootInActiveWindow() {
		return mService.getRootInActiveWindow();
	}

	public void performGlobalAction(int action) {
		mService.performGlobalAction(action);
	}

	public long getWindowTimeConsume() {
		return mService.getWindowTimeConsume();
	}

	public boolean isLocked() {
		return mSwitchUnlock.isEnabled();
	}

	public void setLockEnable(long delay, boolean force) {
		mSwitchBack.cancel();
		mSwitchUnlock.post(delay, force);
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

	public void postMessageNode(AccessibilityNodeInfo node) {
		String text = CavanString.fromCharSequence(node.getText());

		if (text.length() > 0 && RedPacketFinder.containsUrl(text) == false) {
			FloatEditorDialog dialog = FloatEditorDialog.getInstance(mService, text, true);
			dialog.show(6000);
		}

		Intent intent = new Intent(MainActivity.ACTION_CONTENT_RECEIVED);
		intent.putExtra("package", node.getPackageName());
		intent.putExtra("desc", "用户点击");
		intent.putExtra("content", text);
		mService.sendBroadcast(intent);
	}

	public void performGlobalBack() {
		CavanAccessibility.performGlobalBack(mService);
	}

	public void performGlobalBack(long delay) {
		setLockEnable(delay, false);
		CavanAccessibility.performGlobalBack(mService);
	}

	public void performGlobalBackDelayed(long delay) {
		mSwitchBack.post(delay, false);
	}

	public void cancelGlobalBack() {
		mSwitchBack.cancel();
	}
}