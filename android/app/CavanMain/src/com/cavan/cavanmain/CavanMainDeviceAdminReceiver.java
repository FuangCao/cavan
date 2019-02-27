package com.cavan.cavanmain;

import com.cavan.android.CavanAndroid;

import android.app.Activity;
import android.app.admin.DeviceAdminReceiver;
import android.app.admin.DevicePolicyManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;

public class CavanMainDeviceAdminReceiver extends DeviceAdminReceiver {

	@Override
	public void onReceive(Context context, Intent intent) {
		CavanAndroid.dLog("onReceive: " + intent);
		super.onReceive(context, intent);
	}

	public static ComponentName getComponentName(Context context) {
		return new ComponentName(context, CavanMainDeviceAdminReceiver.class);
	}

	public static DevicePolicyManager getDevicePolicyManager(Context context) {
		return (DevicePolicyManager) context.getSystemService(Context.DEVICE_POLICY_SERVICE);
	}

	public static boolean isAdminActive(Activity activity) {
		DevicePolicyManager dpm = getDevicePolicyManager(activity);
		ComponentName name = getComponentName(activity);

		if (dpm.isAdminActive(name)) {
			return true;
		}

		Intent intent = new Intent(DevicePolicyManager.ACTION_ADD_DEVICE_ADMIN);
		intent.putExtra(DevicePolicyManager.EXTRA_DEVICE_ADMIN, name);
		activity.startActivity(intent);

		return false;
	}

	public static void lockScreen(Context context) {
		getDevicePolicyManager(context).lockNow();
	}
}
