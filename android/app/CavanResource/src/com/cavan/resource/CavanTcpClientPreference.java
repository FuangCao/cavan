package com.cavan.resource;

import java.util.ArrayList;

import com.cavan.android.CavanAndroid;
import com.cavan.service.CavanTcpConnService;
import com.cavan.service.ICavanTcpConnCallback;
import com.cavan.service.ICavanTcpConnService;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.util.AttributeSet;

public abstract class CavanTcpClientPreference extends EditableMultiSelectListPreference {

	private static final int EVENT_STATE_CHANGED = 1;

	private ICavanTcpConnService mService;
	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case EVENT_STATE_CHANGED:
				CavanAndroid.dLog("EVENT_STATE_CHANGED: " + msg.arg1);
				updateSummary(msg.arg1);
				break;
			}
		}
	};

	private ICavanTcpConnCallback mCallback = new ICavanTcpConnCallback.Stub() {

		@Override
		public void onServiceStateChanged(int state) throws RemoteException {
			Message message = mHandler.obtainMessage(EVENT_STATE_CHANGED);
			message.arg1 = state;
			message.sendToTarget();
		}
	};

	private ServiceConnection mConnection = new ServiceConnection() {

		@Override
		public void onServiceDisconnected(ComponentName name) {
			setEnabled(false);
			mService = null;
		}

		@Override
		public void onServiceConnected(ComponentName name, IBinder service) {
			mService = ICavanTcpConnService.Stub.asInterface(service);

			applyAddresses();

			try {
				mService.addCallback(mCallback);
			} catch (RemoteException e) {
				e.printStackTrace();
			}

			setEnabled(true);
		}
	};

	public CavanTcpClientPreference(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
		super(context, attrs, defStyleAttr, defStyleRes);
		startService(context);
	}

	public CavanTcpClientPreference(Context context, AttributeSet attrs, int defStyleAttr) {
		super(context, attrs, defStyleAttr);
		startService(context);
	}

	public CavanTcpClientPreference(Context context, AttributeSet attrs) {
		super(context, attrs);
		startService(context);
	}

	public CavanTcpClientPreference(Context context) {
		super(context);
		startService(context);
	}

	public abstract Intent getServiceIntent();

	public void startService(Context context) {
		setEnabled(false);
		updateSummary(CavanTcpConnService.STATE_STOPPED);

		Intent service = getServiceIntent();
		context.startService(service);
		context.bindService(service, mConnection, 0);
	}

	protected void updateSummary(int state) {
		switch (state) {
		case CavanTcpConnService.STATE_CONNECTED:
			if (mService != null) {
				try {
					String address = mService.getCurrentAddress();
					String summary = getContext().getString(R.string.connected_to, address);
					setSummary(summary);
					break;
				} catch (RemoteException e) {
					e.printStackTrace();
				}
			}

			setSummary(R.string.connected);
			break;

		case CavanTcpConnService.STATE_CONNECTING:
			if (mService != null) {
				try {
					String address = mService.getCurrentAddress();
					String summary = getContext().getString(R.string.connecting_to, address);
					setSummary(summary);
					break;
				} catch (RemoteException e) {
					e.printStackTrace();
				}
			}

			setSummary(R.string.connecting);
			break;

		case CavanTcpConnService.STATE_DISCONNECTED:
			setSummary(R.string.disconnect);
			break;

		case CavanTcpConnService.STATE_RUNNING:
			setSummary(R.string.running);
			break;

		case CavanTcpConnService.STATE_STOPPED:
			setSummary(R.string.stopped);
			break;
		}
	}

	private void applyAddresses() {
		ArrayList<String> lines = load(getContext(), getKey());
		if (mService != null) {
			try {
				mService.setAddresses(lines);
			} catch (RemoteException e) {
				e.printStackTrace();
			}
		}
	}

	public synchronized void unbindService(Context context) {
		if (mService != null) {
			try {
				mService.removeCallback(mCallback);
			} catch (RemoteException e) {
				e.printStackTrace();
			}
		}

		context.unbindService(mConnection);
	}

	@Override
	protected void onDialogClosed(boolean positiveResult) {
		super.onDialogClosed(positiveResult);

		if (positiveResult) {
			applyAddresses();
		}
	}
}
