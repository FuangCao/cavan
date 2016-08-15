package com.cavan.cavanmain;

import java.util.Calendar;

import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.os.IBinder;
import android.os.RemoteException;
import android.preference.PreferenceManager;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager.LayoutParams;
import android.widget.TextView;

import com.cavan.android.FloatWidowService;

public class FloatMessageService extends FloatWidowService {

	public static final int TEXT_PADDING = 8;
	public static final float TEXT_SIZE_TIME = 16;
	public static final float TEXT_SIZE_MESSAGE = 12;
	public static final int TEXT_COLOR_TIME = Color.WHITE;
	public static final int TEXT_COLOR_MESSAGE = Color.YELLOW;

	private int mLastSecond;
	private TextView mTimeView;

	private IFloatMessageService.Stub mBinder = new IFloatMessageService.Stub() {

		@Override
		public boolean setTimerEnable(boolean enable) throws RemoteException {
			return FloatMessageService.this.setTimerEnable(enable);
		}

		@Override
		public boolean getTimerState() throws RemoteException {
			return mTimeView != null && mTimeView.getVisibility() != View.INVISIBLE;
		}

		@Override
		public int addMessage(CharSequence message) throws RemoteException {
			TextView view = (TextView) FloatMessageService.this.addText(message, -1);
			if (view == null) {
				return -1;
			}

			return view.getId();
		}

		@Override
		public boolean hasMessage(CharSequence message) throws RemoteException {
			return FloatMessageService.this.hasText(message);
		}

		@Override
		public void removeMessage(CharSequence message) throws RemoteException {
			FloatMessageService.this.removeText(message);
		}
	};

	private Runnable mRunnableTime = new Runnable() {

		@Override
		public void run() {
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
	};

	public String getTimeText(Calendar calendar, int second) {
		int hour = calendar.get(Calendar.HOUR_OF_DAY);
		int minute = calendar.get(Calendar.MINUTE);

		return String.format("%02d:%02d:%02d", hour, minute, second);
	}

	public String getTimeText(Calendar calendar) {
		return getTimeText(calendar, calendar.get(Calendar.SECOND));
	}

	public String getTimeText() {
		return getTimeText(Calendar.getInstance());
	}

	public boolean setTimerEnable(boolean enable) {
		if (mTimeView == null) {
			return false;
		}

		if (enable) {
			mLastSecond = -1;
			mTimeView.setVisibility(View.VISIBLE);
			mTimeView.post(mRunnableTime);
		} else {
			mTimeView.removeCallbacks(mRunnableTime);
			mTimeView.setVisibility(View.INVISIBLE);
		}

		return true;
	}

	public void initTextView(TextView view, CharSequence text) {
		view.setText(text);
		view.setMaxLines(1);
		view.setPadding(TEXT_PADDING, 0, TEXT_PADDING, 0);
		view.setBackgroundResource(R.drawable.desktop_timer_bg);
	}

	@Override
	protected View createRootView() {
		return View.inflate(this, R.layout.float_message, null);
	}

	@Override
	protected LayoutParams createRootViewLayoutParams() {
		LayoutParams params = super.createRootViewLayoutParams();
		if (params == null) {
			return null;
		}

		params.width = LayoutParams.MATCH_PARENT;

		return params;
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

		initTextView(view, text);

		view.setTextSize(TEXT_SIZE_MESSAGE);
		view.setTextColor(TEXT_COLOR_MESSAGE);
		view.setGravity(Gravity.LEFT | Gravity.CENTER_VERTICAL);

		return view;
	}

	@Override
	protected CharSequence getViewText(View arg0) {
		TextView view = (TextView) arg0;
		return view.getText();
	}

	@Override
	protected ViewGroup findViewGroup() {
		return (ViewGroup) findViewById(R.id.layoutMessage);
	}

	@Override
	protected boolean doInitialize() {
		mTimeView = (TextView) findViewById(R.id.textViewTime);

		initTextView(mTimeView, getTimeText());

		mTimeView.setTextSize(TEXT_SIZE_TIME);
		mTimeView.setTextColor(TEXT_COLOR_TIME);

		SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);
		if (preferences != null && preferences.getBoolean(MainActivity.KEY_FLOAT_TIMER, false)) {
			setTimerEnable(true);
		} else {
			setTimerEnable(false);
		}

		return super.doInitialize();
	}
}
