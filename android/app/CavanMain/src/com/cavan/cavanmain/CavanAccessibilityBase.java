package com.cavan.cavanmain;

import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;

import android.os.Handler;
import android.view.KeyEvent;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityNodeInfo;

import com.cavan.android.CavanAccessibilityHelper;
import com.cavan.android.CavanAndroid;
import com.cavan.android.DelayedRunnable;
import com.cavan.java.CavanString;
import com.cavan.java.RedPacketFinder;

public abstract class CavanAccessibilityBase<E> extends Handler implements Runnable {

	private static final long POLL_DELAY = 200;
	private static final long STABLE_DELAY = 200;

	private static final HashSet<CharSequence> sExcludeMessages = new HashSet<CharSequence>();

	static {
		sExcludeMessages.add("发送");
		sExcludeMessages.add("完成");
		sExcludeMessages.add("单选");
		sExcludeMessages.add("多选");
	};

	private long mStableTime;
	private int mStableTimes;
	private boolean mForceUnpack;
	private boolean mGotoIdleEnable;

	protected CavanAccessibilityService mService;
	protected LinkedList<E> mPackets = new LinkedList<E>();
	protected String mClassName = CavanString.EMPTY_STRING;
	protected String mClassNamePrev = CavanString.EMPTY_STRING;
	protected String mPackageName = CavanString.EMPTY_STRING;
	protected HashSet<Integer> mContentChangedHashCodes = new HashSet<Integer>();

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

	private Runnable mRunnableContentStable = new Runnable() {

		@Override
		public void run() {
			long time = System.currentTimeMillis();
			if (mStableTime > time) {
				postDelayed(this, mStableTime - time);
			} else if (mContentChangedHashCodes.size() > 0 && getWindowTimeConsume() > 500) {
				CavanAndroid.dLog("mStableTimes = " + mStableTimes);

				if (!onWindowContentStable(mStableTimes) && ++mStableTimes < 3) {
					postDelayed(this, POLL_DELAY);
				} else {
					mContentChangedHashCodes.clear();
				}
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
		removeCallbacks(mRunnableContentStable);

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

	protected boolean onWindowContentChanged(AccessibilityEvent event, AccessibilityNodeInfo source) {
		return false;
	}

	protected boolean onWindowContentStable(int times) {
		return true;
	}

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
		mClassNamePrev = mClassName;
		mClassName = className;
		onWindowStateChanged(event);
	}

	public void performWindowContentChanged(AccessibilityEvent event) {
		if (mPackets.size() > 0) {
			return;
		}

		AccessibilityNodeInfo source = event.getSource();
		if (source == null) {
			return;
		}

		if (onWindowContentChanged(event, source)) {
			mContentChangedHashCodes.add(source.hashCode());
		}

		long time = System.currentTimeMillis();
		if (mStableTime < time) {
			postDelayed(mRunnableContentStable, STABLE_DELAY);
		}

		mStableTimes = 0;
		mStableTime = time + STABLE_DELAY;

		source.recycle();
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

	public void postClickEventMessage(AccessibilityEvent event) {
		List<CharSequence> texts = event.getText();
		if (texts == null || texts.size() != 1) {
			return;
		}

		String text = CavanString.fromCharSequence(texts.get(0));
		if (sExcludeMessages.contains(text)) {
			CavanAndroid.dLog("Exclude message: " + text);
			return;
		}

		if (text.length() > 0 && RedPacketFinder.containsUrl(text) == false) {
			FloatEditorDialog dialog = FloatEditorDialog.getInstance(mService, text, true, true);
			dialog.show(6000);
		}

		RedPacketListenerService listener = RedPacketListenerService.getInstance();
		if (listener != null) {
			listener.addRedPacketContent(event.getPackageName(), text, "用户点击", false, true, 0);
		}
	}

	public void performGlobalBack() {
		CavanAccessibilityHelper.performGlobalBack(mService);
	}

	public void performGlobalBack(long delay) {
		setLockEnable(delay, false);
		CavanAccessibilityHelper.performGlobalBack(mService);
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

	public boolean commitText(AccessibilityNodeInfo root, CavanInputMethod ime) {
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
