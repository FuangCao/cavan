package com.cavan.cavanjni;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.RemoteException;

import com.cavan.android.CavanAndroid;
import com.cavan.java.CavanMessageQueue;
import com.cavan.java.CavanMessageQueue.CavanMessage;

public abstract class CavanService extends Service {

	public static final int STATE_STOPPED = 0;
	public static final int STATE_PREPARE = 1;
	public static final int STATE_RUNNING = 2;
	public static final int STATE_WAITING = 3;

	private static final int MSG_START = 1;
	private static final int MSG_STOP = 2;

	protected int mState = STATE_STOPPED;

	protected CavanMessageQueue mMessageQueue = new CavanMessageQueue() {

		@Override
		protected void handleMessage(CavanMessage message) {
			switch (message.what) {
			case MSG_START:
				setServiceState(STATE_PREPARE);
				CavanService.this.start(message.getInt(0));
				break;

			case MSG_STOP:
				CavanService.this.stop();
				setServiceState(STATE_STOPPED);
				break;
			}
		}
	};

	private ICavanService.Stub mBind = new ICavanService.Stub() {

		@Override
		public void start(int port) throws RemoteException {
			CavanMessage message = mMessageQueue.obtainMessage(MSG_START, port);
			mMessageQueue.sendMessage(message);
		}

		@Override
		public int getState() throws RemoteException {
			return mState;
		}

		@Override
		public int getPort() throws RemoteException {
			return CavanService.this.getPort();
		}

		@Override
		public String getAction() throws RemoteException {
			return CavanService.this.getAction();
		}

		@Override
		public void stop() throws RemoteException {
			mMessageQueue.sendEmptyMessage(MSG_STOP);
		}

		@Override
		public boolean isEnabled() throws RemoteException {
			return CavanService.this.isEnabled();
		}
	};

	public abstract void start(int port);
	public abstract int getPort();
	public abstract void stop();
	public abstract boolean isEnabled();
	public abstract String getServiceName();

	protected void sendStateBroadcast(int state) {
		Intent intent = new Intent(getAction());
		intent.putExtra("state", state);
		sendBroadcast(intent);
	}

	protected void setServiceState(int state) {
		if (mState != state) {
			mState = state;
			sendStateBroadcast(state);
			onServiceStateChanged(state);
		}
	}

	protected void onServiceStateChanged(int state) {
		CavanAndroid.dLog("onServiceStateChanged: " + getServiceName() + " = " + state);
	}

	public String getAction() {
		return "com.cavan.intent." + getServiceName();
	}

	@Override
	public IBinder onBind(Intent intent) {
		return mBind;
	}
}
