package com.cavan.jwaoodepthsensor;

import android.bluetooth.BluetoothDevice;
import android.graphics.Color;
import android.os.Bundle;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanWaveView;
import com.cavan.resource.JwaooToyActivity;
import com.jwaoo.android.JwaooBleToy;
import com.jwaoo.android.JwaooDepthDecoder;
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

	private JwaooDepthDecoder mDecoder = new JwaooDepthDecoder(10);
	private Runnable mRunnableUpdateTitle = new Runnable() {

		@Override
		public void run() {
			String text = String.format("depth = %4.2f, freq = %4.2f", mDecoder.getDepth(), mDecoder.getFreq());
			// CavanAndroid.eLog("freq = " + mDecoder.getFreq());
			setTitle(text);
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
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
		mWaveView9.setValueRange(0, JwaooToySensor.SENSOR_COUNT);
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
				mBleToy.setSensorEnable(true, SENSOR_DELAY);
				return super.onInitialize();
			}

			@Override
			protected void onSensorDataReceived(byte[] arg0) {
				mSensor.putBytes(arg0);

				double capacitys[] = mSensor.getCapacitys();
				// CavanAndroid.eLog(String.format("capacity: [%7.2f, %7.2f, %7.2f, %7.2f]", capacitys[0], capacitys[1], capacitys[2], capacitys[3]));

				mDecoder.putCapacityValue(capacitys);
				JwaooDepthSquareWaveGenerator[] generators = mDecoder.getGenerators();

				double value = capacitys[0];

				mWaveView1.addValue(value);
				mWaveView2.addValue(generators[0].getValue() ? 1 : 0);

				value = capacitys[1];
				mWaveView3.addValue(value);
				mWaveView4.addValue(generators[1].getValue() ? 1 : 0);

				value = capacitys[2];
				mWaveView5.addValue(value);
				mWaveView6.addValue(generators[2].getValue() ? 1 : 0);

				value = capacitys[3];
				mWaveView7.addValue(value);
				mWaveView8.addValue(generators[3].getValue() ? 1 : 0);

				mWaveView9.addValue(mDecoder.getDepth());

				runOnUiThread(mRunnableUpdateTitle);
			}
		};
	}
}
