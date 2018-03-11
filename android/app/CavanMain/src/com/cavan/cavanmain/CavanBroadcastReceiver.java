package com.cavan.cavanmain;

import java.util.Calendar;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.media.AudioManager;

import com.cavan.accessibility.CavanAccessibilityService;
import com.cavan.android.CavanAndroid;
import com.cavan.java.CavanJava;

public class CavanBroadcastReceiver extends BroadcastReceiver {

	public static boolean sBootCompleted;

	public static void setOnTimeNotifyAlarm(Context context) {
		AlarmManager manager = (AlarmManager) CavanAndroid.getSystemServiceCached(context, Context.ALARM_SERVICE);
		if (manager != null) {
			long timeNow = System.currentTimeMillis();
			long timeAlarm = timeNow / 1800000 * 1800000 + 1680000;

			if (timeAlarm < timeNow) {
				timeAlarm += 1800000;
			}

			Intent intent = new Intent(context, CavanBroadcastReceiver.class).setAction(CavanMessageActivity.ACTION_ON_TIME_NOTIFY);
			PendingIntent operation = PendingIntent.getBroadcast(context, (int) timeAlarm, intent, PendingIntent.FLAG_UPDATE_CURRENT);

			manager.cancel(operation);
			CavanAndroid.setAlarm(manager, timeAlarm, operation);

			CavanAndroid.dLog("timeNow   = " + timeNow);
			CavanAndroid.dLog("timeAlarm = " + timeAlarm);
		}
	}

	public static void setOnTimeMuteAlarm(Context context, boolean enable, String value) {
		AlarmManager manager = (AlarmManager) CavanAndroid.getSystemServiceCached(context, Context.ALARM_SERVICE);
		if (manager != null) {
			Intent intent = new Intent(context, CavanBroadcastReceiver.class).setAction(CavanMessageActivity.ACTION_ON_TIME_MUTE);
			PendingIntent operation = PendingIntent.getBroadcast(context, 0, intent, PendingIntent.FLAG_UPDATE_CURRENT);

			manager.cancel(operation);

			if (enable) {
				Calendar calendar = Calendar.getInstance();
				int minute = CavanJava.parseInt(value);
				long timeNow = calendar.getTimeInMillis();

				calendar.set(Calendar.HOUR_OF_DAY, minute / 60);
				calendar.set(Calendar.MINUTE, minute % 60);
				calendar.set(Calendar.SECOND, 0);

				long timeAlarm = calendar.getTimeInMillis();
				if (timeAlarm < timeNow) {
					timeAlarm += 86400000;
				}

				CavanAndroid.setAlarm(manager, timeAlarm, operation);

				CavanAndroid.dLog("timeNow   = " + timeNow);
				CavanAndroid.dLog("timeAlarm = " + timeAlarm);
			}
		}
	}

	public static void setOnTimeMuteAlarm(Context context, boolean enable) {
		setOnTimeMuteAlarm(context, enable, CavanMessageActivity.getOnTimeMuteSetting(context));
	}

	@Override
	public void onReceive(Context context, Intent intent) {
		String action = intent.getAction();
		CavanAndroid.dLog("action = " + action);

		if (CavanAccessibilityService.ACTION_CODE_RECEIVED.equals(action)) {
			String code = intent.getStringExtra("code");
			if (code != null) {
				RedPacketListenerService listener = RedPacketListenerService.instance;
				if (listener != null) {
					listener.addRedPacketCode(code, "定时器", false);
				}
			}
		} else if (CavanMessageActivity.ACTION_ON_TIME_NOTIFY.equals(action)) {
			if (CavanMessageActivity.isOnTimeNotifyEnabled(context)) {
				FloatMessageService service = FloatMessageService.instance;
				if (service != null) {
					service.showOnTimeNotify();
				}

				setOnTimeNotifyAlarm(context);
			}
		} else if (CavanMessageActivity.ACTION_ON_TIME_MUTE.equals(action)) {
			if (CavanMessageActivity.isOnTimeMuteEnabled(context)) {
				AudioManager manager = (AudioManager) CavanAndroid.getSystemServiceCached(context, Context.AUDIO_SERVICE);
				manager.setStreamVolume(AudioManager.STREAM_MUSIC, 0, 0);
				setOnTimeMuteAlarm(context, true);
			}
		} else if (Intent.ACTION_BOOT_COMPLETED.equals(action)) {
			sBootCompleted = true;
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