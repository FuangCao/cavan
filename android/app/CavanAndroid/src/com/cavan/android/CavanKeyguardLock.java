package com.cavan.android;

import android.app.KeyguardManager;
import android.app.KeyguardManager.KeyguardLock;
import android.content.Context;

@SuppressWarnings("deprecation")
public class CavanKeyguardLock {

	private String mTag;
	private KeyguardLock mLock;

	public CavanKeyguardLock(String tag) {
		mTag = tag;
	}

	public CavanKeyguardLock(Class<?> cls) {
		this(cls.getCanonicalName());
	}

	public CavanKeyguardLock() {
		this(CavanKeyguardLock.class);
	}

	public void release() {
		if (mLock != null) {
			mLock.reenableKeyguard();
		}
	}

	public boolean acquire(KeyguardManager manager) {
		if (mLock == null) {
			mLock = manager.newKeyguardLock(mTag);
			if (mLock == null) {
				return false;
			}
		}

		mLock.disableKeyguard();

		return true;
	}

	public boolean acquire(Context context) {
		KeyguardManager manager = (KeyguardManager) CavanAndroid.getSystemServiceCached(context, Context.KEYGUARD_SERVICE);
		if (manager == null) {
			return false;
		}

		return acquire(manager);
	}
}
