package com.cavan.accessibility;

import java.util.HashMap;
import java.util.List;

import android.app.Notification;
import android.graphics.Rect;
import android.os.Handler;
import android.os.Parcelable;
import android.view.KeyEvent;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;

import com.cavan.android.CavanAndroid;
import com.cavan.android.SystemProperties;
import com.cavan.java.CavanString;

public class CavanAccessibilityPackage {

	public static final int CMD_SEND_TEXT = 1;
	public static final int CMD_LOGIN = 2;
	public static final int CMD_REFRESH = 3;
	public static final int CMD_SIGNIN = 4;
	public static final int CMD_FOLLOW = 5;
	public static final int CMD_UNFOLLOW = 6;
	public static final int CMD_BACK = 7;
	public static final int CMD_HOME = 8;

	public static int WAIT_DELAY = 500;
	public static int BACK_DELAY = 5000;
	public static int POLL_DELAY = 500;
	public static int LOCK_DELAY = 2000;
	public static int POLL_TIMES = 10;
	public static int FAIL_TIMES = 10;

	public static final int PENDING_PACKET = 1 << 0;
	public static final int PENDING_HOME = 1 << 1;
	public static final int PENDING_ALL = PENDING_PACKET | PENDING_HOME;

	protected HashMap<Integer, CavanAccessibilityWindow> mActivitys = new HashMap<>();
	protected HashMap<String, CavanAccessibilityWindow> mWindows = new HashMap<>();

	protected CavanAccessibilityService mService;
	protected CavanAccessibilityWindow mWindow;
	protected CavanRedPacket mCurrentPacket;
	protected boolean mForceUnpack = true;
	protected int mPending;
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

		@Override
		public boolean isPopWindow() {
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

		CavanAccessibilityWindow previous = mWindows.put(win.getName(), win);
		if (previous != null) {
			CavanAndroid.wLog("Override: " + previous.getName());
		}
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

	public synchronized CavanAccessibilityWindow getWindow(int hashCode) {
		CavanAccessibilityWindow win = mWindow;

		if (win != null && win.getActivityHashCode() == hashCode) {
			return win;
		}

		win = mActivitys.get(hashCode);
		if (win == null) {
			return null;
		}

		CavanAndroid.dLog("getWindow: " + win);

		mWindow = win;

		return win;
	}

	public synchronized CavanAccessibilityWindow waitPopWindow(int hashCode) throws Exception {
		CavanAccessibilityWindow win = mWindow;
		if (win == null) {
			throw new Exception("window is null");
		}

		if (win.isPopWindow()) {
			win = mActivitys.get(hashCode);
			if (win == null) {
				return mWindow;
			}

			if (win.isPopWindow()) {
				return null;
			}
		}

		return win;
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
		return (mPending != 0);
	}

	public synchronized void setPending(boolean enabled, int mask) {
		if (enabled) {
			mPending |= mask;
			mForceUnpack = true;
			resetTimes();
			startPoll();
		} else {
			mPending &= ~mask;
		}
	}

	public synchronized void setPending(boolean enabled) {
		CavanAndroid.dLog("setPending: " + enabled);
		setPending(enabled, PENDING_PACKET);
	}

	public synchronized void setGotoHome(boolean enabled) {
		CavanAndroid.dLog("setGotoHome: " + enabled);
		setPending(enabled, PENDING_HOME);
	}

	public synchronized void setComplete() {
		mPending = 0;
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
		startPoll();
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

	public void startPoll() {
		mService.startPoll();
	}

	public Handler getHandler() {
		return mService.getHandler();
	}

	public void post(Runnable runnable) {
		getHandler().post(runnable);
	}

	public void postDelayed(Runnable runnable, long delay) {
		getHandler().postDelayed(runnable, delay);
	}

	public void cancel(Runnable runnable) {
		getHandler().removeCallbacks(runnable);
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
			return isCurrentPackage(root);
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

	public boolean isDialog(AccessibilityNodeInfo root) {
		Rect bounds = new Rect();
		root.getBoundsInScreen(bounds);
		return bounds.left > 0 || bounds.top > 0;
	}

	public int getEventTypes() {
		return mService.getEventTypes();
	}

	public boolean tryCloseDialog(AccessibilityNodeInfo root) {
		if (isDialog(root)) {
			AccessibilityNodeInfo child = CavanAccessibilityHelper.getChild(root, -1);
			if (child != null) {
				try {
					if (CavanAccessibilityHelper.isButton(child)) {
						return CavanAccessibilityHelper.performClick(child);
					}
				} catch (Exception e) {
					e.printStackTrace();
				} finally {
					child.recycle();
				}
			}

			return mService.performActionBack();
		}

		return false;
	}

	public synchronized void performPackageUpdated() {
		for (CavanAccessibilityWindow win : mWindows.values()) {
			win.onPackageUpdated();
		}

		onPackageUpdated();
	}

	protected synchronized CavanAccessibilityWindow onWindowStateChanged(AccessibilityNodeInfo root, CavanAccessibilityWindow win) {
		if (win != mWindow) {
			if (win != null && win.isProgressView()) {
				if (mWindow != null) {
					mWindow.onProgress(win);
				}

				return mWindow;
			}

			if (mWindow != null) {
				mWindow.onLeave(root);
			}

			mWindow = win;

			if (win != null) {
				int types = win.getEventTypes(this);
				mService.setEventTypes(types);
				win.onEnter(root);
				resetTimes();
			}
		}

		if (isPending()) {
			setUnlockTime(0);
		}

		return win;
	}

	protected synchronized CavanAccessibilityWindow onWindowStateChanged(AccessibilityNodeInfo root, AccessibilityEvent event) {
		String name = CavanString.fromCharSequence(event.getClassName(), null);
		if (name == null) {
			return null;
		}

		CavanAndroid.dLog("onWindowStateChanged: " + mName + "/" + name);
		touchUpdateTime();

		CavanAccessibilityWindow win = getWindow(name);

		if (win != null) {
			setActivityHashCode(win, root.hashCode());

			if (win.isPopWindow()) {
				mService.postWaitPopWindow(this, 100);
			}
		}

		if (win == null && name.startsWith("android.widget.")) {
			win = mWindow;

			if (win != null) {
				win.onAndroidWidget(name);
			}

			return win;
		}

		return onWindowStateChanged(root, win);
	}

	private synchronized void setActivityHashCode(CavanAccessibilityWindow win, int hashCode) {
		win.setActivityHashCode(hashCode);
		mActivitys.put(hashCode, win);
	}

	public synchronized void onAccessibilityEvent(AccessibilityNodeInfo root, AccessibilityEvent event) {
		switch (event.getEventType()) {
		case AccessibilityEvent.TYPE_WINDOW_STATE_CHANGED:
			mService.setPackage(this);
			onWindowStateChanged(root, event);
			break;

		case AccessibilityEvent.TYPE_WINDOW_CONTENT_CHANGED:
			if (mWindow != null) {
				mWindow.onWindowContentChanged(root, event);
			}
			break;

		case AccessibilityEvent.TYPE_VIEW_CLICKED:
			if (mWindow != null) {
				mWindow.onViewClicked(root, event);
			}

			int dump = SystemProperties.getInt("debug.cavan.dump.click", 0);
			if (dump > 0) {
				CavanAndroid.dLog("event = " + event);

				if (dump > 1) {
					AccessibilityNodeInfo source = event.getSource();
					CavanAndroid.dLog("source = " + source);

					if (source != null) {
						source.recycle();
					}
				}
			}
			break;

		case AccessibilityEvent.TYPE_VIEW_TEXT_CHANGED:
			if (mWindow != null) {
				mWindow.onViewTextChanged(root, event);
			}
			break;

		case AccessibilityEvent.TYPE_NOTIFICATION_STATE_CHANGED:
			Parcelable data = event.getParcelableData();
			if (data instanceof Notification) {
				onNotificationStateChanged(root, (Notification) data);
			}
			break;
		}
	}

	protected void onKeyEvent(CavanAccessibilityPackage pkg, AccessibilityNodeInfo root, KeyEvent event) {
		if (mWindow != null) {
			mWindow.onKeyEvent(root, event);
		}
	}

	public synchronized void onNotificationStateChanged(AccessibilityNodeInfo root, Notification data) {}

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
							}

							if (win.isHomePage(root)) {
								onHomePage(packet);
								return -1;
							}

							return POLL_DELAY;
						}

						packet.setCompleted();

						if (win.performActionBack(root, this)) {
							setUnlockDelay(LOCK_DELAY);
						}

						if (win.isHomePage(root)) {
							onHomePage(packet);
							return -1;
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

	public boolean doCommand(AccessibilityNodeInfo root, int command, Object[] args) {
		CavanAccessibilityWindow win = getWindow(root.hashCode());
		if (win == null) {
			return false;
		}

		switch (command) {
		case CMD_SEND_TEXT:
			CavanAndroid.dLog("CMD_SEND_TEXT");
			return win.doSendText(root, (String) args[0], (boolean) args[1]);

		case CMD_LOGIN:
			CavanAndroid.dLog("CMD_LOGIN");
			return win.doLogin(root, (String) args[0], (String) args[1]);

		case CMD_REFRESH:
			CavanAndroid.dLog("CMD_REFRESH");
			return win.doRefresh(root);

		case CMD_SIGNIN:
			CavanAndroid.dLog("CMD_SIGNIN");
			return win.doSignin(root);

		case CMD_FOLLOW:
			CavanAndroid.dLog("CMD_FOLLOW");
			return win.doFollow(root);

		case CMD_UNFOLLOW:
			CavanAndroid.dLog("CMD_UNFOLLOW");
			return win.doUnfollow(root);

		case CMD_BACK:
			CavanAndroid.dLog("CMD_BACK");
			return win.doActionBack(root);

		case CMD_HOME:
			CavanAndroid.dLog("CMD_HOME");
			return win.doActionHome(root);

		default:
			CavanAndroid.eLog("Invalid command: " + command);
			return false;
		}
	}

	protected void initWindows() {}
	protected void onPackageUpdated() {}
	protected void onPacketAdded(CavanRedPacket packet) {}
	protected void onPacketRemoved(CavanRedPacket packet) {}

	protected void onPacketCreated(CavanRedPacket packet) {
		addPacket(packet);
	}

	protected void onHomePage(CavanRedPacket packet) {
		mService.removePacket(packet);
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
