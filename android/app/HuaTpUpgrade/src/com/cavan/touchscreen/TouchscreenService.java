package com.cavan.touchscreen;

import android.app.Service;
import android.content.Intent;
import android.os.Build;
import android.os.IBinder;
import android.os.RemoteException;

public class TouchscreenService extends Service {
	public static final int FW_STATE_START = 0;
	public static final int FW_STATE_RETRY = 1;
	public static final int FW_STATE_FAILED = 2;
	public static final int FW_STATE_SUCCESS = 3;
	public static final String FW_STATE_CHANGED_ACTION = "com.cavan.touchscreen.FW_STATE_CHANGED";

	private TouchscreenDevice mDevice = null;

	@Override
	public void onCreate() {
		TouchscreenDevice[] devList = {
			new TouchscreenCY8C242(),
			new TouchscreenFT5216(),
		};

		for (TouchscreenDevice device : devList) {
			if (device.isAttach()) {
				mDevice = device;
				break;
			}
		}

		super.onCreate();
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

		private boolean upgradeFirmwareRetry(String pathname, int retry) {
			if (mDevice == null) {
				return false;
			}

			while (retry-- > 0) {
				sendFirmwareState(FW_STATE_START, false);

				if (mDevice.upgradeFirmware(pathname)) {
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
		public String getDevPath() throws RemoteException {
			if (mDevice == null) {
				return null;
			}

			return mDevice.getDevPath();
		}

		@Override
		public String getDevName() throws RemoteException {
			if (mDevice == null) {
				return null;
			}

			return mDevice.getDevName();
		}

		@Override
		public DeviceID readDevID() throws RemoteException {
			if (mDevice == null) {
				return null;
			}

			return mDevice.readDevID();
		}

		@Override
		public String getFwName() throws RemoteException {
			if (mDevice == null) {
				return null;
			}

			DeviceID devID = mDevice.readDevID();
			if (devID == null) {
				return null;
			}

			return String.format("%s_%s_%s", Build.BOARD, mDevice.getFwName(), devID.getVendorShortName());
		}
	};

	@Override
	public IBinder onBind(Intent intent) {
		return mBinder;
	}
}