package com.cavan.glcube;

import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.support.v7.app.ActionBarActivity;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.Toast;

public class MainActivity extends ActionBarActivity implements OnClickListener {

	@SuppressWarnings("unused")
	private final String TAG = "Cavan";
	private final double ROTATE_STEP = 10.0;

	private CubeGLSurfaceView mSurfaceView;
	private Button mButtonRotateX;
	private Button mButtonRotateY;
	private Button mButtonRotateZ;

	private SensorEventListener mSensorEventListener = new SensorEventListener() {

		private long mTimestamp;

		@Override
		public void onSensorChanged(SensorEvent event) {
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
		setContentView(R.layout.activity_main);

		mSurfaceView = (CubeGLSurfaceView) findViewById(R.id.glSurfaceView);

		mButtonRotateX = (Button) findViewById(R.id.buttonRotateX);
		mButtonRotateX.setOnClickListener(this);

		mButtonRotateY = (Button) findViewById(R.id.buttonRotateY);
		mButtonRotateY.setOnClickListener(this);

		mButtonRotateZ = (Button) findViewById(R.id.buttonRotateZ);
		mButtonRotateZ.setOnClickListener(this);

		SensorManager manager = (SensorManager) getSystemService(SENSOR_SERVICE);
		Sensor sensor = manager.getDefaultSensor(Sensor.TYPE_GYROSCOPE);
		if (sensor != null) {
			Toast.makeText(this, R.string.text_sensor_open_success, Toast.LENGTH_LONG).show();
			manager.registerListener(mSensorEventListener, sensor, SensorManager.SENSOR_DELAY_GAME);
		} else {
			Toast.makeText(this, R.string.text_sensor_open_failed, Toast.LENGTH_LONG).show();
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

	@Override
	public void onClick(View v) {
		switch (v.getId()) {
		case R.id.buttonRotateX:
			mSurfaceView.rotateAdd(ROTATE_STEP, 0.0, 0.0);
			break;

		case R.id.buttonRotateY:
			mSurfaceView.rotateAdd(0.0, ROTATE_STEP, 0.0);
			break;

		case R.id.buttonRotateZ:
			mSurfaceView.rotateAdd(0.0, 0.0, ROTATE_STEP);
			break;
		}
	}
}
