package com.cavan.weixinredpacket;

import com.cavan.android.CavanAndroid;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

public class CavanBroadcastReceiver extends BroadcastReceiver {

	@Override
	public void onReceive(Context context, Intent intent) {
		String action = intent.getAction();
		CavanAndroid.dLog("action = " + action);

		switch (action) {
		case Intent.ACTION_SCREEN_OFF:
			CavanAndroid.setLockScreenEnable(context, true);
			break;

		case Intent.ACTION_SCREEN_ON:
			break;
		}
	}
}
