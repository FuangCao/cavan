package com.cavan.jwaootoy;

import android.annotation.SuppressLint;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.EditText;
import android.widget.ProgressBar;

import com.cavan.android.CavanAndroid;
import com.cavan.java.CavanProgressListener;
import com.cavan.resource.JwaooToyActivity;
import com.jwaoo.android.JwaooBleToy;

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
	private Button mButtonMotoUp;
	private Button mButtonMotoDown;
	private Button mButtonCapacityUp;
	private Button mButtonCapacityDown;

	private CheckBox mCheckBoxSensor;
	private CheckBox mCheckBoxClick;
	private CheckBox mCheckBoxLongClick;
	private CheckBox mCheckBoxMultiClick;

	private ProgressBar mProgressBar;
	private EditText mEditTextBdAddr;

	private int mMotoLevel;
	private int mCapacityOffset;

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
		mListViews.add(mButtonSend);

		mButtonUpgrade = (Button) findViewById(R.id.buttonUpgrade);
		mButtonUpgrade.setOnClickListener(this);
		mListViews.add(mButtonUpgrade);

		mButtonReboot = (Button) findViewById(R.id.buttonReboot);
		mButtonReboot.setOnClickListener(this);
		mListViews.add(mButtonReboot);

		mButtonDisconnect = (Button) findViewById(R.id.buttonDisconnect);
		mButtonDisconnect.setOnClickListener(this);
		mListViews.add(mButtonDisconnect);

		mButtonReadBdAddr = (Button) findViewById(R.id.buttonReadBdAddr);
		mButtonReadBdAddr.setOnClickListener(this);
		mListViews.add(mButtonReadBdAddr);

		mButtonWriteBdAddr = (Button) findViewById(R.id.buttonWriteBdAddr);
		mButtonWriteBdAddr.setOnClickListener(this);
		mListViews.add(mButtonWriteBdAddr);

		mButtonMotoUp = (Button) findViewById(R.id.buttonMotoUp);
		mButtonMotoUp.setOnClickListener(this);
		mListViews.add(mButtonMotoUp);

		mButtonMotoDown = (Button) findViewById(R.id.buttonMotoDown);
		mButtonMotoDown.setOnClickListener(this);
		mListViews.add(mButtonMotoDown);

		mButtonCapacityUp = (Button) findViewById(R.id.buttonCapacityUp);
		mButtonCapacityUp.setOnClickListener(this);
		mListViews.add(mButtonCapacityUp);

		mButtonCapacityDown = (Button) findViewById(R.id.buttonCapacityDown);
		mButtonCapacityDown.setOnClickListener(this);
		mListViews.add(mButtonCapacityDown);

		mCheckBoxSensor = (CheckBox) findViewById(R.id.checkBoxSensor);
		mCheckBoxSensor.setOnCheckedChangeListener(this);
		mListViews.add(mCheckBoxSensor);

		mCheckBoxClick = (CheckBox) findViewById(R.id.checkBoxClick);
		mCheckBoxClick.setOnCheckedChangeListener(this);
		mListViews.add(mCheckBoxClick);

		mCheckBoxLongClick = (CheckBox) findViewById(R.id.checkBoxLongClick);
		mCheckBoxLongClick.setOnCheckedChangeListener(this);
		mListViews.add(mCheckBoxLongClick);

		mCheckBoxMultiClick = (CheckBox) findViewById(R.id.checkBoxMultiClick);
		mCheckBoxMultiClick.setOnCheckedChangeListener(this);
		mListViews.add(mCheckBoxMultiClick);

		mProgressBar = (ProgressBar) findViewById(R.id.progressBar1);
		mListViews.add(mProgressBar);

		mEditTextBdAddr = (EditText) findViewById(R.id.editTextBdAddr);
		mListViews.add(mEditTextBdAddr);

		showScanActivity();
	}

	private void setUpgradeProgress(int progress) {
		mHandler.obtainMessage(EVENT_PROGRESS_UPDATED, progress, 0).sendToTarget();
	}

	private void setMotoLevel(int level) {
		if (mBleToy.setMotoLevel(level)) {
			mMotoLevel = level;
		} else {
			CavanAndroid.logE("Failed to setMotoLevel");
		}

		CavanAndroid.logE("mMotoLevel = " + mMotoLevel);
	}

	private void setCapacityValue(int value) {
		if (mFdc1004 != null && mFdc1004.setCapacityDac(value)) {
			mCapacityOffset = value;
			mFdc1004.setEnable(false);
			mFdc1004.setEnable(true);
		} else {
			CavanAndroid.logE("Failed to setOffset");
		}

		CavanAndroid.logE("mCapacityOffset = " + mCapacityOffset);
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

		case R.id.buttonMotoUp:
			if (mMotoLevel < JwaooBleToy.MOTO_LEVEL_MAX) {
				setMotoLevel(mMotoLevel + 1);
			} else {
				CavanAndroid.logE("Nothing to be done");
			}
			break;

		case R.id.buttonMotoDown:
			if (mMotoLevel > 0) {
				setMotoLevel(mMotoLevel - 1);
			} else {
				CavanAndroid.logE("Nothing to be done");
			}
			break;

		case R.id.buttonCapacityUp:
			setCapacityValue(mCapacityOffset + 1);
			break;

		case R.id.buttonCapacityDown:
			setCapacityValue(mCapacityOffset - 1);
			break;
		}
	}

	@Override
	protected boolean onInitialize() {
		CavanAndroid.logE("mBleToy = " + mBleToy);
		if (!mBleToy.setSensorEnable(mCheckBoxSensor.isChecked(), SENSOR_DELAY)) {
			CavanAndroid.logE("Failed to setSensorEnable");
			return false;
		}

		if (!mBleToy.setClickEnable(mCheckBoxClick.isChecked())) {
			CavanAndroid.logE("Failed to setClickEnable");
			return false;
		}

		if (!mBleToy.setLongClickEnable(mCheckBoxLongClick.isChecked())) {
			CavanAndroid.logE("Failed to setLongClickEnable");
			return false;
		}

		if (!mBleToy.setMultiClickEnable(mCheckBoxMultiClick.isChecked())) {
			CavanAndroid.logE("Failed to setMultiClickEnable");
			return false;
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
