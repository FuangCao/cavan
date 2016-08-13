package com.cavan.cavanmain;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

public class RedPacketBroadcastReceiver extends BroadcastReceiver {

	@Override
	public void onReceive(Context context, Intent intent) {
		String code = intent.getStringExtra("code");
		if (code != null) {
			RedPacketListenerService.postRedPacketCode(context, code);
			RedPacketListenerService.startAlipayActivity(context);
		}
	}
}