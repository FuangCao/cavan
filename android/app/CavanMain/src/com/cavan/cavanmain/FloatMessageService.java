package com.cavan.cavanmain;

import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
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

import com.cavan.android.CavanAndroid;
import com.cavan.android.FloatWidowService;

public class FloatMessageService extends FloatWidowService {

	public static final int TEXT_PADDING = 8;
	public static final float TEXT_SIZE_TIME = 16;
	public static final float TEXT_SIZE_MESSAGE = 12;
	public static final int TEXT_COLOR_TIME = Color.WHITE;
	public static final int TEXT_COLOR_MESSAGE = Color.YELLOW;

	private int mLastSecond;
	private TextView mTimeView;
	private List<String> mCodeList = new ArrayList<String>();
	private BroadcastReceiver mReceiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {
			String action = intent.getAction();
			if (Intent.ACTION_SCREEN_OFF.equals(action)) {
				CavanAndroid.setLockScreenEnable(FloatMessageService.this, true);
			}
		}
	};

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
		public int addMessage(CharSequence message, CharSequence code) throws RemoteException {
			CavanAndroid.setLockScreenEnable(FloatMessageService.this, false);

			TextView view = (TextView) FloatMessageService.this.addText(message, -1);
			if (view == null) {
				return -1;
			}

			if (code != null) {
				mCodeList.add(code.toString());
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

			if (getTextCount() == 0) {
				mCodeList.clear();
			}
		}

		@Override
		public List<String> getMessages() throws RemoteException {
			List<String> list = new ArrayList<String>();
			for (CharSequence text : getTextSet()) {
				list.add(text.toString());
			}

			return list;
		}

		@Override
		public List<String> getCodes() throws RemoteException {
			return mCodeList;
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

	public static Intent startService(Context context) {
		Intent intent = new Intent(context, FloatMessageService.class);
		context.startService(intent);
		return intent;
	}

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
	public void onCreate() {
		IntentFilter filter = new IntentFilter(Intent.ACTION_SCREEN_OFF);
		registerReceiver(mReceiver, filter );
		super.onCreate();
	}

	@Override
	public void onDestroy() {
		unregisterReceiver(mReceiver);
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