package com.cavan.jwaoodepthsensor;

import android.bluetooth.BluetoothDevice;
import android.graphics.Color;
import android.os.Bundle;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanWaveView;
import com.cavan.resource.JwaooToyActivity;
import com.jwaoo.android.JwaooBleToy;
import com.jwaoo.android.JwaooDepthSquareWaveGenerator;
import com.jwaoo.android.JwaooToySensor;

public class MainActivity extends JwaooToyActivity {

	private static int WAVE_ZOOM = 3;

	private CavanWaveView mWaveView1;
	private CavanWaveView mWaveView2;
	private CavanWaveView mWaveView3;
	private CavanWaveView mWaveView4;
	private CavanWaveView mWaveView5;
	private CavanWaveView mWaveView6;
	private CavanWaveView mWaveView7;
	private CavanWaveView mWaveView8;
	private CavanWaveView mWaveView9;

	private Runnable mRunnableUpdateTitle = new Runnable() {

		@Override
		public void run() {
			if (mBleToy != null) {
				String text = String.format("depth = %4.2f, freq = %4.2f", mBleToy.getDepth(), mBleToy.getFreq());
				setTitle(text);
			}
		}
	};

	public void addSquareWave(CavanWaveView view, boolean value) {
		if (value) {
			view.addValue(1);
			view.setLineColor(Color.GREEN);
		} else {
			view.addValue(0);
			view.setLineColor(Color.WHITE);
		}
	}

	@Override
	protected void onCreateBle(Bundle savedInstanceState) {
		setContentView(R.layout.activity_main);

		CavanAndroid.setSuspendEnable(this, false);

		mWaveView1 = (CavanWaveView) findViewById(R.id.waveView1);
		mWaveView1.setValueRange(JwaooToySensor.CAPACITY_MIN, JwaooToySensor.CAPACITY_MAX);
		mWaveView1.setZoom(WAVE_ZOOM);

		mWaveView2 = (CavanWaveView) findViewById(R.id.waveView2);
		mWaveView2.setLineColor(Color.WHITE);
		mWaveView2.setValueRange(0, 1);
		mWaveView2.setZoom(WAVE_ZOOM);

		mWaveView3 = (CavanWaveView) findViewById(R.id.waveView3);
		mWaveView3.setValueRange(JwaooToySensor.CAPACITY_MIN, JwaooToySensor.CAPACITY_MAX);
		mWaveView3.setZoom(WAVE_ZOOM);

		mWaveView4 = (CavanWaveView) findViewById(R.id.waveView4);
		mWaveView4.setLineColor(Color.WHITE);
		mWaveView4.setValueRange(0, 1);
		mWaveView4.setZoom(WAVE_ZOOM);

		mWaveView5 = (CavanWaveView) findViewById(R.id.waveView5);
		mWaveView5.setValueRange(JwaooToySensor.CAPACITY_MIN, JwaooToySensor.CAPACITY_MAX);
		mWaveView5.setZoom(WAVE_ZOOM);

		mWaveView6 = (CavanWaveView) findViewById(R.id.waveView6);
		mWaveView6.setLineColor(Color.WHITE);
		mWaveView6.setValueRange(0, 1);
		mWaveView6.setZoom(WAVE_ZOOM);

		mWaveView7 = (CavanWaveView) findViewById(R.id.waveView7);
		mWaveView7.setValueRange(JwaooToySensor.CAPACITY_MIN, JwaooToySensor.CAPACITY_MAX);
		mWaveView7.setZoom(WAVE_ZOOM);

		mWaveView8 = (CavanWaveView) findViewById(R.id.waveView8);
		mWaveView8.setLineColor(Color.WHITE);
		mWaveView8.setValueRange(0, 1);
		mWaveView8.setZoom(WAVE_ZOOM);

		mWaveView9 = (CavanWaveView) findViewById(R.id.waveView9);
		mWaveView9.setLineColor(Color.GREEN);
		mWaveView9.setValueRange(0, 1);
		mWaveView9.setZoom(WAVE_ZOOM);

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
			protected void onConnectionStateChange(boolean connected) {
				if (!connected) {
					showScanActivity();
				}
			}

			@Override
			protected boolean onInitialize() {
				setFreqFuzz(10);
				setDepthFuzz(100);
				setSensorEnable(true, SENSOR_DELAY);
				return super.onInitialize();
			}

			@Override
			protected void onSensorDataReceived(byte[] arg0) {
				super.onSensorDataReceived(arg0);

				double capacitys[] = mSensor.getCapacitys();
				CavanAndroid.dLog("capacity: " + mSensor.getCapacityText());

				mParser.putCapacityValue(capacitys);
				JwaooDepthSquareWaveGenerator[] generators = mParser.getGenerators();

				double value = capacitys[0];

				mWaveView1.addValue(value);
				addSquareWave(mWaveView2, generators[0].getValue());

				value = capacitys[1];
				mWaveView3.addValue(value);
				addSquareWave(mWaveView4, generators[1].getValue());

				value = capacitys[2];
				mWaveView5.addValue(value);
				addSquareWave(mWaveView6, generators[2].getValue());

				value = capacitys[3];
				mWaveView7.addValue(value);
				addSquareWave(mWaveView8, generators[3].getValue());

				mWaveView9.addValue(mParser.getDepth());

				runOnUiThread(mRunnableUpdateTitle);
			}
		};
	}
}
