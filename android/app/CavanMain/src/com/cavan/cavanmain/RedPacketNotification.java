package com.cavan.cavanmain;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import android.app.Notification;
import android.app.PendingIntent;
import android.content.Intent;
import android.content.SharedPreferences;
import android.media.Ringtone;
import android.media.RingtoneManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.preference.PreferenceManager;

import com.cavan.accessibility.CavanNotification;
import com.cavan.accessibility.CavanRedPacketAlipay;
import com.cavan.android.CavanAndroid;
import com.cavan.java.RedPacketFinder;

public class RedPacketNotification extends CavanNotificationTable {

	private static final long CODE_OVERTIME = 3600000;
	private static final long REPEAT_CODE_OVERTIME = 20000;

	private static String[] sSoundExtensions = {
		"m4a", "ogg", "wav", "mp3", "ac3", "wma"
	};

	public static HashMap<CharSequence, Long> sCodeTimeMap = new HashMap<CharSequence, Long>();
	public static List<String> sExcludeCodes = new ArrayList<String>();

	private RedPacketListenerService mService;
	private CavanRedPacketAlipay mPacket;
	private boolean mCodeOnly;

	public RedPacketNotification(RedPacketListenerService service, CavanNotification notification) {
		super(notification);
		mService = service;
	}

	public void setCodeOnly(boolean only) {
		mCodeOnly = only;
	}

	public CavanRedPacketAlipay getPacket() {
		return mPacket;
	}

	public void setPacket(CavanRedPacketAlipay packet) {
		mPacket = packet;
	}

	public Bundle getExtras() {
		return mNotification.getExtras();
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

	public String getApplicationName() {
		String packageName = getPackageName();
		if (packageName == null) {
			return null;
		}

		return CavanAndroid.getApplicationLabel(mService, packageName);
	}

	public String getUserDescription() {
		return mNotification.getUserDescription(mService);
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

	public Notification buildRedPacketNotifyAlipay(CavanRedPacketAlipay node) {
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

	public boolean sendRedPacketNotifyAlipay(CavanRedPacketAlipay packet, long time) {
		CavanAndroid.dLog("packet = " + packet);
		if (packet == null || packet.isCompleted()) {
			return false;
		}

		Notification notification = buildRedPacketNotifyAlipay(packet);
		if (notification != null) {
			packet.setUnpackTime(time);
			String code = packet.getCode();
			mService.sendNotification(notification, "支付宝@" + getUserDescription() + ": " + code, null, code, FloatMessageService.AUTO_UNLOCK_RED_PACKET);
		}

		return true;
	}

	public int sendRedPacketNotifyAlipay(RedPacketFinder finder) {
		List<String> codes = finder.getRedPacketCodes();
		int count = codes.size();
		if (count <= 0) {
			return 0;
		}

		long time = System.currentTimeMillis() + mNotification.getCodeDelay();

		for (String code : codes) {
			CavanRedPacketAlipay packet = mNotification.getRedPacketAlipay(code);
			sendRedPacketNotifyAlipay(packet, time);
		}

		return count;
	}

	public boolean sendRedPacketNotifyNormal(String content, String message, String pkg, int level) {
		Notification notification = mNotification.getNotification();
		PendingIntent intent;

		if (notification != null) {
			intent = notification.contentIntent;
		} else {
			intent = null;
		}

		notification = buildNotification(content, intent);
		mService.sendNotification(notification, message, pkg, null, level);

		return true;
	}

	public boolean sendRedPacketNotifyAlipayPredict(RedPacketFinder finder) {
		String code = finder.getPredictCode();
		if (code == null) {
			return false;
		}

		return sendRedPacketNotifyNormal(code, code + "@" + getUserDescription(), null, FloatMessageService.AUTO_UNLOCK_INFOMATION);
	}

	public boolean sendKeyword(RedPacketFinder finder) {
		String keyword = mService.getKeyword(mNotification.getGroupName(), finder);
		if (keyword == null) {
			return false;
		}

		String message = keyword + "@" + getUserDescription() + ": " + mNotification.getContent();
		if (CavanMessageActivity.isKeywordNotifyOnly(mService)) {
			FloatMessageService.showNotify(message);
		} else {
			return sendRedPacketNotifyNormal(keyword, message, null, FloatMessageService.AUTO_UNLOCK_INFOMATION);
		}

		return true;
	}

	public String getRedPacketCodeNormal(RedPacketFinder finder, String user) {
		if (mNotification.addRedPacket(user)) {
			return mNotification.getPacketName();
		}

		return finder.getNormalCode(getPackageName(), CavanMessageActivity.isFuDaiNotifyEnabled(mService));
	}

	public boolean sendRedPacketNotifyNormal(RedPacketFinder finder) {
		if (mCodeOnly) {
			return false;
		}

		String user = getUserDescription();
		String code = getRedPacketCodeNormal(finder, user);
		if (code == null) {
			return false;
		}

		return sendRedPacketNotifyNormal(code + "红包", code + "@" + user, mNotification.getPackageName(), FloatMessageService.AUTO_UNLOCK_RED_PACKET);
	}

	public boolean sendRedPacketNotifyAuto() {
		RedPacketFinder finder = mNotification.getFinder();

		if (sendRedPacketNotifyNormal(finder)) {
			return true;
		}

		if (sendRedPacketNotifyAlipay(finder) > 0) {
			return true;
		}

		if (sendRedPacketNotifyAlipayPredict(finder)) {
			return true;
		}

		if (sendKeyword(finder)) {
			return true;
		}

		return false;
	}
}
