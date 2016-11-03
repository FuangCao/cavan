package com.cavan.cavanmain;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

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
import com.cavan.java.RedPacketFinder;

public class RedPacketNotification extends CavanNotification {

	private static final long OVER_TIME = 3600000;

	private static String[] sSoundExtensions = {
		"m4a", "ogg", "wav", "mp3", "ac3", "wma"
	};

	private static final String[] sTimerPackages = {
		CavanPackageName.CALENDAR,
		CavanPackageName.DESKCLOCK,
	};

	private static final String[] sSavePackages = {
		CavanPackageName.QQ,
		CavanPackageName.MM,
		CavanPackageName.TMALL,
		CavanPackageName.TAOBAO,
		CavanPackageName.ALIPAY,
	};

	public static HashMap<CharSequence, Long> sCodeTimeMap = new HashMap<CharSequence, Long>();
	public static List<String> sExcludeCodes = new ArrayList<String>();

	private boolean mTestOnly;
	private boolean mNeedSave;
	private RedPacketFinder mFinder = new RedPacketFinder();

	private boolean mIsCode;
	private boolean mNetShared;
	private boolean mIsTimedCode;
	private String mDescription;
	private RedPacketListenerService mService;
	private StatusBarNotification mNotification;

	public RedPacketNotification(RedPacketListenerService service, StatusBarNotification sbn, boolean test) {
		super(sbn);

		mTestOnly = test;
		mService = service;
		mNotification = sbn;
		mIsTimedCode = CavanJava.ArrayContains(sTimerPackages, mPackageName);

		if (mTitle != null && mIsTimedCode) {
			mFinder.addLine(mTitle);
		}

		mNeedSave = CavanJava.ArrayContains(sSavePackages, mPackageName);
		if (mNeedSave || mIsTimedCode || mService.getPackageName().equals(getPackageName())) {
			mFinder.split(mContent);
		}
	}

	public RedPacketNotification(RedPacketListenerService service, String user, String content, boolean isCode, boolean shared) {
		super(service.getPackageName(), user, null, null, content);

		mNetShared = shared;
		mService = service;
		mIsCode = isCode;

		if (isCode) {
			mFinder.addLine(content);
		} else {
			mFinder.split(content);
		}
	}

	public RedPacketNotification(RedPacketListenerService service, String pkgName, String content, String desc, boolean hasPrefix) {
		super(pkgName, content, hasPrefix);

		mDescription = desc;
		mService = service;
		mFinder.split(mContent);
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

		if (mDescription != null) {
			return mDescription;
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
					CavanAndroid.dLog("ringtone: title = " + ringtone.getTitle(mService));
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
			CavanAndroid.dLog("ringtone: uri = " + ringtone);

			builder.setDefaults(Notification.DEFAULT_LIGHTS | Notification.DEFAULT_VIBRATE);
			builder.setSound(ringtone);
		}

		return builder.build();
	}

	public Notification buildRedPacketNotifyAlipay(String code) {
		if (sExcludeCodes.indexOf(code) >= 0) {
			CavanAndroid.dLog("exclude code = " + code);
			return null;
		}

		long timeNow = System.currentTimeMillis();
		Long time = sCodeTimeMap.get(code);

		if (time != null && timeNow - time < OVER_TIME) {
			CavanAndroid.dLog("skip time = " + time);
			return null;
		}

		sCodeTimeMap.put(code, timeNow);

		Intent intent = new Intent(mService, RedPacketBroadcastReceiver.class).putExtra("code", code);

		return buildNotification(code, PendingIntent.getBroadcast(mService, mService.createRequestCode(), intent, PendingIntent.FLAG_UPDATE_CURRENT));
	}

	// ================================================================================

	private long getCodeDelay() {
		if (mGroupName != null) {
			if (mGroupName.equals("【VIP】内部福利6群")) {
				return 8000;
			}

			if (mGroupName.equals("【小六04】内部VIP群")) {
				return 10000;
			}
		}

		if (mContent != null) {
			if (mContent.contains("手气王")) {
				return 15000;
			}

			if (mContent.contains("运气王")) {
				return 5000;
			}
		}

		return 0;
	}

	public int sendRedPacketNotifyAlipay() {
		List<String> codes;
		long time = System.currentTimeMillis();

		if (mIsCode) {
			codes = new ArrayList<String>();
			codes.add(mContent);
		} else {
			codes = mFinder.getRedPacketCodes();
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
				RedPacketCode node = RedPacketCode.getInstence(code, true, mTestOnly);
				node.setTime(time);

				if (mNetShared) {
					node.setShared();
				}

				if (mIsTimedCode) {
					node.setRepeatable();
				}

				mService.sendNotification(notification, "支付宝口令@" + getUserDescription() + ": " + code, code);
			}
		}

		return count;
	}

	public boolean sendRedPacketNotifyNormal(String content, String message) {
		PendingIntent intent = mNotification.getNotification().contentIntent;
		Notification notification = buildNotification(content, intent);

		if (intent != null && MainActivity.isAutoOpenAppEnabled(mService) && mService.getCodeCount() == 0) {
			try {
				intent.send();
			} catch (CanceledException e) {
				e.printStackTrace();
			}
		}

		mService.sendNotification(notification, message, null);

		return true;
	}

	public boolean sendRedPacketNotifyAlipayPicture() {
		if (mFinder.isPictureCode()) {
			return sendRedPacketNotifyNormal("支付宝口令图片", "支付宝口令图片@" + getUserDescription());
		}

		return false;
	}

	public boolean sendRedPacketNotifyAlipayPredict() {
		if (mFinder.isPredictCode()) {
			if (mFinder.contains("预告") || mFinder.contains("早知道")) {
				return false;
			}

			return sendRedPacketNotifyNormal("准备抢红包", "准备抢红包@" + getUserDescription());
		}

		return false;
	}

	public String getRedPacketCodeNormal() {
		return mFinder.getNormalCode(getPackageName());
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

		if (sendRedPacketNotifyAlipayPredict()) {
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
