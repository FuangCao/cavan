package com.cavan.android;

import com.cavan.java.CavanIndexGenerator;

import android.app.KeyguardManager;
import android.app.KeyguardManager.KeyguardLock;
import android.content.Context;

@SuppressWarnings("deprecation")
public class CavanKeyguardLock {

	private static final CavanIndexGenerator sGenerator = new CavanIndexGenerator();

	private String mTag;
	private KeyguardLock mLock;

	public CavanKeyguardLock(String tag) {
		setTag(tag);
	}

	public CavanKeyguardLock() {
		this(CavanKeyguardLock.class.getCanonicalName());
	}

	public synchronized void setTag(String tag) {
		mTag = tag + sGenerator.genIndex();
	}

	public synchronized void release() {
		if (mLock != null) {
			mLock.reenableKeyguard();
		}
	}

	public synchronized boolean acquire(KeyguardManager manager) {
		if (mLock == null) {
			mLock = manager.newKeyguardLock(mTag);
			if (mLock == null) {
				return false;
			}
		}

		mLock.disableKeyguard();

		return true;
	}

	public synchronized boolean acquire(Context context) {
		KeyguardManager manager = (KeyguardManager) CavanAndroid.getSystemServiceCached(context, Context.KEYGUARD_SERVICE);
		if (manager == null) {
			return false;
		}

		return acquire(manager);
	}
}
