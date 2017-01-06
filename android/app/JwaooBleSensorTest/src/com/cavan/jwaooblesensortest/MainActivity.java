package com.cavan.jwaooblesensortest;

import android.app.AlertDialog;
import android.bluetooth.BluetoothDevice;
import android.os.Bundle;
import android.os.Message;
import android.widget.CheckBox;
import android.widget.TextView;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanWaveView;
import com.cavan.resource.JwaooToyActivity;
import com.jwaoo.android.JwaooBleToy;

public class MainActivity extends JwaooToyActivity {

	private static final int MSG_DISCONNECTED = 100;
	private static final int MSG_UPDATE_SPEED = 101;
	private static final int MSG_BATTERY_INFO = 102;

	private CavanWaveView mWaveViewX;
	private CavanWaveView mWaveViewY;
	private CavanWaveView mWaveViewZ;
	private CheckBox mCheckBoxReConn;
	private TextView mTextViewTime;
	private TextView mTextViewState;
	private TextView mTextViewBattery;

	private int mBatteryLevel;
	private double mBatteryVoltage;

	private long mConnStartTime;
	private long mConnLastTime;

	private int mPackageCount;
	private long mPackageTotal;

	@Override
	public boolean handleMessage(Message msg) {
		switch (msg.what) {
		case MSG_DISCONNECTED:
			AlertDialog.Builder builder = new AlertDialog.Builder(MainActivity.this);
			builder.setCancelable(false);
			builder.setMessage(R.string.disconnected);
			builder.setPositiveButton(android.R.string.ok, null);
			builder.create().show();
			break;

		case MSG_UPDATE_SPEED:
			int count = mPackageCount;
			mPackageCount = 0;

			mHandler.sendEmptyMessageDelayed(MSG_UPDATE_SPEED, 1000);
			mPackageTotal += count;

			double speed;

			if (count > 0) {
				speed = 1000.0 / count;

				mConnLastTime = System.currentTimeMillis();
				updateConnTime();
			} else {
				speed = 0;
			}

			mTextViewState.setText(getResources().getString(R.string.speed_info, mPackageTotal, speed));
			break;

		case MSG_BATTERY_INFO:
			mTextViewBattery.setText(getResources().getString(R.string.battery_info, mBatteryLevel, mBatteryVoltage));
			break;

		default:
			return super.handleMessage(msg);
		}

		return true;
	}

	private void updateConnTime() {
		long second;

		if (mConnStartTime > 0) {
			second = (mConnLastTime - mConnStartTime) / 1000;
		} else {
			second = 0;
		}

		long minute = second / 60;
		long hour = minute / 60;
		long day = hour / 24;

		StringBuilder builder = new StringBuilder();

		if (day > 0) {
			builder.append(day);
			builder.append(getResources().getString(R.string.day));
		}

		long value;

		value = hour - day * 24;
		if (value > 0) {
			builder.append(value);
			builder.append(getResources().getString(R.string.hour));
		}

		value = minute - hour * 60;
		if (value > 0) {
			builder.append(value);
			builder.append(getResources().getString(R.string.minute));
		}

		value = second - minute * 60;
		if (value > 0) {
			builder.append(value);
			builder.append(getResources().getString(R.string.second));
		}

		mTextViewTime.setText(getResources().getString(R.string.link_time, builder.toString()));
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		CavanAndroid.setSuspendEnable(this, false);

		mWaveViewX = (CavanWaveView) findViewById(R.id.waveViewX);
		mWaveViewX.setValueRange(-19.6, 19.6);
		mWaveViewX.setZoom(2);

		mWaveViewY = (CavanWaveView) findViewById(R.id.waveViewY);
		mWaveViewY.setValueRange(-19.6, 19.6);
		mWaveViewY.setZoom(2);

		mWaveViewZ = (CavanWaveView) findViewById(R.id.waveViewZ);
		mWaveViewZ.setValueRange(-19.6, 19.6);
		mWaveViewZ.setZoom(2);

		mCheckBoxReConn = (CheckBox) findViewById(R.id.checkBoxAutoReConnect);
		mTextViewBattery = (TextView) findViewById(R.id.textViewBattery);
		mTextViewState = (TextView) findViewById(R.id.textViewSpeed);
		mTextViewTime = (TextView) findViewById(R.id.textViewTime);

		showScanActivity();
	}

	@Override
	protected void onDestroy() {
		CavanAndroid.setSuspendEnable(this, true);
		super.onDestroy();
	}

	@Override
	protected JwaooBleToy createJwaooBleToy(BluetoothDevice device) {
		mConnStartTime = 0;

		return new JwaooBleToy(device) {

			@Override
			protected void onConnectionStateChange(boolean connected) {
				updateUI(connected);
				mHandler.removeMessages(MSG_UPDATE_SPEED);

				if (connected) {
					showProgressDialog(false);
					mConnStartTime = System.currentTimeMillis();
					mHandler.sendEmptyMessage(MSG_UPDATE_SPEED);
				} else if (mConnStartTime == 0 || mCheckBoxReConn.isChecked()) {
					showProgressDialog(true);
				} else {
					disconnect();
					mHandler.sendEmptyMessage(MSG_DISCONNECTED);
				}
			}

			@Override
			protected void onConnectFailed() {
				showScanActivity();
			}

			@Override
			protected boolean onInitialize() {
				mBleToy.setBatteryEventEnable(true);
				mBleToy.setSensorEnable(true, 20);
				return super.onInitialize();
			}

			@Override
			protected void onSensorDataReceived(byte[] arg0) {
				mPackageCount++;

				mSensor.putBytes(arg0);
				mWaveViewX.addValue(mSensor.getAxisX());
				mWaveViewY.addValue(mSensor.getAxisY());
				mWaveViewZ.addValue(mSensor.getAxisZ());
			}

			@Override
			protected void onBatteryStateChanged(int state, int level, double voltage) {
				mBatteryLevel = level;
				mBatteryVoltage = voltage;
				mHandler.sendEmptyMessage(MSG_BATTERY_INFO);
			}
		};
	}
}
