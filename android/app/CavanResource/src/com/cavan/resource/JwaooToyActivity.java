package com.cavan.resource;

import java.util.ArrayList;
import java.util.List;

import android.annotation.SuppressLint;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothDevice;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Handler.Callback;
import android.os.Message;
import android.view.View;

import com.cavan.android.CavanAndroid;
import com.jwaoo.android.JwaooBleToy;
import com.jwaoo.android.JwaooBleToy.JwaooBleToyEventListener;
import com.jwaoo.android.JwaooToySensor;

@SuppressLint("HandlerLeak")
public class JwaooToyActivity extends CavanBleActivity implements OnCancelListener, Callback, JwaooBleToyEventListener {

	private final int MSG_UPDATE_UI = 1;
	private final int MSG_SHOW_PROGRESS_DIALOG = 2;

	public static final int SENSOR_DELAY = 30;

	private boolean mUserCancel;
	private String[] mAddresses;
	protected ProgressDialog mProgressDialog;
	protected Handler mHandler = new Handler(this);
	protected JwaooBleToy mBleToy;

	protected List<View> mListViews = new ArrayList<View>();

	public JwaooToyActivity() {
		super();
	}

	public boolean isUserCanceled() {
		return mUserCancel;
	}

	@Override
	public boolean handleMessage(Message msg) {
		switch (msg.what) {
		case MSG_UPDATE_UI:
			updateUI((Boolean) msg.obj);
			return true;

		case MSG_SHOW_PROGRESS_DIALOG:
			showProgressDialog((Boolean) msg.obj);
			return true;

		default:
			return false;
		}
	}

	public void updateUI(boolean enable) {
		if (CavanAndroid.isMainThread()) {
			for (View view : mListViews) {
				view.setEnabled(enable);
			}
		} else {
			mHandler.obtainMessage(MSG_UPDATE_UI, enable).sendToTarget();
		}
	}

	protected String buildProgressDialogMessage() {
		return getResources().getString(R.string.text_connect_inprogress);
	}

	synchronized public void showProgressDialog(boolean show) {
		if (CavanAndroid.isMainThread()) {
			CavanAndroid.dLog("showProgressDialog: " + show);

			if (show) {
				if (mProgressDialog == null) {
					String message = buildProgressDialogMessage();
					mProgressDialog = ProgressDialog.show(this, null, message, true, true, this);
				} else if (!mProgressDialog.isShowing()) {
					mProgressDialog.show();
				}
			} else if (mProgressDialog != null) {
				mProgressDialog.dismiss();
				mProgressDialog = null;
			}
		} else {
			mHandler.obtainMessage(MSG_SHOW_PROGRESS_DIALOG, show).sendToTarget();
		}
	}

	public void setAddresses(String[] addresses) {
		mAddresses = addresses;
	}

	public void setAddresses2(String... addresses) {
		mAddresses = addresses;
	}

	public void showScanActivity() {
		updateUI(false);
		showProgressDialog(false);
		CavanBleScanActivity.show(this, JwaooBleToy.BT_NAMES, mAddresses); // , JwaooBleToy.BT_UUIDS);
	}

	public void disconnect() {
		mUserCancel = true;

		if (mBleToy != null) {
			mBleToy.disconnect();
		}

		showScanActivity();
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		mBleToy = new JwaooBleToy(this);
		mBleToy.setEventListener(this);
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		CavanAndroid.dLog("onActivityResult: requestCode = " + requestCode + ", resultCode = " + resultCode + ", data = " + data);
		if (resultCode == RESULT_OK && data != null) {
			BluetoothDevice device = data.getParcelableExtra("device");
			if (device == null) {
				finish();
			} else {
				mUserCancel = false;
				showProgressDialog(true);
				setAddresses2(device.getAddress());

				onScanComplete(device);

				if (mBleToy.connect(device) == false) {
					showScanActivity();
				}
			}
		} else {
			finish();
		}
	}

	protected void onScanComplete(BluetoothDevice device) {
		CavanAndroid.dLog("onScanComplete: " + device);
	}

	@Override
	public View findViewById(int id) {
		View view = super.findViewById(id);
		if (view != null && mListViews.indexOf(view) < 0) {
			mListViews.add(view);
		}

		return view;
	}

	@Override
	protected void onDestroy() {
		mBleToy.disconnect();
		System.exit(0);

		super.onDestroy();
	}

	@Override
	public void onCancel(DialogInterface dialog) {
		disconnect();
	}

	@Override
	public void onBackPressed() {
		disconnect();
	}

	@Override
	public boolean onInitialize() {
		CavanAndroid.dLog("onInitialize");
		return true;
	}

	@Override
	public void onConnectFailed() {
		CavanAndroid.dLog("onInitialize");
		showScanActivity();
	}

	@Override
	public void onConnectionStateChanged(boolean connected) {
		CavanAndroid.dLog("onConnectionStateChanged: connected = " + connected);

		if (connected) {
			showProgressDialog(false);
			mAddresses = null;
		} else {
			showProgressDialog(true);
		}

		updateUI(connected);
	}

	@Override
	public void onUpgradeComplete(boolean success) {
		CavanAndroid.dLog("onUpgradeComplete: success = " + success);
	}

	@Override
	public void onSensorDataReceived(JwaooToySensor sensor, byte[] data) {
		// CavanAndroid.dLog("onSensorDataReceived: length = " + data.length);
	}

	@Override
	public void onMotoStateChanged(int mode, int level) {
		CavanAndroid.dLog("onMotoStateChanged: mode = " +  mode + ", level = " + level);
	}

	@Override
	public void onKeyStateChanged(int code, int state) {
		CavanAndroid.dLog("onKeyStateChanged: code = " + code + ", state = " + state);
	}

	@Override
	public void onKeyLongClicked(int code) {
		CavanAndroid.dLog("onKeyLongClicked: code = " + code);
	}

	@Override
	public void onKeyClicked(int code, int count) {
		CavanAndroid.dLog("onKeyClicked: code = " + code + ", count = " + count);
	}

	@Override
	public void onDebugDataReceived(byte[] data) {
		CavanAndroid.dLog("onDebugDataReceived: length = " + data.length);
	}

	@Override
	public void onBatteryStateChanged(int state, int level, double voltage) {
		CavanAndroid.dLog("onBatteryStateChanged: state = " + state + ", level = " + level + ", voltage = " + voltage);
	}

	@Override
	public void onBluetoothAdapterStateChanged(boolean enabled) {
		CavanAndroid.dLog("onBluetoothAdapterStateChanged: enabled = " + enabled);
	}
}
