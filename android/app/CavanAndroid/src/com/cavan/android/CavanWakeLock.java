package com.cavan.android;

import android.content.Context;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;

import com.cavan.java.CavanIndexGenerator;

public class CavanWakeLock {

	private static final CavanIndexGenerator sGenerator = new CavanIndexGenerator();

	public static class FullLock extends CavanWakeLock {

		public FullLock() {
			super();
		}

		public FullLock(String tag) {
			super(tag);
		}

		@SuppressWarnings("deprecation")
		@Override
		protected int onGetWakeLockFlags() {
			return PowerManager.FULL_WAKE_LOCK;
		}
	}

	public static class WakupLock extends CavanWakeLock {

		public WakupLock() {
			super();
		}

		public WakupLock(String tag) {
			super(tag);
		}

		@SuppressWarnings("deprecation")
		@Override
		protected int onGetWakeLockFlags() {
			return PowerManager.FULL_WAKE_LOCK | PowerManager.ACQUIRE_CAUSES_WAKEUP;
		}
	}

	public static class DimLock extends CavanWakeLock {

		public DimLock() {
			super();
		}

		public DimLock(String tag) {
			super(tag);
		}

		@SuppressWarnings("deprecation")
		@Override
		protected int onGetWakeLockFlags() {
			return PowerManager.SCREEN_DIM_WAKE_LOCK | PowerManager.ON_AFTER_RELEASE;
		}
	}

	public static class BrightLock extends CavanWakeLock {

		public BrightLock() {
			super();
		}

		public BrightLock(String tag) {
			super(tag);
		}

		@SuppressWarnings("deprecation")
		@Override
		protected int onGetWakeLockFlags() {
			return PowerManager.SCREEN_BRIGHT_WAKE_LOCK | PowerManager.ON_AFTER_RELEASE;
		}
	}

	private String mTag;
	private WakeLock mLock;

	public CavanWakeLock(String tag) {
		setTag(tag);
	}

	public CavanWakeLock() {
		this(CavanWakeLock.class.getCanonicalName());
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
			mLock = manager.newWakeLock(onGetWakeLockFlags(), mTag + sGenerator.genIndex());
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

	protected int onGetWakeLockFlags() {
		return PowerManager.PARTIAL_WAKE_LOCK;
	}
}
