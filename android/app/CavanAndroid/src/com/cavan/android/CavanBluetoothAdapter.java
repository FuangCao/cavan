package com.cavan.android;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;

public class CavanBluetoothAdapter {

	protected Context mContext;
	protected BluetoothAdapter mAdapter;

	private boolean mPoweredOn;
	private int mLastState = -1;
	private boolean mBroadcastReceiverRegistered;

	private IntentFilter mIntentFilter;
	private final BroadcastReceiver mReceiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {
			onBroadcastReceived(intent);
		}
	};

	protected IntentFilter doCreateIntentFilter() {
		return new IntentFilter(BluetoothAdapter.ACTION_STATE_CHANGED);
	}

	protected void onBluetoothAdapterStateChanged(boolean enabled) {
		CavanAndroid.dLog("onBluetoothAdapterStateChanged: enabled = " + enabled);
	}

	protected void onBroadcastReceiverRegistered(BroadcastReceiver receiver) {
		CavanAndroid.dLog("onBroadcastReceiverRegistered");
	}

	protected void onBroadcastReceiverUnregistered(BroadcastReceiver receiver) {
		CavanAndroid.dLog("onBroadcastReceiverUnregistered");
	}

	protected void onBluetoothAdapterStateChanged(int state) {
		CavanAndroid.dLog("onBluetoothAdapterStateChanged: state = " + state);

		switch (state) {
		case BluetoothAdapter.STATE_ON:
			if (mPoweredOn) {
				break;
			}

			mPoweredOn = true;
			onBluetoothAdapterStateChanged(true);
			break;

		case BluetoothAdapter.STATE_OFF:
			if (mPoweredOn) {
				mPoweredOn = false;
				onBluetoothAdapterStateChanged(false);
			}
			break;
		}
	}

	public CavanBluetoothAdapter(Context context) {
		mContext = context;
		mAdapter = BluetoothAdapter.getDefaultAdapter();
		mPoweredOn = isAdapterEnabled();
	}

	public Context getContext() {
		return mContext;
	}

	public BluetoothAdapter getAdapter() {
		return mAdapter;
	}

	public boolean setAdapterEnable(boolean enable) {
		if (enable) {
			return mAdapter.enable();
		}

		return mAdapter.disable();
	}

	public boolean isAdapterEnabled() {
		return mAdapter.isEnabled();
	}

	public boolean isPoweredOn() {
		return isAdapterEnabled();
	}

	public BluetoothDevice getRemoteDevice(String address) {
		if (address == null || address.isEmpty()) {
			return null;
		}

		try {
			return mAdapter.getRemoteDevice(address);
		} catch (Exception e) {
			e.printStackTrace();
			return null;
		}
	}

	public BluetoothManager getBluetoothManager() {
		return (BluetoothManager) CavanAndroid.getSystemServiceCached(mContext, Context.BLUETOOTH_SERVICE);
	}

	public int getConnectionState(BluetoothDevice device, int profile, int defValue) {
		BluetoothManager manager = getBluetoothManager();
		if (manager == null) {
			CavanAndroid.eLog("Failed to getBluetoothManager");
			return defValue;
		}

		return manager.getConnectionState(device, profile);
	}

	public boolean isConnected(int state) {
		return state == BluetoothProfile.STATE_CONNECTED;
	}

	public boolean isDisconnected(int state) {
		return state == BluetoothProfile.STATE_DISCONNECTED || state == BluetoothProfile.STATE_DISCONNECTING;
	}

	protected void onBroadcastReceived(Intent intent) {
		String action = intent.getAction();

		CavanAndroid.dLog("onBroadcastReceived: action = " + action);

		if (BluetoothAdapter.ACTION_STATE_CHANGED.equals(action)) {
			int state = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, BluetoothAdapter.ERROR);

			if (mLastState != state) {
				mLastState = state;
				onBluetoothAdapterStateChanged(state);
			}
		}
	}

	synchronized public boolean registerBroadcastReceiver() {
		if (mBroadcastReceiverRegistered) {
			return false;
		}

		if (mIntentFilter == null) {
			mIntentFilter = doCreateIntentFilter();
			if (mIntentFilter == null) {
				return false;
			}
		}

		mContext.registerReceiver(mReceiver, mIntentFilter);
		mBroadcastReceiverRegistered = true;
		onBroadcastReceiverRegistered(mReceiver);

		return true;
	}

	synchronized public void unregisterBroadcastReceiver() {
		if (mBroadcastReceiverRegistered) {
			mContext.unregisterReceiver(mReceiver);

			mBroadcastReceiverRegistered = false;
			onBroadcastReceiverUnregistered(mReceiver);
		}
	}

	@Override
	protected void finalize() throws Throwable {
		unregisterBroadcastReceiver();
		super.finalize();
	}
}