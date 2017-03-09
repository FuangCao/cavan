package com.cavan.cavanjni;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.RemoteException;

import com.cavan.android.CavanAndroid;

public abstract class CavanService extends Service {

	protected int mPort;
	protected boolean mState;
	protected boolean mEnabled;

	class MyThread extends Thread {

		@Override
		public synchronized void start() {
			mEnabled = true;
			super.start();
		}

		@Override
		public void run() {
			CavanAndroid.dLog("Enter: service " + getServiceName());

			while (mEnabled) {
				setState(true);
				mainServiceLoop(mPort);
				setState(false);

				try {
					for (int i = 0; i < 10 && mEnabled; i++) {
						Thread.sleep(200);
					}
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}

			CavanAndroid.dLog("Exit: service " + getServiceName());
		}
	}

	class MyBinder extends ICavanService.Stub {

		@Override
		public void start(int port) throws RemoteException {
			if (mState) {
				sendStateBroadcast(true);
			} else {
				mPort = port;
				new MyThread().start();
			}
		}

		@Override
		public boolean getState() throws RemoteException {
			return mState;
		}

		@Override
		public int getPort() throws RemoteException {
			return mPort;
		}

		@Override
		public String getAction() throws RemoteException {
			return getServiceAction();
		}

		@Override
		public boolean stop() throws RemoteException {
			mEnabled = false;
			return stopService();
		}

		@Override
		public boolean isEnabled() throws RemoteException {
			return mEnabled;
		}
	}

	public abstract int getDefaultPort();
	public abstract boolean stopService();
	public abstract String getServiceName();
	protected abstract void mainServiceLoop(int port);

	public CavanService() {
		super();
		mPort = getDefaultPort();
	}

	private void sendStateBroadcast(boolean state) {
		Intent intent = new Intent(getServiceAction());
		intent.putExtra("state", state);
		sendBroadcast(intent);
	}

	private void setState(boolean state) {
		mState = state;
		sendStateBroadcast(state);
	}

	public String getServiceAction() {
		return "cavan.intent.action." + getServiceName();
	}

	@Override
	public void onCreate() {
		CavanJni.setupEnv(this);
		super.onCreate();
	}

	@Override
	public IBinder onBind(Intent arg0) {
		return new MyBinder();
	}
}
