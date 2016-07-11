package com.cavan.jwaootoy;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ProgressBar;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanBleScanner;
import com.cavan.java.CavanProgressListener;
import com.jwaoo.android.JwaooBleToy;
import com.jwaoo.android.JwaooToySensor;

@SuppressLint("HandlerLeak")
public class MainActivity extends Activity implements OnClickListener {

	private static final int EVENT_DATA_RECEIVED = 1;
	private static final int EVENT_OTA_START = 2;
	private static final int EVENT_OTA_FAILED = 3;
	private static final int EVENT_OTA_SUCCESS = 4;
	private static final int EVENT_PROGRESS_UPDATED = 5;
	private static final int EVENT_FREQ_CHANGED = 6;
	private static final int EVENT_DEPTH_CHANGED = 7;
	private static final int EVENT_CONNECTED = 10;
	private static final int EVENT_DISCONNECTED = 11;

	private int mFreq;
	private int mDepth;

	private BluetoothDevice mDevice;
	private JwaooBleToy mBleToy;
	private boolean mOtaBusy;
	private boolean mSensorEnable;
	private JwaooToySensor mSensor = new JwaooToySensor(100, 1) {

		@Override
		protected void onDepthChanged(int depth) {
			mDepth = depth;
			mHandler.sendEmptyMessage(EVENT_DEPTH_CHANGED);
		}

		@Override
		protected void onFreqChanged(int freq) {
			mFreq = freq;
			mHandler.sendEmptyMessage(EVENT_FREQ_CHANGED);
		}
	};

	private Button mButtonSend;
	private Button mButtonUpgrade;
	private Button mButtonReboot;
	private Button mButtonSensor;
	private Button mButtonDisconnect;
	private ProgressBar mProgressBar;
	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case EVENT_DATA_RECEIVED:
				break;

			case EVENT_OTA_START:
				updateUI(false);
				CavanAndroid.showToast(getApplicationContext(), R.string.text_upgrade_start);
				break;

			case EVENT_OTA_FAILED:
				mButtonUpgrade.setEnabled(true);
				mButtonSend.setEnabled(true);
				CavanAndroid.showToast(getApplicationContext(), R.string.text_upgrade_failed);
				break;

			case EVENT_OTA_SUCCESS:
				updateUI(true);
				CavanAndroid.showToast(getApplicationContext(), R.string.text_upgrade_successfull);
				break;

			case EVENT_PROGRESS_UPDATED:
				mProgressBar.setProgress(msg.arg1);
				break;

			case EVENT_FREQ_CHANGED:
			case EVENT_DEPTH_CHANGED:
				setTitle("Depth = " + mDepth + ", Freq = " + mFreq);
				break;

			case EVENT_CONNECTED:
				updateUI(true);
				break;

			case EVENT_DISCONNECTED:
				updateUI(false);
				CavanBleScanner.show(MainActivity.this);
				break;
			}
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		mButtonSend = (Button) findViewById(R.id.buttonSend);
		mButtonSend.setOnClickListener(this);

		mButtonUpgrade = (Button) findViewById(R.id.buttonUpgrade);
		mButtonUpgrade.setOnClickListener(this);

		mButtonReboot = (Button) findViewById(R.id.buttonReboot);
		mButtonReboot.setOnClickListener(this);

		mButtonSensor = (Button) findViewById(R.id.buttonSensor);
		mButtonSensor.setOnClickListener(this);

		mButtonDisconnect = (Button) findViewById(R.id.buttonDisconnect);
		mButtonDisconnect.setOnClickListener(this);

		mProgressBar = (ProgressBar) findViewById(R.id.progressBar1);

		updateUI(false);
		CavanBleScanner.show(this);
	}

	private void updateUI(boolean enable) {
		mButtonUpgrade.setEnabled(enable);
		mButtonReboot.setEnabled(enable);
		mButtonSend.setEnabled(enable);
		mButtonSensor.setEnabled(enable);
		mButtonDisconnect.setEnabled(enable);
	}

	private void setUpgradeProgress(int progress) {
		mHandler.obtainMessage(EVENT_PROGRESS_UPDATED, progress, 0).sendToTarget();
	}

	@Override
	public void onClick(View v) {
		switch (v.getId()) {
		case R.id.buttonSend:
			String identify = mBleToy.doIdentify();
			if (identify == null) {
				break;
			}

			CavanAndroid.logE("identify = " + identify);

			String buildDate = mBleToy.readBuildDate();
			if (buildDate == null) {
				break;
			}

			CavanAndroid.logE("buildDate = " + buildDate);

			int version = mBleToy.readVersion();
			if (version == 0) {
				break;
			}

			CavanAndroid.logE("version = " + Integer.toHexString(version));
			break;

		case R.id.buttonUpgrade:
			if (mOtaBusy) {
				break;
			}

			mOtaBusy = true;

			new Thread() {

				@Override
				public void run() {
					mHandler.sendEmptyMessage(EVENT_OTA_START);
					if (mBleToy.doOtaUpgrade("/mnt/sdcard/jwaoo-toy.hex", new CavanProgressListener() {

						@Override
						public void onProgressUpdated(int progress) {
							setUpgradeProgress(progress);
						}
					})) {
						mHandler.sendEmptyMessage(EVENT_OTA_SUCCESS);
					} else {
						mHandler.sendEmptyMessage(EVENT_OTA_FAILED);
					}

					mOtaBusy = false;
				}
			}.start();
			break;

		case R.id.buttonReboot:
			mBleToy.doReboot();
			break;

		case R.id.buttonSensor:
			if (mSensorEnable) {
				if (mBleToy.setSensorEnable(false)) {
					mSensorEnable = false;
					mButtonSensor.setText(R.string.text_open_sensor);
				}
			} else if (mBleToy.setSensorEnable(true)) {
				mSensorEnable = true;
				mButtonSensor.setText(R.string.text_close_sensor);
			}
			break;

		case R.id.buttonDisconnect:
			if (mBleToy != null && mBleToy.isConnected()) {
				mBleToy.disconnect();
			} else {
				CavanBleScanner.show(MainActivity.this);
			}
			break;
		}
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		CavanAndroid.logE("onActivityResult: requestCode = " + requestCode + ", resultCode = " + resultCode + ", data = " + data);
		if (resultCode == RESULT_OK && data != null) {
			mDevice = data.getParcelableExtra("device");
			if (mDevice == null) {
				finish();
			} else {
				mBleToy = new JwaooBleToy(getApplicationContext(), mDevice) {

					@Override
					protected void onConnectionStateChange(boolean connected) {
						CavanAndroid.logE("onConnectionStateChange: connected = " + connected);
						if (connected) {
							mHandler.sendEmptyMessage(EVENT_CONNECTED);
						} else if (mDevice != null) {
							CavanBleScanner.show(MainActivity.this);
						}
					}

					@Override
					protected void onSensorDataReceived(byte[] data) {
						mSensor.putData(data);
					}
				};

				if (!mBleToy.connect(true)) {
					CavanBleScanner.show(MainActivity.this);
				}
			}
		} else {
			finish();
		}
	}

	@Override
	protected void onDestroy() {
		mDevice = null;

		if (mBleToy != null) {
			mBleToy.disconnect();
		}

		super.onDestroy();
	}
}
