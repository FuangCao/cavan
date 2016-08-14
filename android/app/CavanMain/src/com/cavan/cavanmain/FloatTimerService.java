package com.cavan.cavanmain;

import java.util.Calendar;

import android.content.Intent;
import android.graphics.Color;
import android.os.IBinder;
import android.os.RemoteException;
import android.view.View;
import android.widget.TextView;

import com.cavan.android.FloatWidowService;

public class FloatTimerService extends FloatWidowService {

	public static final float TIME_TEXT_SIZE = 16;
	public static final boolean TIME_TEXT_BOLD = false;
	public static final boolean SHOW_SECOND_ONLY = false;
	public static final int TIME_TEXT_COLOR = Color.WHITE;

	private int mLastSecond;
	private TextView mTimeView;

	private Runnable mRunnableTime = new Runnable() {

		@Override
		public void run() {
			if (mTimeView != null) {
				Calendar calendar = Calendar.getInstance();
				int second = calendar.get(Calendar.SECOND);
				if (second == mLastSecond) {
					mTimeView.postDelayed(this, 100);
				} else {
					mLastSecond = second;
					mTimeView.postDelayed(this, 1000);

					if (SHOW_SECOND_ONLY) {
						mTimeView.setText(String.format(" %02d ", second));
					} else {
						int hour = calendar.get(Calendar.HOUR_OF_DAY);
						int minute = calendar.get(Calendar.MINUTE);

						mTimeView.setText(String.format(" %02d:%02d:%02d ", hour, minute, second));
					}
				}
			}
		}
	};

	private IFloatTimerService.Stub mBinder = new IFloatTimerService.Stub() {

		@Override
		public boolean setEnable(boolean enable) throws RemoteException {
			if (enable) {
				if (mTimeView == null) {
					mTimeView = (TextView) addText(null, -1);
					if (mTimeView == null) {
						return false;
					}

					mTimeView.post(mRunnableTime);
				}
			} else if (mTimeView != null) {
				removeView(mTimeView);
				mTimeView = null;
			}

			return true;
		}

		@Override
		public boolean getState() throws RemoteException {
			return mTimeView != null;
		}
	};

	@Override
	public void onDestroy() {
		try {
			mBinder.setEnable(false);
		} catch (RemoteException e) {
			e.printStackTrace();
		}

		super.onDestroy();
	}

	@Override
	public IBinder onBind(Intent intent) {
		return mBinder;
	}

	@Override
	protected View createView(CharSequence text) {
		TextView view = new TextView(getApplicationContext());
		view.setBackgroundResource(R.drawable.desktop_timer_bg);
		view.setTextColor(TIME_TEXT_COLOR);

		if (TIME_TEXT_BOLD) {
			view.getPaint().setFakeBoldText(true);
		}

		if (TIME_TEXT_SIZE > 0) {
			view.setTextSize(TIME_TEXT_SIZE);
		}

		view.setText(text);

		return view;
	}

	@Override
	protected View findView(CharSequence text) {
		for (View view : mViewMap.values()) {
			if (text.equals(((TextView) view).getText())) {
				return view;
			}
		}

		return null;
	}
}
