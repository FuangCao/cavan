package com.cavan.accessibility;

import java.util.HashMap;
import java.util.LinkedList;

import android.app.Notification;
import android.os.Parcelable;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;

import com.cavan.android.CavanAndroid;

public abstract class CavanAccessibilityPackage<E> {

	public static int WINDOW_WAIT_TIME = 500;
	public static int BACK_WAIT_TIME = 5000;

	private HashMap<String, CavanAccessibilityWindow> mWindows = new HashMap<String, CavanAccessibilityWindow>();
	private LinkedList<E> mPackets = new LinkedList<E>();

	private CavanAccessibilityService mService;
	private CavanAccessibilityWindow mWindow;
	private boolean mPending;
	private long mUpdateTime;
	private long mUnpckTime;

	public CavanAccessibilityPackage(CavanAccessibilityService service) {
		mService = service;
	}

	public abstract String getPackageName();
	public abstract int getEventTypes();

	public void addWindow(String name, CavanAccessibilityWindow win) {
		mWindows.put(name, win);
	}

	public CavanAccessibilityWindow getWindow() {
		return mWindow;
	}

	public CavanAccessibilityService getService() {
		return mService;
	}

	public void addPacket(E code) {
		mPackets.add(code);
	}

	public void clearPackets() {
		mPackets.clear();
	}

	public int getPacketCount() {
		return mPackets.size();
	}

	public boolean isPending() {
		return mPending || mPackets.size() > 0;
	}

	public void setPending(boolean pending) {
		mPending = pending;
	}

	public void touchUpdateTime() {
		mUpdateTime = System.currentTimeMillis();
	}

	public long getUpdateTime() {
		return mUpdateTime;
	}

	public long getTimeConsume() {
		return System.currentTimeMillis() - mUpdateTime;
	}

	public void setUnpackTime(long time) {
		mUnpckTime = time;
	}

	public long getUnpackTime() {
		return mUnpckTime;
	}

	public boolean launch() {
		return CavanAndroid.startActivity(mService, getPackageName());
	}

	public void post(long delay) {
		mService.post(delay);
	}

	protected CavanAccessibilityWindow onWindowStateChanged(AccessibilityEvent event) {
		touchUpdateTime();

		CharSequence sequence = event.getClassName();
		if (sequence == null) {
			return null;
		}

		String className = sequence.toString();
		if (className.startsWith("android.widget.")) {
			return mWindow;
		}

		CavanAccessibilityWindow win = mWindows.get(className);
		if (win != mWindow) {
			if (mWindow != null) {
				mWindow.leave();
			}

			mWindow = win;

			if (win != null) {
				win.enter();
			}
		}

		return win;
	}

	public void onAccessibilityEvent(AccessibilityEvent event) {
		switch (event.getEventType()) {
		case AccessibilityEvent.TYPE_WINDOW_STATE_CHANGED:
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

	public void onNotificationStateChanged(Notification data) {}

	public long run(AccessibilityNodeInfo root) {
		if (isPending()) {
			long timeNow = System.currentTimeMillis();
			long consume = timeNow - mUpdateTime;

			if (mWindow != null) {
				if (consume < WINDOW_WAIT_TIME) {
					return WINDOW_WAIT_TIME - consume;
				}

				return mWindow.run(root);
			} else if (consume < BACK_WAIT_TIME) {
				return BACK_WAIT_TIME - consume;
			} else {
				mService.performActionBack();
				return WINDOW_WAIT_TIME;
			}
		}

		return 0;
	}

	public void onCreate() {}
	public void onDestroy() {}
}
