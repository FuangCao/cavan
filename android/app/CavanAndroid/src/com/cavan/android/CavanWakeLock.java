package com.cavan.android;

import com.cavan.java.CavanIndexGenerator;

import android.content.Context;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;

@SuppressWarnings("deprecation")
public class CavanWakeLock {

	private static final CavanIndexGenerator sGenerator = new CavanIndexGenerator();

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
		this(CavanWakeLock.class.getCanonicalName(), wakeup);
	}

	public CavanWakeLock() {
		this(false);
	}

	synchronized public void release() {
		if (mLock != null && mLock.isHeld()) {
			mLock.release();
		}
	}

	synchronized public boolean acquire(PowerManager manager, long overtime) {
		if (mLock == null) {
			mLock = manager.newWakeLock(mFlags, mTag + sGenerator.genIndex());
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

	synchronized public boolean acquire(PowerManager manager) {
		return acquire(manager, 0);
	}

	synchronized public boolean acquire(Context context, long overtime) {
		PowerManager manager = (PowerManager) CavanAndroid.getCachedSystemService(context, Context.POWER_SERVICE);
		if (manager == null) {
			return false;
		}

		return acquire(manager, overtime);
	}

	synchronized public boolean acquire(Context context) {
		return acquire(context, 0);
	}

	synchronized public boolean isHeld() {
		return mLock != null && mLock.isHeld();
	}
}
