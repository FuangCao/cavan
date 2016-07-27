package com.cavan.jwaootoy;

import java.util.ArrayList;
import java.util.List;

import android.annotation.SuppressLint;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.EditText;
import android.widget.ProgressBar;
import android.widget.Spinner;

import com.cavan.android.CavanAndroid;
import com.cavan.java.CavanProgressListener;
import com.cavan.resource.JwaooToyActivity;

@SuppressLint("HandlerLeak")
public class MainActivity extends JwaooToyActivity implements OnClickListener, OnCheckedChangeListener {

	private static final int EVENT_OTA_START = 3;
	private static final int EVENT_OTA_FAILED = 4;
	private static final int EVENT_OTA_SUCCESS = 5;
	private static final int EVENT_PROGRESS_UPDATED = 6;
	private static final int EVENT_FREQ_CHANGED = 7;
	private static final int EVENT_DEPTH_CHANGED = 8;
	private static final int EVENT_CONNECTED = 9;
	private static final int EVENT_DISCONNECTED = 10;

	private int mFreq;
	private int mDepth;
	private boolean mOtaBusy;

	private Button mButtonSend;
	private Button mButtonUpgrade;
	private Button mButtonReboot;
	private Button mButtonDisconnect;
	private Button mButtonReadBdAddr;
	private Button mButtonWriteBdAddr;

	private CheckBox mCheckBoxSensor;
	private CheckBox mCheckBoxClick;
	private CheckBox mCheckBoxLongClick;
	private CheckBox mCheckBoxMultiClick;

	private ProgressBar mProgressBar;
	private EditText mEditTextBdAddr;
	private Spinner mSpinnerMotoMode;
	private Spinner mSpinnerMotoLevel;

	private int mMotoMode;
	private int mMotoLevel = 1;

	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case EVENT_OTA_START:
				updateUI(false);
				CavanAndroid.showToast(getApplicationContext(), R.string.text_upgrade_start);
				break;

			case EVENT_OTA_FAILED:
				mButtonUpgrade.setEnabled(true);
				mButtonSend.setEnabled(true);
				CavanAndroid.showToast(getApplicationContext(), R.string.text_upgrade_failed);
				break;

			case EVENT_OTA_SUCCESS:
				updateUI(true);
				CavanAndroid.showToast(getApplicationContext(), R.string.text_upgrade_successfull);
				break;

			case EVENT_PROGRESS_UPDATED:
				mProgressBar.setProgress(msg.arg1);
				break;

			case EVENT_FREQ_CHANGED:
			case EVENT_DEPTH_CHANGED:
				setTitle("Depth = " + mDepth + ", Freq = " + mFreq);
				break;

			case EVENT_CONNECTED:
				updateUI(true);
				break;

			case EVENT_DISCONNECTED:
				updateUI(false);
				showScanActivity();
				break;
			}
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		mButtonSend = (Button) findViewById(R.id.buttonSend);
		mButtonSend.setOnClickListener(this);

		mButtonUpgrade = (Button) findViewById(R.id.buttonUpgrade);
		mButtonUpgrade.setOnClickListener(this);

		mButtonReboot = (Button) findViewById(R.id.buttonReboot);
		mButtonReboot.setOnClickListener(this);

		mButtonDisconnect = (Button) findViewById(R.id.buttonDisconnect);
		mButtonDisconnect.setOnClickListener(this);

		mButtonReadBdAddr = (Button) findViewById(R.id.buttonReadBdAddr);
		mButtonReadBdAddr.setOnClickListener(this);

		mButtonWriteBdAddr = (Button) findViewById(R.id.buttonWriteBdAddr);
		mButtonWriteBdAddr.setOnClickListener(this);

		mCheckBoxSensor = (CheckBox) findViewById(R.id.checkBoxSensor);
		mCheckBoxSensor.setOnCheckedChangeListener(this);

		mCheckBoxClick = (CheckBox) findViewById(R.id.checkBoxClick);
		mCheckBoxClick.setOnCheckedChangeListener(this);

		mCheckBoxLongClick = (CheckBox) findViewById(R.id.checkBoxLongClick);
		mCheckBoxLongClick.setOnCheckedChangeListener(this);

		mCheckBoxMultiClick = (CheckBox) findViewById(R.id.checkBoxMultiClick);
		mCheckBoxMultiClick.setOnCheckedChangeListener(this);

		mProgressBar = (ProgressBar) findViewById(R.id.progressBarUpgrade);
		mEditTextBdAddr = (EditText) findViewById(R.id.editTextBdAddr);

		List<CharSequence> list = new ArrayList<CharSequence>();
		String text = getResources().getString(R.string.text_moto_close);
		list.add(text);

		text = getResources().getString(R.string.text_moto_mode);

		for (int i = 1; i <= 6; i++) {
			list.add(text + i);
		}

		ArrayAdapter<CharSequence> adapter = new ArrayAdapter<CharSequence>(this, android.R.layout.simple_spinner_item, list);
		adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
		mSpinnerMotoMode = (Spinner) findViewById(R.id.spinnerMotoMode);
		mSpinnerMotoMode.setAdapter(adapter);
		mSpinnerMotoMode.setOnItemSelectedListener(new OnItemSelectedListener() {

			@Override
			public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
				mMotoMode = position;
				setMotoMode();
			}

			@Override
			public void onNothingSelected(AdapterView<?> parent) {}
		});

		list = new ArrayList<CharSequence>();
		text = getResources().getString(R.string.text_moto_level);
		for (int i = 1; i <= 18; i++) {
			list.add(text + i);
		}

		adapter = new ArrayAdapter<CharSequence>(this, android.R.layout.simple_spinner_item, list);
		adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
		mSpinnerMotoLevel = (Spinner) findViewById(R.id.spinnerMotoLevel);
		mSpinnerMotoLevel.setAdapter(adapter);
		mSpinnerMotoLevel.setOnItemSelectedListener(new OnItemSelectedListener() {

			@Override
			public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
				mMotoLevel = position + 1;
				if (mMotoMode == 1) {
					setMotoMode();
				}
			}

			@Override
			public void onNothingSelected(AdapterView<?> parent) {}
		});

		showScanActivity();
	}

	private void setUpgradeProgress(int progress) {
		mHandler.obtainMessage(EVENT_PROGRESS_UPDATED, progress, 0).sendToTarget();
	}

	private boolean setMotoMode() {
		if (mBleToy != null && mBleToy.isConnected()) {
			boolean success = mBleToy.setMotoMode(mMotoMode, mMotoLevel);

			if (success) {
				CavanAndroid.logE("Moto: mode = " + mMotoMode + ", level = " + mMotoLevel);
			} else {
				CavanAndroid.logE("Failed to setMotoMode");
				CavanAndroid.dumpstack();
			}

			return success;
		}

		return false;
	}

	@Override
	public void onClick(View v) {
		switch (v.getId()) {
		case R.id.buttonSend:
			String identify = mBleToy.doIdentify();
			if (identify == null) {
				break;
			}

			CavanAndroid.logE("identify = " + identify);

			String buildDate = mBleToy.readBuildDate();
			if (buildDate == null) {
				break;
			}

			CavanAndroid.logE("buildDate = " + buildDate);

			int version = mBleToy.readVersion();
			if (version == 0) {
				break;
			}

			CavanAndroid.logE("version = " + Integer.toHexString(version));
			break;

		case R.id.buttonUpgrade:
			if (mOtaBusy) {
				break;
			}

			mOtaBusy = true;

			new Thread() {

				@Override
				public void run() {
					mHandler.sendEmptyMessage(EVENT_OTA_START);
					if (mBleToy.doOtaUpgrade("/mnt/sdcard/jwaoo-toy.hex", new CavanProgressListener() {

						@Override
						public void onProgressUpdated(int progress) {
							setUpgradeProgress(progress);
						}
					})) {
						mHandler.sendEmptyMessage(EVENT_OTA_SUCCESS);
					} else {
						mHandler.sendEmptyMessage(EVENT_OTA_FAILED);
					}

					mOtaBusy = false;
				}
			}.start();
			break;

		case R.id.buttonReboot:
			mBleToy.doReboot();
			break;

		case R.id.buttonDisconnect:
			if (mBleToy != null && mBleToy.isConnected()) {
				mBleToy.disconnect();
			} else {
				showScanActivity();
			}
			break;

		case R.id.buttonReadBdAddr:
			String addr = mBleToy.readBdAddressString();
			if (addr != null) {
				mEditTextBdAddr.setText(addr);
			} else {
				CavanAndroid.logE("Failed to readBdAddress");
			}
			break;

		case R.id.buttonWriteBdAddr:
			if (mBleToy.writeBdAddress(mEditTextBdAddr.getText().toString())) {
				CavanAndroid.logE("writeBdAddress successfull");
			} else {
				CavanAndroid.logE("Failed to writeBdAddress");
			}
			break;
		}
	}

	@Override
	protected boolean onInitialize() {
		if (!mBleToy.setSensorEnable(mCheckBoxSensor.isChecked(), SENSOR_DELAY)) {
			CavanAndroid.logE("Failed to setSensorEnable");
			return false;
		}

		if (mBleToy.setClickEnable(mCheckBoxClick.isChecked()) == false && mBleToy.isCommandTimeout()) {
			CavanAndroid.logE("Failed to setClickEnable");
			return false;
		}

		if (mBleToy.setLongClickEnable(mCheckBoxLongClick.isChecked()) == false && mBleToy.isCommandTimeout()) {
			CavanAndroid.logE("Failed to setLongClickEnable");
			return false;
		}

		if (mBleToy.setMultiClickEnable(mCheckBoxMultiClick.isChecked()) == false && mBleToy.isCommandTimeout()) {
			CavanAndroid.logE("Failed to setMultiClickEnable");
			return false;
		}

		if (mMotoMode > 0) {
			if (setMotoMode() == false && mBleToy.isCommandTimeout()) {
				return false;
			}
		}

		return true;
	}

	@Override
	protected void onConnected() {
		mHandler.sendEmptyMessage(EVENT_CONNECTED);
	}

	@Override
	protected void onSensorDataReceived(byte[] data) {
		mDepth = mBleToy.getDepth();
		mFreq = mBleToy.getFreq();
		mHandler.sendEmptyMessage(EVENT_FREQ_CHANGED);
	}

	@Override
	public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
		switch (buttonView.getId()) {
		case R.id.checkBoxSensor:
			mBleToy.setSensorEnable(isChecked, SENSOR_DELAY);
			break;

		case R.id.checkBoxClick:
			mBleToy.setClickEnable(isChecked);
			break;

		case R.id.checkBoxLongClick:
			mBleToy.setLongClickEnable(isChecked);
			break;

		case R.id.checkBoxMultiClick:
			mBleToy.setMultiClickEnable(isChecked);
			break;
		}
	}
}
