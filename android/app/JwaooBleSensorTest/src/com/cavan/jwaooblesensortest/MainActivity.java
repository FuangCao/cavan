package com.cavan.jwaooblesensortest;

import android.app.AlertDialog;
import android.app.Dialog;
import android.bluetooth.BluetoothDevice;
import android.os.Bundle;
import android.os.Message;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.TextView;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanWaveView;
import com.cavan.resource.JwaooToyActivity;
import com.jwaoo.android.JwaooToySensor;

public class MainActivity extends JwaooToyActivity implements OnCheckedChangeListener {

	private static final int SENSOR_DELAY = 20;
	private static final int SENSOR_DELAY_MIN = SENSOR_DELAY - 2;
	private static final int SENSOR_DELAY_MAX = SENSOR_DELAY + 2;

	private static final int MSG_DISCONNECTED = 100;
	private static final int MSG_UPDATE_SPEED = 101;
	private static final int MSG_BATTERY_INFO = 102;

	private CavanWaveView mWaveViewX;
	private CavanWaveView mWaveViewY;
	private CavanWaveView mWaveViewZ;

	private CheckBox mCheckBoxReConn;
	private CheckBox mCheckBoxLogEnable;
	private CheckBox mCheckBoxShowDelay;
	private CheckBox mCheckBoxOptimizeSensorSpeed;

	private TextView mTextViewTime;
	private TextView mTextViewState;
	private TextView mTextViewBattery;

	private Dialog mDialogDisconnected;

	private int mBatteryLevel;
	private double mBatteryVoltage;

	private long mConnStartTime;
	private long mConnLastTime;
	private long mSensorDataTime;

	private int mPackageCount;
	private long mPackageTotal;

	@Override
	public boolean handleMessage(Message msg) {
		switch (msg.what) {
		case MSG_DISCONNECTED:
			if (mDialogDisconnected == null) {
				AlertDialog.Builder builder = new AlertDialog.Builder(MainActivity.this);
				builder.setCancelable(false);
				builder.setMessage(R.string.disconnected);
				builder.setPositiveButton(android.R.string.ok, null);
				mDialogDisconnected = builder.create();
			}

			mDialogDisconnected.show();
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
	protected void onCreateBle(Bundle savedInstanceState) {
		setContentView(R.layout.activity_main);

		CavanAndroid.acquireWakeLock(this);

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
		mCheckBoxLogEnable = (CheckBox) findViewById(R.id.checkBoxLogEnable);
		mCheckBoxShowDelay = (CheckBox) findViewById(R.id.checkBoxShowDelay);
		mCheckBoxOptimizeSensorSpeed = (CheckBox) findViewById(R.id.checkBoxOptimizeSensorSpeed);
		mCheckBoxOptimizeSensorSpeed.setOnCheckedChangeListener(this);

		mTextViewBattery = (TextView) findViewById(R.id.textViewBattery);
		mTextViewState = (TextView) findViewById(R.id.textViewSpeed);
		mTextViewTime = (TextView) findViewById(R.id.textViewTime);

		showScanActivity();
	}

	@Override
	protected void onDestroy() {
		CavanAndroid.releaseWakeLock();
		super.onDestroy();
	}

	@Override
	protected void onScanComplete(BluetoothDevice device) {
		mConnStartTime = 0;

		if (mDialogDisconnected != null) {
			mDialogDisconnected.dismiss();
		}
	}

	@Override
	public void onConnectionStateChanged(boolean connected) {
		updateUI(connected);
		mHandler.removeMessages(MSG_UPDATE_SPEED);

		if (connected) {
			showProgressDialog(false);
			mConnStartTime = System.currentTimeMillis();
			mHandler.sendEmptyMessage(MSG_UPDATE_SPEED);
		} else if (mConnStartTime == 0 || mCheckBoxReConn.isChecked()) {
			showProgressDialog(true);
		} else if (!isUserCanceled()) {
			disconnect();
			mHandler.sendEmptyMessage(MSG_DISCONNECTED);
		}
	}

	@Override
	public boolean onInitialize() {
		if (!super.onInitialize()) {
			return false;
		}

		try {
			mBleToy.setBatteryEventEnable(true);
			mBleToy.setSensorEnable(true, SENSOR_DELAY);
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}

		mBleToy.setSensorSpeedOptimizeEnable(mCheckBoxOptimizeSensorSpeed.isChecked());

		return true;
	}

	@Override
	public void onSensorDataReceived(JwaooToySensor sensor, byte[] arg0) {
		mPackageCount++;

		if (mCheckBoxShowDelay.isChecked()) {
			long time = System.currentTimeMillis();
			long delay = time - mSensorDataTime;

			mSensorDataTime = time;

			if (delay < SENSOR_DELAY_MIN || delay > SENSOR_DELAY_MAX) {
				CavanAndroid.dLog("delay = " + delay);
			}
		} else {
			if (mCheckBoxLogEnable.isChecked()) {
				CavanAndroid.dLog(sensor.getAccelText());
			}

			mWaveViewX.addValue(sensor.getAxisX());
			mWaveViewY.addValue(sensor.getAxisY());
			mWaveViewZ.addValue(sensor.getAxisZ());
		}
	}

	@Override
	public void onBatteryStateChanged(int state, int level, double voltage) {
		mBatteryLevel = level;
		mBatteryVoltage = voltage;
		mHandler.sendEmptyMessage(MSG_BATTERY_INFO);
	}

	@Override
	public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
		if (buttonView == mCheckBoxOptimizeSensorSpeed) {
			if (mBleToy != null) {
				mBleToy.setSensorSpeedOptimizeEnable(isChecked);
			}
		}
	}
}
