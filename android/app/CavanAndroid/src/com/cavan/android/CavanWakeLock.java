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
		setTag(tag);
		mFlags = flags;
	}

	public CavanWakeLock(String tag, boolean light, boolean wakeup) {
		setTag(tag);

		if (wakeup) {
			mFlags = PowerManager.FULL_WAKE_LOCK | PowerManager.ACQUIRE_CAUSES_WAKEUP;
		} else if (light) {
			mFlags = PowerManager.FULL_WAKE_LOCK;
		} else {
			mFlags = PowerManager.PARTIAL_WAKE_LOCK;
		}
	}

	public CavanWakeLock(String tag, boolean wakeup) {
		this(tag, false, wakeup);
	}

	public CavanWakeLock(String tag) {
		this(tag, false);
	}

	public CavanWakeLock(boolean light, boolean wakeup) {
		this(CavanWakeLock.class.getCanonicalName(), light, wakeup);
	}

	public CavanWakeLock(boolean wakeup) {
		this(false, wakeup);
	}

	public CavanWakeLock() {
		this(false);
	}

	synchronized public void setTag(String tag) {
		mTag = tag + sGenerator.genIndex();
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
		PowerManager manager = (PowerManager) CavanAndroid.getSystemServiceCached(context, Context.POWER_SERVICE);
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
