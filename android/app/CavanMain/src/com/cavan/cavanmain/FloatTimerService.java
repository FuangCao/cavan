package com.cavan.cavanmain;

import java.util.Calendar;

import android.app.Service;
import android.content.Intent;
import android.graphics.Color;
import android.graphics.PixelFormat;
import android.os.IBinder;
import android.os.RemoteException;
import android.view.Gravity;
import android.view.WindowManager;
import android.view.WindowManager.LayoutParams;
import android.widget.TextView;

public class FloatTimerService extends Service {

	public static final float TIME_TEXT_SIZE = 16;
	public static final boolean TIME_TEXT_BOLD = false;
	public static final boolean SHOW_SECOND_ONLY = false;
	public static final int TIME_TEXT_COLOR = Color.WHITE;

	private int mLastSecond;
	private TextView mTimeView;
	private WindowManager mManager;

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
			if (mManager == null) {
				return false;
			}

			if (enable) {
				if (mTimeView != null) {
					return true;
				}

				LayoutParams params = new LayoutParams();

				params.x = params.y = 0;
				params.type = LayoutParams.TYPE_PHONE;
				params.format = PixelFormat.RGBA_8888;
				params.gravity = Gravity.RIGHT | Gravity.TOP;
				params.width = WindowManager.LayoutParams.WRAP_CONTENT;
				params.height = WindowManager.LayoutParams.WRAP_CONTENT;
				params.flags = LayoutParams.FLAG_NOT_FOCUSABLE | LayoutParams.FLAG_NOT_TOUCHABLE;

				TextView view = new TextView(getApplicationContext());
				view.setBackgroundResource(R.drawable.desktop_timer_bg);
				view.setTextColor(TIME_TEXT_COLOR);

				if (TIME_TEXT_BOLD) {
					view.getPaint().setFakeBoldText(true);
				}

				if (TIME_TEXT_SIZE > 0) {
					view.setTextSize(TIME_TEXT_SIZE);
				}

				mManager.addView(view, params);
				mTimeView = view;

				mLastSecond = -1;
				mTimeView.post(mRunnableTime);
			} else if (mTimeView != null) {
				mManager.removeView(mTimeView);
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
	public void onCreate() {
		mManager = (WindowManager) getApplicationContext().getSystemService(WINDOW_SERVICE);

		super.onCreate();
	}

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
}
