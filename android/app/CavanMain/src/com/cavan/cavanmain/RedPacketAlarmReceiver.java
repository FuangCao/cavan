package com.cavan.cavanmain;

import com.cavan.android.CavanAndroid;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

public class RedPacketAlarmReceiver extends BroadcastReceiver {

	@Override
	public void onReceive(Context context, Intent intent) {
		String code = intent.getStringExtra("code");
		CavanAndroid.dLog("RedPacketAlarmReceiver: code = " + code);

		if (code != null) {
			intent = new Intent(MainActivity.ACTION_CODE_RECEIVED);
			intent.putExtra("code", code).putExtra("type", "定时器");
			context.sendBroadcast(intent);
		}
	}
}