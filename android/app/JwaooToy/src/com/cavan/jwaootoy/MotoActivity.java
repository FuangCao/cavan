package com.cavan.jwaootoy;

import android.annotation.SuppressLint;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.Spinner;

import com.cavan.android.CavanAndroid;
import com.cavan.resource.JwaooToyActivity;
import com.jwaoo.android.JwaooBleToy;

@SuppressLint("HandlerLeak")
public class MotoActivity extends JwaooToyActivity implements OnItemSelectedListener, OnSeekBarChangeListener {

	private Spinner mSpinnerMotoMode;
	private SeekBar mSeekBarStep;
	private SeekBar mSeekBarAddDelay;
	private SeekBar mSeekBarSubDelay;
	private SeekBar mSeekBarMaxSpeed;
	private SeekBar mSeekBarMinSpeed;

	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			applyMotoMode();
		}
	};

	@Override
	protected void onCreateBle(Bundle savedInstanceState) {
		setContentView(R.layout.activity_moto);

		mSpinnerMotoMode = (Spinner) findViewById(R.id.spinnerMotoMode);
		mSpinnerMotoMode.setOnItemSelectedListener(this);

		mSeekBarStep = (SeekBar) findViewById(R.id.seekBarStep);
		mSeekBarStep.setOnSeekBarChangeListener(this);

		mSeekBarAddDelay = (SeekBar) findViewById(R.id.seekBarAddDelay);
		mSeekBarAddDelay.setOnSeekBarChangeListener(this);

		mSeekBarSubDelay = (SeekBar) findViewById(R.id.seekBarSubDelay);
		mSeekBarSubDelay.setOnSeekBarChangeListener(this);

		mSeekBarMaxSpeed = (SeekBar) findViewById(R.id.seekBarMaxSpeed);
		mSeekBarMaxSpeed.setOnSeekBarChangeListener(this);

		mSeekBarMinSpeed = (SeekBar) findViewById(R.id.seekBarMinSpeed);
		mSeekBarMinSpeed.setOnSeekBarChangeListener(this);

		showScanActivity();
	}

	private boolean applyMotoMode() {
		int mode = mSpinnerMotoMode.getSelectedItemPosition();
		if (mode > 0) {
			mode += JwaooBleToy.MOTO_MODE_USER - 1;
		}

		int min = mSeekBarMinSpeed.getProgress();
		int max = mSeekBarMaxSpeed.getProgress();
		int step = mSeekBarStep.getProgress() + 1;
		int add_delay = mSeekBarAddDelay.getProgress() + 1;
		int sub_delay = mSeekBarSubDelay.getProgress() + 1;

		CavanAndroid.dLog("mode = " + mode);
		CavanAndroid.dLog("min = " + min);
		CavanAndroid.dLog("max = " + max);
		CavanAndroid.dLog("step = " + step);
		CavanAndroid.dLog("add_delay = " + add_delay);
		CavanAndroid.dLog("sub_delay = " + sub_delay);

		try {
			return mBleToy.setMotoMode(mode, min, max, step, add_delay, sub_delay);
		} catch (Exception e) {
			e.printStackTrace();
		}

		return false;
	}

	@Override
	public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
		CavanAndroid.dLog("onItemSelected: " + position);
		mHandler.sendEmptyMessage(0);
	}

	@Override
	public void onNothingSelected(AdapterView<?> parent) {
		CavanAndroid.dLog("onNothingSelected");
	}

	@Override
	public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
		CavanAndroid.dLog("onProgressChanged: " + progress);
		mHandler.sendEmptyMessage(0);
	}

	@Override
	public void onStartTrackingTouch(SeekBar seekBar) {
		CavanAndroid.dLog("onStartTrackingTouch");
	}

	@Override
	public void onStopTrackingTouch(SeekBar seekBar) {
		CavanAndroid.dLog("onStopTrackingTouch");
	}
}
