package com.cavan.accessibility;

import android.view.KeyEvent;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;

import com.cavan.accessibility.CavanAccessibilityService.CavanAccessibilityCommand;
import com.cavan.android.CavanAndroid;

import java.util.concurrent.locks.ReentrantLock;


public class CavanAccessibilityWindow {

	public static final int WAIT_DELAY = 300;
	public static final int WAIT_TIMES = 5;

	public static final int CMD_SEND_TEXT = 1;
	public static final int CMD_LOGIN = 2;
	public static final int CMD_REFRESH = 3;
	public static final int CMD_SIGNIN = 4;
	public static final int CMD_FOLLOW = 5;
	public static final int CMD_UNFOLLOW = 6;
	public static final int CMD_BACK = 7;
	public static final int CMD_HOME = 8;
	public static final int CMD_WEB = 9;
	public static final int CMD_SHARE = 10;

	public static final int SHARE_FRIEND = 1;
	public static final int SHARE_CIRCLE = 2;
	public static final int SHARE_FRIEND_FAKE = 4;
	public static final int SHARE_CIRCLE_FAKE = 8;

	protected CavanAccessibilityPackage mPackage;
	protected int mActivityHashCode;
	protected String mBackViewId;
	protected String mName;

	protected long mEnterTime;
	protected long mLeaveTime;

	public static boolean isShareFriendMode(int mode) {
		return (mode & (SHARE_FRIEND | SHARE_FRIEND_FAKE)) != 0;
	}

	public static boolean isShareCircleMode(int mode) {
		return (mode & (SHARE_CIRCLE | SHARE_CIRCLE_FAKE)) != 0;
	}

	public static boolean isShareFakeMode(int mode) {
		return (mode & (SHARE_FRIEND_FAKE | SHARE_CIRCLE_FAKE)) != 0;
	}

	protected Thread mWaitReadyThread = new Thread() {

		private ReentrantLock mLock = new ReentrantLock();
		private long mUpdateTime;
		private int mWaitTimes;

		@Override
		public void start() {
			mLock.lock();

			if (mUpdateTime == 0) {
				synchronized (this) {
					if (isAlive()) {
						notify();
					} else {
						super.start();
					}
				}
			}

			mUpdateTime = System.currentTimeMillis();

			mLock.unlock();
		}

		private boolean performWindowContentReady(int times) {
			mLock.unlock();
			boolean yes = onWindowContentReady(times);
			mLock.lock();
			return yes;
		}

		@Override
		public void run() {
			mLock.lock();

			while (true) {
				long timeNow = System.currentTimeMillis();
				long time = mUpdateTime + WAIT_DELAY;

				if (time > timeNow) {
					try {
						mLock.unlock();

						synchronized (this) {
							wait(time - timeNow);
						}

						mLock.lock();
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				} else if (++mWaitTimes > WAIT_TIMES || performWindowContentReady(mWaitTimes)) {
					mUpdateTime = 0;
					mWaitTimes = 0;

					mLock.unlock();

					try {
						synchronized (this) {
							wait();
						}
					} catch (InterruptedException e) {
						e.printStackTrace();
					}

					mLock.lock();
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

	public long getEnterTime() {
		return mEnterTime;
	}

	public long getEnterDelay(long time) {
		return time - mEnterTime;
	}

	public long getLeaveTime() {
		return mLeaveTime;
	}

	public long getLeaveDelay(long time) {
		return time - mLeaveTime;
	}

	public void startWaitReady() {
		mWaitReadyThread.start();
	}

	public CavanAccessibilityPackage getPackage() {
		return mPackage;
	}

	public void setPackage(CavanAccessibilityPackage package1) {
		mPackage = package1;
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

	protected boolean doCommandShare(AccessibilityNodeInfo root, int mode) {
		return false;
	}

	protected boolean doActionBack(AccessibilityNodeInfo root) {
		return mPackage.performActionBack(root, true);
	}

	protected boolean doActionHome(AccessibilityNodeInfo root) {
		return doActionBack(root);
	}

	protected void onPackageUpdated() {}
	protected void onProgress(CavanAccessibilityWindow win) {}
	protected void onAndroidWidget(String name) {}

	protected void onEnter(AccessibilityNodeInfo root, long time) {
		mEnterTime = time;
	}

	protected void onLeave(AccessibilityNodeInfo root, long time) {
		mLeaveTime = time;
	}

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

	public boolean isCommandCompleted(CavanAccessibilityCommand command) {
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

	public boolean processCommand(AccessibilityNodeInfo root, CavanAccessibilityCommand command) {
		if (isCommandCompleted(command)) {
			return true;
		}

		Object[] args = command.getArgs();

		switch (command.getCommand()) {
		case CMD_SEND_TEXT:
			CavanAndroid.dLog("CMD_SEND_TEXT");
			return doSendText(root, (String) args[0], (boolean) args[1]);

		case CMD_LOGIN:
			CavanAndroid.dLog("CMD_LOGIN");
			return doLogin(root, (String) args[0], (String) args[1]);

		case CMD_WEB:
			CavanAndroid.dLog("CMD_WEB");
			return doWebCommand(root, (String) args[0]);

		case CMD_REFRESH:
			CavanAndroid.dLog("CMD_REFRESH");
			return doRefresh(root);

		case CMD_SIGNIN:
			CavanAndroid.dLog("CMD_SIGNIN");
			return doSignin(root);

		case CMD_FOLLOW:
			CavanAndroid.dLog("CMD_FOLLOW");
			return doFollow(root);

		case CMD_UNFOLLOW:
			CavanAndroid.dLog("CMD_UNFOLLOW");
			return doUnfollow(root);

		case CMD_SHARE:
			CavanAndroid.dLog("CMD_SHARE");
			return doCommandShare(root, (int) args[0]);

		case CMD_BACK:
			CavanAndroid.dLog("CMD_BACK");
			doActionBack(root);
			return false;

		case CMD_HOME:
			CavanAndroid.dLog("CMD_HOME");
			if (isHomePage()) {
				return false;
			}

			return doActionHome(root);

		default:
			CavanAndroid.eLog("Invalid command: " + command);
			return false;
		}
	}
}
