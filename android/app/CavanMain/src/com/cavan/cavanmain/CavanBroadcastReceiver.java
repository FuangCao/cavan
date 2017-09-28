package com.cavan.cavanmain;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.cavan.android.CavanAndroid;

public class CavanBroadcastReceiver extends BroadcastReceiver {

	public static void setOnTimeNotifyAlarm(Context context) {
		AlarmManager manager = (AlarmManager) CavanAndroid.getSystemServiceCached(context, Context.ALARM_SERVICE);
		if (manager != null) {
			long timeNow = System.currentTimeMillis();
			long timeAlarm = timeNow / 3600000 * 3600000 + 3480000;

			if (timeAlarm <= timeNow) {
				timeAlarm += 3600000;
			}

			Intent intent = new Intent(context, CavanBroadcastReceiver.class).setAction(CavanMessageActivity.ACTION_ON_TIME_NOTIFY);
			PendingIntent operation = PendingIntent.getBroadcast(context, (int) timeAlarm, intent, PendingIntent.FLAG_UPDATE_CURRENT);

			manager.cancel(operation);
			CavanAndroid.setAlarm(manager, timeAlarm, operation);

			CavanAndroid.dLog("timeNow   = " + timeNow);
			CavanAndroid.dLog("timeAlarm = " + timeAlarm);
		}
	}

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
		} else if (CavanMessageActivity.ACTION_ON_TIME_NOTIFY.equals(action)) {
			if (CavanMessageActivity.isOnTimeNotifyEnabled(context)) {
				FloatMessageService service = FloatMessageService.getInstance();
				if (service != null) {
					service.showOnTimeNotify();
				}

				setOnTimeNotifyAlarm(context);
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