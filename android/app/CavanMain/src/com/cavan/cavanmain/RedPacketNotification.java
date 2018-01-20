package com.cavan.cavanmain;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
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
import com.cavan.java.RedPacketFinder;

public class RedPacketNotification extends CavanNotification {

	private static final long CODE_OVERTIME = 3600000;
	private static final long REPEAT_CODE_OVERTIME = 20000;

	private static final HashSet<CharSequence> sExcludeUsersQQ = new HashSet<CharSequence>();
	private static final HashSet<CharSequence> sExcludeUsersMM = new HashSet<CharSequence>();
	private static final HashSet<CharSequence> sTimerPackages = new HashSet<CharSequence>();
	private static final HashSet<CharSequence> sSavePackages = new HashSet<CharSequence>();

	static {
		sExcludeUsersQQ.add("QQ钱包");
		sExcludeUsersMM.add("微信游戏");

		sTimerPackages.add(CavanPackageName.CALENDAR);
		sTimerPackages.add(CavanPackageName.DESKCLOCK);

		sSavePackages.add(CavanPackageName.QQ);
		sSavePackages.add(CavanPackageName.MM);
		sSavePackages.add(CavanPackageName.TMALL);
		sSavePackages.add(CavanPackageName.TAOBAO);
		sSavePackages.add(CavanPackageName.ALIPAY);
	}

	private static String[] sSoundExtensions = {
		"m4a", "ogg", "wav", "mp3", "ac3", "wma"
	};

	public static HashMap<CharSequence, Long> sCodeTimeMap = new HashMap<CharSequence, Long>();
	public static List<String> sExcludeCodes = new ArrayList<String>();

	private boolean mTestOnly;
	private boolean mNeedSave;
	private RedPacketFinder mFinder = new RedPacketFinder();

	private int mPriority;
	private boolean mIsCode;
	private boolean mNetShared;
	private boolean mCodeOnly;
	private boolean mIsTimedCode;
	private String mDescription;
	private RedPacketListenerService mService;
	private StatusBarNotification mNotification;

	public RedPacketNotification(RedPacketListenerService service, StatusBarNotification sbn, boolean test) {
		super(sbn);

		mTestOnly = test;
		mService = service;
		mNotification = sbn;
		mIsTimedCode = sTimerPackages.contains(mPackageName);

		if (mTitle != null && mIsTimedCode) {
			mFinder.addLine(mTitle);
		}

		mNeedSave = sSavePackages.contains(mPackageName);
		if (mNeedSave || mIsTimedCode || mService.getPackageName().equals(getPackageName())) {
			mFinder.split(mContent);
		}
	}

	public RedPacketNotification(RedPacketListenerService service, String user, String content, boolean isCode) {
		super(service.getPackageName(), user, null, null, content);

		mService = service;
		mIsCode = isCode;

		if (isCode) {
			mFinder.addLine(content);
		} else {
			mFinder.split(content);
		}
	}

	public RedPacketNotification(RedPacketListenerService service, String packageName, String content, String desc, boolean hasPrefix) {
		super(packageName, content, hasPrefix);

		mDescription = desc;
		mService = service;
		mFinder.split(mContent);
	}

	public void setPriority(int priority) {
		mPriority = priority;
	}

	public void setNetShared(boolean shared) {
		mNetShared = shared;
	}

	public void setCodeOnly(boolean only) {
		mCodeOnly = only;
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

	public String getApplicationName() {
		String packageName = getPackageName();
		if (packageName == null) {
			return null;
		}

		return CavanAndroid.getApplicationLabel(mService, packageName);
	}

	public String getUserDescription() {
		if (mGroupName != null) {
			return mGroupName;
		}

		if (mUserName != null) {
			return mUserName;
		}

		if (mDescription != null) {
			return mDescription;
		}

		String name = getApplicationName();
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
			String value = preferences.getString(CavanMessageActivity.KEY_RED_PACKET_NOTIFY_RINGTONE, null);
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
		CavanAndroid.acquireWakeupLock(mService, 20000);

		Notification.Builder builder = new Notification.Builder(mService)
			.setSmallIcon(R.drawable.ic_launcher)
			.setContentTitle(getUserDescription())
			.setContentText(content)
			.setContentIntent(intent);

		int defaluts = Notification.DEFAULT_LIGHTS;
		int setting = CavanMessageActivity.getNotifySetting(mService);

		if ((setting & 1) != 0) {
			Uri ringtone = getRingtoneUri();
			if (ringtone != null) {
				CavanAndroid.dLog("ringtone: uri = " + ringtone);
				builder.setSound(ringtone);
			} else {
				defaluts |= Notification.DEFAULT_SOUND;
			}
		}

		if ((setting & 2) != 0) {
			defaluts |= Notification.DEFAULT_VIBRATE;
		}

		builder.setDefaults(defaluts);

		return builder.build();
	}

	public Notification buildRedPacketNotifyAlipay(RedPacketCode node) {
		String code = node.getCode();

		if (sExcludeCodes.indexOf(code) >= 0) {
			CavanAndroid.dLog("exclude code = " + code);
			return null;
		}

		long timeNow = System.currentTimeMillis();

		Long time = sCodeTimeMap.get(code);
		if (time != null) {
			long overtime = node.isRepeatable() ? REPEAT_CODE_OVERTIME : CODE_OVERTIME;

			if (timeNow - time < overtime) {
				CavanAndroid.dLog("skip time = " + time);
				return null;
			}
		}

		sCodeTimeMap.put(code, timeNow);

		Intent intent = new Intent(mService, CavanBroadcastReceiver.class).putExtra("code", code);

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

			if (mGroupName.equals("11-VIP客户内部福利群")) {
				return 5000;
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

		boolean needStartAlipay = true;

		for (String code : codes) {
			RedPacketCode node = RedPacketCode.getInstence(code, mPriority, true, mTestOnly, false);
			CavanAndroid.dLog("node = " + node);
			CavanAndroid.dLog("needSkip = " + node.needSkip());
			if (node == null || node.needSkip()) {
				continue;
			}

			if (needStartAlipay && !node.isCompleted()) {
				needStartAlipay = false;
				mService.startAlipayActivity();
			}

			Notification notification = buildRedPacketNotifyAlipay(node);
			if (notification != null) {
				node.setTime(time);

				if (mNetShared) {
					node.setSendEnable(false);
				}

				if (mIsTimedCode) {
					node.setRepeatable();
				}

				mService.sendNotification(notification, "支付宝@" + getUserDescription() + ": " + code, code);
			}
		}

		return count;
	}

	public boolean sendRedPacketNotifyNormal(String content, String message, boolean send) {
		PendingIntent intent;
		Notification notification;

		if (mNotification != null) {
			intent = mNotification.getNotification().contentIntent;

			if (send && intent != null && CavanMessageActivity.isAutoOpenAppEnabled(mService) &&
					(CavanMessageActivity.isAutoOpenAlipayEnabled(mService) == false || mService.getCodePending() == 0)) {
				try {
					intent.send();
				} catch (CanceledException e) {
					e.printStackTrace();
				}
			}
		} else {
			intent = null;
		}

		notification = buildNotification(content, intent);
		mService.sendNotification(notification, message, null);

		return true;
	}

	public boolean sendRedPacketNotifyAlipayPredict() {
		String code = mFinder.getPredictCode();
		if (code != null) {
			return sendRedPacketNotifyNormal(code, code + "@" + getUserDescription(), false);
		}

		return false;
	}

	public boolean sendKeyword() {
		String keyword = mService.getKeyword(mFinder);
		if (keyword != null) {
			String message = getUserDescription() + ": " + keyword;
			if (CavanMessageActivity.isKeywordNotifyOnly(mService)) {
				FloatMessageService.showNotify(message);
			} else {
				return sendRedPacketNotifyNormal(keyword, "关键字@" + message, false);
			}
		}

		return false;
	}

	public String getRedPacketCodeNormal() {
		return mFinder.getNormalCode(getPackageName(), CavanMessageActivity.isFuDaiNotifyEnabled(mService));
	}

	private boolean isMmFilterEnabled(String user) {
		CavanAccessibilityService service = CavanAccessibilityService.instance;
		if (service != null && service.getAccessibilityMM().isWebViewUi()) {
			return true;
		}

		return CavanMessageActivity.isMmFilterEnabled(mService) && sExcludeUsersMM.contains(user);
	}

	public boolean sendRedPacketNotifyNormal() {
		if (mCodeOnly) {
			return false;
		}

		String code = getRedPacketCodeNormal();
		if (code == null) {
			return false;
		}

		boolean send = true;
		String user = getUserDescription();

		if ("QQ".equals(code)) {
			if (CavanMessageActivity.isQqFilterEnabled(mService) && sExcludeUsersQQ.contains(user)) {
				CavanAndroid.dLog("Exclude user: " + user);
				send = false;
			} else {
				CavanAccessibilityQQ qq = CavanAccessibilityQQ.instance;
				if (qq != null) {
					qq.addPacket(user);
				}
			}
		} else if ("微信".equals(code)) {
			if (isMmFilterEnabled(user)) {
				CavanAndroid.dLog("Exclude user: " + user);
				send = false;
			} else {
				CavanAccessibilityMM mm = CavanAccessibilityMM.instance;
				if (mm != null) {
					mm.addPacket(user);
				}
			}
		}

		return sendRedPacketNotifyNormal(code + "红包", code + "@" + user, send);
	}

	public boolean sendRedPacketNotifyAuto() {
		if (sendRedPacketNotifyNormal()) {
			return true;
		}

		if (sendRedPacketNotifyAlipay() > 0) {
			return true;
		}

		if (sendRedPacketNotifyAlipayPredict()) {
			return true;
		}

		if (sendKeyword()) {
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
