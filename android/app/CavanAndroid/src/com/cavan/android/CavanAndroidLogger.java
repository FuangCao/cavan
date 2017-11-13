package com.cavan.android;

import android.util.Log;

import com.cavan.java.CavanLogger;

public class CavanAndroidLogger extends CavanLogger {

	@Override
	public void dLog(String message) {
		if (CavanAndroid.DLOG_ENABLE) {
			Log.d(CavanAndroid.TAG, message);
		}
	}

	@Override
	public void wLog(String message) {
		if (CavanAndroid.WLOG_ENABLE) {
			Log.w(CavanAndroid.TAG, message);
		}
	}

	@Override
	public void eLog(String message) {
		if (CavanAndroid.ELOG_ENABLE) {
			Log.e(CavanAndroid.TAG, message);
		}
	}

	@Override
	public void pLog(int index) {
		if (CavanAndroid.PLOG_ENABLE) {
			super.pLog(index + 1);
		}
	}

	@Override
	public void pfLog(int index, String format, Object... args) {
		if (CavanAndroid.PLOG_ENABLE) {
			super.pfLog(index + 1, format, args);
		}
	}
}