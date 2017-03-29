package com.cavan.android;

import android.content.Context;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;

@SuppressWarnings("deprecation")
public class CavanWakeLock {

	private int mFlags;
	private String mTag;
	private WakeLock mLock;

	public CavanWakeLock(String tag, int flags) {
		mTag = tag;
		mFlags = flags;
	}

	public CavanWakeLock(String tag, boolean wakeup) {
		mTag = tag;
		mFlags = PowerManager.FULL_WAKE_LOCK;

		if (wakeup) {
			mFlags |= PowerManager.ACQUIRE_CAUSES_WAKEUP;
		}
	}

	public CavanWakeLock(String tag) {
		this(tag, false);
	}

	public CavanWakeLock(boolean wakeup) {
		this(CavanWakeLock.class.getCanonicalName());
	}

	public CavanWakeLock() {
		this(false);
	}

	public void release() {
		if (mLock != null && mLock.isHeld()) {
			mLock.release();
		}
	}

	public boolean acquire(PowerManager manager, long overtime) {
		if (mLock == null) {
			mLock = manager.newWakeLock(mFlags, mTag);
			if (mLock == null) {
				return false;
			}
		}

		if (overtime > 0) {
			mLock.acquire(overtime);
		} else {
			mLock.acquire();
		}

		return true;
	}

	public boolean acquire(PowerManager manager) {
		return acquire(manager, 0);
	}

	public boolean acquire(Context context, long overtime) {
		PowerManager manager = (PowerManager) CavanAndroid.getCachedSystemService(context, Context.POWER_SERVICE);
		if (manager == null) {
			return false;
		}

		return acquire(manager, overtime);
	}

	public boolean acquire(Context context) {
		return acquire(context, 0);
	}

	public boolean isHeld() {
		return mLock != null && mLock.isHeld();
	}
}
