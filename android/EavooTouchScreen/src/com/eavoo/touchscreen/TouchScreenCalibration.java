package com.eavoo.touchscreen;

import android.content.Context;
import android.os.Handler;
import android.os.Message;
import android.preference.PreferenceScreen;
import android.util.Log;

public class TouchScreenCalibration extends Handler
{
	private static final String TAG = "TouchScreenCalibration";

	private static final int MSG_CALIBRATION_COUNT_DOWN = 1;
	private static final int MSG_CALIBRATION_START = 2;
	private static final int MSG_CALIBRATIION_RUNNING = 3;
	private static final int MSG_CALIBRATIION_SUCCESS = 4;
	private static final int MSG_CALIBRATIION_FAILED = 5;

	private final PreferenceScreen mPreferenceScreen;
	private final TouchScreen mTouchScreen;
	private Context mContext;
	private boolean mPendding = false;

	public TouchScreenCalibration(Context context, TouchScreen touchscreen, PreferenceScreen preference)
	{
		this.mContext = context;
		this.mPreferenceScreen = preference;
		this.mTouchScreen = touchscreen;

		this.mPreferenceScreen.setPersistent(false);
	}

	@Override
	public void handleMessage(Message msg)
	{
		switch (msg.what)
		{
		case MSG_CALIBRATION_START:
			msg = Message.obtain(this, MSG_CALIBRATION_COUNT_DOWN, 5, 0);
			sendMessage(msg);
			break;
		case MSG_CALIBRATION_COUNT_DOWN:
			if (msg.arg1 > 0)
			{
				mPreferenceScreen.setSummary(mContext.getResources().getString(R.string.calibration_summary_start, msg.arg1));

				int count = msg.arg1 - 1;
				msg = Message.obtain(this, MSG_CALIBRATION_COUNT_DOWN, count, 0);
				sendMessageDelayed(msg, 1000);
			}
			else
			{
				Thread thread = new Thread(mRunnableCalibration);
				thread.start();
			}
			break;
		case MSG_CALIBRATIION_RUNNING:
			mPreferenceScreen.setSummary(R.string.calibration_summary_running);
			break;
		case MSG_CALIBRATIION_SUCCESS:
			mPreferenceScreen.setSummary(R.string.calibration_summary_success);
			mPendding = false;
			break;
		case MSG_CALIBRATIION_FAILED:
			mPreferenceScreen.setSummary(R.string.calibration_summary_failed);
			mPendding = false;
			break;
		}

		super.handleMessage(msg);
	}

	private Runnable mRunnableCalibration = new Runnable()
	{
		@Override
		public void run()
		{
			Log.i(TAG, "start calibration");

			sendEmptyMessage(MSG_CALIBRATIION_RUNNING);

			if (mTouchScreen.CalibrationNative())
			{
				sendEmptyMessage(MSG_CALIBRATIION_SUCCESS);
				Log.e(TAG, "calibration success");
			}
			else
			{
				sendEmptyMessage(MSG_CALIBRATIION_FAILED);
				Log.i(TAG, "calibration failed");
			}
		}
	};

	public void start()
	{
		mPendding = true;
		sendEmptyMessage(MSG_CALIBRATION_START);
	}

	public boolean getPendding()
	{
		return mPendding;
	}
}
