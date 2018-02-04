package com.cavan.accessibility;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;

import android.app.Notification;
import android.os.Parcelable;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;

import com.cavan.android.CavanAndroid;
import com.cavan.java.CavanString;

public abstract class CavanAccessibilityPackage {

	public static int WAIT_DELAY = 500;
	public static int BACK_DELAY = 5000;
	public static int POLL_DELAY = 500;
	public static int LOCK_DELAY = 2000;

	protected HashMap<String, CavanAccessibilityWindow> mWindows = new HashMap<String, CavanAccessibilityWindow>();
	protected LinkedList<CavanRedPacket> mPackets = new LinkedList<CavanRedPacket>();

	protected CavanAccessibilityService mService;
	protected CavanAccessibilityWindow mWindow;
	protected boolean mForceUnpack = true;
	protected boolean mGotoIdleEnabled;
	protected boolean mPending;
	protected long mUpdateTime;
	protected long mUnpackTime;
	protected long mUnlockTime;
	protected int mPollTimes;
	protected String[] mNames;
	protected String mName;

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

	public abstract void initWindows();

	public synchronized void addWindow(CavanAccessibilityWindow win) {
		CavanAndroid.dLog(mName + " <= " + win.getName());
		mWindows.put(win.getName(), win);
	}

	public synchronized CavanAccessibilityWindow getWindow() {
		return mWindow;
	}

	public synchronized CavanAccessibilityWindow getWindow(String name) {
		return mWindows.get(name);
	}

	public synchronized void setWindow(CavanAccessibilityWindow win) {
		if (win != mWindow) {
			if (mWindow != null) {
				mWindow.onLeave();
			}

			mWindow = win;
			mPollTimes = 0;
		}

		if (win != null) {
			win.onEnter();
		}
	}

	public synchronized CavanAccessibilityService getService() {
		return mService;
	}

	public long getUnpackDelay() {
		return 0;
	}

	public void showCountDownView() {
		mService.showCountDownView(this);
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

	public synchronized long getUnpackRemain() {
		if (mUnpackTime > 0) {
			long timeNow = System.currentTimeMillis();
			if (mUnpackTime > timeNow) {
				return mUnpackTime - timeNow;
			}
		}

		return 0;
	}

	public synchronized boolean addPacket(CavanRedPacket packet) {
		long delay = getUnpackDelay();
		if (delay < 0) {
			return false;
		}

		mService.acquireWakeLock(20000);

		if (mPackets.contains(packet)) {
			return true;
		}

		long time = System.currentTimeMillis();
		if (mUnpackTime < time) {
			mUnpackTime = time + delay;
		}

		mPackets.add(packet);
		onPacketAdded(packet);

		if (packet != null) {
			mGotoIdleEnabled = true;
		}

		setPending(true);

		return true;
	}

	public synchronized boolean removePacket(CavanRedPacket packet) {
		if (mPackets.remove(packet)) {
			if (mPackets.isEmpty()) {
				setPending(false);
			}

			return true;
		}

		return false;
	}

	public synchronized void clearPackets() {
		mPackets.clear();
		setPending(false);
	}

	public synchronized int getPacketCount() {
		return mPackets.size();
	}

	public synchronized boolean isPending() {
		return mPending;
	}

	public synchronized void setGotoIdleEnable(boolean enabled) {
		mGotoIdleEnabled = enabled;
	}

	public synchronized boolean isGotoIdleEnabled() {
		boolean enabled = mGotoIdleEnabled;
		mGotoIdleEnabled = false;
		return enabled;
	}

	public synchronized void setPendingRaw(boolean pending) {
		CavanAndroid.dLog("setPendingRaw: " + pending);

		if (pending) {
			mForceUnpack = true;
			mPending = true;
			mPollTimes = 0;
			post();
		} else {
			mPending = false;
			mUnpackTime = 0;
		}
	}

	public synchronized void setPending(boolean pending) {
		setPendingRaw(pending || mPackets.size() > 0);
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

	public synchronized void setUnpackTime(long time) {
		mUnpackTime = time;
	}

	public synchronized long getUnpackTime() {
		return mUnpackTime;
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
		post();
	}

	public synchronized boolean launch() {
		CavanAndroid.dLog("Launch: " + getName());

		if (mPackets.size() > 0) {
			CavanRedPacket packet = mPackets.get(0);
			if (packet == null) {
				return true;
			}

			if (packet.send()) {
				return true;
			}
		}

		return CavanAndroid.startActivity(mService, getName());
	}

	public void post() {
		mService.post();
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
			return mWindow;
		}

		CavanAccessibilityWindow win = getWindow(name);
		setWindow(win);
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

	public synchronized long poll(AccessibilityNodeInfo root) {
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

				if (win.poll(root, ++mPollTimes)) {
					if (isPending()) {
						return POLL_DELAY;
					}

					return 0;
				}

				CavanAndroid.dLog("mPollTimes = " + mPollTimes);

				if (win.onPollFailed(mPollTimes)) {
					return POLL_DELAY;
				}

				clearPackets();

				return 0;
			} else {
				if (consume < BACK_DELAY) {
					return BACK_DELAY - consume;
				}

				mService.performActionBack();
				return POLL_DELAY;
			}
		}

		return 0;
	}

	public void onEnter() {}
	public void onLeave() {}
	public void onCreate() {}
	public void onDestroy() {}
	public void onPackageUpdated() {}
	public void onPacketAdded(CavanRedPacket packet) {}

	@Override
	public String toString() {
		return mName;
	}
}
