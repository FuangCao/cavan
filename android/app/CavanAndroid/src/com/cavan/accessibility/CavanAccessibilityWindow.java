package com.cavan.accessibility;

import android.view.KeyEvent;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;


public class CavanAccessibilityWindow {

	public static final int WAIT_DELAY = 200;
	public static final int WAIT_TIMES = 5;

	protected int mActivityHashCode;
	protected String mBackViewId;
	protected String mName;

	protected Thread mWaitReadyThread = new Thread() {

		private long mUpdateTime;
		private int mWaitTimes;

		@Override
		public synchronized void start() {
			if (mUpdateTime == 0) {
				if (isAlive()) {
					notify();
				} else {
					super.start();
				}
			}

			mUpdateTime = System.currentTimeMillis();
		}

		@Override
		public synchronized void run() {
			while (true) {
				long timeNow = System.currentTimeMillis();
				long time = mUpdateTime + WAIT_DELAY;

				if (time > timeNow) {
					try {
						wait(time - timeNow);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				} else if (++mWaitTimes > WAIT_TIMES || onWindowContentReady(mWaitTimes)) {
					mUpdateTime = 0;
					mWaitTimes = 0;

					try {
						wait();
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				} else {
					mUpdateTime = timeNow + WAIT_DELAY;
				}
			}
		}
	};

	public CavanAccessibilityWindow(String name) {
		mName = name;
	}

	public String getName() {
		return mName;
	}

	public void setBackViewId(String id) {
		mBackViewId = id;
	}

	public String getBackViewId() {
		return mBackViewId;
	}

	public void startWaitReady() {
		mWaitReadyThread.start();
	}

	public boolean poll(CavanRedPacket packet, AccessibilityNodeInfo root, int times) {
		return false;
	}

	public boolean performActionBack(AccessibilityNodeInfo root) {
		String vid = mBackViewId;
		if (vid == null) {
			return false;
		}

		return (CavanAccessibilityHelper.performClickByViewIds(root, vid) > 0);
	}

	public boolean performActionBack(AccessibilityNodeInfo root, CavanAccessibilityPackage pkg) {
		if (performActionBack(root)) {
			return true;
		}

		if (pkg != null) {
			return pkg.performActionBack(root, true);
		}

		return false;
	}

	protected boolean doSendText(AccessibilityNodeInfo root, String message, boolean commit) {
		return false;
	}

	protected boolean doLogin(AccessibilityNodeInfo root, String username, String password) {
		return false;
	}

	protected boolean doWebCommand(AccessibilityNodeInfo root, String action) {
		return false;
	}

	protected boolean doRefresh(AccessibilityNodeInfo root) {
		return false;
	}

	protected boolean doSignin(AccessibilityNodeInfo root) {
		return false;
	}

	protected boolean doFollow(AccessibilityNodeInfo root) {
		return false;
	}

	protected boolean doUnfollow(AccessibilityNodeInfo root) {
		return false;
	}

	protected boolean doCommandShare(AccessibilityNodeInfo root, boolean friends) {
		return false;
	}

	protected boolean doActionBack(AccessibilityNodeInfo root) {
		return false;
	}

	protected boolean doActionHome(AccessibilityNodeInfo root) {
		return doActionBack(root);
	}

	protected void onPackageUpdated() {}
	protected void onProgress(CavanAccessibilityWindow win) {}
	protected void onAndroidWidget(String name) {}
	protected void onEnter(AccessibilityNodeInfo root) {}
	protected void onLeave(AccessibilityNodeInfo root) {}
	protected void onWindowContentChanged(AccessibilityNodeInfo root, AccessibilityEvent event) {}
	protected void onViewClicked(AccessibilityNodeInfo root, AccessibilityEvent event) {}
	protected void onViewTextChanged(AccessibilityNodeInfo root, AccessibilityEvent event) {}
	protected void onKeyDown(AccessibilityNodeInfo root, int keyCode) {}
	protected void onKeyUp(AccessibilityNodeInfo root, int keyCode) {}

	protected void onKeyEvent(AccessibilityNodeInfo root, KeyEvent event) {
		switch (event.getAction()) {
		case KeyEvent.ACTION_DOWN:
			onKeyDown(root, event.getKeyCode());
			break;

		case KeyEvent.ACTION_UP:
			onKeyUp(root, event.getKeyCode());
			break;
		}
	}

	protected boolean onWindowContentReady(int times) {
		return true;
	}

	protected boolean onPollFailed(CavanRedPacket packet, int times) {
		return (times < CavanAccessibilityPackage.FAIL_TIMES);
	}

	public boolean isHomePage() {
		return false;
	}

	public boolean isProgressView() {
		return false;
	}

	public boolean isPopWindow() {
		return false;
	}

	@Override
	public String toString() {
		return mName;
	}

	public int getEventTypes(CavanAccessibilityPackage pkg) {
		return pkg.getEventTypes();
	}

	public void setActivityHashCode(int hashCode) {
		mActivityHashCode = hashCode;
	}

	public int getActivityHashCode() {
		return mActivityHashCode;
	}
}