package com.cavan.service;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.util.Log;

public class TcpDdService extends Service {
	private static final String TAG = "TcpDdService";

	public static final int SERVICE_STATE_RUNNING = 0;
	public static final int SERVICE_STATE_STOPPED = 1;
	public static final String ACTION_SERVICE_STATE_CHANGE = "com.cavan.service.STATE_CHANGE";

	private Process mProcessService;
	private File mFileCavanMain;
	private int mState = SERVICE_STATE_STOPPED;
	private int mPort = 8888;

	class MonitorThread extends Thread {
		private TcpDdService mService;

		public MonitorThread(TcpDdService mService) {
			super();
			this.mService = mService;
		}

		@Override
		public void run() {
			mService.setState(SERVICE_STATE_RUNNING);

			InputStream stream = mProcessService.getInputStream();

			try {
				int length;
				byte []buff = new byte[1024];

				while ((length = stream.read(buff)) > 0) {
					Log.d(TAG, new String(buff, 0, length));
				}
			} catch (IOException e) {
				e.printStackTrace();
			} finally {
				try {
					stream.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}

			try {
				mProcessService.waitFor();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}

			mService.setState(SERVICE_STATE_STOPPED);
			mProcessService = null;
		}
	}

	class ServiceBinder extends Binder {
		TcpDdService getService() {
			return TcpDdService.this;
		}
	}

	@Override
	public IBinder onBind(Intent intent) {
		Log.d(TAG, "onBind");
		return new ServiceBinder();
	}

	@Override
	public void onCreate() {
		mFileCavanMain = new File(getFilesDir(), "cavan-main");

		try {
			prepareCavanMain(R.raw.cavan, mFileCavanMain);
		} catch (IOException e) {
			e.printStackTrace();
		}

		super.onCreate();
	}
	public int getState() {
		return mState;
	}

	public void stop() {
		if (mProcessService != null) {
			mProcessService.destroy();
		}

		Intent intent = new Intent(getApplicationContext(), TcpDdService.class);
		stopService(intent);
	}

	public boolean start(int port) {
		mPort = port;

		Intent intent = new Intent(getApplicationContext(), TcpDdService.class);
		startService(intent);

		return true;
	}

	public int getPort() {
		return mPort;
	}

	@Override
	public void onDestroy() {
		Log.d(TAG, "onDestroy");

		if (mProcessService != null) {
			mProcessService.destroy();
		}
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		Log.d(TAG, "onStartCommand");

		try {
			mProcessService = Runtime.getRuntime().exec(mFileCavanMain.getAbsolutePath() + " tcp_dd_server -s 0 -p " + mPort);
		} catch (IOException e) {
			e.printStackTrace();
			return -1;
		}

		new MonitorThread(this).start();

		return super.onStartCommand(intent, flags, startId);
	}

	private void notifyStateChanged() {
		Intent intent = new Intent(ACTION_SERVICE_STATE_CHANGE);
		intent.putExtra("state", mState);
		sendBroadcast(intent);
	}

	public boolean setState(int state) {
		if (state == mState) {
			return true;
		}

		mState = state;
		notifyStateChanged();

		return true;
	}

	private boolean prepareCavanMain(int id, File file) throws IOException {
		if (file.exists()) {
			return true;
		}

		Log.d(TAG, "Extra file " + file.getAbsolutePath());

		InputStream inputStream = getResources().openRawResource(id);

		FileOutputStream outputStream;
		try {
			outputStream = new FileOutputStream(file);
		} catch (FileNotFoundException e) {
			e.printStackTrace();
			inputStream.close();
			return false;
		}

		int length;
		byte []buff = new byte[1024];

		try {
			while ((length = inputStream.read(buff)) > 0) {
				outputStream.write(buff, 0, length);
			}
		} catch (IOException e) {
			e.printStackTrace();
		} finally {
			outputStream.close();
			inputStream.close();
		}

		file.setExecutable(true);

		return true;
	}
}