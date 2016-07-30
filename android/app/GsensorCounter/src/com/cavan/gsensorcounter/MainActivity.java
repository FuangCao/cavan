package com.cavan.gsensorcounter;

import android.app.Activity;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanWaveView;
import com.cavan.java.CavanSquareWaveCounter;

public class MainActivity extends Activity implements SensorEventListener {

	private long mCount;
	private CavanSquareWaveCounter mCounter = new CavanSquareWaveCounter(1, 1000, 3000);

	private CavanWaveView mWaveView1;
	private CavanWaveView mWaveView2;
	private CavanWaveView mWaveView3;
	private CavanWaveView mWaveView4;

	private Runnable mRunnableUpdateTitle = new Runnable() {

		@Override
		public void run() {
			String text = String.format("freq = %2.2f, count = %d", mCounter.getFreq(), mCount);

			setTitle(text);
			CavanAndroid.logE(text);
		}
	};

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mWaveView1 = (CavanWaveView) findViewById(R.id.waveView1);
        mWaveView1.setValueRange(-19.6, 19.6);
        mWaveView1.setZoom(3);

        mWaveView2 = (CavanWaveView) findViewById(R.id.waveView2);
        mWaveView2.setValueRange(-19.6, 19.6);
        mWaveView2.setZoom(3);

        mWaveView3 = (CavanWaveView) findViewById(R.id.waveView3);
        mWaveView3.setValueRange(-19.6, 19.6);
        mWaveView3.setZoom(3);

        mWaveView4 = (CavanWaveView) findViewById(R.id.waveView4);
        mWaveView4.setValueRange(0, 1);
        mWaveView4.setZoom(3);

        SensorManager manager = (SensorManager) getSystemService(SENSOR_SERVICE);
        Sensor sensor = manager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
        manager.registerListener(this, sensor, 30);
    }

	@Override
	public void onAccuracyChanged(Sensor sensor, int arg1) {
		CavanAndroid.logE("onAccuracyChanged: sensor = " + sensor);
	}

	@Override
	public void onSensorChanged(SensorEvent event) {
		float[] values = event.values;

		mWaveView1.addValue(values[0]);
		mWaveView2.addValue(values[1]);
		mWaveView3.addValue(values[2]);

		mCounter.putFreqValue(values[2]);

		if (mCounter.getValue()) {
			mWaveView4.addValue(1);
		} else {
			mWaveView4.addValue(0);
		}

		long count = mCounter.getCount();
		if (count != mCount) {
			mCount = count;
			runOnUiThread(mRunnableUpdateTitle);
		}
	}
}
