package com.cavan.glcube;

import android.support.v7.app.ActionBarActivity;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;

public class MainActivity extends ActionBarActivity {

	private final String TAG = "Cavan";

	private GLCubeRender mRender;
	
	private SensorEventListener mSensorEventListener = new SensorEventListener() {
		
		private long mTimestamp;
		
		@Override
		public void onSensorChanged(SensorEvent event) {
			double time = (event.timestamp - mTimestamp) / 1000000000.0;

			if (time < 2) {
				double x = event.values[0] * time * 180.0 / Math.PI;
				double y = event.values[1] * time * 180.0 / Math.PI;
				double z = event.values[2] * time * 180.0 / Math.PI;

				mRender.addRotate(x, y, z);
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
        GLSurfaceView glView = new GLSurfaceView(this);
        mRender = new GLCubeRender(getResources());
        glView.setRenderer(mRender);
        setContentView(glView);

        SensorManager manager = (SensorManager) getSystemService(SENSOR_SERVICE);
        Sensor sensor = manager.getDefaultSensor(Sensor.TYPE_GYROSCOPE);
        if (sensor != null) {
        	Log.d(TAG, "have sensor TYPE_GYROSCOPE");
        	manager.registerListener(mSensorEventListener, sensor, SensorManager.SENSOR_DELAY_GAME);
        } else {
        	Log.e(TAG, "no sensor TYPE_GYROSCOPE");
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
