package com.cavan.huahardwareinfo;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class HuaTpFwReceiver extends BroadcastReceiver {
	private static final String TAG = "Cavan";

	@Override
	public void onReceive(Context context, Intent intent) {
		String action = intent.getAction();
		String fwName = HuaTouchscreenDevice.getPendingFirmware(context);
		Log.d(TAG, "action = " + action + ", fwName = " + fwName);

		HuaTouchscreenDevice touchscreenDevice = HuaTouchscreenDevice.getTouchscreenDevice();
		if (touchscreenDevice == null) {
			return;
		}

		boolean recovery;

		if (fwName != null && fwName.length() > 0) {
			recovery = true;
		} else if (HuaTouchscreenDevice.getAutoUpgrade(context) && touchscreenDevice.ifNeedAutoUpgrade()) {
			recovery = false;
			fwName = null;
		} else {
			return;
		}

		HuaTpUpgradeDialog dialog = new HuaTpUpgradeDialog(context, touchscreenDevice, fwName);
		dialog.show();
	}
}