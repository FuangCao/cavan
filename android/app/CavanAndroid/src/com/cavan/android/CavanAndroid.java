package com.cavan.android;

import android.app.KeyguardManager;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.KeyguardManager.KeyguardLock;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.os.Looper;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.provider.Settings;
import android.util.Log;
import android.widget.Toast;

import com.cavan.java.CavanJava;

@SuppressWarnings("deprecation")
public class CavanAndroid extends CavanJava {

	public static final String TAG = "Cavan";

	public static final String ENABLED_NOTIFICATION_LISTENERS = "enabled_notification_listeners";

	public static final int EVENT_CLEAR_TOAST = 1;

	private static Toast sToast;
	private static final Object sToastLock = new Object();

	private static WakeLock sWakeLock;
	private static PowerManager sPowerManager;

	private static KeyguardLock sKeyguardLock;
	private static KeyguardManager sKeyguardManager;

	private static ClipboardManager sClipboardManager;
	private static NotificationManager sNotificationManager;

	public static void logE(String message) {
		Log.e(TAG, message);
	}

	public static void logE(String message, Throwable throwable) {
		Log.e(TAG, message, throwable);
	}

	public static void logW(String message) {
		Log.w(TAG, message);
	}

	public static void logW(Throwable throwable) {
		Log.w(TAG, throwable);
	}

	public static void logW(String message, Throwable throwable) {
		Log.w(TAG, message, throwable);
	}

	public static void logD(String message) {
		Log.d(TAG, message);
	}

	public static int logD(String message, Throwable throwable) {
		return Log.d(TAG, message, throwable);
	}

	public static void logP() {
		logE(buildPosMessage());
	}

	public static void logP(String message) {
		logE(buildPosMessage(message));
	}

	public static void logfE(String format, Object... args) {
		logE(String.format(format, args));
	}

	public static void logfW(String format, Object... args) {
		logW(String.format(format, args));
	}

	public static void logfD(String format, Object... args) {
		logD(String.format(format, args));
	}

	public static void logfP(String format, Object... args) {
		logP(String.format(format, args));
	}

	public static void dumpstack(Throwable throwable) {
		logW(throwable);
	}

	public static void dumpstack() {
		logW(new Throwable());
	}

	public static void printSep() {
		logD(buildPrintSep());
	}

	public static void cancelToastLocked() {
		if (sToast != null) {
			sToast.cancel();
			sToast = null;
		}
	}

	public static void cancelToast() {
		synchronized (sToastLock) {
			cancelToastLocked();
		}
	}

	public static void showToast(Context context, String text, int duration) {
		logD(text);

		Toast toast = Toast.makeText(context, text, duration);
		synchronized (sToastLock) {
			cancelToastLocked();

			sToast = toast;
			toast.show();
		}
	}

	public static void showToast(Context context, String text) {
		showToast(context, text, Toast.LENGTH_SHORT);
	}

	public static void showToastLong(Context context, String text) {
		showToast(context, text, Toast.LENGTH_LONG);
	}

	public static void showToast(Context context, int resId, int duration) {
		String text = context.getResources().getString(resId);
		if (text != null) {
			showToast(context, text, duration);
		}
	}

	public static void showToast(Context context, int resId) {
		showToast(context, resId, Toast.LENGTH_SHORT);
	}

	public static void showToastLong(Context context, int resId) {
		showToast(context, resId, Toast.LENGTH_LONG);
	}

	public static boolean isMainThread() {
		return Looper.myLooper() == Looper.getMainLooper();
	}

	public static boolean isSubThread() {
		return Looper.myLooper() != Looper.getMainLooper();
	}

	public static boolean setLockScreenEnable(KeyguardManager manager, boolean enable) {
		if (enable) {
			if (sKeyguardLock != null) {
				sKeyguardLock.reenableKeyguard();
			}
		} else {
			if (sKeyguardLock == null) {
				sKeyguardLock = manager.newKeyguardLock(CavanAndroid.class.getName());
				if (sKeyguardLock == null) {
					return false;
				}
			}

			sKeyguardLock.disableKeyguard();
		}

		return true;
	}

	public static boolean setLockScreenEnable(Context context, boolean enable) {
		if (sKeyguardManager == null) {
			sKeyguardManager = (KeyguardManager) context.getSystemService(Context.KEYGUARD_SERVICE);
			if (sKeyguardManager == null) {
				return false;
			}
		}

		return setLockScreenEnable(sKeyguardManager, enable);
	}

	public static boolean setSuspendEnable(PowerManager manager, boolean enable, long timeout) {
		if (enable) {
			if (sWakeLock != null && sWakeLock.isHeld()) {
				sWakeLock.release();
			}
		} else {
			if (sWakeLock == null) {
				sWakeLock = manager.newWakeLock(PowerManager.ACQUIRE_CAUSES_WAKEUP | PowerManager.FULL_WAKE_LOCK, CavanAndroid.class.getName());
				if (sWakeLock == null) {
					return false;
				}
			}

			if (timeout > 0) {
				sWakeLock.acquire(timeout);
			} else {
				sWakeLock.acquire();
			}
		}

		return true;
	}

	public static boolean setSuspendEnable(PowerManager manager, boolean enable) {
		return setSuspendEnable(manager, enable, 0);
	}

	public static boolean setSuspendEnable(Context context, boolean enable, long timeout) {
		if (sPowerManager == null) {
			sPowerManager = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
			if (sPowerManager == null) {
				return false;
			}
		}

		return setSuspendEnable(sPowerManager, enable, timeout);
	}

	public static boolean setSuspendEnable(Context context, boolean enable) {
		return setSuspendEnable(context, enable, 0);
	}

	public static void postClipboardText(ClipboardManager manager, CharSequence label, CharSequence text) {
		manager.setPrimaryClip(ClipData.newPlainText(label, text));
	}

	public static void postClipboardText(ClipboardManager manager, CharSequence text) {
		postClipboardText(manager, "cavan", text);
	}

	public static boolean postClipboardText(Context context, CharSequence label, CharSequence text) {
		if (sClipboardManager == null) {
			sClipboardManager = (ClipboardManager) context.getSystemService(Context.CLIPBOARD_SERVICE);
			if (sClipboardManager == null) {
				return false;
			}
		}

		postClipboardText(sClipboardManager, label, text);

		return true;
	}

	public static boolean postClipboardText(Context context, CharSequence text) {
		return postClipboardText(context, "cavan", text);
	}

	public static boolean sendNotification(Context context, int id, Notification notification) {
		if (sNotificationManager == null) {
			sNotificationManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
			if (sNotificationManager == null) {
				return false;
			}
		}

		sNotificationManager.notify(id, notification);

		return true;
	}

	public static String[] getEnabledNotificationListeners(Context context) {
		String text = Settings.Secure.getString(context.getContentResolver(), ENABLED_NOTIFICATION_LISTENERS);
		if (text == null) {
			return null;
		}

		return text.split(":");
	}

	public static boolean isNotificationListenerEnabled(Context context, String service) {
		String[] listeners = getEnabledNotificationListeners(context);
		if (listeners == null) {
			return false;
		}

		for (String listener : listeners) {
			if (listener.equals(service)) {
				return true;
			}
		}

		return false;
	}

	public static boolean isNotificationListenerEnabled(Context context, Class<?> cls) {
		String service = context.getPackageName() + "/" + cls.getName();

		return isNotificationListenerEnabled(context, service);
	}
}
