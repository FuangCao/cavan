package com.cavan.blecounter;

import android.bluetooth.BluetoothDevice;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;

import com.cavan.android.CavanWaveView;
import com.cavan.java.CavanPeakValleyFinder;
import com.cavan.java.CavanPeakValleyValue;
import com.cavan.resource.JwaooToyActivity;
import com.jwaoo.android.JwaooBleToy;

public class MainActivity extends JwaooToyActivity {

	private static final int MSG_SHOW_SPEED = 1;

	private CavanPeakValleyFinder mFinders[] = {
			new CavanPeakValleyFinder(100, 1),
			new CavanPeakValleyFinder(100, 1),
			new CavanPeakValleyFinder(100, 1),
			new CavanPeakValleyFinder(100, 1),
	};

	private CavanWaveView mWaveView1;
	private CavanWaveView mWaveView2;
	private CavanWaveView mWaveView3;
	private CavanWaveView mWaveView4;

	private int mCount;
	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case MSG_SHOW_SPEED:
				int count = mCount;
				mCount = 0;

				if (count > 0) {
					setTitle("freq = " + mFinders[0].getFreq() + ", count = " + count + ", speed = " + (1000.0 / count));
				}

				mHandler.sendEmptyMessageDelayed(MSG_SHOW_SPEED, 1000);
				break;
			}
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		mWaveView1 = (CavanWaveView) findViewById(R.id.waveViewX);
		mWaveView1.setValueRange(-19.6, 19.6);
		mWaveView1.setZoom(3);

		mWaveView2 = (CavanWaveView) findViewById(R.id.waveViewY);
		mWaveView2.setValueRange(-19.6, 19.6);
		mWaveView2.setZoom(3);

		mWaveView3 = (CavanWaveView) findViewById(R.id.waveViewZ);
		mWaveView3.setValueRange(0, 19.6);
		mWaveView3.setZoom(3);

		mWaveView4 = (CavanWaveView) findViewById(R.id.waveViewDepth);
		mWaveView4.setValueRange(0, 500);
		mWaveView4.setZoom(3);

		mHandler.sendEmptyMessage(MSG_SHOW_SPEED);
		showScanActivity();
	}

	@Override
	protected JwaooBleToy createJwaooBleToy(BluetoothDevice device) {
		return new JwaooBleToy(MainActivity.this, device) {

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
				mSensor.putBytes(arg0);

				double value = mSensor.getAxisZ();

				mWaveView1.addValue(value);

				CavanPeakValleyValue result = mFinders[0].putFreqValue(value);
				if (result != null) {
					mWaveView3.addValue(result.getDiff());
				} else {
					mWaveView3.addValue(0);
				}

				mWaveView2.addValue(mFinders[0].getAvgValue());
				mWaveView4.addValue(mFinders[0].getFreq());

				mCount++;
			}
		};

	}
}
