package com.cavan.service;

import java.io.IOException;
import java.io.InputStream;

import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.util.Log;

public abstract class CavanService extends Service {
	private static final String TAG = "TcpDdService";

	public static final int SERVICE_STATE_RUNNING = 0;
	public static final int SERVICE_STATE_STOPPED = 1;

	private Process mProcessService;
	private String mCommand;

	protected int mState = SERVICE_STATE_STOPPED;
	protected int mPort;

	class MonitorThread extends Thread {
		private CavanService mService;

		public MonitorThread(CavanService mService) {
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
		CavanService getService() {
			return CavanService.this;
		}
	}

	@Override
	public IBinder onBind(Intent intent) {
		Log.d(TAG, "onBind");
		return new ServiceBinder();
	}

	@Override
	public void onCreate() {
		Log.d(TAG, "onCreate()");
		super.onCreate();
	}

	public int getState() {
		return mState;
	}

	public void stop() {
		if (mProcessService != null) {
			mProcessService.destroy();
		}

		Intent intent = new Intent(getApplicationContext(), CavanService.class);
		stopService(intent);
	}

	public boolean start(String format, int port) {
		mPort = port;
		mCommand = String.format(format, port);

		Intent intent = new Intent(getApplicationContext(), getClass());
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
		Log.d(TAG, "onStartCommand command = " + mCommand);

		try {
			mProcessService = Runtime.getRuntime().exec(mCommand);
		} catch (IOException e) {
			e.printStackTrace();
			return -1;
		}

		new MonitorThread(this).start();

		return super.onStartCommand(intent, flags, startId);
	}

	public boolean setState(int state) {
		if (state == mState) {
			return true;
		}

		mState = state;
		notifyStateChanged();

		return true;
	}

	protected abstract void notifyStateChanged();
}