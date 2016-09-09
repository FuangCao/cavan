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
import com.cavan.java.CavanJava;
import com.cavan.java.CavanString;

public class RedPacketNotification extends CavanNotification {

	public static final String PACKAGE_NAME_DESKCLOCK = "com.android.deskclock";
	public static final String PACKAGE_NAME_CALENDAR = "com.android.calendar";
	public static final String PACKAGE_NAME_QQ = "com.tencent.mobileqq";
	public static final String PACKAGE_NAME_MM = "com.tencent.mm";
	public static final String PACKAGE_NAME_TMALL = "com.tmall.wireless";
	public static final String PACKAGE_NAME_TAOBAO = "com.taobao.taobao";
	public static final String PACKAGE_NAME_ALIPAY = "com.eg.android.AlipayGphone";

	public static final long OVER_TIME = 3600000;
	public static final String NORMAL_PATTERN = "(\\w+红包)";
	public static final String DIGIT_PATTERN = "([\\d\\s]+)";
	public static final String WORD_PATTERN = "([\\w\\s]+)";

	public static String[] sSoundExtensions = {
		"m4a", "ogg", "wav", "mp3", "ac3", "wma"
	};

	public static final Pattern sGroupPattern = Pattern.compile("([^\\(]+)\\((.+)\\)\\s*$");

	public static final Pattern[] sNormalPatterns = {
		Pattern.compile("^\\[" + NORMAL_PATTERN + "\\]"),
		Pattern.compile("^【" + NORMAL_PATTERN + "】"),
	};

	public static final Pattern[] sDigitPatterns = {
		Pattern.compile("支付宝.*红包\\D*" + DIGIT_PATTERN),
		Pattern.compile("支付宝.*口令\\D*" + DIGIT_PATTERN),
		Pattern.compile("红包口令\\D*" + DIGIT_PATTERN),
		Pattern.compile("口令红包\\D*" + DIGIT_PATTERN),
		Pattern.compile("红包\\s*[:：]?\\s*" + DIGIT_PATTERN),
		Pattern.compile("口令\\s*[:：]?\\s*" + DIGIT_PATTERN),
		Pattern.compile("[:：]\\s*" + DIGIT_PATTERN),
		Pattern.compile("\\b" + DIGIT_PATTERN + "\\s*$"),
	};

	public static final Pattern[] sWordPatterns = {
		Pattern.compile("支付宝.*红包\\s*[:：]\\s*" + WORD_PATTERN),
		Pattern.compile("支付宝.*口令\\s*[:：]\\s*" + WORD_PATTERN),
		Pattern.compile("红包口令\\s*[:：]\\s*" + WORD_PATTERN),
		Pattern.compile("口令红包\\s*[:：]\\s*" + WORD_PATTERN),
		Pattern.compile("红包\\s*[:：]\\s*" + WORD_PATTERN + "\\s*$"),
		Pattern.compile("口令\\s*[:：]\\s*" + WORD_PATTERN + "\\s*$"),
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
		Pattern.compile("[a-z]+://\\S+", Pattern.CASE_INSENSITIVE),
	};

	public static final String[] sFindTitlePackages = {
		PACKAGE_NAME_CALENDAR,
		PACKAGE_NAME_DESKCLOCK,
	};

	public static final String[] sSavePackages = {
		PACKAGE_NAME_QQ,
		PACKAGE_NAME_MM,
		PACKAGE_NAME_TMALL,
		PACKAGE_NAME_TAOBAO,
		PACKAGE_NAME_ALIPAY,
	};

	public static HashMap<CharSequence, Long> sCodeTimeMap = new HashMap<CharSequence, Long>();
	public static HashMap<String, String> sPackageCodeMap = new HashMap<String, String>();
	public static List<String> sExcludeCodes = new ArrayList<String>();

	static {
		sPackageCodeMap.put("com.tencent.mobileqq", "QQ红包");
		sPackageCodeMap.put("com.tencent.mm", "微信红包");
	}

	private boolean mNeedSave;
	private String mJoinedLines;
	private List<String> mLines = new ArrayList<String>();

	private RedPacketListenerService mService;
	private StatusBarNotification mNotification;

	public RedPacketNotification(RedPacketListenerService service, StatusBarNotification sbn) {
		super(sbn);

		mService = service;
		mNotification = sbn;

		boolean needFindTitle = CavanJava.ArrayContains(sFindTitlePackages, mPackageName);
		if (mTitle != null && needFindTitle) {
			mLines.add(mTitle);
		}

		mNeedSave = CavanJava.ArrayContains(sSavePackages, mPackageName);
		if (mContent != null && (mNeedSave || needFindTitle || mService.getPackageName().equals(getPackageName()))) {
			for (String line : mContent.split("\n")) {
				line = CavanString.strip(line);
				for (Pattern pattern : sExcludePatterns) {
					Matcher matcher = pattern.matcher(line);
					line = matcher.replaceAll(CavanString.EMPTY_STRING);
				}

				mLines.add(line);
			}
		}

		mJoinedLines = CavanString.join(mLines, " ");
		mLines.add(mJoinedLines);
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
					CavanAndroid.eLog("ringtone: title = " + ringtone.getTitle(mService));
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

	public static boolean isRedPacketDigitCode(String text, String code) {
		if (code.length() % 8 != 0) {
			return false;
		}

		int count = 0;
		char[] array = text.toCharArray();

		for (int i = array.length - 1; i > 0; i--) {
			if (CavanJava.isDigit(array[i])) {
				if (++count % 8 == 0) {
					if (CavanJava.isDigit(array[--i])) {
						return false;
					}
				}
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

		/* length += found;

		if (length < 6 || length > 20) {
			return false;
		} */

		return true;
	}

	public List<String> getRedPacketCodes(Pattern[] patterns, List<String> codes, boolean strip) {
		for (String line : mLines) {
			for (Pattern pattern : patterns) {
				Matcher matcher = pattern.matcher(line);

				while (matcher.find()) {
					String code = matcher.group(1);
					if (strip) {
						code = CavanString.deleteSpace(code);
					}

					codes.add(code);
				}
			}
		}

		return codes;
	}

	public List<String> getRedPacketCodes(Pattern[] patterns, boolean strip) {
		return getRedPacketCodes(patterns, new ArrayList<String>(), strip);
	}

	public boolean addRedPacketCode(List<String> codes, String code) {
		for (String a : codes) {
			if (code.indexOf(a) >= 0) {
				return false;
			}
		}

		codes.add(code);

		return true;
	}

	public List<String> getRedPacketCodes() {
		List<String> codes = new ArrayList<String>();

		for (String text : getRedPacketCodes(sDigitPatterns, false)) {
			String code = CavanString.deleteSpace(text);

			if (isRedPacketDigitCode(text, code)) {
				for (int end = code.length(); end >= 8; end -= 8) {
					addRedPacketCode(codes, code.substring(end - 8, end));
				}
			}
		}

		for (String code : getRedPacketCodes(sWordPatterns, true)) {
			if (isRedPacketWordCode(code)) {
				addRedPacketCode(codes, code);
			}
		}

		return getRedPacketCodes(sOtherPatterns, codes, true);
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
			CavanAndroid.eLog("ringtone: uri = " + ringtone);

			builder.setDefaults(Notification.DEFAULT_LIGHTS | Notification.DEFAULT_VIBRATE);
			builder.setSound(ringtone);
		}

		return builder.build();
	}

	public Notification buildRedPacketNotifyAlipay(String code) {
		if (sExcludeCodes.indexOf(code) >= 0) {
			CavanAndroid.eLog("exclude code = " + code);
			return null;
		}

		long timeNow = System.currentTimeMillis();
		Long time = sCodeTimeMap.get(code);

		if (time != null && timeNow - time < OVER_TIME) {
			CavanAndroid.eLog("skip time = " + time);
			return null;
		}

		sCodeTimeMap.put(code, timeNow);

		Intent intent = new Intent(mService, RedPacketBroadcastReceiver.class).putExtra("code", code);

		return buildNotification(code, PendingIntent.getBroadcast(mService, mService.createRequestCode(), intent, PendingIntent.FLAG_UPDATE_CURRENT));
	}

	// ================================================================================

	private int getCodeDelay() {
		if (mContent == null || mContent.indexOf("手气王") < 0) {
			return 0;
		}

		return 15;
	}

	public int sendRedPacketNotifyAlipay() {
		List<String> codes = getRedPacketCodes();

		for (String code : codes) {
			Notification notification = buildRedPacketNotifyAlipay(code);
			if (notification != null) {
				mService.startAlipayActivity();
				mService.postRedPacketCode(code);
				mService.sendNotification(notification, code, "支付宝口令@" + getUserDescription() + ": " + code, getCodeDelay());
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

		mService.sendNotification(notification, null, message, getCodeDelay());

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

	public String getRedPacketCodeNormal() {
		String code = sPackageCodeMap.get(getPackageName());
		if (code != null) {
			if (getContent().startsWith("[" + code + "]")) {
				return code;
			}

			return null;
		}

		for (Pattern pattern : sNormalPatterns) {
			Matcher matcher = pattern.matcher(mJoinedLines);
			if (matcher.find()) {
				String content = matcher.group(1);
				return content;
			}
		}

		return null;
	}

	public boolean sendRedPacketNotifyNormal() {
		String code = getRedPacketCodeNormal();
		if (code == null) {
			return false;
		}

		return sendRedPacketNotifyNormal(code, code + "@" + getUserDescription());
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
		if (mPackageName == null || mContent == null) {
			return null;
		}

		if (mNeedSave) {
			return super.insert(resolver);
		}

		return null;
	}
}
