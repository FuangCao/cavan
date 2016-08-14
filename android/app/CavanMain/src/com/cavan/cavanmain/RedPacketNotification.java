package com.cavan.cavanmain;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import android.app.Notification;
import android.app.PendingIntent;
import android.app.PendingIntent.CanceledException;
import android.content.Intent;
import android.content.SharedPreferences;
import android.media.Ringtone;
import android.media.RingtoneManager;
import android.net.Uri;
import android.os.Environment;
import android.preference.PreferenceManager;
import android.service.notification.StatusBarNotification;

import com.cavan.android.CavanAndroid;
import com.cavan.java.CavanString;

public class RedPacketNotification {

	public static final long OVER_TIME = 3600000;
	public static String PACKAGE_QQ = "com.tencent.mobileqq";
	public static String PACKAGE_MICRO_MSG = "com.tencent.mm";

	public static String[] sSoundExtensions = {
		"m4a", "ogg", "wav", "mp3", "ac3", "wma"
	};

	public static final Pattern sNormalPattern = Pattern.compile("^\\[(\\w+红包)\\]");

	public static final Pattern[] sDigitPatterns = {
		Pattern.compile("支付宝.*红包\\D*(\\d+)"),
		Pattern.compile("支付宝.*口令\\D*(\\d+)"),
		Pattern.compile("红包口令\\D*(\\d+)"),
		Pattern.compile("口令红包\\D*(\\d+)"),
		Pattern.compile("红包\\s*[:：]?\\s*(\\d+)"),
		Pattern.compile("口令\\s*[:：]?\\s*(\\d+)"),
		Pattern.compile("[:：]\\s*(\\d+)"),
		Pattern.compile("\\b(\\d+)\\s*$"),
	};

	public static final Pattern[] sWordPatterns = {
		Pattern.compile("支付宝.*红包\\s*[:：]\\s*(\\w+)"),
		Pattern.compile("支付宝.*口令\\s*[:：]\\s*(\\w+)"),
		Pattern.compile("红包口令\\s*[:：]\\s*(\\w+)"),
		Pattern.compile("口令红包\\s*[:：]\\s*(\\w+)"),
		Pattern.compile("红包\\s*[:：]\\s*(\\w+)\\s*$"),
		Pattern.compile("口令\\s*[:：]\\s*(\\w+)\\s*$"),
	};

	public static final Pattern[] sPicturePatterns = {
		Pattern.compile("支付宝红包.*\\[图片\\]"),
		Pattern.compile("支付宝口令.*\\[图片\\]"),
		Pattern.compile("口令红包.*\\[图片\\]"),
		Pattern.compile("红包口令.*\\[图片\\]"),
		Pattern.compile("\\[图片\\].*支付宝红包"),
		Pattern.compile("\\[图片\\].*支付宝口令"),
		Pattern.compile("\\[图片\\].*红包口令"),
		Pattern.compile("\\[图片\\].*口令红包"),
	};

	public static final Pattern[] sOtherPatterns = {
		Pattern.compile("(\\b华美\\S{2})\\b"),
		Pattern.compile("口令.*(华美\\S{2})"),
	};

	public static final Pattern[] sExcludePatterns = {
		Pattern.compile("https?://\\S*\\d+\\s*$"),
	};

	public static HashMap<CharSequence, Long> sCodeMap = new HashMap<CharSequence, Long>();
	public static HashMap<String, Boolean> sExcludeCodeMap = new HashMap<String, Boolean>();

	static {
		sExcludeCodeMap.put("华美月饼", true);
	}

	private String mName;
	private String mContent;
	private List<String> mLines = new ArrayList<String>();

	private RedPacketListenerService mService;
	private StatusBarNotification mNotification;

	public RedPacketNotification(RedPacketListenerService service, StatusBarNotification sbn) {
		super();

		mService = service;
		mNotification = sbn;
	}

	public String getName() {
		return mName;
	}

	public String getContent() {
		return mContent;
	}

	public static boolean isValidLine(String line) {
		for (Pattern pattern : sExcludePatterns) {
			Matcher matcher = pattern.matcher(line);
			if (matcher.find()) {
				return false;
			}
		}

		return true;
	}

	public boolean parse() {
		Notification notification = mNotification.getNotification();
		CharSequence text = notification.tickerText;

		if (text == null) {
			text = notification.extras.getCharSequence(Notification.EXTRA_TEXT);
			if (text == null) {
				return false;
			}
		}

		String content = text.toString();

		CavanAndroid.logD("================================================================================");
		CavanAndroid.logD(content);

		String[] contents = content.split(":", 2);

		if (contents.length < 2) {
			mName = "红包";
			content = content.trim();
		} else {
			mName = contents[0].trim();
			content = contents[1].trim();
		}

		mContent = content.replaceAll("\\s*\n\\s*", " ");

		for (String line : content.split("\n")) {
			if (isValidLine(line)) {
				mLines.add(line.trim());
			}
		}

		return true;
	}

	public File getRingtoneFile() {
		File dir = Environment.getExternalStorageDirectory();
		for (String extension : sSoundExtensions) {
			File file = new File(dir, "CavanRedPacket." + extension);
			if (file.isFile()) {
				return file;
			}
		}

		return null;
	}

	public Uri getRingtoneUri() {
		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(mService);
		if (preferences != null) {
			String value = preferences.getString(MainActivity.KEY_RED_PACKET_NOTIFY_RINGTONE, null);
			if (value != null) {
				Uri uri = Uri.parse(value);
				Ringtone ringtone = RingtoneManager.getRingtone(mService, uri);
				if (ringtone != null) {
					CavanAndroid.logE("ringtone: title = " + ringtone.getTitle(mService));
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

	// ================================================================================

	public static void removeCode(CharSequence code) {
		sCodeMap.remove(code);
	}

	public static boolean isRedPacketDigitCode(String code) {
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

	public static boolean isRedPacketWordCode(String code) {
		int found = 0;
		int length = code.length();

		for (int i = 0; i < length; i++) {
			if (CavanString.isChineseChar(code.charAt(i))) {
				found++;
			}
		}

		if (found == 0) {
			return false;
		}

		length += found;

		if (length < 6 || length > 20) {
			return false;
		}

		return true;
	}

	public List<String> getRedPacketCodes(Pattern[] patterns, List<String> codes) {
		for (String line : mLines) {
			for (Pattern pattern : patterns) {
				Matcher matcher = pattern.matcher(line);

				while (matcher.find()) {
					codes.add(matcher.group(1));
				}
			}
		}

		return codes;
	}

	public List<String> getRedPacketCodes(Pattern[] patterns) {
		return getRedPacketCodes(patterns, new ArrayList<String>());
	}

	public List<String> getRedPacketCodes() {
		List<String> codes = new ArrayList<String>();

		for (String code : getRedPacketCodes(sDigitPatterns)) {
			if (isRedPacketDigitCode(code)) {
				codes.add(code);
			}
		}

		for (String code : getRedPacketCodes(sWordPatterns)) {
			if (isRedPacketWordCode(code)) {
				codes.add(code);
			}
		}

		return getRedPacketCodes(sOtherPatterns, codes);
	}

	public Notification buildNotification(CharSequence content, PendingIntent intent) {
		CavanAndroid.setSuspendEnable(mService, false, 5000);

		Notification.Builder builder = new Notification.Builder(mService)
			.setSmallIcon(R.drawable.ic_launcher)
			.setContentTitle(mName)
			.setContentText(content)
			.setContentIntent(intent);

		Uri ringtone = getRingtoneUri();
		if (ringtone == null) {
			builder.setDefaults(Notification.DEFAULT_ALL);
		} else {
			CavanAndroid.logE("ringtone: uri = " + ringtone);

			builder.setDefaults(Notification.DEFAULT_LIGHTS | Notification.DEFAULT_VIBRATE);
			builder.setSound(ringtone);
		}

		return builder.build();
	}

	public Notification buildRedPacketNotifyAlipay(String code) {
		if (sExcludeCodeMap.containsKey(code)) {
			CavanAndroid.logE("exclude code = " + code);
			return null;
		}

		long timeNow = System.currentTimeMillis();
		Long time = sCodeMap.get(code);

		if (time != null && timeNow - time < OVER_TIME) {
			CavanAndroid.logE("skip time = " + time);
			return null;
		}

		sCodeMap.put(code, timeNow);

		Intent intent = new Intent(mService, RedPacketBroadcastReceiver.class).putExtra("code", code);

		return buildNotification(code, PendingIntent.getBroadcast(mService, mService.createRequestCode(), intent, PendingIntent.FLAG_UPDATE_CURRENT));
	}

	// ================================================================================

	public int sendRedPacketNotifyAlipay() {
		List<String> codes = getRedPacketCodes();

		for (String code : codes) {
			Notification notification = buildRedPacketNotifyAlipay(code);
			if (notification != null) {
				mService.startAlipayActivity();
				mService.postRedPacketCode(code);
				mService.sendNotification(notification, code, mName + "@支付宝口令: " + code);
			}
		}

		return codes.size();
	}

	public boolean sendRedPacketNotifyNormal(String content, String message) {
		PendingIntent intent = mNotification.getNotification().contentIntent;
		Notification notification = buildNotification(content, intent);

		if (intent != null) {
			try {
				intent.send();
			} catch (CanceledException e) {
				e.printStackTrace();
			}
		}

		mService.sendNotification(notification, null, message);

		return true;
	}

	public boolean sendRedPacketNotifyAlipayPicture() {
		for (Pattern pattern : sPicturePatterns) {
			Matcher matcher = pattern.matcher(mContent);
			if (matcher.find()) {
				return sendRedPacketNotifyNormal("支付宝口令图片", mName + "@支付宝口令图片");
			}
		}

		return false;
	}

	public boolean sendRedPacketNotifyNormal() {
		Matcher matcher = sNormalPattern.matcher(mContent);
		if (matcher.find()) {
			String content = matcher.group(1);
			return sendRedPacketNotifyNormal(content, mName + "@" + content);
		}

		return false;
	}

	public boolean sendRedPacketNotifyAuto() {
		if (parse()) {
			if (sendRedPacketNotifyNormal()) {
				return true;
			}

			if (sendRedPacketNotifyAlipay() > 0) {
				return true;
			}

			if (sendRedPacketNotifyAlipayPicture()) {
				return true;
			}
		}

		return false;
	}
}