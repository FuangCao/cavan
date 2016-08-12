package com.cavan.cavanmain;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.media.Ringtone;
import android.media.RingtoneManager;
import android.net.Uri;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.preference.PreferenceManager;
import android.service.notification.NotificationListenerService;
import android.service.notification.StatusBarNotification;

import com.cavan.android.CavanAndroid;

public class RedPacketListenerService extends NotificationListenerService {

	public static final int NOTIFY_TEST = -1;

	public static String[] mSoundExtensions = {
		"m4a", "ogg", "wav", "mp3", "ac3", "wma"
	};

	public static final Pattern[] mNumberPatterns = {
		Pattern.compile("支付宝.*红包\\D*(\\d+)"),
		Pattern.compile("支付宝.*口令\\D*(\\d+)"),
		Pattern.compile("红包\\s*[:：]?\\s*(\\d+)"),
		Pattern.compile("口令\\s*[:：]?\\s*(\\d+)"),
		Pattern.compile("(\\d+)\\s*$"),
	};

	public static final Pattern[] mWordPatterns = {
		Pattern.compile("红包\\s*[:：]\\s*(\\S+)\\s*$"),
		Pattern.compile("口令\\s*[:：]\\s*(\\S+)\\s*$"),
		Pattern.compile("(\\b华美\\S{2})\\b"),
		Pattern.compile("口令.*(华美\\S{2})"),
	};

	// ================================================================================

	private ClipboardManager mClipboardManager;
	private NotificationManager mNotificationManager;
	private HashMap<CharSequence, Long> mCodeMap = new HashMap<CharSequence, Long>();
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

			Notification notification = sbn.getNotification();
			CharSequence text = notification.tickerText;

			if (text == null) {
				text = notification.extras.getCharSequence(Notification.EXTRA_TEXT);
				if (text == null) {
					return;
				}
			}

			String content = text.toString();

			CavanAndroid.logD("================================================================================");
			CavanAndroid.logD(content);

			String name;
			String message;
			String[] contents = content.split(":", 2);

			if (contents.length < 2) {
				name = "支付宝红包口令";
				message = content.trim();
			} else {
				name = contents[0].trim();
				message = contents[1].trim();
			}

			for (String code : getRedPacketCode(message)) {
				sendRedPacketNotify(name, code);
			}
		}
	};

	// ================================================================================

	public static boolean isRedPacketNumberCode(String code) {
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

	public static List<String> getRedPacketCode(Pattern[] patterns, String text, List<String> codes) {
		if (codes == null) {
			codes = new ArrayList<String>();
		}

		for (Pattern pattern : patterns) {
			Matcher matcher = pattern.matcher(text);
			if (matcher.find()) {
				do {
					codes.add(matcher.group(1));
				} while (matcher.find());

				break;
			}
		}

		return codes;
	}

	public static List<String> getRedPacketCode(String text) {
		List<String> codes = new ArrayList<String>();

		for (String code : getRedPacketCode(mNumberPatterns, text, null)) {
			if (isRedPacketNumberCode(code)) {
				codes.add(code);
			}
		}

		return getRedPacketCode(mWordPatterns, text, codes);
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

	public static void postRedPacketCode(ClipboardManager clipboard, CharSequence code) {
		ClipData data = ClipData.newPlainText("支付宝红包口令", code);
		CavanAndroid.logE("ClipData = " + data);

		clipboard.setPrimaryClip(data);
	}

	public static boolean postRedPacketCode(Context context, CharSequence code) {
		ClipboardManager clipboard = (ClipboardManager) context.getSystemService(CLIPBOARD_SERVICE);
		if (clipboard == null) {
			return false;
		}

		postRedPacketCode(clipboard, code);

		return true;
	}

	// ================================================================================

	public void postRedPacketCode(CharSequence code) {
		if (mClipboardManager != null) {
			postRedPacketCode(mClipboardManager, code);
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
		CavanAndroid.setSuspendEnable(this, false, 5000);

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
		startAlipayActivity(this);

		if (mNotificationManager != null) {
			Notification.Builder builder = new Notification.Builder(this)
				.setSmallIcon(R.drawable.ic_launcher)
				.setTicker("支付宝红包口令: " + code)
				.setContentTitle(name)
				.setContentText(code);

			Intent intent = new Intent(this, RedPacketBroadcastReceiver.class);
			intent.putExtra("code", code);
			builder.setContentIntent(PendingIntent.getBroadcast(this, (int) (timeNow & 0x7FFFFFFF), intent, PendingIntent.FLAG_UPDATE_CURRENT));

			Uri ringtone = getRingtoneUri();
			if (ringtone == null) {
				builder.setDefaults(Notification.DEFAULT_ALL);
			} else {
				CavanAndroid.logE("ringtone: uri = " + ringtone);

				builder.setDefaults(Notification.DEFAULT_LIGHTS | Notification.DEFAULT_VIBRATE);
				builder.setSound(ringtone);
			}

			mNotificationManager.notify(code, 0, builder.build());
		}
	}

	// ================================================================================

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
				mCodeMap.remove(code);
			}
		}
	}
}