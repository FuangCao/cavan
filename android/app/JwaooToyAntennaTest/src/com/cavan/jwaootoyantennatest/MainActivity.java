package com.cavan.jwaootoyantennatest;

import java.util.Random;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;

import com.cavan.java.CavanThread;
import com.cavan.resource.JwaooToyActivity;
import com.jwaoo.android.JwaooBleToy;
import com.jwaoo.android.JwaooToySensor;

public class MainActivity extends JwaooToyActivity implements OnClickListener {

	private static final int MSG_UPDATE_SPEED = 1;
	private static final int MSG_UPDATE_SETTINGS = 2;

	private TextView mTextViewStatus;
	private Button mButtonStartStop;
	private boolean mEnabled;
	private int mSendTimes;
	private int mSendSpeed;
	private int mRecvTimes;
	private int mRecvSpeed;

	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case MSG_UPDATE_SPEED:
				removeMessages(MSG_UPDATE_SPEED);

				if (mEnabled) {
					sendEmptyMessageDelayed(MSG_UPDATE_SPEED, 1000);

					mSendSpeed = mSendTimes;
					mSendTimes = 0;

					mRecvSpeed = mRecvTimes;
					mRecvTimes = 0;

					mTextViewStatus.setText("send: " + mSendSpeed + ", recv: " + mRecvSpeed);
				} else {
					mTextViewStatus.setText(R.string.please_press_start);
				}
				break;

			case MSG_UPDATE_SETTINGS:
				try {
					if (mEnabled) {
						mBleToy.setSensorEnable(true, 20);
						mSendThread.wakeup();
						mButtonStartStop.setText(R.string.stop);
						mHandler.sendEmptyMessage(MSG_UPDATE_SPEED);
					} else {
						mBleToy.setSensorEnable(false);
						mButtonStartStop.setText(R.string.start);
					}
				} catch (Exception e) {
					e.printStackTrace();
				}
				break;
			}
		}
	};

	private CavanThread mSendThread = new CavanThread() {

		@Override
		public void run() {
			Random random = new Random();

			while (true) {
				while (mEnabled && mBleToy.isConnected()) {
					int speed = random.nextInt(JwaooBleToy.MOTO_LEVEL_MAX) + 1;

					try {
						mBleToy.setMotoMode(JwaooBleToy.MOTO_MODE_LINE, speed);
					} catch (Exception e) {
						e.printStackTrace();
						break;
					}

					mSendTimes++;
				}

				mEnabled = false;
				mHandler.sendEmptyMessage(MSG_UPDATE_SETTINGS);

				try {
					mBleToy.setMotoMode(JwaooBleToy.MOTO_MODE_IDLE, 0);
				} catch (Exception e) {
					e.printStackTrace();
				}

				synchronized (this) {
					try {
						wait();
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
			}
		}
	};

	@Override
	protected void onCreateBle(Bundle savedInstanceState) {
		setContentView(R.layout.activity_main);

		mTextViewStatus = (TextView) findViewById(R.id.textViewStatus);

		mButtonStartStop = (Button) findViewById(R.id.buttonStartStop);
		mButtonStartStop.setOnClickListener(this);

		showScanActivity();
	}

	@Override
	public void onClick(View v) {
		try {
			mEnabled = !mEnabled;
			mHandler.sendEmptyMessage(MSG_UPDATE_SETTINGS);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	@Override
	public void onConnectionStateChanged(boolean connected) {
		super.onConnectionStateChanged(connected);

		if (connected) {
			mEnabled = false;
		}

		mHandler.sendEmptyMessage(MSG_UPDATE_SETTINGS);
	}

	@Override
	public void onSensorDataReceived(JwaooToySensor sensor, byte[] data) {
		mRecvTimes++;
	}
}
