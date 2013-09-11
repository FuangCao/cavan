package com.cavan.touchscreen;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.util.Map;

import android.content.Context;
import android.content.Intent;
import android.content.IntentSender.SendIntentException;
import android.util.Log;

public class TouchscreenDevice {
	private static final String TAG = "Cavan";

	public static final int FW_STATE_START = 0;
	public static final int FW_STATE_FAILED = 1;
	public static final int FW_STATE_SUCCESS = 2;

	public static final String FW_STATE_CHANGED_ACTION = "com.cavan.touchscreen.FW_STATE_CHANGED";
	private static final String[][] mDevPaths = {
		{"CY8C242", "/dev/HUA-CY8C242", "/sys/devices/platform/sc8810-i2c.2/i2c-2/2-0024/firmware_id"},
		{"FT6306", "/dev/HUA-FT5216", "/sys/devices/platform/sc8810-i2c.2/i2c-2/2-0038/firmware_id"},
	};

	private Context mContext;
	private String[] mCurrenDeviceMap;

	public TouchscreenDevice(Context context) {
		mContext = context;
		mCurrenDeviceMap = detectDevice();
	}

	private String[] detectDevice() {
		for (String[] map : mDevPaths) {
			File file = new File(map[1]);
			if (file.exists()) {
				return map;
			}
		}

		return null;
	}

	public String getDevName() {
		if (mCurrenDeviceMap == null) {
			return null;
		}
		
		return mCurrenDeviceMap[0];
	}

	public String getDevPath() {
		if (mCurrenDeviceMap == null) {
			return null;
		}
		
		return mCurrenDeviceMap[1];
	}
	
	public int readFwID() {
		if (mCurrenDeviceMap == null) {
			return -1;
		}
		
		FileReader reader;
		
		try {
			reader = new FileReader(mCurrenDeviceMap[2]);
		} catch (FileNotFoundException e) {
			e.printStackTrace();
			return -1;
		}

		char[] buff = new char[4];
		int iReadLen;
		try {
			iReadLen = reader.read(buff);
		} catch (IOException e) {
			e.printStackTrace();
			iReadLen = -1;
		}

		try {
			reader.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
		
		if (iReadLen < 0) {
			return iReadLen;
		}

		return Integer.parseInt(new String(buff), 16);
	}

	public boolean upgradeFirmwareBase(String pathname) throws IOException {
		Log.d(TAG, "pathname = " + pathname);
		FileReader reader = new FileReader(pathname);
		FileWriter writer = new FileWriter(mCurrenDeviceMap[1]);

		while (true) {
			char[] buff = new char[1024];
			int iReadLen = reader.read(buff);
			if (iReadLen < 0) {
				break;
			}

			writer.write(buff, 0, iReadLen);
		}

		reader.close();
		writer.close();

		return true;
	}
	
	class UpgradeThread extends Thread {
		private String mPathname;

		public UpgradeThread(String pathname) {
			super();
			mPathname = pathname;
		}

		boolean upgradeRetry(int retry) {
			while (retry-- > 0) {
				try {
					if (upgradeFirmwareBase(mPathname)) {
						return true;
					}
				} catch (IOException e) {
					e.printStackTrace();
				}
			}

			return false;
		}

		public void run() {
			Intent intent = new Intent(FW_STATE_CHANGED_ACTION);
			intent.putExtra("state", FW_STATE_START);
			mContext.sendBroadcast(intent);

			intent = new Intent(FW_STATE_CHANGED_ACTION);
			intent.putExtra("state", upgradeRetry(10) ? FW_STATE_SUCCESS : FW_STATE_FAILED);
			mContext.sendStickyBroadcast(intent);
		}
	}

	public void upgradeFirmware(String pathname) {
		UpgradeThread thread = new UpgradeThread(pathname);
		thread.start();
	}
}