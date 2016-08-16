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
import android.content.ContentResolver;
import android.content.Intent;
import android.content.SharedPreferences;
import android.media.Ringtone;
import android.media.RingtoneManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.preference.PreferenceManager;
import android.service.notification.StatusBarNotification;

import com.cavan.android.CavanAndroid;
import com.cavan.java.CavanString;

public class RedPacketNotification extends CavanNotification {

	public static final long OVER_TIME = 3600000;

	public static String[] sSoundExtensions = {
		"m4a", "ogg", "wav", "mp3", "ac3", "wma"
	};

	public static final Pattern sGroupPattern = Pattern.compile("([^\\(]+)\\((.+)\\)\\s*$");

	public static final Pattern[] sNormalPatterns = {
		Pattern.compile("^\\[(\\w+红包)\\]"),
		Pattern.compile("^【(\\w+红包)】"),
	};

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
		Pattern.compile("\\[图片\\].*支付宝红包"),
		Pattern.compile("\\[图片\\].*支付宝口令"),
		Pattern.compile("\\[图片\\].*支付宝.*红包.*口令"),
		Pattern.compile("\\[图片\\].*支付宝.*口令.*红包"),
		Pattern.compile("\\[图片\\].*红包口令"),
		Pattern.compile("\\[图片\\].*口令红包"),
	};

	public static final Pattern[] sOtherPatterns = {
		/* Pattern.compile("(\\b华美\\S{2})\\b"),
		Pattern.compile("口令.*(华美\\S{2})"), */
	};

	public static final Pattern[] sExcludePatterns = {
		Pattern.compile("https?://\\S*\\d+\\s*$"),
	};

	public static HashMap<CharSequence, Long> sCodeTimeMap = new HashMap<CharSequence, Long>();
	public static List<String> sExcludeCodes = new ArrayList<String>();
	public static List<String> sSavePackages = new ArrayList<String>();

	static {
		sSavePackages.add("com.tencent.mobileqq");
		sSavePackages.add("com.tencent.mm");
		sSavePackages.add("com.tmall.wireless");
		sSavePackages.add("com.taobao.taobao");
		sSavePackages.add("com.eg.android.AlipayGphone");
	}

	private String mJoinedLines;
	private List<String> mLines = new ArrayList<String>();

	private RedPacketListenerService mService;
	private StatusBarNotification mNotification;

	public RedPacketNotification(RedPacketListenerService service, StatusBarNotification sbn) throws Exception {
		super(sbn);

		for (String line : mContent.split("\\s*\n\\s*")) {
			if (isValidLine(line)) {
				mLines.add(line.trim());
			}
		}

		mJoinedLines = CavanString.join(mLines, " ");

		mService = service;
		mNotification = sbn;
	}

	public Notification getNotification() {
		return mNotification.getNotification();
	}

	public Bundle getExtras() {
		return getNotification().extras;
	}

	public CharSequence getExtra(String key) {
		return getExtras().getCharSequence(key);
	}

	public CharSequence getExtraTitle() {
		return getExtra(Notification.EXTRA_TITLE);
	}

	public CharSequence getExtraText() {
		return getExtra(Notification.EXTRA_TEXT);
	}

	public String getPackageName() {
		return mNotification.getPackageName();
	}

	public CharSequence getApplicationName() {
		String pkgName = getPackageName();
		if (pkgName == null) {
			return null;
		}

		return CavanAndroid.getApplicationLabel(mService, pkgName);
	}

	public CharSequence getUserDescription() {
		if (mGroupName != null) {
			return mGroupName;
		}

		if (mUserName != null) {
			return mUserName;
		}

		CharSequence name = getApplicationName();
		if (name != null) {
			return name;
		}

		return "未知用户";
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
		sCodeTimeMap.remove(code);
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
			.setContentTitle(getUserDescription())
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
		if (sExcludeCodes.indexOf(code) >= 0) {
			CavanAndroid.logE("exclude code = " + code);
			return null;
		}

		long timeNow = System.currentTimeMillis();
		Long time = sCodeTimeMap.get(code);

		if (time != null && timeNow - time < OVER_TIME) {
			CavanAndroid.logE("skip time = " + time);
			return null;
		}

		sCodeTimeMap.put(code, timeNow);

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
				mService.sendNotification(notification, code, "支付宝口令@" + getUserDescription() + ": " + code);
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
			Matcher matcher = pattern.matcher(mJoinedLines);
			if (matcher.find()) {
				return sendRedPacketNotifyNormal("支付宝口令图片", "支付宝口令图片@" + getUserDescription());
			}
		}

		return false;
	}

	public boolean sendRedPacketNotifyNormal() {
		for (Pattern pattern : sNormalPatterns) {
			Matcher matcher = pattern.matcher(mJoinedLines);
			if (matcher.find()) {
				String content = matcher.group(1);
				return sendRedPacketNotifyNormal(content, content + "@" + getUserDescription());
			}
		}

		return false;
	}

	public boolean sendRedPacketNotifyAuto() {
		if (sendRedPacketNotifyNormal()) {
			return true;
		}

		if (sendRedPacketNotifyAlipay() > 0) {
			return true;
		}

		if (sendRedPacketNotifyAlipayPicture()) {
			return true;
		}

		return false;
	}

	@Override
	public Uri insert(ContentResolver resolver) {
		if (mPackageName == null || sSavePackages.indexOf(mPackageName) < 0) {
			return null;
		}

		return super.insert(resolver);
	}
}
