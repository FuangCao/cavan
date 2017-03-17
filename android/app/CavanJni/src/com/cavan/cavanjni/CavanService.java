package com.cavan.cavanjni;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.RemoteException;

import com.cavan.android.CavanAndroid;

public abstract class CavanService extends Service {

	public static final int STATE_STOPPED = 0;
	public static final int STATE_PREPARE = 1;
	public static final int STATE_RUNNING = 2;
	public static final int STATE_WAITING = 3;

	protected int mPort;
	protected boolean mEnabled;
	protected int mState = STATE_STOPPED;

	class MyThread extends Thread {

		@Override
		public void run() {
			setServiceState(STATE_PREPARE);

			while (mEnabled) {
				setServiceState(STATE_RUNNING);
				doMainLoop(mPort);

				setServiceState(STATE_WAITING);

				try {
					for (int i = 0; i < 10 && mEnabled; i++) {
						Thread.sleep(200);
					}
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}

			setServiceState(STATE_STOPPED);
		}
	}

	class MyBinder extends ICavanService.Stub {

		@Override
		public void start(int port) throws RemoteException {
			mEnabled = true;

			if (mState != STATE_STOPPED) {
				sendStateBroadcast(mState);
			} else {
				mPort = port;
				new MyThread().start();
			}
		}

		@Override
		public int getState() throws RemoteException {
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
			return doStopService();
		}

		@Override
		public boolean isEnabled() throws RemoteException {
			return mEnabled;
		}
	}

	public abstract int getDefaultPort();
	public abstract String getServiceName();
	public abstract boolean doStopService();
	protected abstract void doMainLoop(int port);

	protected void onServiceStateChanged(int state) {
		CavanAndroid.dLog("onServiceStateChanged: " + getServiceName() + " = " + state);
	}

	public CavanService() {
		super();
		mPort = getDefaultPort();
	}

	private void sendStateBroadcast(int state) {
		Intent intent = new Intent(getServiceAction());
		intent.putExtra("state", state);
		sendBroadcast(intent);
	}

	private void setServiceState(int state) {
		if (mState != state) {
			mState = state;
			sendStateBroadcast(state);
			onServiceStateChanged(state);
		}
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
