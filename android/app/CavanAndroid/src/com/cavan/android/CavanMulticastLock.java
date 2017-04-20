package com.cavan.android;

import android.content.Context;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiManager.MulticastLock;

public class CavanMulticastLock {

	private String mTag;
	private MulticastLock mLock;

	public CavanMulticastLock(String tag) {
		mTag = tag;
	}

	public CavanMulticastLock() {
		this(CavanMulticastLock.class.getCanonicalName());
	}

	public void release() {
		if (mLock != null) {
			mLock.release();
		}
	}

	public boolean acquire(WifiManager manager) {
		if (mLock == null) {
			mLock = manager.createMulticastLock(mTag);
			if (mLock == null) {
				return false;
			}
		}

		mLock.acquire();

		return true;
	}

	public boolean acquire(Context context) {
		WifiManager manager = (WifiManager) CavanAndroid.getSystemServiceCached(context, Context.WIFI_SERVICE);
		if (manager == null) {
			return false;
		}

		return acquire(manager);
	}
}
