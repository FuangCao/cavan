package com.cavan.service;

import java.util.HashSet;

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
	private HashSet<ICavanServiceCallback> mCallbacks = new HashSet<ICavanServiceCallback>();

	protected CavanMessageQueue mMessageQueue = new CavanMessageQueue() {

		@Override
		protected void handleMessage(CavanMessage message) {
			switch (message.what) {
			case MSG_START:
				CavanService.this.start(message.getInt(0));
				break;

			case MSG_STOP:
				CavanService.this.stop();
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
		public void stop() throws RemoteException {
			mMessageQueue.sendEmptyMessage(MSG_STOP);
		}

		@Override
		public boolean isEnabled() throws RemoteException {
			return CavanService.this.isEnabled();
		}

		@Override
		public boolean addCallback(final ICavanServiceCallback callback) throws RemoteException {
			CavanAndroid.dLog("addCallback: " + callback);

			boolean added;

			synchronized (mCallbacks) {
				added = mCallbacks.add(callback);
			}

			if (added) {
				callback.asBinder().linkToDeath(new DeathRecipient() {

					@Override
					public void binderDied() {
						try {
							removeCallback(callback);
						} catch (RemoteException e) {
							e.printStackTrace();
						}
					}
				}, 0);
			}

			callback.onServiceStateChanged(mState);

			return added;
		}

		@Override
		public boolean removeCallback(ICavanServiceCallback callback) throws RemoteException {
			CavanAndroid.dLog("removeCallback: " + callback);

			synchronized (mCallbacks) {
				return mCallbacks.remove(callback);
			}
		}
	};

	public abstract void start(int port);
	public abstract int getPort();
	public abstract void stop();
	public abstract boolean isEnabled();
	public abstract String getServiceName();

	protected void performServiceStateChanged(int state) {
		synchronized (mCallbacks) {
			for (ICavanServiceCallback callback : mCallbacks) {
				try {
					callback.onServiceStateChanged(state);
				} catch (RemoteException e) {
					e.printStackTrace();
				}
			}
		}
	}

	protected void setServiceState(int state) {
		if (mState != state) {
			mState = state;
			performServiceStateChanged(state);
			onServiceStateChanged(state);
		}
	}

	protected void onServiceStateChanged(int state) {
		CavanAndroid.dLog("onServiceStateChanged: " + getServiceName() + " = " + state);
	}

	@Override
	public IBinder onBind(Intent intent) {
		return mBind;
	}

	@Override
	public void onDestroy() {
		mCallbacks.clear();
		super.onDestroy();
	}
}
