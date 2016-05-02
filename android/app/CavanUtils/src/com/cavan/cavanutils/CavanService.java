package com.cavan.cavanutils;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.RemoteException;

public abstract class CavanService extends Service {

	protected int mPort;
	protected boolean mState;

	class MyThread extends Thread {

		@Override
		public void run() {
			CavanUtils.logE("Enter: service " + getName());
			setState(true);
			mainServiceLoop(mPort);
			setState(false);
			CavanUtils.logE("Exit: service " + getName());
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
	}

	public CavanService() {
		super();
		mPort = getDefaultPort();
	}

	@Override
	public IBinder onBind(Intent arg0) {
		return new MyBinder();
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

	public abstract int getDefaultPort();
	public abstract String getServiceName();
	protected abstract void mainServiceLoop(int port);
}
