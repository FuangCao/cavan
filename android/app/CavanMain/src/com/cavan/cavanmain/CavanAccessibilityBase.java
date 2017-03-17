package com.cavan.cavanmain;

import java.util.LinkedList;

import android.content.Intent;
import android.os.Handler;
import android.view.KeyEvent;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;

import com.cavan.activity.MainActivity;
import com.cavan.android.CavanAccessibility;
import com.cavan.android.CavanAndroid;
import com.cavan.android.DelayedRunnable;
import com.cavan.java.CavanString;
import com.cavan.java.RedPacketFinder;

public abstract class CavanAccessibilityBase<E> extends Handler implements Runnable {

	private static final long POLL_DELAY = 200;

	private boolean mForceUnpack;
	private boolean mGotoIdleEnable;

	protected CavanAccessibilityService mService;
	protected LinkedList<E> mPackets = new LinkedList<E>();
	protected String mClassName = CavanString.EMPTY_STRING;
	protected String mPackageName = CavanString.EMPTY_STRING;

	private DelayedRunnable mRunnableUnlock = new DelayedRunnable(this) {

		@Override
		protected void onRunableStateChanged(boolean enabled) {
			onLockStateChanged(enabled);
		}
	};

	private DelayedRunnable mRunnableBack = new DelayedRunnable(this) {

		@Override
		protected void onRunableFire() {
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

		if (locked) {
			removeCallbacks(this);
		} else {
			post(this);
		}
	}

	public abstract String getPackageName();

	public int getPacketCount() {
		int size = mPackets.size();

		CavanAndroid.dLog(getPackageName() + ": getPacketCount = " + size);
		return size;
	}

	public boolean addPacket(E packet) {
		if (mPackets.contains(packet)) {
			return false;
		}

		mGotoIdleEnable = true;
		mPackets.add(packet);
		setLockEnable(POLL_DELAY, false);

		return true;
	}

	public void clearPackets() {
		mForceUnpack = false;
		mPackets.clear();
	}

	public boolean setForceUnpackEnable(boolean enable) {
		if (mPackets.size() > 0) {
			mForceUnpack = false;
		} else if (enable) {
			mForceUnpack = true;
			mGotoIdleEnable = false;
			setLockEnable(POLL_DELAY, true);
		} else {
			mForceUnpack = false;
		}

		CavanAndroid.dLog(getPackageName() + ": mForceUnpack = " + mForceUnpack);

		return mForceUnpack;
	}

	protected void onWindowStateChanged(AccessibilityEvent event) {
		if (getPacketCount() > 0) {
			setLockEnable(POLL_DELAY, true);
		}
	}

	protected void onWindowContentChanged(AccessibilityEvent event) {}
	protected void onViewClicked(AccessibilityEvent event) {}
	protected void onViewTextChanged(AccessibilityEvent event) {}

	protected boolean isValidPackageName(CharSequence pkgName) {
		return getPackageName().equals(pkgName);
	}

	protected long onPollEventFire(AccessibilityNodeInfo root) {
		return 0;
	}

	protected boolean onKeyEvent(KeyEvent event) {
		if (event.getKeyCode() == KeyEvent.KEYCODE_VOLUME_DOWN && getPacketCount() > 0) {
			if (event.getAction() == KeyEvent.ACTION_UP) {
				clearPackets();
			}

			return true;
		}

		return false;
	}

	public void performWindowStateChanged(AccessibilityEvent event, String packageName, String className) {
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
		return mRunnableUnlock.isEnabled();
	}

	public void setLockEnable(long delay, boolean force) {
		mRunnableBack.cancel();
		mRunnableUnlock.post(delay, force);
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
		mRunnableBack.post(delay, false);
	}

	public void cancelGlobalBack() {
		mRunnableBack.cancel();
	}

	public boolean startNextActivity() {
		if (mService.startNextPendingActivity()) {
			return true;
		}

		CavanAndroid.dLog("mGotoIdleEnable = " + mGotoIdleEnable);

		if (mGotoIdleEnable) {
			mService.startIdleActivity();
		}

		return false;
	}

	@Override
	public void run() {
		removeCallbacks(this);

		CavanAndroid.dLog("mForceUnpack = " + mForceUnpack);

		boolean pollEnable;

		if (isLocked()) {
			CavanAndroid.dLog("isLocked");
			pollEnable = false;
		} else if (mForceUnpack) {
			pollEnable = true;
		} else if (getPacketCount() < 1) {
			startNextActivity();
			pollEnable = false;
		} else if (mService.getMessageCount() < 1) {
			clearPackets();
			pollEnable = false;
		} else {
			pollEnable = true;
		}

		CavanAndroid.dLog("pollEnable = " + pollEnable);

		if (pollEnable) {
			long delay;

			AccessibilityNodeInfo root = getRootInActiveWindow();
			if (root != null) {
				CharSequence pkgName = root.getPackageName();

				CavanAndroid.dLog("getPackageName = " + pkgName);

				if (isValidPackageName(pkgName)) {
					delay = onPollEventFire(root);
					if (delay > 0) {
						postDelayed(this, delay);
					} else {
						postDelayed(this, POLL_DELAY);
					}
				}
			} else {
				postDelayed(this, POLL_DELAY);
			}
		}
	}
}