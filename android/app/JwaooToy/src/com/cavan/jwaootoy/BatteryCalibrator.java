package com.cavan.jwaootoy;

import java.io.FileOutputStream;
import java.io.IOException;

import android.Manifest;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.EditText;
import android.widget.Spinner;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanBusyLock;
import com.cavan.java.CavanFile;
import com.cavan.resource.JwaooToyActivity;
import com.jwaoo.android.JwaooBleToy.JwaooToyBatteryInfo;

public class BatteryCalibrator extends JwaooToyActivity implements OnItemSelectedListener, OnCheckedChangeListener, OnClickListener {

	private static final int TOGGLE_OPEN_TIME = 600000;
	private static final int TOGGLE_CLOSE_TIME = 60000;

	private static final int MSG_SET_TOGGLE = 1;
	private static final int MSG_APPEND_LOG = 2;
	private static final int MSG_CONNECTED = 3;
	private static final int MSG_THREAD_ENTER = 4;
	private static final int MSG_THREAD_EXIT = 5;

	private Spinner mSpinnerPort;
	private Spinner mSpinnerPin;
	private CheckBox mCheckboxToggle;
	private Button mButtonStart;
	private Button mButtonStop;
	private EditText mEditTextLog;
	private BatteryCalibrationThread mThread;
	private CavanBusyLock mLock = new CavanBusyLock(1000);

	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			removeMessages(msg.what);

			switch (msg.what) {
			case MSG_SET_TOGGLE:
				mCheckboxToggle.setChecked(msg.arg1 > 0);
				break;

			case MSG_APPEND_LOG:
				mEditTextLog.append((CharSequence) msg.obj);
				break;

			case MSG_CONNECTED:
				if (mLock.acquire(this)) {
					getGpioValue();
				}
				break;

			case MSG_THREAD_ENTER:
				mCheckboxToggle.setEnabled(false);
				mSpinnerPin.setEnabled(false);
				mSpinnerPort.setEnabled(false);
				mButtonStart.setEnabled(false);
				mButtonStop.setEnabled(true);
				break;

			case MSG_THREAD_EXIT:
				mCheckboxToggle.setEnabled(true);
				mSpinnerPin.setEnabled(true);
				mSpinnerPort.setEnabled(true);
				mButtonStart.setEnabled(true);
				mButtonStop.setEnabled(false);
				break;
			}
		}
	};

	public boolean getGpioValue() {
		try {
			mCheckboxToggle.setChecked(mBleToy.getGpioValue(mSpinnerPort.getSelectedItemPosition(), mSpinnerPin.getSelectedItemPosition()));
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}

		return true;
	}

	public boolean setGpioValue(boolean value) {
		try {
			mBleToy.setGpioValue(mSpinnerPort.getSelectedItemPosition(), mSpinnerPin.getSelectedItemPosition(), value);
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}

		return true;
	}

	@Override
	public boolean onInitialize() {
		try {
			mBleToy.setBatteryEventEnable(true);
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}

		return super.onInitialize();
	}

	@Override
	public void onConnectionStateChanged(boolean connected) {
		super.onConnectionStateChanged(connected);

		if (connected) {
			mHandler.sendEmptyMessage(MSG_CONNECTED);
		}
	}

	@Override
	protected void onCreateBle(Bundle savedInstanceState) {
		setContentView(R.layout.battery_calibrator);

		mSpinnerPort = (Spinner) findViewById(R.id.spinnerGpioPort);
		mSpinnerPin = (Spinner) findViewById(R.id.spinnerGpioPin);
		mCheckboxToggle = (CheckBox) findViewById(R.id.checkBoxToggle);
		mButtonStart = (Button) findViewById(R.id.buttonStart);
		mButtonStop = (Button) findViewById(R.id.buttonStop);
		mEditTextLog = (EditText) findViewById(R.id.editTextLog);

		mSpinnerPort.setSelection(2);
		mSpinnerPin.setSelection(7);
		mButtonStop.setEnabled(false);

		mSpinnerPort.setOnItemSelectedListener(this);
		mSpinnerPin.setOnItemSelectedListener(this);
		mCheckboxToggle.setOnCheckedChangeListener(this);
		mButtonStart.setOnClickListener(this);
		mButtonStop.setOnClickListener(this);

		showScanActivity();
	}

	@Override
	public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
		if (mLock.acquire(view)) {
			getGpioValue();
		}
	}

	@Override
	public void onNothingSelected(AdapterView<?> parent) {
	}

	@Override
	public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
		if (mLock.acquire(buttonView)) {
			setGpioValue(isChecked);
		}
	}

	@Override
	public void onClick(View v) {
		if (v == mButtonStart) {
			if (CavanAndroid.checkAndRequestPermissions(this, new String[] { Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE })) {
				if (mThread == null) {
					mEditTextLog.getEditableText().clear();
					mThread = new BatteryCalibrationThread();
					mThread.start();
				}
			}
		} else if (v == mButtonStop) {
			if (mThread != null) {
				BatteryCalibrationThread thread = mThread;
				mThread = null;

				synchronized (thread) {
					thread.notifyAll();
				}
			}
		}
	}

	public class BatteryCalibrationThread extends Thread {

		@Override
		public void run() {
			mHandler.sendEmptyMessage(MSG_THREAD_ENTER);

			do {
				CavanFile file = new CavanFile(Environment.getExternalStorageDirectory(), "model06_battery.txt");
				FileOutputStream stream = file.openOutputStream();
				if (stream == null) {
					CavanAndroid.eLog("openOutputStream");
					break;
				}

				long time = System.currentTimeMillis();

				while (mThread != null) {
					try {
						mHandler.obtainMessage(MSG_SET_TOGGLE, 1, 0).sendToTarget();

						synchronized (this) {
							wait(TOGGLE_OPEN_TIME);
							if (mThread == null) {
								break;
							}
						}

						JwaooToyBatteryInfo info0 = mBleToy.getBatteryInfo();
						CavanAndroid.dLog("info0 = " + info0);

						mHandler.obtainMessage(MSG_SET_TOGGLE, 0, 0).sendToTarget();

						synchronized (this) {
							wait(TOGGLE_CLOSE_TIME);
							if (mThread == null) {
								break;
							}
						}

						JwaooToyBatteryInfo info1 = mBleToy.getBatteryInfo();
						CavanAndroid.dLog("info1 = " + info1);

						String text = "{ " + info0.getVoltage() + ", "+ info1.getVoltage() +" }, // " + (System.currentTimeMillis() - time) + "\n";
						mHandler.obtainMessage(MSG_APPEND_LOG, text).sendToTarget();

						stream.write(text.getBytes());
						stream.flush();
					} catch (InterruptedException e) {
						e.printStackTrace();
					} catch (Exception e) {
						e.printStackTrace();
						break;
					}
				}

				try {
					stream.close();
				} catch (IOException e) {
					e.printStackTrace();
				}

			} while (false);

			mHandler.sendEmptyMessage(MSG_THREAD_EXIT);
			mThread = null;
		}
	}
}
