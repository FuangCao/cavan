package com.cavan.cavanmain;

import android.app.Notification;
import android.app.NotificationManager;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.Message;
import android.service.notification.NotificationListenerService;
import android.service.notification.StatusBarNotification;

import com.cavan.android.CavanAndroid;

public class RedPacketListenerService extends NotificationListenerService {

	public static final int NOTIFY_TEST = -1;

	private ClipboardManager mClipboardManager;
	private NotificationManager mNotificationManager;
	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			StatusBarNotification sbn = (StatusBarNotification) msg.obj;

			String pkgName = sbn.getPackageName();
			if (getPackageName().equals(pkgName)) {
				if (sbn.getId() != NOTIFY_TEST) {
					return;
				}
			}

			RedPacketNotification notification = new RedPacketNotification(RedPacketListenerService.this, sbn);
			notification.sendRedPacketNotifyAuto();
		}
	};

	public boolean sendNotification(String tag, Notification notification) {
		if (mNotificationManager == null) {
			return false;
		}

		mNotificationManager.notify(tag, 0, notification);

		return true;
	}

	public static boolean postRedPacketCode(ClipboardManager manager, CharSequence code) {
		if (manager == null) {
			return false;
		}

		ClipData data = ClipData.newPlainText("支付宝红包口令", code);

		CavanAndroid.logE("ClipData = " + data);
		manager.setPrimaryClip(data);

		return true;
	}

	public static boolean postRedPacketCode(Context context, String code) {
		ClipboardManager manager = (ClipboardManager) context.getSystemService(CLIPBOARD_SERVICE);
		return postRedPacketCode(manager, code);
	}

	public boolean postRedPacketCode(CharSequence code) {
		return postRedPacketCode(mClipboardManager, code);
	}

	public static boolean startAlipayActivity(Context context) {
		Intent intent = context.getPackageManager().getLaunchIntentForPackage("com.eg.android.AlipayGphone");
		if (intent == null) {
			return false;
		}

		intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED);
		context.startActivity(intent);

		return true;
	}

	public boolean startAlipayActivity() {
		return startAlipayActivity(this);
	}

	@Override
	public void onCreate() {
		mClipboardManager = (ClipboardManager) getSystemService(CLIPBOARD_SERVICE);
		mNotificationManager = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);

		super.onCreate();
	}

	@Override
	public void onNotificationPosted(StatusBarNotification sbn) {
		mHandler.obtainMessage(0, sbn).sendToTarget();
	}

	@Override
	public void onNotificationRemoved(StatusBarNotification sbn) {
		if (getPackageName().equals(sbn.getPackageName())) {
			CharSequence code = sbn.getNotification().extras.getCharSequence(Notification.EXTRA_TEXT);
			if (code != null) {
				RedPacketNotification.remove(code);
			}
		}
	}
}
