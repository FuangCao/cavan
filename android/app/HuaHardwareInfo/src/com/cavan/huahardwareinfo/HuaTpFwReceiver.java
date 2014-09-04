package com.cavan.huahardwareinfo;

import java.io.IOException;

import org.xmlpull.v1.XmlPullParserException;

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

		if (fwName != null && fwName.length() > 0) {
			HuaTpUpgradeDialog dialog = new HuaTpUpgradeDialog(context, touchscreenDevice, fwName);
			dialog.show();
		} else {
			HuaTouchscreenVendorInfo vendorInfo = touchscreenDevice.getVendorInfo();
			if (vendorInfo == null) {
				return;
			}

			int version;
			try {
				version = HuaTouchscreenDevice.getFwVersionFromXml(touchscreenDevice.getFwName());
			} catch (XmlPullParserException e) {
				e.printStackTrace();
				return;
			} catch (IOException e) {
				e.printStackTrace();
				return;
			}

			Log.d(TAG, "version = " + version);

			if (version < 0) {
				return;
			}

			if (version > vendorInfo.getFwVersion()) {
				HuaTpUpgradeDialog dialog = new HuaTpUpgradeDialog(context, touchscreenDevice, null);
				dialog.show();
			}
		}
	}
}