package com.cavan.android;

import android.annotation.SuppressLint;
import android.app.AlertDialog.Builder;
import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.preference.EditTextPreference;
import android.text.InputType;
import android.util.AttributeSet;
import android.view.View;
import android.widget.EditText;

@SuppressLint("DefaultLocale")
public class SensorPreference extends EditTextPreference {

	private boolean mRunning;
	private Sensor mSensor;
	private SensorManager mManager;
	private SensorEventListener mListener;

	public SensorPreference(Context context, AttributeSet attrs, int defStyleAttr) {
		super(context, attrs, defStyleAttr);
	}

	public SensorPreference(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	public SensorPreference(Context context) {
		super(context);
	}

	public boolean init(SensorManager manager, Sensor sensor) {
		mManager = manager;
		mSensor = sensor;
		setTitle(sensor.getName());
		setDefaultValue(Integer.toString(200));

		switch (sensor.getType()) {
		case Sensor.TYPE_ACCELEROMETER:
		case Sensor.TYPE_GYROSCOPE:
		case Sensor.TYPE_GYROSCOPE_UNCALIBRATED:
		case Sensor.TYPE_ROTATION_VECTOR:
		case Sensor.TYPE_MAGNETIC_FIELD:
		case Sensor.TYPE_MAGNETIC_FIELD_UNCALIBRATED:
		case Sensor.TYPE_GAME_ROTATION_VECTOR:
		case Sensor.TYPE_GEOMAGNETIC_ROTATION_VECTOR:
			mListener = new SensorEventListenerVector();
			break;

		default:
			mListener = new SensorEventListenerSimple();
		}

		setSummary(R.string.text_not_start);

		return true;
	}

	public boolean start(int delayMsec) {
		mManager.unregisterListener(mListener);
		if (!mManager.registerListener(mListener , mSensor, delayMsec * 1000)) {
			return false;
		}

		mRunning = true;
		setSummary(R.string.text_running);

		return true;
	}

	public void stop() {
		mManager.unregisterListener(mListener);
		setSummary(R.string.text_stopped);
		mRunning = false;
	}

	@Override
	protected void onDialogClosed(boolean positiveResult) {
		super.onDialogClosed(positiveResult);

		if (positiveResult) {
			if (mRunning) {
				stop();
			} else {
				start(Integer.parseInt(getText()));
			}
		}
	}

	@Override
	protected void showDialog(Bundle state) {
		super.showDialog(state);
	}

	@Override
	protected void onAddEditTextToDialogView(View dialogView, EditText editText) {
		if (mRunning) {
			editText.setEnabled(false);
		} else {
			editText.setEnabled(true);
			editText.setInputType(InputType.TYPE_CLASS_NUMBER);
		}

		super.onAddEditTextToDialogView(dialogView, editText);
	}

	@Override
	protected void onPrepareDialogBuilder(Builder builder) {
		builder.setMessage(R.string.text_sample_rate);

		if (mRunning) {
			builder.setPositiveButton(R.string.text_stop, this);
		} else {
			builder.setPositiveButton(R.string.text_start, this);
		}

		super.onPrepareDialogBuilder(builder);
	}

	class SensorEventListenerSimple implements SensorEventListener {

		@Override
		public void onSensorChanged(SensorEvent event) {
			setSummary(Float.toString(event.values[0]));
		}

		@Override
		public void onAccuracyChanged(Sensor sensor, int accuracy) {
		}
	}

	class SensorEventListenerVector implements SensorEventListener {

		@Override
		public void onSensorChanged(SensorEvent event) {
			String summary = String.format("[%f, %f, %f]", event.values[0], event.values[1], event.values[2]);
			setSummary(summary);
		}

		@Override
		public void onAccuracyChanged(Sensor sensor, int accuracy) {
		}
	}
}
