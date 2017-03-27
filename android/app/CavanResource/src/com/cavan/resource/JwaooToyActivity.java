package com.cavan.resource;

import java.util.ArrayList;
import java.util.List;

import android.annotation.SuppressLint;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothDevice;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.Intent;
import android.os.Handler;
import android.os.Handler.Callback;
import android.os.Message;
import android.view.View;

import com.cavan.android.CavanAndroid;
import com.jwaoo.android.JwaooBleToy;

@SuppressLint("HandlerLeak")
public class JwaooToyActivity extends CavanBleActivity implements OnCancelListener, Callback {

	private final int MSG_UPDATE_UI = 1;
	private final int MSG_SHOW_PROGRESS_DIALOG = 2;

	public static final int SENSOR_DELAY = 30;

	private boolean mUserCancel;

	protected JwaooBleToy mBleToy;
	protected ProgressDialog mProgressDialog;
	protected Handler mHandler = new Handler(this);
	protected List<View> mListViews = new ArrayList<View>();

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

	protected boolean onInitialize() {
		return true;
	}

	protected void onConnectionStateChange(boolean connected) {
		CavanAndroid.dLog("onConnectionStateChange: connected = " + connected);
		updateUI(connected);
	}

	protected void onSensorDataReceived(byte[] data) {}
	protected void onBatteryStateChanged(int state, int level, double voltage) {}

	protected JwaooBleToy createJwaooBleToy(BluetoothDevice device) {
		return new JwaooBleToy(device) {

			@Override
			protected boolean onInitialize() {
				if (!JwaooToyActivity.this.onInitialize()) {
					CavanAndroid.dLog("Failed to JwaooToyActivity.this.onInitialize");
					return false;
				}

				return super.onInitialize();
			}

			@Override
			protected void onConnectionStateChange(boolean connected) {
				CavanAndroid.dLog("JwaooBleToy.onConnectionStateChange: connected = " + connected);
				showProgressDialog(!connected);
				JwaooToyActivity.this.onConnectionStateChange(connected);
			}

			@Override
			protected void onConnectFailed() {
				showScanActivity();
			}

			@Override
			protected void onSensorDataReceived(byte[] data) {
				super.onSensorDataReceived(data);
				JwaooToyActivity.this.onSensorDataReceived(data);
			}

			@Override
			protected void onBatteryStateChanged(int state, int level, double voltage) {
				JwaooToyActivity.this.onBatteryStateChanged(state, level, voltage);
			}
		};
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

	public void showScanActivity() {
		updateUI(false);
		showProgressDialog(false);
		CavanBleScanActivity.show(this, JwaooBleToy.BT_NAMES); // , JwaooBleToy.BT_UUIDS);
	}

	public void disconnect() {
		mUserCancel = true;

		if (mBleToy != null) {
			mBleToy.disconnect();
		}

		showScanActivity();
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
				mBleToy = createJwaooBleToy(device);

				showProgressDialog(true);

				if (mBleToy == null || mBleToy.connect() == false) {
					showScanActivity();
				}
			}
		} else {
			finish();
		}
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
		if (mBleToy != null) {
			mBleToy.disconnect();
		}

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
}
