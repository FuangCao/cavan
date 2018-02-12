package com.cavan.accessibility;

import java.util.HashMap;
import java.util.List;

import android.app.Notification;
import android.os.Parcelable;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;

import com.cavan.android.CavanAndroid;
import com.cavan.java.CavanString;

public class CavanAccessibilityPackage {

	public static int WAIT_DELAY = 500;
	public static int BACK_DELAY = 5000;
	public static int POLL_DELAY = 500;
	public static int LOCK_DELAY = 2000;
	public static int POLL_TIMES = 10;
	public static int FAIL_TIMES = 10;

	protected HashMap<String, CavanAccessibilityWindow> mWindows = new HashMap<String, CavanAccessibilityWindow>();
	protected CavanAccessibilityService mService;
	protected CavanAccessibilityWindow mWindow;
	protected CavanRedPacket mCurrentPacket;
	protected boolean mForceUnpack = true;
	protected boolean mPending;
	protected long mUpdateTime;
	protected long mUnlockTime;
	protected int mPollTimes;
	protected int mFailTimes;
	protected String[] mNames;
	protected String mName;

	public class ProgressWindow extends CavanAccessibilityWindow {

		public ProgressWindow(String name) {
			super(name);
		}

		@Override
		public boolean isProgressView() {
			return true;
		}
	};

	public CavanAccessibilityPackage(CavanAccessibilityService service, String[] names) {
		mService = service;
		mName = names[0];
		mNames = names;
		initWindows();
	}

	public CavanAccessibilityPackage(CavanAccessibilityService service, String name) {
		this(service, new String[] { name });
	}

	public String getName() {
		return mName;
	}

	public String[] getNames() {
		return mNames;
	}

	public synchronized void resetTimes() {
		mPollTimes = 0;
		mFailTimes = 0;
	}

	public synchronized void addWindow(CavanAccessibilityWindow win) {
		CavanAndroid.dLog(mName + " <= " + win.getName());
		mWindows.put(win.getName(), win);
	}

	public synchronized void addProgressWindow(String name) {
		addWindow(new ProgressWindow(name));
	}

	public synchronized CavanAccessibilityWindow getWindow() {
		return mWindow;
	}

	public synchronized CavanAccessibilityWindow getWindow(String name) {
		return mWindows.get(name);
	}

	public synchronized CavanAccessibilityService getService() {
		return mService;
	}

	public long getUnpackDelay() {
		return 0;
	}

	public void showCountDownView(CavanRedPacket packet) {
		mService.showCountDownView(packet);
	}

	public void dismissCountDownView() {
		mService.showCountDownView(null);
	}

	public boolean addRecycleNode(AccessibilityNodeInfo node) {
		return mService.addRecycleNode(node);
	}

	public int addRecycleNodes(AccessibilityNodeInfo... nodes) {
		return mService.addRecycleNodes(nodes);
	}

	public int addRecycleNodes(List<AccessibilityNodeInfo> nodes) {
		return mService.addRecycleNodes(nodes);
	}

	public synchronized boolean addPacket(CavanRedPacket packet) {
		long delay = getUnpackDelay();
		if (delay < 0) {
			return false;
		}

		packet.setUnpackDelay(delay);

		if (!mService.addPacket(this, packet)) {
			return false;
		}

		setPending(true);

		return true;
	}

	public CavanRedPacketList getPackets() {
		return mService.getPackets();
	}

	public void removePacket(CavanRedPacket packet) {
		mService.removePacket(packet);
	}

	public int removePackets() {
		return mService.removePackets(this);
	}

	public synchronized boolean isPending() {
		return mPending;
	}

	public synchronized void setPending(boolean pending) {
		CavanAndroid.dLog("setPending: " + pending);

		if (pending) {
			mForceUnpack = true;
			mPending = true;
			resetTimes();
			startPollThread();
		} else {
			mPending = false;
		}
	}

	public synchronized CavanRedPacket getCurrentPacket() {
		return mCurrentPacket;
	}

	public synchronized void setCurrentPacket(CavanRedPacket packet) {
		mCurrentPacket = packet;
	}

	public synchronized void touchUpdateTime() {
		mUpdateTime = System.currentTimeMillis();
	}

	public synchronized long getUpdateTime() {
		return mUpdateTime;
	}

	public synchronized long getTimeConsume() {
		return System.currentTimeMillis() - mUpdateTime;
	}

	public synchronized void setForceUnpackEnable(boolean enabled) {
		mForceUnpack = enabled;
	}

	public synchronized boolean isForceUnpackEnabled() {
		CavanAndroid.dLog("mForceUnpack = " + mForceUnpack);
		return mForceUnpack;
	}

	public synchronized void setUnlockDelay(long delay) {
		mUnlockTime = System.currentTimeMillis() + delay;
	}

	public synchronized void setUnlockTime(long time) {
		mUnlockTime = time;
		startPollThread();
	}

	public String[] getBackViewIds() {
		return null;
	}

	public boolean performActionBack(AccessibilityNodeInfo root, boolean force) {
		String[] vids = getBackViewIds();

		if (vids != null && CavanAccessibilityHelper.performClickByViewIds(root, vids) > 0) {
			return true;
		}

		if (force) {
			return mService.performActionBack();
		}

		return false;
	}

	public synchronized boolean launch() {
		CavanAndroid.dLog("Launch: " + getName());
		return CavanAndroid.startActivity(mService, getName());
	}

	public void startPollThread() {
		mService.startPollThread();
	}

	public boolean isCurrentPackage(String pkgName) {
		for (String name : getNames()) {
			if (name.equals(pkgName)) {
				return true;
			}
		}

		return false;
	}

	public boolean isCurrentPackage(AccessibilityNodeInfo root) {
		CharSequence name = root.getPackageName();
		if (name == null) {
			return false;
		}

		return isCurrentPackage(name.toString());
	}

	public boolean isCurrentPackage() {
		AccessibilityNodeInfo root = getRootInActiveWindow();
		if (root == null) {
			return false;
		}

		try {
			return isCurrentPackage(getRootInActiveWindow());
		} catch (Exception e) {
			e.printStackTrace();
		} finally {
			root.recycle();
		}

		return false;
	}

	public AccessibilityNodeInfo getRootInActiveWindow() {
		return mService.getRootInActiveWindow(10);
	}

	public synchronized void performPackageUpdated() {
		for (CavanAccessibilityWindow win : mWindows.values()) {
			win.onPackageUpdated();
		}

		onPackageUpdated();
	}

	protected synchronized CavanAccessibilityWindow onWindowStateChanged(AccessibilityEvent event) {
		String name = CavanString.fromCharSequence(event.getClassName(), null);
		if (name == null) {
			return null;
		}

		CavanAndroid.dLog("onWindowStateChanged: " + mName + "/" + name);
		touchUpdateTime();

		if (name.startsWith("android.widget.")) {
			CavanAccessibilityWindow win = mWindow;
			if (win != null) {
				win.onAndroidWidget(name);
			}

			return win;
		}

		CavanAccessibilityWindow win = getWindow(name);
		if (win != mWindow) {
			if (win != null && win.isProgressView()) {
				if (mWindow != null) {
					mWindow.onProgress(name);
				}

				return mWindow;
			}

			if (mWindow != null) {
				mWindow.onLeave();
			}

			mWindow = win;

			if (win != null) {
				win.onEnter();
				resetTimes();
			}
		}

		if (isPending()) {
			setUnlockTime(0);
		}

		return win;
	}

	public synchronized void onAccessibilityEvent(AccessibilityEvent event) {
		switch (event.getEventType()) {
		case AccessibilityEvent.TYPE_WINDOW_STATE_CHANGED:
			mService.setPackage(this);
			onWindowStateChanged(event);
			break;

		case AccessibilityEvent.TYPE_WINDOW_CONTENT_CHANGED:
			if (mWindow != null) {
				mWindow.onWindowContentChanged(event);
			}
			break;

		case AccessibilityEvent.TYPE_VIEW_CLICKED:
			if (mWindow != null) {
				mWindow.onViewClicked(event);
			}
			break;

		case AccessibilityEvent.TYPE_VIEW_TEXT_CHANGED:
			if (mWindow != null) {
				mWindow.onViewTextChanged(event);
			}
			break;

		case AccessibilityEvent.TYPE_NOTIFICATION_STATE_CHANGED:
			Parcelable data = event.getParcelableData();
			if (data instanceof Notification) {
				onNotificationStateChanged((Notification) data);
			}
			break;
		}
	}

	public synchronized void onNotificationStateChanged(Notification data) {}

	public synchronized long poll(CavanRedPacket packet, AccessibilityNodeInfo root) {
		CavanAndroid.dLog("package = " + mName);

		if (isPending()) {
			long timeNow = System.currentTimeMillis();
			if (timeNow < mUnlockTime) {
				return mUnlockTime - timeNow;
			}

			long consume = timeNow - mUpdateTime;
			if (consume < WAIT_DELAY) {
				return WAIT_DELAY - consume;
			}

			CavanAccessibilityWindow win = mWindow;
			if (win != null) {
				CavanAndroid.dLog("window = " + win);

				try {
					mPollTimes++;
					CavanAndroid.dLog("mPollTimes = " + mPollTimes);

					if (win.poll(packet, root, mPollTimes)) {
						if (mPollTimes < POLL_TIMES) {
							mFailTimes = 0;

							if (packet.isPending()) {
								return packet.getUnpackDelay(POLL_DELAY);
							} else if (win.isMainActivity()) {
								mService.removePacket(packet);
								return -1;
							}

							return POLL_DELAY;
						} else {
							packet.setCompleted();

							if (win.performActionBack(root, this)) {
								setUnlockDelay(LOCK_DELAY);
							}
						}
					}

					mFailTimes++;
					CavanAndroid.dLog("mFailTimes = " + mFailTimes);

					if (win.onPollFailed(packet, mFailTimes)) {
						return POLL_DELAY;
					}
				} catch (Exception e) {
					e.printStackTrace();
				}

				mService.removePackets(this);
				return -1;
			} else {
				if (consume < BACK_DELAY) {
					if (performActionBack(root, false)) {
						return POLL_DELAY;
					}

					return BACK_DELAY - consume;
				}

				performActionBack(root, true);
				return POLL_DELAY;
			}
		}

		return -1;
	}

	protected void initWindows() {}
	protected void onPackageUpdated() {}
	protected void onPacketAdded(CavanRedPacket packet) {}
	protected void onPacketRemoved(CavanRedPacket packet) {}

	protected void onPacketCreated(CavanRedPacket packet) {
		addPacket(packet);
	}

	protected void onEnter() {}
	protected void onLeave() {}
	protected void onCreate() {}
	protected void onDestroy() {}
	protected void onPollStarted() {}
	protected void onPollStopped() {}

	@Override
	public String toString() {
		return mName;
	}
}
