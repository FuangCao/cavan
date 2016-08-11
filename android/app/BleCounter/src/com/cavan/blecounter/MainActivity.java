package com.cavan.blecounter;

import android.bluetooth.BluetoothDevice;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanWaveView;
import com.cavan.java.CavanSquareWaveCounter;
import com.cavan.resource.JwaooToyActivity;
import com.jwaoo.android.JwaooBleToy;
import com.jwaoo.android.JwaooToySensor;

public class MainActivity extends JwaooToyActivity {

	private static final int MSG_SHOW_SPEED = 1;
	private static final int MSG_PUT_DATA = 2;

	private CavanWaveView mWaveView1;
	private CavanWaveView mWaveView2;
	private CavanWaveView mWaveView3;
	private CavanWaveView mWaveView4;

	private int mCount;
	private double mFreq;
	private CavanSquareWaveCounter mCounterCapacity = new CavanSquareWaveCounter(5, 1000, 2000, 5000);
	private CavanSquareWaveCounter mCounterAccel = new CavanSquareWaveCounter(2.0, 1000, 2000, 5000);
	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case MSG_SHOW_SPEED:
				int count = mCount;
				mCount = 0;

				if (count > 0) {
					setTitle(String.format("freq = %2.2f, count = %2d, speed = %2.2f", mFreq, count, 1000.0 / count));
				}

				mHandler.sendEmptyMessageDelayed(MSG_SHOW_SPEED, 1000);
				break;

			case MSG_PUT_DATA:
				JwaooToySensor sensor = (JwaooToySensor) msg.obj;
				double value = sensor.getAxisX();
				mFreq = mCounterAccel.putFreqValue(value);
				mWaveView1.addValue(value);
				mWaveView2.addValue(mCounterAccel.getValue() ? 1 : 0);

				double capacity = sensor.getCapacitySum();
				mCounterCapacity.putFreqValue(capacity);
				mWaveView3.addValue(capacity);
				mWaveView4.addValue(mCounterCapacity.getValue() ? 1 : 0);
				break;
			}
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		CavanAndroid.setSuspendEnable(this, false);

		mWaveView1 = (CavanWaveView) findViewById(R.id.waveView1);
		mWaveView1.setValueRange(-19.6, 19.6);
		mWaveView1.setZoom(3);

		mWaveView2 = (CavanWaveView) findViewById(R.id.waveView2);
		mWaveView2.setValueRange(0, 1);
		mWaveView2.setZoom(3);

		mWaveView3 = (CavanWaveView) findViewById(R.id.waveView3);
		mWaveView3.setValueRange(-512, 508);
		mWaveView3.setZoom(3);

		mWaveView4 = (CavanWaveView) findViewById(R.id.waveView4);
		mWaveView4.setValueRange(0, 1);
		mWaveView4.setZoom(3);

		mHandler.sendEmptyMessage(MSG_SHOW_SPEED);
		showScanActivity();
	}

	@Override
	protected void onDestroy() {
		CavanAndroid.setSuspendEnable(this, true);
		super.onDestroy();
	}

	@Override
	protected JwaooBleToy createJwaooBleToy(BluetoothDevice device) {
		return new JwaooBleToy(device) {

			@Override
			protected boolean onInitialize() {
				setSensorEnable(true, SENSOR_DELAY);
				return super.onInitialize();
			}

			@Override
			protected void onConnectionStateChange(boolean connected) {
				if (!connected) {
					showScanActivity();
				}
			}

			@Override
			protected void onSensorDataReceived(byte[] arg0) {
				mCount++;
				mSensor.putBytes(arg0);
				mHandler.obtainMessage(MSG_PUT_DATA, mSensor).sendToTarget();
			}
		};

	}
}
