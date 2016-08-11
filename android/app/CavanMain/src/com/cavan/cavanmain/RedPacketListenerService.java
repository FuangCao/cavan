package com.cavan.cavanmain;

import java.io.File;
import java.util.HashMap;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Intent;
import android.content.SharedPreferences;
import android.media.Ringtone;
import android.media.RingtoneManager;
import android.net.Uri;
import android.os.Environment;
import android.preference.PreferenceManager;
import android.service.notification.NotificationListenerService;
import android.service.notification.StatusBarNotification;

import com.cavan.android.CavanAndroid;

public class RedPacketListenerService extends NotificationListenerService {

	public static final int NOTIFY_TEST = 0;

	public static String[] mSoundExtensions = {
		"m4a", "ogg", "wav", "mp3", "ac3", "wma"
	};

	public static final Pattern[] mPatterns = {
		Pattern.compile("支付宝.*红包.*\\D*(\\d{8})\\D*"),
		Pattern.compile("支付宝.*口令.*\\D*(\\d{8})\\D*"),
		Pattern.compile("红包\\s*[:：]?\\s*(\\d{8})\\D*"),
		Pattern.compile("口令\\s*[:：]?\\s*(\\d{8})\\D*"),
	};

	private ClipboardManager mClipboardManager;
	private NotificationManager mNotificationManager;
	private HashMap<CharSequence, Long> mCodeMap = new HashMap<CharSequence, Long>();

	public void postRedPacketCode(CharSequence code) {
		if (mClipboardManager != null) {
			ClipData data = ClipData.newPlainText("支付宝红包口令", code);

			CavanAndroid.logE("ClipData = " + data);
			mClipboardManager.setPrimaryClip(data);
		}
	}

	public File getRingtoneFile() {
		File dir = Environment.getExternalStorageDirectory();
		for (String extension : mSoundExtensions) {
			File file = new File(dir, "CavanRedPacket." + extension);
			if (file.isFile()) {
				return file;
			}
		}

		return null;
	}

	public Uri getRingtoneUri() {
		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);
		if (preferences != null) {
			String value = preferences.getString(MainActivity.KEY_RED_PACKET_NOTIFY_RINGTONE, null);
			if (value != null) {
				Uri uri = Uri.parse(value);
				Ringtone ringtone = RingtoneManager.getRingtone(this, uri);
				if (ringtone != null) {
					CavanAndroid.logE("ringtone: title = " + ringtone.getTitle(this));
					return uri;
				}
			}
		}

		File file = getRingtoneFile();
		if (file != null) {
			return Uri.fromFile(file);
		}

		return null;
	}

	public void sendRedPacketNotify(String name, String code) {
		CavanAndroid.logE("支付宝红包口令: " + code);

		long timeNow = System.currentTimeMillis();
		for (CharSequence key : mCodeMap.keySet()) {
			if (timeNow - mCodeMap.get(key) > 3600000) {
				mCodeMap.remove(key);
			}
		}

		Long time = mCodeMap.get(code);
		if (time != null) {
			CavanAndroid.logE("skip time = " + time);
			return;
		}

		mCodeMap.put(code, timeNow);

		postRedPacketCode(code);

		Intent intent = getPackageManager().getLaunchIntentForPackage("com.eg.android.AlipayGphone");
		if (intent != null) {
			intent.putExtra("code", code);
			intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED);
			startActivity(intent);
		}

		if (mNotificationManager != null) {
			Notification.Builder builder = new Notification.Builder(this)
				.setSmallIcon(R.drawable.ic_launcher)
				.setTicker("支付宝红包口令: " + code)
				.setContentTitle(name)
				.setContentText(code)
				.setAutoCancel(true);

			if (intent != null) {
				builder.setContentIntent(PendingIntent.getActivity(this, 0, intent, 0));
			}

			Uri ringtone = getRingtoneUri();
			if (ringtone == null) {
				builder.setDefaults(Notification.DEFAULT_ALL);
			} else {
				CavanAndroid.logE("ringtone: uri = " + ringtone);

				builder.setDefaults(Notification.DEFAULT_LIGHTS | Notification.DEFAULT_VIBRATE);
				builder.setSound(ringtone);
			}

			mNotificationManager.notify((int) (timeNow / 1000), builder.build());
		}
	}

	private boolean isRedPacketCode(String code) {
		if (code.length() != 8) {
			return false;
		}

		for (char c : code.toCharArray()) {
			if (c < '0' || c > '9') {
				return false;
			}
		}

		return true;
	}

	@Override
	public void onCreate() {
		mClipboardManager = (ClipboardManager) getSystemService(CLIPBOARD_SERVICE);
		mNotificationManager = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);

		super.onCreate();
	}

	@Override
	public void onNotificationPosted(StatusBarNotification sbn) {
		String name;
		String code;

		String pkgName = sbn.getPackageName();
		if (getPackageName().equals(pkgName)) {
			if (sbn.getId() != NOTIFY_TEST) {
				return;
			}

			mCodeMap.clear();

			name = null;
			code = sbn.getNotification().tickerText.toString();
		} else {
			Notification notification = sbn.getNotification();
			CharSequence content = notification.tickerText;

			if (content == null) {
				content = notification.extras.getCharSequence(Notification.EXTRA_TEXT);
				if (content == null) {
					return;
				}
			}

			// CavanAndroid.logE(content.toString());

			String[] contents = content.toString().split("\\s*:\\s*", 2);

			if (contents.length < 2) {
				name = "支付宝红包口令";
				code = contents[0].trim();
			} else {
				name = contents[0].trim();
				code = contents[1].trim();
			}
		}

		for (Pattern pattern : mPatterns) {
			Matcher matcher = pattern.matcher(code);
			if (matcher.find()) {
				code = matcher.group(1);
				break;
			}
		}

		if (isRedPacketCode(code)) {
			sendRedPacketNotify(name, code);
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
