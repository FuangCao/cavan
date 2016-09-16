package com.cavan.cavanmain;

import com.cavan.android.CavanAndroid;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceActivity;

public class PermissionSettingsActivity extends PreferenceActivity {

	public static final String ACTION_HSM_PROTECTED_APPS = "huawei.intent.action.HSM_PROTECTED_APPS";
	public static final String ACTION_NOTIFICATIONMANAGER = "huawei.intent.action.NOTIFICATIONMANAGER";

	private static final String KEY_PROTECTED_APPS = "protected_apps";
	private static final String KEY_NOTIFICATION_ACCESS = "notification_access";
	private static final String KEY_NOTIFICATION_MANAGER = "notification_manager";
	private static final String KEY_ACCESSIBILITY_SETTINGS = "accessibility_settings";

	private Preference mPreferenceProtectedApps;
	private Preference mPreferenceNotificationAccess;
	private Preference mPreferenceNotificationManager;
	private Preference mPreferenceAccessibilitySettings;

	@SuppressWarnings("deprecation")
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		addPreferencesFromResource(R.xml.permission_settings);

		mPreferenceProtectedApps = findPreference(KEY_PROTECTED_APPS);
		mPreferenceNotificationAccess = findPreference(KEY_NOTIFICATION_ACCESS);
		mPreferenceNotificationManager = findPreference(KEY_NOTIFICATION_MANAGER);
		mPreferenceAccessibilitySettings = findPreference(KEY_ACCESSIBILITY_SETTINGS);

		mPreferenceNotificationAccess.setIntent(getNotificationAccessIntent());
		mPreferenceAccessibilitySettings.setIntent(getAccessibilitySettingsIntent());

		if (CavanAndroid.isHuaweiPhone()) {
			mPreferenceProtectedApps.setIntent(new Intent(ACTION_HSM_PROTECTED_APPS));
			mPreferenceNotificationManager.setIntent(new Intent(ACTION_NOTIFICATIONMANAGER));
		}
	}

	public static Intent getNotificationAccessIntent() {
		return new Intent("android.settings.ACTION_NOTIFICATION_LISTENER_SETTINGS");
	}

	public static Intent getAccessibilitySettingsIntent() {
		return new Intent("android.settings.ACCESSIBILITY_SETTINGS");
	}

	public static void startNotificationListenerSettingsActivity(Context context) {
		Intent intent = getNotificationAccessIntent();
		context.startActivity(intent);
	}

	public static Intent getIntent(Context context) {
		return new Intent(context, PermissionSettingsActivity.class);
	}
}
