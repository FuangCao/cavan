package com.cavan.location;

import android.app.Activity;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;

import com.cavan.android.CavanAndroid;
import com.cavan.android.CavanBusyLock;
import com.cavan.cavanmain.R;
import com.cavan.java.CavanJava;
import com.cavan.java.CavanTcpPacketClient;

public class MockLocationActivity extends Activity implements OnClickListener {

	private EditText mEditTextHost;
	private EditText mEditTextPort;
	private EditText mEditTextLatitude;
	private EditText mEditTextLatitudeStep;
	private EditText mEditTextLongitude;
	private EditText mEditTextLongitudeStep;

	private Button mButtonConn;
	private Button mButtonLatitudeAdd;
	private Button mButtonLatitudeSub;
	private Button mButtonLongitudeAdd;
	private Button mButtonLongitudeSub;

	private double mLatitude;
	private double mLatitudeStep;
	private double mLongitude;
	private double mLongitudeStep;

	private CavanTcpPacketClient mTcpPacketClient = new CavanTcpPacketClient() {

		@Override
		protected boolean onPacketReceived(byte[] bytes, int length) {
			return true;
		}

		@Override
		protected boolean onTcpConnFailed(int times) {
			return false;
		}

		@Override
		public void prErrInfo(String message) {
			CavanAndroid.eLog(message);
		}

		@Override
		public void prWarnInfo(String message) {
			CavanAndroid.dLog(message);
		}

		@Override
		public void prDbgInfo(String message) {
			CavanAndroid.dLog(message);
		}
	};

	private CavanBusyLock mBusyLock = new CavanBusyLock(300);

	private boolean sendLocation() {
		CavanAndroid.dLog("mLatitude = " + mLatitude);
		CavanAndroid.dLog("Longitude = " + mLongitude);

		StringBuilder builder = new StringBuilder();
		builder.append("set_location ");
		builder.append(mLatitude).append(' ').append(mLongitude);

		CavanAndroid.dLog("command = " + builder);

		return false;
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_mock_location);

		mEditTextHost = (EditText) findViewById(R.id.editTextHost);
		mEditTextPort = (EditText) findViewById(R.id.editTextPort);
		mEditTextLatitude = (EditText) findViewById(R.id.editTextLatitude);
		mEditTextLatitudeStep = (EditText) findViewById(R.id.editTextLatitudeStep);
		mEditTextLongitude = (EditText) findViewById(R.id.editTextLongitude);
		mEditTextLongitudeStep = (EditText) findViewById(R.id.editTextLongitudeStep);

		mButtonConn = (Button) findViewById(R.id.buttonConn);
		mButtonLatitudeAdd = (Button) findViewById(R.id.buttonLatitudeAdd);
		mButtonLatitudeSub = (Button) findViewById(R.id.buttonLatitudeSub);
		mButtonLongitudeAdd = (Button) findViewById(R.id.buttonLongitudeAdd);
		mButtonLongitudeSub = (Button) findViewById(R.id.buttonLongitudeSub);

		mLatitude = CavanJava.parseDouble(mEditTextLatitude.getText().toString());
		mLatitudeStep = CavanJava.parseDouble(mEditTextLatitudeStep.getText().toString());
		mLongitude = CavanJava.parseDouble(mEditTextLongitude.getText().toString());
		mLongitudeStep = CavanJava.parseDouble(mEditTextLongitudeStep.getText().toString());

		mEditTextLatitude.addTextChangedListener(new TextWatcher() {

			@Override
			public void onTextChanged(CharSequence s, int start, int before, int count) {}

			@Override
			public void beforeTextChanged(CharSequence s, int start, int count, int after) {}

			@Override
			public void afterTextChanged(Editable s) {
				if (mBusyLock.acquire(mEditTextLatitude)) {
					mLatitude = CavanJava.parseDouble(s.toString());
					sendLocation();
				}
			}
		});

		mEditTextLatitudeStep.addTextChangedListener(new TextWatcher() {

			@Override
			public void onTextChanged(CharSequence s, int start, int before, int count) {}

			@Override
			public void beforeTextChanged(CharSequence s, int start, int count, int after) {}

			@Override
			public void afterTextChanged(Editable s) {
				mLatitudeStep = CavanJava.parseDouble(s.toString());
			}
		});

		mEditTextLongitude.addTextChangedListener(new TextWatcher() {

			@Override
			public void onTextChanged(CharSequence s, int start, int before, int count) {}

			@Override
			public void beforeTextChanged(CharSequence s, int start, int count, int after) {}

			@Override
			public void afterTextChanged(Editable s) {
				if (mBusyLock.acquire(mEditTextLongitude)) {
					mLongitude = CavanJava.parseDouble(s.toString());
					sendLocation();
				}
			}
		});

		mEditTextLongitudeStep.addTextChangedListener(new TextWatcher() {

			@Override
			public void onTextChanged(CharSequence s, int start, int before, int count) {}

			@Override
			public void beforeTextChanged(CharSequence s, int start, int count, int after) {}

			@Override
			public void afterTextChanged(Editable s) {
				mLongitudeStep = CavanJava.parseDouble(s.toString());
			}
		});

		mButtonConn.setOnClickListener(this);
		mButtonLatitudeAdd.setOnClickListener(this);
		mButtonLatitudeSub.setOnClickListener(this);
		mButtonLongitudeAdd.setOnClickListener(this);
		mButtonLongitudeSub.setOnClickListener(this);
	}

	@Override
	public void onClick(View v) {
		if (v == mButtonConn) {
			if (mTcpPacketClient.isConnDisabled()) {
				int port = CavanJava.parseInt(mEditTextPort.getText().toString());
				String host = mEditTextHost.getText().toString();
				mTcpPacketClient.connect(host, port);
				mButtonConn.setText(R.string.disconnect);
			} else {
				mTcpPacketClient.disconnect();
				mButtonConn.setText(R.string.connect);
			}
		} else if (v == mButtonLatitudeAdd) {
			mLatitude += mLatitudeStep;
			sendLocation();
			mBusyLock.acquire(v);
			mEditTextLatitude.setText(Double.toString(mLatitude));
		} else if (v == mButtonLatitudeSub) {
			mLatitude -= mLatitudeStep;
			sendLocation();
			mBusyLock.acquire(v);
			mEditTextLatitude.setText(Double.toString(mLatitude));
		} else if (v == mButtonLongitudeAdd) {
			mLongitude += mLongitudeStep;
			sendLocation();
			mBusyLock.acquire(v);
			mEditTextLongitude.setText(Double.toString(mLongitude));
		} else if (v == mButtonLongitudeSub) {
			mLongitude -= mLongitudeStep;
			sendLocation();
			mBusyLock.acquire(v);
			mEditTextLongitude.setText(Double.toString(mLongitude));
		}
	}
}
