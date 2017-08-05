package com.cavan.cavanmain;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.cavan.android.CavanAndroid;

public class CavanBroadcastReceiver extends BroadcastReceiver {

	@Override
	public void onReceive(Context context, Intent intent) {
		String action = intent.getAction();
		CavanAndroid.dLog("action = " + action);

		if (CavanMessageActivity.ACTION_CODE_RECEIVED.equals(action)) {
			String code = intent.getStringExtra("code");
			if (code != null) {
				RedPacketListenerService listener = RedPacketListenerService.getInstance();
				if (listener != null) {
					listener.addRedPacketCode(code, "定时器", false);
				}
			}
		} else if (Intent.ACTION_BOOT_COMPLETED.equals(action)) {
			CavanAccessibilityService.sBootComplete = true;
			CavanAndroid.acquireWakeupLock(context, 60000);
			CavanAndroid.setLockScreenEnable(context, false);
			context.startService(new Intent(context, FloatMessageService.class));
			context.sendBroadcast(new Intent(CavanMessageActivity.ACTION_BOOT_COMPLETED));
		} else {
			String code = intent.getStringExtra("code");
			if (code != null) {
				RedPacketListenerService.postRedPacketCode(context, code);
				RedPacketListenerService.startAlipayActivity(context);
			}
		}
	}
}