package com.cavan.touchrecorder;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Build;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceManager;
import android.preference.PreferenceScreen;
import android.provider.Settings;
import android.util.Log;
import android.widget.Toast;

import androidx.annotation.Nullable;

import java.util.prefs.Preferences;

public class TouchSettings extends PreferenceFragment {

    public static final String TAG = MainActivity.TAG;

    public static final String KeyCheckPermission = "check_permissions";
    public static final String KeyListenVolumeKey = "listen_volume_key";
    public static final String KeyPlayDelay = "play_delay";

    private static TouchSettings Instance;

    public static TouchSettings getInstance() {
        TouchSettings instance = Instance;
        if (instance != null) {
            return instance;
        }

        instance = new TouchSettings();
        Instance = instance;
        return instance;
    }

    public static boolean IsPreferenceEnabled(Context context, String key) {
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
        if (preferences == null) {
            return false;
        }

        return preferences.getBoolean(key, false);
    }

    public static int PreferenceIntValue(Context context, String key) {
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
        if (preferences == null) {
            return 0;
        }

        String value = preferences.getString(key, null);
        if (value == null) {
            return 0;
        }

        try {
            return Integer.parseInt(value);
        } catch (Exception e) {
            e.printStackTrace();
            return 0;
        }
    }

    public static boolean IsListenVolumeKeyEnabled(Context context) {
        return IsPreferenceEnabled(context, KeyListenVolumeKey);
    }

    public static int PlayDelay(Context context) {
        return PreferenceIntValue(context, KeyPlayDelay);
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.touch_settings);
    }

    public void ShowToast(int resId)
    {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            Toast.makeText(getContext(), resId, Toast.LENGTH_SHORT).show();
        }
    }

    private int CheckPermissions()
    {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M && Settings.canDrawOverlays(getContext()) == false) {
            startActivity(new Intent(Settings.ACTION_MANAGE_OVERLAY_PERMISSION));
            return R.string.please_allow_float_window;
        }

        if (TouchRecorderService.Instance == null) {
            startActivity(new Intent("android.settings.ACCESSIBILITY_SETTINGS"));
            return R.string.please_enable_touch_recoder_service;
        }

        return R.string.permission_ok;
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        String key = preference.getKey();
        Log.e(TAG, "onPreferenceTreeClick: " + key);

        if ("check_permission".equals(key)) {
            int message = CheckPermissions();
            ShowToast(message);
            preference.setSummary(message);
        }

        return super.onPreferenceTreeClick(preferenceScreen, preference);
    }
}
