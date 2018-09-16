package com.cavan.accessibility;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;

import com.cavan.android.CavanAndroid;

public class CavanUnlockActivity extends Activity implements Runnable {

	public static void setLockScreenEnable(Context context, boolean enable) {
		if (enable || CavanAndroid.SDK_VERSION < CavanAndroid.SDK_VERSION_80) {
			CavanAndroid.setLockScreenEnable(context, enable);
		} else {
			CavanAndroid.startActivity(context, CavanUnlockActivity.class);
		}
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		CavanAndroid.setActivityKeyguardEnable(this, false);
		CavanAndroid.requestDismissKeyguard(this);
		CavanAndroid.postRunnable(this);
	}

	@Override
	public void run() {
		finish();
	}
}
