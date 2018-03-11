package com.cavan.accessibility;

import com.cavan.android.CavanAndroid;

import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;


public class CavanAccessibilityWindow {

	public static final int WAIT_DELAY = 200;
	public static final int WAIT_TIMES = 5;

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

	protected void onEnter() {}
	protected void onLeave() {}
	protected void onPackageUpdated() {}
	protected void onProgress(String name) {}
	protected void onAndroidWidget(String name) {}
	protected void onWindowContentChanged(AccessibilityEvent event) {}
	protected void onViewClicked(AccessibilityEvent event) {}
	protected void onViewTextChanged(AccessibilityEvent event) {}

	protected boolean onWindowContentReady(int times) {
		CavanAndroid.pLog();
		return true;
	}

	protected boolean onPollFailed(CavanRedPacket packet, int times) {
		return (times < CavanAccessibilityPackage.FAIL_TIMES);
	}

	public boolean isMainActivity() {
		return false;
	}

	public boolean isProgressView() {
		return false;
	}

	@Override
	public String toString() {
		return mName;
	}
}