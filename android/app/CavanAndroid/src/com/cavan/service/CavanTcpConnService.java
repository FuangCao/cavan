package com.cavan.service;

import java.net.InetSocketAddress;
import java.net.Socket;
import java.util.HashSet;
import java.util.List;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.RemoteException;

import com.cavan.android.CavanAndroid;
import com.cavan.java.CavanTcpClient;
import com.cavan.java.CavanTcpClient.CavanTcpClientListener;

public abstract class CavanTcpConnService extends Service implements CavanTcpClientListener {

	public static final int STATE_STOPPED = 0;
	public static final int STATE_RUNNING = 1;
	public static final int STATE_CONNECTING = 2;
	public static final int STATE_CONNECTED = 3;
	public static final int STATE_DISCONNECTED = 4;

	private int mState;
	private CavanTcpClient mTcpClient;
	private HashSet<ICavanTcpConnCallback> mCallbacks = new HashSet<ICavanTcpConnCallback>();

	protected ICavanTcpConnService.Stub mBinder = new ICavanTcpConnService.Stub() {

		@Override
		public boolean addCallback(final ICavanTcpConnCallback callback) throws RemoteException {
			boolean added;

			synchronized (mCallbacks) {
				added = mCallbacks.add(callback);
			}

			if (added) {
				callback.onServiceStateChanged(mState);
				callback.asBinder().linkToDeath(new DeathRecipient() {

					@Override
					public void binderDied() {
						CavanAndroid.dLog("binderDied: " + callback);

						try {
							removeCallback(callback);
						} catch (RemoteException e) {
							e.printStackTrace();
						}
					}
				}, 0);
			}

			return added;
		}

		@Override
		public void removeCallback(ICavanTcpConnCallback callback) throws RemoteException {
			synchronized (mCallbacks) {
				mCallbacks.remove(callback);
			}
		}

		@Override
		public void setAddresses(List<String> lines) throws RemoteException {
			int count = mTcpClient.setAddressesByUrl(getDefaultPort(), lines);
			if (count > 0) {
				mTcpClient.connect();
			}
		}

		@Override
		public int getState() throws RemoteException {
			return mState;
		}

		@Override
		public String getCurrentAddress() throws RemoteException {
			return mTcpClient.getCurrentAddressString();
		}

		@Override
		public boolean connect() throws RemoteException {
			mTcpClient.connect();
			return true;
		}

		@Override
		public void disconnect() throws RemoteException {
			mTcpClient.disconnect();
		}
	};

	@Override
	public void onCreate() {
		mTcpClient = doCreateTcpClient();
	}

	@Override
	public void onDestroy() {
		mTcpClient.disconnect();
	}

	@Override
	public IBinder onBind(Intent intent) {
		return mBinder;
	}

	protected abstract int getDefaultPort();

	protected CavanTcpClient doCreateTcpClient() {
		CavanTcpClient client = new CavanTcpClient();
		client.setTcpClientListener(this);
		return client;
	}

	public synchronized int getServiceState() {
		return mState;
	}

	protected void setServiceState(int state) {
		synchronized (this) {
			mState = state;
		}

		synchronized (mCallbacks) {
			for (ICavanTcpConnCallback callback : mCallbacks) {
				try {
					callback.onServiceStateChanged(state);
				} catch (RemoteException e) {
					e.printStackTrace();
				}
			}
		}

		onServiceStateChanged(state);
	}

	public CavanTcpClient getTcpClient() {
		return mTcpClient;
	}

	protected void onServiceStateChanged(int state) {
		CavanAndroid.dLog("onServiceStateChanged: " + state);
	}

	@Override
	public void onTcpClientRunning() {
		setServiceState(STATE_RUNNING);
	}

	@Override
	public void onTcpClientStopped() {
		setServiceState(STATE_STOPPED);
	}

	@Override
	public void onTcpConnecting(InetSocketAddress address) {
		setServiceState(STATE_CONNECTING);
	}

	@Override
	public boolean onTcpConnected(Socket socket) {
		setServiceState(STATE_CONNECTED);
		return true;
	}

	@Override
	public void onTcpDisconnected() {
		setServiceState(STATE_DISCONNECTED);
	}

	@Override
	public long onTcpConnFailed(int times) {
		return 0;
	}

	@Override
	public boolean onDataReceived(byte[] bytes, int length) {
		return false;
	}
}