package com.cavan.cavanmain;

import java.util.Calendar;

import android.content.Intent;
import android.graphics.Color;
import android.os.IBinder;
import android.os.RemoteException;
import android.view.Gravity;
import android.view.View;
import android.widget.TextView;

import com.cavan.android.FloatWidowService;

public class FloatTimerService extends FloatWidowService {

	public static final int TEXT_PADDING = 10;

	public static final float TIME_TEXT_SIZE = 16;
	public static final int TIME_TEXT_COLOR = Color.WHITE;
	public static final boolean TIME_TEXT_BOLD = false;
	public static final boolean TIME_SHOW_SECOND_ONLY = false;

	public static final float MESSAGE_TEXT_SIZE = 12;
	public static final int MESSAGE_TEXT_COLOR = Color.YELLOW;

	private int mLastSecond;
	private TextView mTimeView;

	private IFloatTimerService.Stub mBinder = new IFloatTimerService.Stub() {

		@Override
		public boolean setTimerEnable(boolean enable) throws RemoteException {
			return FloatTimerService.this.setTimerEnable(enable);
		}

		@Override
		public boolean getTimerState() throws RemoteException {
			return mTimeView != null;
		}

		@Override
		public int addMessage(CharSequence message) throws RemoteException {
			TextView view = (TextView) FloatTimerService.this.addText(message, -1);
			if (view == null) {
				return -1;
			}

			view.setTextSize(MESSAGE_TEXT_SIZE);
			view.setTextColor(MESSAGE_TEXT_COLOR);
			view.setGravity(Gravity.LEFT | Gravity.CENTER_VERTICAL);

			return view.getId();
		}

		@Override
		public boolean hasMessage(CharSequence message) throws RemoteException {
			return FloatTimerService.this.hasText(message);
		}

		@Override
		public void removeMessage(CharSequence message) throws RemoteException {
			FloatTimerService.this.removeText(message);
		}
	};

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
					mTimeView.setText(getTimeText(calendar, second));
				}
			}
		}
	};

	public String getTimeText(Calendar calendar, int second) {
		if (TIME_SHOW_SECOND_ONLY) {
			return String.format("%02d", second);
		} else {
			int hour = calendar.get(Calendar.HOUR_OF_DAY);
			int minute = calendar.get(Calendar.MINUTE);

			return String.format("%02d:%02d:%02d", hour, minute, second);
		}
	}

	public String getTimeText(Calendar calendar) {
		return getTimeText(calendar, calendar.get(Calendar.SECOND));
	}

	public String getTimeText() {
		return getTimeText(Calendar.getInstance());
	}

	public boolean setTimerEnable(boolean enable) {
		if (enable) {
			if (mTimeView == null) {
				mTimeView = (TextView) addText(getTimeText(), 0);
				if (mTimeView == null) {
					return false;
				}

				mTimeView.setTextColor(TIME_TEXT_COLOR);
				mTimeView.setTextSize(TIME_TEXT_SIZE);
				mTimeView.setGravity(Gravity.RIGHT | Gravity.CENTER_VERTICAL);

				if (TIME_TEXT_BOLD) {
					mTimeView.getPaint().setFakeBoldText(true);
				}

				mLastSecond = -1;
				mTimeView.post(mRunnableTime);
			}
		} else if (mTimeView != null) {
			mTimeView.removeCallbacks(mRunnableTime);
			removeView(mTimeView);
			mTimeView = null;
		}

		return true;
	}

	@Override
	public void onDestroy() {
		setTimerEnable(false);
		super.onDestroy();
	}

	@Override
	public IBinder onBind(Intent intent) {
		return mBinder;
	}

	@Override
	protected View createView(CharSequence text) {
		TextView view = new TextView(getApplicationContext());

		view.setText(text);
		view.setSingleLine();
		view.setPadding(TEXT_PADDING, 0, TEXT_PADDING, 0);
		view.setBackgroundResource(R.drawable.desktop_timer_bg);

		return view;
	}

	@Override
	protected CharSequence getViewText(View arg0) {
		TextView view = (TextView) arg0;
		return view.getText();
	}
}
