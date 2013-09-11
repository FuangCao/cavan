package com.cavan.touchscreen;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;

public class TouchscreenService extends Service {
	private static final String TAG = "Cavan";

	public static final int FW_STATE_START = 0;
	public static final int FW_STATE_RETRY = 1;
	public static final int FW_STATE_FAILED = 2;
	public static final int FW_STATE_SUCCESS = 3;
	public static final String FW_STATE_CHANGED_ACTION = "com.cavan.touchscreen.FW_STATE_CHANGED";

	private String[] mCurrenDeviceMap;
	private static final String[][] mDevPaths = {
		{"CY8C242", "/dev/HUA-CY8C242", "/sys/devices/platform/sc8810-i2c.2/i2c-2/2-0024/firmware_id"},
		{"FT6306", "/dev/HUA-FT5216", "/sys/devices/platform/sc8810-i2c.2/i2c-2/2-0038/firmware_id"},
	};

	@Override
	public void onCreate() {
		mCurrenDeviceMap = detectDevice();
		super.onCreate();
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

	ITouchscreenService.Stub mBinder = new ITouchscreenService.Stub() {
		private void sendFirmwareState(int state, boolean sticky) {
			Intent intent = new Intent(FW_STATE_CHANGED_ACTION);
			intent.putExtra("state", state);

			if (sticky) {
				sendStickyBroadcast(intent);
			} else {
				sendBroadcast(intent);
			}
		}

		private boolean upgradeFirmwareBase(String pathname) {
			Log.d(TAG, "pathname = " + pathname);
			FileInputStream inputStream;
			try {
				inputStream = new FileInputStream(pathname);
			} catch (FileNotFoundException e2) {
				e2.printStackTrace();
				return false;
			}

			FileOutputStream outputStream;
			try {
				outputStream = new FileOutputStream(mCurrenDeviceMap[1]);
			} catch (FileNotFoundException e2) {
				e2.printStackTrace();

				try {
					inputStream.close();
				} catch (IOException e) {
					e.printStackTrace();
				}

				return false;
			}

			boolean result;

			try {
				while (true) {
					byte[] buff = new byte[1024];
					int iReadLen = inputStream.read(buff);
					if (iReadLen < 0) {
						break;
					}

					outputStream.write(buff, 0, iReadLen);
				}

				result = true;
			} catch (Exception e) {
				e.printStackTrace();
				result = false;
			} finally {
				try {
					inputStream.close();
				} catch (IOException e1) {
					e1.printStackTrace();
				}

				try {
					outputStream.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}

			return result;
		}

		private boolean upgradeFirmwareRetry(String pathname, int retry) {
			while (retry-- > 0) {
				sendFirmwareState(FW_STATE_START, false);

				if (upgradeFirmwareBase(pathname)) {
					return true;
				}

				sendFirmwareState(FW_STATE_RETRY, false);

				synchronized(this) {
					try {
						wait(2000);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
			}

			return false;
		}

		class UpgradeThread extends Thread {
			private String mPathname;

			public UpgradeThread(String pathname) {
				super();
				mPathname = pathname;
			}

			public void run() {
				if (upgradeFirmwareRetry(mPathname, 5)) {
					sendFirmwareState(FW_STATE_SUCCESS, true);
				} else {
					sendFirmwareState(FW_STATE_FAILED, true);
				}
			}
		}

		@Override
		public void upgradeFirmware(String pathname) throws RemoteException {
			UpgradeThread thread = new UpgradeThread(pathname);
			thread.start();
		}

		@Override
		public int readFwID() throws RemoteException {
			if (mCurrenDeviceMap == null) {
				return -1;
			}

			FileInputStream stream;

			try {
				stream = new FileInputStream(mCurrenDeviceMap[2]);
			} catch (FileNotFoundException e) {
				e.printStackTrace();
				return 0;
			}

			int iFirmwareID;

			try {
				byte[] buff = new byte[4];
				stream.read(buff);
				iFirmwareID = Integer.parseInt(new String(buff), 16);
			} catch (IOException e) {
				e.printStackTrace();
				iFirmwareID = 0;
			} finally {
				try {
					stream.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}

			return iFirmwareID;
		}

		@Override
		public String getDevPath() throws RemoteException {
			if (mCurrenDeviceMap == null) {
				return null;
			}

			return mCurrenDeviceMap[1];
		}

		@Override
		public String getDevName() throws RemoteException {
			if (mCurrenDeviceMap == null) {
				return null;
			}

			return mCurrenDeviceMap[0];
		}

		@Override
		public String getVendorName(int id) throws RemoteException {
			return null;
		}
	};

	@Override
	public IBinder onBind(Intent intent) {
		return mBinder;
	}
}