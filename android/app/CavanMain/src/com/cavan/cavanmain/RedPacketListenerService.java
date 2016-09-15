package com.cavan.cavanmain;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.InetAddress;
import java.net.MulticastSocket;

import android.app.Notification;
import android.app.NotificationManager;
import android.content.ClipData;
import android.content.ClipDescription;
import android.content.ClipboardManager;
import android.content.ClipboardManager.OnPrimaryClipChangedListener;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
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

	public static final int LAN_SHARE_PORT = 9898;
	public static final String LAN_SHARE_ADDR = "224.0.0.1";
	public static final String LAN_SHARE_PREFIX = "RedPacketCode: ";
	public static final String CLIP_LABEL = "CavanRedPacketCode";

	public static final int NOTIFY_TEST = -1;
	public static final String EXTRA_CODE = "cavan.code";
	public static final String EXTRA_MESSAGE = "cavan.message";

	private static final int MSG_POST_NOTIFICATION = 1;
	private static final int MSG_REMOVE_NOTIFICATION = 2;
	private static final int MSG_RED_PACKET_NOTIFICATION = 3;

	private UdpServiceThread mLanShareThread;

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

				if (getPackageName().equals(pkgName)) {
					if (sbn.getId() != NOTIFY_TEST) {
						break;
					}
				}

				RedPacketNotification notification = new RedPacketNotification(RedPacketListenerService.this, sbn);
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

	public void sendNotification(Notification notification, CharSequence message, RedPacketCode code) {
		if (mNotificationManager != null) {
			if (code != null) {
				notification.extras.putCharSequence(EXTRA_CODE, code.getCode());
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

		mLanShareThread = new UdpServiceThread();
		mLanShareThread.start();

		super.onCreate();
	}

	@Override
	public void onDestroy() {
		if (mLanShareThread != null) {
			mLanShareThread.setActive(false);
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
			RedPacketNotification notification = new RedPacketNotification(this, "剪切板", text.toString());
			mHandler.obtainMessage(MSG_RED_PACKET_NOTIFICATION, notification).sendToTarget();
		}
	}

	public class UdpServiceThread extends Thread {

		private boolean mActive;
		private MulticastSocket mSocket;

		public void setActive(boolean enable) {
			if (enable) {
				mActive = true;
			} else {
				mActive = false;

				if (mSocket != null) {
					MulticastSocket socket = mSocket;

					mSocket = null;
					socket.close();
				}
			}
		}

		@Override
		public void run() {
			mActive = true;

			while (mActive) {
				if (mSocket != null) {
					mSocket.close();
				}

				try {
					mSocket = new MulticastSocket(LAN_SHARE_PORT);
					mSocket.joinGroup(InetAddress.getByName(LAN_SHARE_ADDR));
				} catch (IOException e) {
					e.printStackTrace();

					synchronized (this) {
						try {
							wait(2000);
						} catch (InterruptedException e1) {
							e1.printStackTrace();
						}
					}

					continue;
				}

				CavanAndroid.eLog("UdpServiceThread running");

				byte[] bytes = new byte[128];
				DatagramPacket pack = new DatagramPacket(bytes, bytes.length);

				while (mActive) {
					try {
						mSocket.receive(pack);

						if (MainActivity.isLanShareEnabled(RedPacketListenerService.this)) {
							String text = new String(pack.getData(), 0, pack.getLength());

							CavanAndroid.eLog("receive = " + text);

							if (text.startsWith(LAN_SHARE_PREFIX)) {
								RedPacketNotification notification = new RedPacketNotification(RedPacketListenerService.this, "网络分享", text.substring(LAN_SHARE_PREFIX.length()));
								mHandler.obtainMessage(MSG_RED_PACKET_NOTIFICATION, notification).sendToTarget();
							}
						}
					} catch (IOException e) {
						e.printStackTrace();
						break;
					}
				}

				CavanAndroid.eLog("UdpServiceThread stopping");
			}

			mLanShareThread = null;

			if (mSocket != null) {
				mSocket.close();
				mSocket = null;
			}

			mActive = false;
		}
	}
}
