package com.cavan.cavanmain;

import java.util.ArrayList;

import android.app.Notification;
import android.app.NotificationManager;
import android.content.BroadcastReceiver;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.ClipboardManager.OnPrimaryClipChangedListener;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
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

	private static final int MSG_POST_NOTIFICATION = 1;
	private static final int MSG_REMOVE_NOTIFICATION = 2;
	private static final int MSG_RED_PACKET_NOTIFICATION = 3;

	private CharSequence mClipText;
	private ClipboardManager mClipboardManager;
	private NotificationManager mNotificationManager;
	private ArrayList<String> mKeywords = new ArrayList<String>();
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

	private BroadcastReceiver mReceiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {
			String action = intent.getAction();

			CavanAndroid.dLog("action = " + action);

			switch (action) {
			case MainActivity.ACTION_CODE_RECEIVED:
				String[] codes = intent.getStringArrayExtra("codes");
				if (codes == null) {
					String code = intent.getStringExtra("code");

					if (code == null) {
						break;
					}

					codes = new String[] { code };
				}

				String type = intent.getStringExtra("type");
				boolean shared = intent.getBooleanExtra("shared", false);

				for (String code : codes) {
					CavanAndroid.dLog("code = " + code);

					RedPacketNotification notification = new RedPacketNotification(RedPacketListenerService.this, type, code, true, shared);
					notification.setPriority(1);
					mHandler.obtainMessage(MSG_RED_PACKET_NOTIFICATION, notification).sendToTarget();
				}
				break;

			case MainActivity.ACTION_CONTENT_RECEIVED:
				String desc = intent.getStringExtra("desc");
				String content = intent.getStringExtra("content");
				String packageName = intent.getStringExtra("package");
				boolean hasPrefix = intent.getBooleanExtra("hasPrefix", false);
				int priority = intent.getIntExtra("priority", 0);

				if (packageName == null) {
					packageName = getPackageName();
				}

				RedPacketNotification notification = new RedPacketNotification(RedPacketListenerService.this, packageName, content, desc, hasPrefix);
				notification.setPriority(priority);
				mHandler.obtainMessage(MSG_RED_PACKET_NOTIFICATION, notification).sendToTarget();
				break;
			}
		}
	};

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
			if (code != null) {
				notification.extras.putCharSequence(EXTRA_CODE, code);
			}

			notification.extras.putCharSequence(EXTRA_MESSAGE, message);

			mNotificationManager.notify(createNotificationId(), notification);
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
		if (MainActivity.isAutoOpenAlipayEnabled(context)) {
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

		Intent intent = context.getPackageManager().getLaunchIntentForPackage(CavanPackageName.TMALL);
		if (intent == null) {
			return false;
		}

		intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK); //  | Intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED);
		context.startActivity(intent);
		return true;
	}

	private void loadKeywords(SharedPreferences preferences) {
		ArrayList<String> keywords = EditableMultiSelectListPreference.load(preferences, MainActivity.KEY_KEYWORD_NOTIFY);
		if (keywords != null) {
			mKeywords = keywords;
		} else {
			mKeywords.clear();
		}

		CavanAndroid.dLog("mKeywords = " + mKeywords);
	}

	public String getKeyword(RedPacketFinder finder) {
		for (String keyword : mKeywords) {
			if (finder.contains(keyword)) {
				return keyword;
			}
		}

		return null;
	}

	@Override
	public void onCreate() {
		mClipboardManager = (ClipboardManager) getSystemService(CLIPBOARD_SERVICE);
		mClipboardManager.addPrimaryClipChangedListener(this);

		mNotificationManager = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);

		Intent service = FloatMessageService.startService(this);
		bindService(service, mFloatMessageConnection, 0);

		IntentFilter filter = new IntentFilter();
		filter.addAction(MainActivity.ACTION_CODE_RECEIVED);
		filter.addAction(MainActivity.ACTION_CONTENT_RECEIVED);
		registerReceiver(mReceiver, filter);

		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);
		if (preferences != null) {
			loadKeywords(preferences);
			preferences.registerOnSharedPreferenceChangeListener(this);
		}

		super.onCreate();
	}

	@Override
	public void onDestroy() {
		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);
		if (preferences != null) {
			preferences.unregisterOnSharedPreferenceChangeListener(this);
		}

		unregisterReceiver(mReceiver);
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
		if (MainActivity.isListenClipEnabled(this)) {
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
			} else if (CavanString.getLineCount(text) == 1) {
				try {
					if (mFloatMessageService != null) {
						mFloatMessageService.sendTcpCommand(FloatMessageService.NET_CMD_CLIPBOARD + text);
					}
				} catch (RemoteException e) {
					e.printStackTrace();
				}
			}

			CavanAndroid.dLog("clip = " + text);

			RedPacketNotification notification = new RedPacketNotification(this, "剪切板", text.toString(), false, false);
			mHandler.obtainMessage(MSG_RED_PACKET_NOTIFICATION, notification).sendToTarget();
		}
	}

	@Override
	public void onSharedPreferenceChanged(SharedPreferences preferences, String key) {
		if (MainActivity.KEY_KEYWORD_NOTIFY.equals(key)) {
			loadKeywords(preferences);
		}
	}
}
