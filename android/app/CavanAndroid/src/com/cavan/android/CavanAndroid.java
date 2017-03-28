package com.cavan.android;

import java.io.BufferedReader;
import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.lang.reflect.Field;
import java.util.HashMap;
import java.util.List;

import android.app.Activity;
import android.app.ActivityManager;
import android.app.ActivityManager.RunningAppProcessInfo;
import android.app.KeyguardManager;
import android.app.KeyguardManager.KeyguardLock;
import android.app.Notification;
import android.app.NotificationManager;
import android.content.ClipData;
import android.content.ClipDescription;
import android.content.ClipboardManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiManager.MulticastLock;
import android.os.Build;
import android.os.Environment;
import android.os.Looper;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.storage.StorageManager;
import android.preference.PreferenceManager;
import android.provider.Settings;
import android.util.Log;
import android.view.Gravity;
import android.view.Window;
import android.view.WindowManager.LayoutParams;
import android.view.inputmethod.InputMethodManager;
import android.widget.Toast;

import com.cavan.java.CavanJava;

@SuppressWarnings("deprecation")
public class CavanAndroid {

	public static final int FLAG_NEEDS_MENU_KEY = 0x40000000;

	public static String TAG = "Cavan";
	public static boolean DLOG_ENABLE = true;
	public static boolean WLOG_ENABLE = true;
	public static boolean ELOG_ENABLE = true;
	public static boolean PLOG_ENABLE = true;

	public static final String CLIP_LABEL_DEFAULT = "Cavan";
	public static final String CLIP_LABEL_TEMP = CLIP_LABEL_DEFAULT + "Temp";

	public static final String ENABLED_NOTIFICATION_LISTENERS = "enabled_notification_listeners";

	public static final int EVENT_CLEAR_TOAST = 1;

	private static HashMap<String, Object> mSystemServiceMap = new HashMap<String, Object>();

	private static Toast sToast;
	private static final Object sToastLock = new Object();

	private static WakeLock sWakeLock;
	private static WakeLock sWakeLockWakeup;
	private static KeyguardLock sKeyguardLock;
	private static MulticastLock sMulticastLock;

	public static void eLog(String message) {
		if (ELOG_ENABLE) {
			Log.e(TAG, message);
		}
	}

	public static void eLog(String message, Throwable throwable) {
		if (ELOG_ENABLE) {
			Log.e(TAG, message, throwable);
		}
	}

	public static void wLog(String message) {
		if (WLOG_ENABLE) {
			Log.w(TAG, message);
		}
	}

	public static void wLog(Throwable throwable) {
		if (WLOG_ENABLE) {
			Log.w(TAG, throwable);
		}
	}

	public static void wLog(String message, Throwable throwable) {
		if (WLOG_ENABLE) {
			Log.w(TAG, message, throwable);
		}
	}

	public static void dLog(String message) {
		if (DLOG_ENABLE) {
			Log.d(TAG, message);
		}
	}

	public static void dLogLarge(String message) {
		ByteArrayInputStream stream = new ByteArrayInputStream(message.getBytes());
		BufferedReader reader = new BufferedReader(new InputStreamReader(stream));

		while (true) {
			try {
				String line = reader.readLine();
				if (line == null) {
					break;
				}

				dLog(line);
			} catch (IOException e) {
				e.printStackTrace();
				break;
			}
		}

		try {
			reader.close();
		} catch (IOException e) {
			e.printStackTrace();
		}

		try {
			stream.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	public static void dLog(String message, Throwable throwable) {
		if (DLOG_ENABLE) {
			Log.d(TAG, message, throwable);
		}
	}

	public static void pLog() {
		if (PLOG_ENABLE) {
			eLog(CavanJava.buildPosMessage());
		}
	}

	public static void pLog(String message) {
		if (PLOG_ENABLE) {
			eLog(CavanJava.buildPosMessage(message));
		}
	}

	public static void efLog(String format, Object... args) {
		eLog(String.format(format, args));
	}

	public static void wfLog(String format, Object... args) {
		wLog(String.format(format, args));
	}

	public static void dfLog(String format, Object... args) {
		dLog(String.format(format, args));
	}

	public static void pfLog(String format, Object... args) {
		pLog(String.format(format, args));
	}

	public static void dumpstack(Throwable throwable) {
		wLog(throwable);
	}

	public static void dumpstack() {
		wLog(new Throwable());
	}

	public static void putCachedSystemService(String name, Object service) {
		mSystemServiceMap.put(name, service);
	}

	public static Object getCachedSystemService(Context context, String name) {
		Object service = mSystemServiceMap.get(name);
		if (service != null) {
			return service;
		}

		service = context.getSystemService(name);
		if (service == null) {
			return null;
		}

		putCachedSystemService(name, service);

		return service;
	}

	public static void cancelToast() {
		synchronized (sToastLock) {
			if (sToast != null) {
				sToast.cancel();
				sToast = null;
			}
		}
	}

	public static void showToast(Context context, String text, int duration) {
		dLog(text);

		synchronized (sToastLock) {
			if (sToast == null) {
				sToast = Toast.makeText(context, text, duration);
				sToast.setGravity(Gravity.CENTER, 0, 0);
			} else {
				sToast.setDuration(duration);
				sToast.setText(text);
			}

			sToast.show();
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
				sKeyguardLock = manager.newKeyguardLock(CavanAndroid.class.getCanonicalName());
				if (sKeyguardLock == null) {
					return false;
				}
			}

			sKeyguardLock.disableKeyguard();
		}

		return true;
	}

	public static boolean setLockScreenEnable(Context context, boolean enable) {
		KeyguardManager manager = (KeyguardManager) getCachedSystemService(context, Context.KEYGUARD_SERVICE);
		if (manager == null) {
			return false;
		}

		return setLockScreenEnable(manager, enable);
	}

	public static void releaseWakeLock() {
		CavanAndroid.dLog("releaseWakeLock");

		if (sWakeLock != null && sWakeLock.isHeld()) {
			sWakeLock.release();
		}

		if (sWakeLockWakeup != null && sWakeLockWakeup.isHeld()) {
			sWakeLockWakeup.release();
		}
	}

	public static boolean acquireWakeLock(PowerManager manager, boolean wakeup, long overtime) {
		CavanAndroid.dLog("acquireWakeLock: wakeup = " + wakeup + ", overtime = " + overtime);

		WakeLock lock;

		if (wakeup) {
			if(sWakeLockWakeup == null) {
				sWakeLockWakeup = manager.newWakeLock(PowerManager.ACQUIRE_CAUSES_WAKEUP | PowerManager.FULL_WAKE_LOCK, CavanAndroid.class.getCanonicalName());
				if (sWakeLockWakeup == null) {
					return false;
				}
			}

			lock = sWakeLockWakeup;
		} else {
			if (sWakeLock == null) {
				sWakeLock = manager.newWakeLock(PowerManager.FULL_WAKE_LOCK, CavanAndroid.class.getCanonicalName());
				if (sWakeLock == null) {
					return false;
				}
			}

			lock = sWakeLock;
		}

		if (overtime > 0) {
			lock.acquire(overtime);
		} else {
			lock.acquire();
		}

		return true;
	}

	public static boolean acquireWakeLock(PowerManager manager, boolean wakeup) {
		return acquireWakeLock(manager, wakeup, 0);
	}

	public static boolean acquireWakeLock(PowerManager manager, long overtime) {
		return acquireWakeLock(manager, true, overtime);
	}

	public static boolean acquireWakeLock(PowerManager manager) {
		return acquireWakeLock(manager, true);
	}

	public static boolean acquireWakeLock(Context context, boolean wakeup, long overtime) {
		PowerManager manager = (PowerManager) getCachedSystemService(context, Context.POWER_SERVICE);
		if (manager == null) {
			return false;
		}

		return acquireWakeLock(manager, wakeup, overtime);
	}

	public static boolean acquireWakeLock(Context context, boolean wakeup) {
		return acquireWakeLock(context, wakeup, 0);
	}

	public static boolean acquireWakeLock(Context context, long overtime) {
		return acquireWakeLock(context, true, overtime);
	}

	public static boolean acquireWakeLock(Context context) {
		return acquireWakeLock(context, true);
	}

	public static void postClipboardText(ClipboardManager manager, CharSequence label, CharSequence text) {
		manager.setPrimaryClip(ClipData.newPlainText(label, text));
	}

	public static void postClipboardText(ClipboardManager manager, CharSequence text) {
		postClipboardText(manager, CLIP_LABEL_DEFAULT, text);
	}

	public static void postClipboardTextTemp(ClipboardManager manager, CharSequence text) {
		postClipboardText(manager, CLIP_LABEL_TEMP, text);
	}

	public static boolean postClipboardText(Context context, CharSequence label, CharSequence text) {
		ClipboardManager manager = (ClipboardManager) getCachedSystemService(context, Context.CLIPBOARD_SERVICE);
		if (manager != null) {
			postClipboardText(manager, label, text);
			return true;
		}

		return false;
	}

	public static boolean postClipboardText(Context context, CharSequence text) {
		return postClipboardText(context, CLIP_LABEL_DEFAULT, text);
	}

	public static boolean postClipboardTextTemp(Context context, CharSequence text) {
		return postClipboardText(context, CLIP_LABEL_TEMP, text);
	}

	public static String getClipboardLabel(ClipData clip) {
		ClipDescription desc = clip.getDescription();
		if (desc == null) {
			return null;
		}

		CharSequence label = desc.getLabel();
		if (label == null) {
			return null;
		}

		return label.toString();
	}

	public static boolean sendNotification(Context context, int id, Notification notification) {
		NotificationManager manager = (NotificationManager) getCachedSystemService(context, Context.NOTIFICATION_SERVICE);
		if (manager == null) {
			return false;
		}

		manager.notify(id, notification);

		return true;
	}

	public static String[] getEnabledAccessibilityServices(Context context) {
		String text = Settings.Secure.getString(context.getContentResolver(), Settings.Secure.ENABLED_ACCESSIBILITY_SERVICES);
		if (text == null) {
			return new String[0];
		}

		return text.split(":");
	}

	public static boolean isAccessibilityServiceEnabled(Context context) {
		return Settings.Secure.getInt(context.getContentResolver(), Settings.Secure.ACCESSIBILITY_ENABLED, 0) > 0;
	}

	public static boolean isAccessibilityServiceEnabled(Context context, String service) {
		if (isAccessibilityServiceEnabled(context)) {
			for (String item : getEnabledAccessibilityServices(context)) {
				if (item.equals(service)) {
					return true;
				}
			}
		}

		return false;
	}

	public static boolean isAccessibilityServiceEnabled(Context context, Class<?> cls) {
		String service = context.getPackageName() + "/" + cls.getName();

		return isAccessibilityServiceEnabled(context, service);
	}

	public static String[] getEnabledNotificationListeners(Context context) {
		String text = Settings.Secure.getString(context.getContentResolver(), ENABLED_NOTIFICATION_LISTENERS);
		if (text == null) {
			return new String[0];
		}

		return text.split(":");
	}

	public static boolean isNotificationListenerEnabled(Context context, String service) {
		for (String listener : getEnabledNotificationListeners(context)) {
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

	public static ApplicationInfo getApplicationInfo(Context context, String packageName) {
		try {
			return context.getPackageManager().getApplicationInfo(packageName, 0);
		} catch (NameNotFoundException e) {
			return null;
		}
	}

	public static CharSequence getApplicationLabel(Context context, String packageName) {
		PackageManager manager = context.getPackageManager();

		try {
			return manager.getApplicationLabel(manager.getApplicationInfo(packageName, 0));
		} catch (NameNotFoundException e) {
			return null;
		}
	}

	public static boolean isHuaweiPhone() {
		String id = SystemProperties.getClientIdBase();
		return (id != null && id.contains("huawei"));
	}

	public static String getDefaultInputMethod(Context context) {
		return Settings.Secure.getString(context.getContentResolver(), Settings.Secure.DEFAULT_INPUT_METHOD);
	}

	public static String getPreference(Context context, String key, String defValue) {
		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
		if (preferences == null) {
			return defValue;
		}

		return preferences.getString(key, defValue);
	}

	public static boolean putPreference(Context context, String key, String value) {
		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
		if (preferences == null) {
			return false;
		}

		Editor editor = preferences.edit();

		editor.putString(key, value);

		return editor.commit();
	}

	public static boolean isPreferenceEnabled(Context context, String key) {
		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
		if (preferences == null) {
			return false;
		}

		return preferences.getBoolean(key, false);
	}

	public static ActivityManager getActivityManager(Context context) {
		return (ActivityManager) getCachedSystemService(context, Context.ACTIVITY_SERVICE);
	}

	public static ComponentName getTopActivityInfo(Context context) {
		ActivityManager manager = getActivityManager(context);
		if (manager == null) {
			return null;
		}

		return manager.getRunningTasks(1).get(0).topActivity;
	}

	public static RunningAppProcessInfo getTopAppProcessInfo(Context context) {
		ActivityManager manager = getActivityManager(context);
		if (manager == null) {
			return null;
		}

		List<RunningAppProcessInfo> infos = manager.getRunningAppProcesses();
		if (infos == null) {
			return null;
		}

		Field field;

		try {
			field = RunningAppProcessInfo.class.getDeclaredField("processState");
		} catch (Exception e) {
			return null;
		}

		for (RunningAppProcessInfo info : infos) {
			if (info == null || info.importance != ActivityManager.RunningAppProcessInfo.IMPORTANCE_FOREGROUND) {
				continue;
			}

			try {
				Integer state = field.getInt(info);
				if (state != null && state == 2) {
					return info;
				}
			} catch (Exception e) {
				return null;
			}
		}

		return null;
	}

	public static String getTopActivieyPackageName(Context context) {
		if (Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP) {
			ComponentName info = getTopActivityInfo(context);
			if (info != null) {
				return info.getPackageName();
			}
		} else {
			RunningAppProcessInfo info = getTopAppProcessInfo(context);
			if (info != null && info.pkgList.length > 0) {
				return info.pkgList[0];
			}
		}

		return null;
	}

	public static String getTopActivityClassName(Context context) {
		if (Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP) {
			ComponentName info = getTopActivityInfo(context);
			if (info != null) {
				return info.getClassName();
			}
		} else {
			RunningAppProcessInfo info = getTopAppProcessInfo(context);
			if (info != null) {
				return info.processName;
			}
		}

		return null;
	}

	public static boolean isTopActivity(Context context, String pkgName) {
		return pkgName.equals(getTopActivieyPackageName(context));
	}

	public static MulticastLock getMulticastLock(Context context) {
		if (sMulticastLock != null) {
			return sMulticastLock;
		}

		WifiManager manager = (WifiManager) getCachedSystemService(context, Context.WIFI_SERVICE);
		if (manager == null) {
			return null;
		}

		sMulticastLock = manager.createMulticastLock(TAG);

		return sMulticastLock;
	}

	public static boolean setMulticastEnabled(Context context, boolean enable) {
		MulticastLock lock = getMulticastLock(context);
		if (lock == null) {
			return false;
		}

		if (enable) {
			lock.acquire();
		} else {
			lock.release();
		}

		return true;
	}

	public static boolean showInputMethodPicker(Context context) {
		InputMethodManager manager = (InputMethodManager) getCachedSystemService(context, Context.INPUT_METHOD_SERVICE);
		if (manager == null) {
			return false;
		}

		manager.showInputMethodPicker();

		return true;
	}

	public static boolean setMenuKeyVisibility(LayoutParams params, boolean visibility) {
		try {
			Field field = LayoutParams.class.getField("needsMenuKey");
			field.setInt(params, visibility ? 1 : 2);
		} catch (Exception e) {
			e.printStackTrace();
		}

		return false;
	}

	public static boolean setMenuKeyVisibility(Window window, boolean visibility) {
		LayoutParams params = window.getAttributes();

		if (params != null && setMenuKeyVisibility(params, visibility)) {
			window.setAttributes(params);
			return true;
		}

		window.setFlags(FLAG_NEEDS_MENU_KEY, FLAG_NEEDS_MENU_KEY);

		return false;
	}

	public static boolean inKeyguardRestrictedInputMode(Context context) {
		KeyguardManager manager = (KeyguardManager) getCachedSystemService(context, Context.KEYGUARD_SERVICE);
		if (manager == null) {
			return false;
		}

		return manager.inKeyguardRestrictedInputMode();
	}

	public static NetworkInfo getActiveNetworkInfo(Context context) {
		ConnectivityManager manager = (ConnectivityManager) getCachedSystemService(context, Context.CONNECTIVITY_SERVICE);
		if (manager == null) {
			return null;
		}

		return manager.getActiveNetworkInfo();
	}

	public static boolean isNetworkAvailable(Context context) {
		NetworkInfo info = getActiveNetworkInfo(context);
		if (info == null) {
			return false;
		}

		return info.isAvailable();
	}

	public static void setWindowKeyguardEnable(Window window, boolean enable) {
		int flags = LayoutParams.FLAG_DISMISS_KEYGUARD | LayoutParams.FLAG_SHOW_WHEN_LOCKED;

		if (enable) {
			window.clearFlags(flags);
		} else {
			window.addFlags(flags);
		}
	}

	public static void setActivityKeyguardEnable(Activity activity, boolean enable) {
		setWindowKeyguardEnable(activity.getWindow(), enable);
	}

	public static boolean startActivity(Context context, Intent intent) {
		try {
			intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED);
			context.startActivity(intent);
			return true;
		} catch (Exception e) {
			e.printStackTrace();
		}

		return false;
	}

	public static boolean startActivity(Context context, String pkgName) {
		Intent intent = context.getPackageManager().getLaunchIntentForPackage(pkgName);
		if (intent == null) {
			return false;
		}

		return startActivity(context, intent);
	}

	public static boolean startActivity(Context context, Class<?> cls) {
		return startActivity(context, new Intent(context, cls));
	}

	public static File getExternalStorageFile(String name) {
		return new File(Environment.getExternalStorageDirectory(), name);
	}

	public static String[] getVolumePaths(StorageManager manager) {
		Object object = CavanJava.invokeMethod(manager, "getVolumePaths");
		if (object != null) {
			return (String[]) object;
		}

		return new String[] { Environment.getExternalStorageDirectory().getPath() };
	}

	public static String[] getSupportedAbis() {
		if (Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP) {
			if (Build.CPU_ABI2.isEmpty()) {
				return new String[] { Build.CPU_ABI };
			} else {
				return new String[] { Build.CPU_ABI, Build.CPU_ABI2 };
			}
		}

		return Build.SUPPORTED_ABIS;
	}
}