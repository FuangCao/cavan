package com.cavan.cavanmain;

import java.io.File;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Intent;
import android.net.Uri;
import android.os.Environment;
import android.service.notification.NotificationListenerService;
import android.service.notification.StatusBarNotification;
import android.support.v4.app.NotificationCompat;

import com.cavan.android.CavanAndroid;

public class CavanRedPacketListenerService extends NotificationListenerService {

	public static String[] mSoundExtensions = {
		"m4a", "ogg", "wav", "mp3", "ac3", "wma"
	};

	public static final Pattern mPatternZFB = Pattern.compile("支付宝.*口令\\D*(\\d{8})\\D*");

	private Uri mUriSound;
	private ClipboardManager mClipboardManager;
	private NotificationManager mNotificationManager;

	public void postRedPacketCode(CharSequence code) {
		if (mClipboardManager != null) {
			ClipData data = ClipData.newPlainText("支付宝红包口令", code);

			CavanAndroid.logE("ClipData = " + data);
			mClipboardManager.setPrimaryClip(data);
		}
	}

	public Uri getNotifySoundUri() {
		File dir = Environment.getExternalStorageDirectory();
		for (String extension : mSoundExtensions) {
			File file = new File(dir, "CavanRedPacket." + extension);
			if (file.isFile()) {
				return Uri.fromFile(file);
			}
		}

		return null;
	}

	public void sendRedPacketNotify(CharSequence code) {
		CavanAndroid.logD("支付宝红包口令: " + code);

		postRedPacketCode(code);

		Intent intent = getPackageManager().getLaunchIntentForPackage("com.eg.android.AlipayGphone");
		if (intent != null) {
			intent.putExtra("code", code);
			intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED);
			startActivity(intent);
		}

		if (mNotificationManager != null) {
			NotificationCompat.Builder builder = new NotificationCompat.Builder(this)
				.setSmallIcon(R.drawable.ic_launcher)
				.setTicker("支付宝红包口令: " + code)
				.setContentTitle("支付宝红包口令")
				.setContentText(code)
				.setAutoCancel(true);

			if (intent != null) {
				builder.setContentIntent(PendingIntent.getActivity(this, 0, intent, 0));
			}

			if (mUriSound == null) {
				builder.setDefaults(Notification.DEFAULT_ALL);
			} else {
				builder.setDefaults(Notification.DEFAULT_LIGHTS | Notification.DEFAULT_VIBRATE);
				builder.setSound(mUriSound);
			}

			CavanAndroid.logE("builder = " + builder);

			mNotificationManager.notify(0, builder.build());
		}
	}

	@Override
	public void onCreate() {
		mUriSound = getNotifySoundUri();
		CavanAndroid.logE("mUriSound = " + mUriSound);

		mClipboardManager = (ClipboardManager) getSystemService(CLIPBOARD_SERVICE);
		mNotificationManager = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);

		super.onCreate();
	}

	@Override
	public void onNotificationPosted(StatusBarNotification sbn) {
		String pkgName = sbn.getPackageName();
		if (getPackageName().equals(pkgName)) {
			return;
		}

		Notification notification = sbn.getNotification();
		CharSequence content = notification.tickerText;

		if (content == null) {
			content = notification.extras.getCharSequence(Notification.EXTRA_TEXT);
			if (content == null) {
				return;
			}
		}

		// CavanAndroid.logE(content.toString());

		Matcher matcher = mPatternZFB.matcher(content);
		if (matcher.find()) {
			String code = matcher.group(1);
			sendRedPacketNotify(code);
		}
	}

	@Override
	public void onNotificationRemoved(StatusBarNotification sbn) {
		if (getPackageName().equals(sbn.getPackageName())) {
			CharSequence code = sbn.getNotification().extras.getCharSequence(Notification.EXTRA_TEXT);
			postRedPacketCode(code);
		}
	}
}