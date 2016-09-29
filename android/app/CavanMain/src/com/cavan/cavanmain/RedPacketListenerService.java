package com.cavan.cavanmain;

import android.app.Notification;
import android.app.NotificationManager;
import android.content.BroadcastReceiver;
import android.content.ClipData;
import android.content.ClipDescription;
import android.content.ClipboardManager;
import android.content.ClipboardManager.OnPrimaryClipChangedListener;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.service.notification.NotificationListenerService;
import android.service.notification.StatusBarNotification;

import com.cavan.android.CavanAndroid;
import com.cavan.java.CavanIndexGenerator;

public class RedPacketListenerService extends NotificationListenerService implements OnPrimaryClipChangedListener {

	public static final String CLIP_LABEL = "CavanRedPacketCode";

	public static final int NOTIFY_TEST = -1;
	public static final String EXTRA_CODE = "cavan.code";
	public static final String EXTRA_MESSAGE = "cavan.message";

	private static final int MSG_POST_NOTIFICATION = 1;
	private static final int MSG_REMOVE_NOTIFICATION = 2;
	private static final int MSG_RED_PACKET_NOTIFICATION = 3;

	private CharSequence mClipText;
	private ClipboardManager mClipboardManager;
	private NotificationManager mNotificationManager;
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
				notification.sendRedPacketNotifyAlipay();
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

			CavanAndroid.eLog("action = " + action);

			switch (action) {
			case MainActivity.ACTION_CODE_RECEIVED:
				String type = intent.getStringExtra("type");
				String code = intent.getStringExtra("code");
				boolean shared = intent.getBooleanExtra("shared", false);

				CavanAndroid.eLog("code = " + code);

				RedPacketNotification notification = new RedPacketNotification(RedPacketListenerService.this, type, code, true, shared);
				mHandler.obtainMessage(MSG_RED_PACKET_NOTIFICATION, notification).sendToTarget();
				break;

			case MainActivity.ACTION_CONTENT_RECEIVED:
				String desc = intent.getStringExtra("desc");
				String content = intent.getStringExtra("content");
				String pkgName = intent.getStringExtra("package");
				boolean hasPrefix = intent.getBooleanExtra("hasPrefix", false);

				if (pkgName == null) {
					pkgName = getPackageName();
				}

				notification = new RedPacketNotification(RedPacketListenerService.this, pkgName, content, desc, hasPrefix);
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

	public void sendNotification(Notification notification, CharSequence message, RedPacketCode code, boolean test) {
		if (mNotificationManager != null) {
			if (code != null) {
				notification.extras.putCharSequence(EXTRA_CODE, code.getCode());
			}

			notification.extras.putCharSequence(EXTRA_MESSAGE, message);

			mNotificationManager.notify(createNotificationId(), notification);
		}

		if (mFloatMessageService != null) {
			try {
				mFloatMessageService.addMessage(message, code, test);
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}

	public static boolean postRedPacketCode(ClipboardManager manager, CharSequence code) {
		if (manager == null) {
			return false;
		}

		ClipData data = ClipData.newPlainText(CLIP_LABEL, code);

		CavanAndroid.eLog("ClipData = " + data);
		manager.setPrimaryClip(data);

		return true;
	}

	public static boolean postRedPacketCode(Context context, String code) {
		ClipboardManager manager = (ClipboardManager) context.getSystemService(CLIPBOARD_SERVICE);
		return postRedPacketCode(manager, code);
	}

	public boolean postRedPacketCode(CharSequence code) {
		return postRedPacketCode(mClipboardManager, code);
	}

	public static boolean startAlipayActivity(Context context) {
		if (!MainActivity.isAutoOpenAppEnabled(context)) {
			return false;
		}

		Intent intent = context.getPackageManager().getLaunchIntentForPackage("com.eg.android.AlipayGphone");
		if (intent == null) {
			return false;
		}

		intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED);
		context.startActivity(intent);
		return true;
	}

	public static Intent buildIntent(Context context) {
		return new Intent(context, RedPacketListenerService.class);
	}

	public boolean startAlipayActivity() {
		return startAlipayActivity(this);
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

		super.onCreate();
	}

	@Override
	public void onDestroy() {
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

			ClipDescription desc = clip.getDescription();
			if (desc != null && CLIP_LABEL.equals(desc.getLabel())) {
				return;
			}

			CharSequence text = clip.getItemAt(0).coerceToText(this);
			if (text == null || text.equals(mClipText)) {
				return;
			}

			CavanAndroid.eLog("clip = " + text);

			mClipText = text;
			RedPacketNotification notification = new RedPacketNotification(this, "剪切板", text.toString(), false, false);
			mHandler.obtainMessage(MSG_RED_PACKET_NOTIFICATION, notification).sendToTarget();
		}
	}
}
