package com.cavan.huahardwareinfo;

import android.app.KeyguardManager;
import android.app.KeyguardManager.KeyguardLock;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class HuaTpFwReceiver extends BroadcastReceiver {
	private static final String TAG = "Cavan";

	@Override
	public void onReceive(Context context, Intent intent) {
		String action = intent.getAction();
		String fwName = HuaTouchscreenDevice.getPendingFirmware();
		Log.d(TAG, "action = " + action + ", fwName = " + fwName);

		if (Intent.ACTION_BOOT_COMPLETED.equals(action) && fwName != null && fwName.length() > 0) {
			KeyguardManager keyguardManager  = (KeyguardManager) context.getSystemService(Context.KEYGUARD_SERVICE);
			KeyguardLock lock = keyguardManager.newKeyguardLock(getClass().getName());
			lock.disableKeyguard();
			HuaTpUpgradeDialog dialog = new HuaTpUpgradeDialog(context);
			dialog.show();
		}
	}
}