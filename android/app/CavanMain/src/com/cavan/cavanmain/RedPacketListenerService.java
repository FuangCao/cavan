package com.cavan.cavanmain;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;

import android.app.Notification;
import android.app.NotificationManager;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.ClipboardManager.OnPrimaryClipChangedListener;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.preference.PreferenceManager;
import android.service.notification.NotificationListenerService;
import android.service.notification.StatusBarNotification;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanPackageName;
import com.cavan.java.CavanIndexGenerator;
import com.cavan.java.CavanString;
import com.cavan.java.RedPacketFinder;
import com.cavan.resource.EditableMultiSelectListPreference;

public class RedPacketListenerService extends NotificationListenerService implements OnPrimaryClipChangedListener, OnSharedPreferenceChangeListener {

	public static final int NOTIFY_TEST = -1;
	public static final String EXTRA_CODE = "cavan.code";
	public static final String EXTRA_MESSAGE = "cavan.message";

	private static final long THANKS_LONG_OVERTIME = 3600000;
	private static final long THANKS_SHORT_OVERTIME = 300000;

	private static final int MSG_POST_NOTIFICATION = 1;
	private static final int MSG_REMOVE_NOTIFICATION = 2;
	private static final int MSG_RED_PACKET_NOTIFICATION = 3;
	private static final int MSG_CANCEL_NOTIFYCATION = 4;

	public static RedPacketListenerService instance;

	private CharSequence mClipText;
	private ClipboardManager mClipboardManager;
	private NotificationManager mNotificationManager;
	private HashSet<String> mKeywords = new HashSet<String>();
	private HashMap<String, ThanksNode> mThanks = new HashMap<String, ThanksNode>();
	private CavanIndexGenerator mGeneratorRequestCode = new CavanIndexGenerator();
	private CavanIndexGenerator mGeneratorNotificationId = new CavanIndexGenerator();

	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {

			switch (msg.what) {
			case MSG_POST_NOTIFICATION:
				StatusBarNotification sbn = (StatusBarNotification) msg.obj;
				String pkgName = sbn.getPackageName();

				boolean testOnly;

				if (getPackageName().equals(pkgName)) {
					if (sbn.getId() != NOTIFY_TEST) {
						break;
					}

					testOnly = true;
				} else {
					testOnly = false;
				}

				RedPacketNotification notification = new RedPacketNotification(RedPacketListenerService.this, sbn, testOnly);
				notification.sendRedPacketNotifyAuto();
				notification.insert(getContentResolver());
				break;

			case MSG_REMOVE_NOTIFICATION:
				sbn = (StatusBarNotification) msg.obj;

				mHandler.removeMessages(MSG_CANCEL_NOTIFYCATION, sbn.getId());

				pkgName = sbn.getPackageName();
				if (getPackageName().equals(pkgName)) {
					Bundle extras = sbn.getNotification().extras;
					CharSequence code = extras.getCharSequence(EXTRA_CODE);
					CharSequence message = extras.getCharSequence(EXTRA_MESSAGE);

					if (code != null) {
						RedPacketNotification.removeCode(code);
					}

					if (message != null && mFloatMessageService != null) {
						try {
							mFloatMessageService.removeMessage(message);
						} catch (RemoteException e) {
							e.printStackTrace();
						}
					}
				}
				break;

			case MSG_CANCEL_NOTIFYCATION:
				if (mNotificationManager != null) {
					mNotificationManager.cancel((int) msg.obj);
				}
				break;

			case MSG_RED_PACKET_NOTIFICATION:
				notification = (RedPacketNotification) msg.obj;
				if (notification.sendRedPacketNotifyAlipay() > 0) {
					break;
				}

				notification.sendRedPacketNotifyNormal();
				break;
			}
		}
	};

	private IFloatMessageService mFloatMessageService;
	private ServiceConnection mFloatMessageConnection = new ServiceConnection() {

		@Override
		public void onServiceDisconnected(ComponentName arg0) {
			mFloatMessageService = null;
		}

		@Override
		public void onServiceConnected(ComponentName arg0, IBinder arg1) {
			mFloatMessageService = IFloatMessageService.Stub.asInterface(arg1);
		}
	};

	public void addRedPacketCode(String code, String type, boolean shared) {
		CavanAndroid.dLog("code = " + code);

		RedPacketNotification notification = new RedPacketNotification(RedPacketListenerService.this, type, code, true);
		notification.setNetShared(shared);
		notification.setPriority(1);
		mHandler.obtainMessage(MSG_RED_PACKET_NOTIFICATION, notification).sendToTarget();
	}

	public void addRedPacketCodes(String[] codes, String type, boolean shared) {
		for (String code : codes) {
			addRedPacketCode(code, type, shared);
		}
	}

	public void addRedPacketContent(CharSequence packageName, String content, String desc, boolean hasPrefix, boolean codeOnly, int priority) {
		if (packageName == null) {
			packageName = getPackageName();
		}

		RedPacketNotification notification = new RedPacketNotification(RedPacketListenerService.this, packageName.toString(), content, desc, hasPrefix);
		notification.setCodeOnly(codeOnly);
		notification.setPriority(priority);
		mHandler.obtainMessage(MSG_RED_PACKET_NOTIFICATION, notification).sendToTarget();
	}

	public int createRequestCode() {
		return mGeneratorRequestCode.genIndex();
	}

	public int createNotificationId() {
		return mGeneratorNotificationId.genIndex();
	}

	public int getMessageCount() {
		if (mFloatMessageService != null) {
			try {
				return mFloatMessageService.getMessageCount();
			} catch (RemoteException e) {
				e.printStackTrace();
			}
		}

		return 0;
	}

	public int getCodeCount() {
		if (mFloatMessageService != null) {
			try {
				return mFloatMessageService.getCodeCount();
			} catch (RemoteException e) {
				e.printStackTrace();
			}
		}

		return 0;
	}

	public int getCodePending() {
		if (mFloatMessageService != null) {
			try {
				return mFloatMessageService.getCodePending();
			} catch (RemoteException e) {
				e.printStackTrace();
			}
		}

		return 0;
	}

	public void sendNotification(Notification notification, CharSequence message, String code) {
		if (mNotificationManager != null) {
			int id = createNotificationId();

			if (code != null) {
				notification.extras.putCharSequence(EXTRA_CODE, code);
			}

			notification.extras.putCharSequence(EXTRA_MESSAGE, message);

			mNotificationManager.notify(id, notification);

			int delay = CavanMessageActivity.getNotifyAutoClear(this);
			if (delay > 0) {
				Message msg = mHandler.obtainMessage(MSG_CANCEL_NOTIFYCATION, id);
				mHandler.sendMessageDelayed(msg, delay * 60000);
			}
		}

		if (mFloatMessageService != null) {
			try {
				mFloatMessageService.addMessage(message, code);
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}

	public static boolean postRedPacketCode(Context context, String code) {
		return CavanAndroid.postClipboardText(context, CavanAndroid.CLIP_LABEL_TEMP, code);
	}

	public void postRedPacketCode(CharSequence code) {
		CavanAndroid.postClipboardText(mClipboardManager, CavanAndroid.CLIP_LABEL_TEMP, code);
	}

	public static boolean startAlipayActivity(Context context) {
		if (CavanMessageActivity.isAutoOpenAlipayEnabled(context)) {
			return CavanAndroid.startActivity(context, CavanPackageName.ALIPAY);
		}

		return false;
	}

	public static Intent buildIntent(Context context) {
		return new Intent(context, RedPacketListenerService.class);
	}

	public boolean startAlipayActivity() {
		return startAlipayActivity(this);
	}

	public static boolean postSecretOrder(Context context, String code) {
		CavanAndroid.dLog("postSecretOrder: " + code);

		CavanAndroid.postClipboardText(context, code);

		return CavanAndroid.startActivity(context, CavanPackageName.TMALL);
	}

	private void loadKeywords(SharedPreferences preferences) {
		mKeywords.clear();

		ArrayList<String> keywords = EditableMultiSelectListPreference.load(preferences, CavanMessageActivity.KEY_KEYWORD_NOTIFY);
		if (keywords != null) {
			for (String keyword : keywords) {
				mKeywords.add(keyword);
			}
		}

		mKeywords.add("你收到一个红包");
		mKeywords.add("商家付款入账通知");

		CavanAndroid.dLog("mKeywords = " + mKeywords);
	}

	public String getKeyword(RedPacketFinder finder) {
		for (String keyword : mKeywords) {
			if (finder.contains(keyword)) {
				return keyword;
			}
		}

		int times = CavanMessageActivity.getThanksNotify(this);
		if (times > 0) {
			Iterator<ThanksNode> iterator = mThanks.values().iterator();
			long time = System.currentTimeMillis();

			while (iterator.hasNext()) {
				ThanksNode node = iterator.next();
				long interval = time - node.getTime();

				if (interval > THANKS_SHORT_OVERTIME) {
					if (interval > THANKS_LONG_OVERTIME || node.getCount() < times) {
						iterator.remove();
					}
				}
			}

			String thanks = finder.getThanks();
			if (thanks != null) {
				ThanksNode node = mThanks.get(thanks);
				if (node == null) {
					node = new ThanksNode();
					mThanks.put(thanks, node);
				}

				if (node.increase(time) == times) {
					String message = thanks + " 大水";

					if (CavanMessageActivity.isThanksShareEnabled(this)) {
						try {
							mFloatMessageService.sendTcpCommand(FloatMessageService.NET_CMD_NOTIFY + message);
						} catch (RemoteException e) {
							e.printStackTrace();
						}
					}

					return message;
				}
			}
		} else {
			mThanks.clear();
		}

		return null;
	}

	@Override
	public void onCreate() {
		super.onCreate();

		mClipboardManager = (ClipboardManager) getSystemService(CLIPBOARD_SERVICE);
		mClipboardManager.addPrimaryClipChangedListener(this);

		mNotificationManager = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);

		Intent service = FloatMessageService.startService(this);
		bindService(service, mFloatMessageConnection, 0);

		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);
		if (preferences != null) {
			loadKeywords(preferences);
			preferences.registerOnSharedPreferenceChangeListener(this);
		}

		instance = this;
	}

	@Override
	public void onDestroy() {
		instance = null;

		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);
		if (preferences != null) {
			preferences.unregisterOnSharedPreferenceChangeListener(this);
		}

		unbindService(mFloatMessageConnection);
		super.onDestroy();
	}

	@Override
	public void onNotificationPosted(StatusBarNotification sbn) {
		mHandler.obtainMessage(MSG_POST_NOTIFICATION, sbn).sendToTarget();
	}

	@Override
	public void onNotificationRemoved(StatusBarNotification sbn) {
		mHandler.obtainMessage(MSG_REMOVE_NOTIFICATION, sbn).sendToTarget();
	}

	@Override
	public void onPrimaryClipChanged() {
		if (CavanMessageActivity.isListenClipEnabled(this)) {
			ClipData clip = mClipboardManager.getPrimaryClip();
			if (clip == null || clip.getItemCount() <= 0) {
				return;
			}

			CharSequence text = clip.getItemAt(0).coerceToText(this);
			if (text == null || text.equals(mClipText)) {
				return;
			}

			mClipText = text;

			String label = CavanAndroid.getClipboardLabel(clip);
			if (label != null && label.startsWith(CavanAndroid.CLIP_LABEL_DEFAULT)) {
				if (label.equals(CavanAndroid.CLIP_LABEL_TEMP)) {
					return;
				}
			} else if (CavanMessageActivity.isClipboardShareEnabled(this) && CavanString.getLineCount(text) == 1) {
				FloatEditorDialog dialog = FloatEditorDialog.getInstance(this, text, false, false);
				dialog.show(6000);
			}

			CavanAndroid.dLog("clip = " + text);

			RedPacketNotification notification = new RedPacketNotification(this, "剪切板", text.toString(), false);
			notification.setCodeOnly(true);
			mHandler.obtainMessage(MSG_RED_PACKET_NOTIFICATION, notification).sendToTarget();
		}
	}

	@Override
	public void onSharedPreferenceChanged(SharedPreferences preferences, String key) {
		if (CavanMessageActivity.KEY_KEYWORD_NOTIFY.equals(key)) {
			loadKeywords(preferences);
		}
	}

	public static boolean checkAndOpenSettingsActivity(Context context) {
		if (CavanAndroid.isNotificationListenerEnabled(context, RedPacketListenerService.class)) {
			return true;
		}

		PermissionSettingsActivity.startNotificationListenerSettingsActivity(context);

		return false;
	}
}
