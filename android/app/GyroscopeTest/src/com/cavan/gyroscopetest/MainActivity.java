package com.cavan.gyroscopetest;

import android.support.v7.app.ActionBarActivity;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.Window;
import android.view.WindowManager;
import android.widget.TextView;
import android.widget.Toast;


public class MainActivity extends ActionBarActivity {

	private CubeGLSurfaceView mSurfaceView;
	private SensorEventListener mSensorEventListener = new SensorEventListener() {

		private long mTimestamp;

		@Override
		public void onSensorChanged(SensorEvent event) {

			setTitle(String.format("(%f, %f, %f)", event.values[0], event.values[2], event.values[2]));

			double time = (event.timestamp - mTimestamp) / 1000000000.0;

			if (time < 2) {
				double x = event.values[0] * time * 180.0 / Math.PI;
				double y = event.values[1] * time * 180.0 / Math.PI;
				double z = event.values[2] * time * 180.0 / Math.PI;

				mSurfaceView.rotateAdd(x, y, z);
			}

			mTimestamp = event.timestamp;
		}

		@Override
		public void onAccuracyChanged(Sensor sensor, int accuracy) {
		}
	};

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // setContentView(R.layout.activity_main);

        mSurfaceView = new CubeGLSurfaceView(this);
        setContentView(mSurfaceView);

        SensorManager manager = (SensorManager) getSystemService(SENSOR_SERVICE);
		Sensor sensor = manager.getDefaultSensor(Sensor.TYPE_GYROSCOPE);
		if (sensor != null) {
			Toast.makeText(this, R.string.gyroscope_open_success, Toast.LENGTH_LONG).show();
			manager.registerListener(mSensorEventListener, sensor, SensorManager.SENSOR_DELAY_GAME);
		} else {
			Toast.makeText(this, R.string.gyroscope_open_failed, Toast.LENGTH_LONG).show();
		}
	}

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();
        if (id == R.id.action_settings) {
            return true;
        }
        return super.onOptionsItemSelected(item);
    }
}
