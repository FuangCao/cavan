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
import com.cavan.android.CavanPackageName;
import com.cavan.java.CavanJava;
import com.cavan.java.CavanString;

public class RedPacketNotification extends CavanNotification {

	public static final long OVER_TIME = 3600000;
	public static final String NORMAL_PATTERN = "(\\w+红包)";
	public static final String DIGIT_PATTERN = "([\\d\\s]+)";
	public static final String DIGIT_JOINED_PATTERN = "((?:\\D*\\d)+)";
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
		Pattern.compile("支\\s*付\\s*宝.*红\\s*包\\D*" + DIGIT_PATTERN),
		Pattern.compile("支\\s*付\\s*宝.*口\\s*令\\D*" + DIGIT_PATTERN),
		Pattern.compile("红\\s*包\\s*口\\s*令\\D*" + DIGIT_PATTERN),
		Pattern.compile("口\\s*令\\s*红\\s*包\\D*" + DIGIT_PATTERN),
		Pattern.compile("红\\s*包\\s*[:：]?\\s*" + DIGIT_PATTERN),
		Pattern.compile("口\\s*令\\s*[:：]?\\s*" + DIGIT_PATTERN),
		Pattern.compile("[:：]\\s*" + DIGIT_PATTERN),
		Pattern.compile("\\b" + DIGIT_PATTERN + "\\s*$"),
	};

	public static final Pattern[] sJoinedDigitPatterns = {
		Pattern.compile("支\\s*付\\s*宝.*红\\s*包" + DIGIT_JOINED_PATTERN),
		Pattern.compile("支\\s*付\\s*宝.*口\\s*令" + DIGIT_JOINED_PATTERN),
		Pattern.compile("红\\s*包\\s*口\\s*令" + DIGIT_JOINED_PATTERN),
		Pattern.compile("口\\s*令\\s*红\\s*包" + DIGIT_JOINED_PATTERN),
		Pattern.compile("红\\s*包\\s*[:：]" + DIGIT_JOINED_PATTERN),
		Pattern.compile("口\\s*令\\s*[:：]" + DIGIT_JOINED_PATTERN),
	};

	public static final Pattern[] sWordPatterns = {
		Pattern.compile("支\\s*付\\s*宝.*红\\s*包\\s*[:：]\\s*" + WORD_PATTERN),
		Pattern.compile("支\\s*付\\s*宝.*口\\s*令\\s*[:：]\\s*" + WORD_PATTERN),
		Pattern.compile("红\\s*包\\s*口\\s*令\\s*[:：]\\s*" + WORD_PATTERN),
		Pattern.compile("口\\s*令\\s*红\\s*包\\s*[:：]\\s*" + WORD_PATTERN),
		Pattern.compile("红\\s*包\\s*[:：]\\s*" + WORD_PATTERN + "\\s*$"),
		Pattern.compile("口\\s*令\\s*[:：]\\s*" + WORD_PATTERN + "\\s*$"),
	};

	public static final Pattern[] sPicturePatterns = {
		Pattern.compile("支\\s*付\\s*宝\\s*红\\s*包"),
		Pattern.compile("支\\s*付\\s*宝\\s*口\\s*令"),
		Pattern.compile("红\\s*包\\s*口\\s*令"),
		Pattern.compile("口\\s*令\\s*红\\s*包"),
	};

	public static final Pattern[] sExcludePatterns = {
		Pattern.compile("[a-z]+://\\S+", Pattern.CASE_INSENSITIVE),
		Pattern.compile("(Q\\s*Q|群|手\\s*机|电\\s*话|微\\s*信|码)\\s*[:：]?\\s*\\d+", Pattern.CASE_INSENSITIVE),
	};

	public static final String[] sFindTitlePackages = {
		CavanPackageName.CALENDAR,
		CavanPackageName.DESKCLOCK,
	};

	public static final String[] sSavePackages = {
		CavanPackageName.QQ,
		CavanPackageName.MM,
		CavanPackageName.TMALL,
		CavanPackageName.TAOBAO,
		CavanPackageName.ALIPAY,
	};

	public static HashMap<CharSequence, Long> sCodeTimeMap = new HashMap<CharSequence, Long>();
	public static HashMap<String, String> sPackageCodeMap = new HashMap<String, String>();
	public static List<String> sExcludeCodes = new ArrayList<String>();

	static {
		sPackageCodeMap.put("com.tencent.mobileqq", "QQ红包");
		sPackageCodeMap.put("com.tencent.mm", "微信红包");
	}

	private boolean mTestOnly;
	private boolean mNeedSave;
	private String mJoinedLines;
	private List<String> mLines = new ArrayList<String>();

	private boolean mIsCode;
	private boolean mNetShared;
	private RedPacketListenerService mService;
	private StatusBarNotification mNotification;

	public RedPacketNotification(RedPacketListenerService service, StatusBarNotification sbn, boolean test) {
		super(sbn);

		mTestOnly = test;
		mService = service;
		mNotification = sbn;

		boolean needFindTitle = CavanJava.ArrayContains(sFindTitlePackages, mPackageName);
		if (mTitle != null && needFindTitle) {
			mLines.add(mTitle);
		}

		mNeedSave = CavanJava.ArrayContains(sSavePackages, mPackageName);
		if (mContent != null && (mNeedSave || needFindTitle || mService.getPackageName().equals(getPackageName()))) {
			splitContent();
		}

		joinLines();
	}

	public RedPacketNotification(RedPacketListenerService service, String user, String content, boolean isCode, boolean shared) {
		super(service.getPackageName(), user, null, null, content);

		mNetShared = shared;
		mService = service;
		mIsCode = isCode;

		if (isCode) {
			mLines.add(content);
			mJoinedLines = content;
		} else {
			splitContent();
			joinLines();
		}
	}

	private void splitContent() {
		for (String line : mContent.split("\n")) {
			line = CavanString.strip(line);
			for (Pattern pattern : sExcludePatterns) {
				Matcher matcher = pattern.matcher(line);
				line = matcher.replaceAll(CavanString.EMPTY_STRING);
			}

			mLines.add(line);
		}
	}

	private void joinLines() {
		mJoinedLines = CavanString.join(mLines, " ");
		mLines.add(mJoinedLines);
	}

	public void setNetShared() {
		mNetShared = true;
	}

	public Notification getNotification() {
		if (mNotification == null) {
			return null;
		}

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
		if (mNotification == null) {
			return null;
		}

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

	public List<String> getRedPacketCodes(String line, Pattern[] patterns, List<String> codes, boolean strip) {
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

		return codes;
	}

	public List<String> getRedPacketCodes(String line, Pattern[] patterns, boolean strip) {
		return getRedPacketCodes(line, patterns, new ArrayList<String>(), strip);
	}

	public List<String> getRedPacketCodes(Pattern[] patterns, List<String> codes, boolean strip) {
		for (String line : mLines) {
			getRedPacketCodes(line, patterns, codes, strip);
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

	public void addRedPacketCodes(List<String> codes, String code) {
		for (int end = 8; end <= code.length(); end += 8) {
			addRedPacketCode(codes, code.substring(end - 8, end));
		}
	}

	public String getRedPacketDigitCode(String text) {
		StringBuilder builder = new StringBuilder();
		int length = text.length();

		for (int i = 0; i < length; i++) {
			char c = text.charAt(i);

			if (c == ':' || c == '：') {
				builder.setLength(0);
			} else if (CavanJava.isDigit(c)) {
				if (builder.length() % 8 == 0 && i > 0 && CavanJava.isDigit(text.charAt(i - 1))) {
					return null;
				}

				builder.append(c);
			} else if (CavanString.isChineseChar(c)) {
				break;
			}
		}

		return builder.toString();
	}

	public List<String> getRedPacketCodes() {
		List<String> codes = new ArrayList<String>();

		for (String text : getRedPacketCodes(sDigitPatterns, false)) {
			String code = getRedPacketDigitCode(text);
			if (code != null && code.length() % 8 == 0) {
				addRedPacketCodes(codes, code);
			}
		}

		for (String text : getRedPacketCodes(mJoinedLines, sJoinedDigitPatterns, false)) {
			String code = getRedPacketDigitCode(text);
			if (code != null) {
				addRedPacketCodes(codes, code);
			}
		}

		for (String code : getRedPacketCodes(sWordPatterns, true)) {
			if (isRedPacketWordCode(code)) {
				addRedPacketCode(codes, code);
			}
		}

		return codes;
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

	private long getCodeDelay() {
		if (mContent == null || mContent.indexOf("手气王") < 0) {
			return 0;
		}

		return 15000;
	}

	public int sendRedPacketNotifyAlipay() {
		List<String> codes;
		long time = System.currentTimeMillis();

		if (mIsCode) {
			codes = new ArrayList<String>();
			codes.add(mContent);
		} else {
			codes = getRedPacketCodes();
			time += getCodeDelay();
		}

		int count = codes.size();
		if (count <= 0) {
			return 0;
		}

		mService.startAlipayActivity();
		// mService.postRedPacketCode(code);

		for (String code : codes) {
			Notification notification = buildRedPacketNotifyAlipay(code);
			if (notification != null) {
				mService.sendNotification(notification, "支付宝口令@" + getUserDescription() + ": " + code, new RedPacketCode(code, time, mNetShared), mTestOnly);
			}
		}

		return count;
	}

	public boolean sendRedPacketNotifyNormal(String content, String message) {
		PendingIntent intent = mNotification.getNotification().contentIntent;
		Notification notification = buildNotification(content, intent);

		if (mService.getCodeCount() == 0 && intent != null) {
			try {
				intent.send();
			} catch (CanceledException e) {
				e.printStackTrace();
			}
		}

		mService.sendNotification(notification, message, null, mTestOnly);

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
			if (mContent != null && mContent.startsWith("[" + code + "]")) {
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
