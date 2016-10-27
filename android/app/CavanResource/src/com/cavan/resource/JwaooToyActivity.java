package com.cavan.resource;

import java.util.ArrayList;
import java.util.List;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothDevice;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.Intent;
import android.os.Handler;
import android.os.Message;
import android.view.View;

import com.cavan.android.CavanAndroid;
import com.jwaoo.android.JwaooBleToy;

@SuppressLint("HandlerLeak")
public class JwaooToyActivity extends Activity implements OnCancelListener {

	public static final String[] DEVICE_NAMES = {
		"JwaooToy", "SenseTube"
	};

	private final int MSG_UPDATE_UI = 1;
	private final int MSG_SHOW_PROGRESS_DIALOG = 2;

	public static final int SENSOR_DELAY = 30;

	protected JwaooBleToy mBleToy;
	protected ProgressDialog mProgressDialog;
	protected List<View> mListViews = new ArrayList<View>();

	protected void handleMessage(Message msg) {}

	protected Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case MSG_UPDATE_UI:
				updateUI((boolean) msg.obj);
				break;

			case MSG_SHOW_PROGRESS_DIALOG:
				showProgressDialog((boolean) msg.obj);
				break;

			default:
				JwaooToyActivity.this.handleMessage(msg);
			}
		}
	};

	protected boolean onInitialize() {
		return true;
	}

	protected void onConnectionStateChange(boolean connected) {
		CavanAndroid.eLog("onConnectionStateChange: connected = " + connected);
		updateUI(connected);
	}

	protected void onSensorDataReceived(byte[] data) {}
	protected void onBatteryStateChanged(int state, int level, double voltage) {}

	protected JwaooBleToy createJwaooBleToy(BluetoothDevice device) {
		return new JwaooBleToy(device) {

			@Override
			protected boolean onInitialize() {
				if (!JwaooToyActivity.this.onInitialize()) {
					CavanAndroid.eLog("Failed to JwaooToyActivity.this.onInitialize");
					return false;
				}

				return super.onInitialize();
			}

			@Override
			protected void onConnectionStateChange(boolean connected) {
				CavanAndroid.eLog("JwaooBleToy.onConnectionStateChange: connected = " + connected);
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

	synchronized public void showProgressDialog(boolean show) {
		if (CavanAndroid.isMainThread()) {
			CavanAndroid.eLog("showProgressDialog: " + show);

			if (show) {
				if (mProgressDialog == null) {
					String message = getResources().getString(R.string.text_connect_inprogress);
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
		CavanBleScanActivity.show(this, DEVICE_NAMES);
	}

	public void disconnect() {
		if (mBleToy != null) {
			mBleToy.disconnect();
		}

		showScanActivity();
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		CavanAndroid.eLog("onActivityResult: requestCode = " + requestCode + ", resultCode = " + resultCode + ", data = " + data);
		if (resultCode == RESULT_OK && data != null) {
			BluetoothDevice device = data.getParcelableExtra("device");
			if (device == null) {
				finish();
			} else {
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
