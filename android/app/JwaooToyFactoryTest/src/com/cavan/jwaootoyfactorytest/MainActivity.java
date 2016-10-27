package com.cavan.jwaootoyfactorytest;

import android.app.Fragment;
import android.app.FragmentTransaction;
import android.bluetooth.BluetoothDevice;
import android.graphics.Color;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.os.Message;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.GridView;
import android.widget.ListView;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;

import com.cavan.android.CavanAndroid;
import com.cavan.java.CavanJava;
import com.cavan.resource.JwaooToyActivity;
import com.jwaoo.android.JwaooBleToy;
import com.jwaoo.android.JwaooBleToy.JwaooToyTestResult;
import com.jwaoo.android.JwaooToySensor;

public class MainActivity extends JwaooToyActivity implements OnClickListener {

	private static final int MSG_SET_TEST_ITEM = 10;
	private static final int MSG_SENSOR_DATA = 11;
	private static final int MSG_KEY_STATE = 12;
	private static final int MSG_BATTERY_STATE = 13;

	private Button mButtonPass;
	private Button mButtonFail;
	private Button mButtonStart;

	private Drawable mDrawablePass;
	private Drawable mDrawableFail;
	private Drawable mDrawableNoTest;

	private int mTestItem;
	private boolean mAutoTestEnable;

	private JwaooToyTestResult mTestResult;
	private TestResultFragment mTestResultFragment = new TestResultFragment();

	private TestItemFragment[] mTestItemFragmanets = {
		new ButtonTestFragment(0),
		new GsensorTestFragment(2),
		new LedTestFragment(3),
		new MotoTestFragment(4),
		new ChargeTestFragment(5),
		// new DepthSensorTestFragment(6),
	};

	public void setTestItem(int item) {
		BaseTestFragment fragment;

		mButtonPass.setVisibility(View.INVISIBLE);

		if (item < 0 || item >= mTestItemFragmanets.length) {
			item = -1;
			fragment = mTestResultFragment;

			mButtonFail.setVisibility(View.INVISIBLE);
			mButtonStart.setVisibility(View.VISIBLE);
		} else {
			if (mBleToy == null || mBleToy.isConnected() == false) {
				return;
			}

			fragment = mTestItemFragmanets[item];

			mButtonFail.setVisibility(View.VISIBLE);
			mButtonStart.setVisibility(View.INVISIBLE);
		}

		FragmentTransaction transaction = getFragmentManager().beginTransaction();
		transaction.replace(R.id.frame_content, fragment);
		transaction.commitAllowingStateLoss();

		setTitle(fragment.getTestName());

		mTestItem = item;
	}

	public void gotoNextTest(boolean pass) {
		if (mTestItem >= 0) {
			mTestItemFragmanets[mTestItem].setTestResult(CavanJava.getBoolValueInt(pass));
		}

		setTestItem(mAutoTestEnable ? (mTestItem + 1) : -1);
	}

	public void setPassEnable() {
		mButtonPass.setVisibility(View.VISIBLE);
	}

	public void setTestComplete(boolean pass) {
		if (mAutoTestEnable) {
			gotoNextTest(pass);
		} else {
			setPassEnable();
		}
	}

	@SuppressWarnings("deprecation")
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		mDrawableNoTest = getResources().getDrawable(R.drawable.no_test);
		mDrawableNoTest.setBounds(0, 0, mDrawableNoTest.getMinimumWidth(), mDrawableNoTest.getMinimumHeight());

		mDrawablePass = getResources().getDrawable(R.drawable.pass);
		mDrawablePass.setBounds(0, 0, mDrawablePass.getMinimumWidth(), mDrawablePass.getMinimumHeight());

		mDrawableFail = getResources().getDrawable(R.drawable.fail);
		mDrawableFail.setBounds(0, 0, mDrawableFail.getMinimumWidth(), mDrawableFail.getMinimumHeight());

		mButtonPass = (Button) findViewById(R.id.buttonPass);
		mButtonPass.setVisibility(View.INVISIBLE);
		mButtonPass.setOnClickListener(this);

		mButtonFail = (Button) findViewById(R.id.buttonFail);
		mButtonFail.setVisibility(View.INVISIBLE);
		mButtonFail.setOnClickListener(this);

		mButtonStart = (Button) findViewById(R.id.buttonStart);
		mButtonStart.setVisibility(View.INVISIBLE);
		mButtonStart.setOnClickListener(this);

		showScanActivity();
	}

	@Override
	protected void onDestroy() {
		CavanAndroid.setSuspendEnable(this, true);

		if (mBleToy != null && mBleToy.isConnected()) {
			mBleToy.setFactoryModeEnable(false);
		}

		super.onDestroy();
	}

	@Override
	protected void handleMessage(Message msg) {
		switch (msg.what) {
		case MSG_SET_TEST_ITEM:
			setTestItem(msg.arg1);
			break;

		default:
			if (mTestItem < 0) {
				break;
			}

			TestItemFragment fragment = mTestItemFragmanets[mTestItem];
			if (fragment.isInitialized()) {
				fragment.handleMessage(msg);
			}
		}
	}

	@Override
	protected JwaooBleToy createJwaooBleToy(BluetoothDevice device) {
		return new JwaooBleToy(device) {

			@Override
			protected void onConnectionStateChange(boolean connected) {
				CavanAndroid.eLog("onConnectionStateChange: connected = " + connected);
				updateUI(connected);
				showProgressDialog(!connected);
			}

			@Override
			protected void onConnectFailed() {
				CavanAndroid.eLog("onAutoConnectFailed");
				showScanActivity();
			}

			@Override
			protected boolean onInitialize() {
				if (!mBleToy.setFactoryModeEnable(true)) {
					CavanAndroid.eLog("Failed to setFactoryModeEnable");
					return false;
				}

				mTestResult = mBleToy.readTestResult();
				if (mTestResult == null) {
					CavanAndroid.eLog("Failed to readTestResult");
					return false;
				}

				mHandler.obtainMessage(MSG_SET_TEST_ITEM, -1, 0).sendToTarget();

				return super.onInitialize();
			}

			@Override
			protected void onSensorDataReceived(byte[] arg0) {
				mSensor.putBytes(arg0);
				mHandler.obtainMessage(MSG_SENSOR_DATA, mSensor).sendToTarget();
			}

			@Override
			protected void onKeyStateChanged(int code, int state) {
				mHandler.obtainMessage(MSG_KEY_STATE, code, state).sendToTarget();;
			}

			@Override
			protected void onBatteryStateChanged(int state, int level, double voltage) {
				mHandler.obtainMessage(MSG_BATTERY_STATE, state, level, voltage).sendToTarget();
			}
		};
	}

	public class MyGridViewAdapter extends BaseAdapter {

		private String[] mItems;
		private GridView mGridView;

		public MyGridViewAdapter(GridView view, int resId) {
			mItems = getResources().getStringArray(resId);

			mGridView = view;
			mGridView.setNumColumns(mItems.length);
		}

		@Override
		public int getCount() {
			return mItems.length;
		}

		@Override
		public Object getItem(int position) {
			return mItems[position];
		}

		@Override
		public long getItemId(int position) {
			return position;
		}

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			TextView view;

			if (convertView == null) {
				view = new TextView(MainActivity.this);
			} else {
				view = (TextView) convertView;
			}

			view.setTextSize(30);
			view.setTextColor(Color.WHITE);
			view.setBackgroundColor(Color.BLACK);
			view.setText(mItems[position]);
			view.setGravity(Gravity.CENTER);

			return view;
		}
	}

	@Override
	public void onClick(View v) {
		switch (v.getId()) {
		case R.id.buttonFail:
			gotoNextTest(false);
			break;

		case R.id.buttonPass:
			gotoNextTest(true);
			break;

		case R.id.buttonStart:
			mAutoTestEnable = true;
			setTestItem(0);
			break;
		}
	}

	public abstract class BaseTestFragment extends Fragment {

		private View mRootView;
		private boolean mInitilized;

		protected abstract int getNameResource();
		protected abstract int getLayoutResource();
		protected abstract boolean doInitialize();

		protected void handleMessage(Message msg) {
		}

		public String getTestName() {
			return MainActivity.this.getResources().getString(getNameResource());
		}

		public View findViewById(int id) {
			return mRootView.findViewById(id);
		}

		public boolean isInitialized() {
			return mInitilized;
		}

		@Override
		public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
			mRootView = inflater.inflate(getLayoutResource(), container, false);
			return mRootView;
		}

		@Override
		public void onStart() {
			if (mRootView != null && doInitialize()) {
				mInitilized = true;
			}

			super.onStart();
		}

		@Override
		public void onStop() {
			mInitilized = false;
			super.onStop();
		}
	}

	public class TestResultFragment extends BaseTestFragment implements OnItemClickListener {

		private ListView mListView;
		private BaseAdapter mAdapter = new BaseAdapter() {

			@Override
			public View getView(int position, View convertView, ViewGroup parent) {
				TextView view = (TextView) convertView;
				if (view == null) {
					view = new TextView(MainActivity.this);
				}

				TestItemFragment fragment = mTestItemFragmanets[position];

				view.setTextColor(Color.BLACK);
				view.setBackgroundColor(Color.WHITE);
				view.setCompoundDrawables(null, null, fragment.getIcon(), null);

				view.setTextSize(25);
				view.setText(fragment.getTestName());
				view.setGravity(Gravity.LEFT | Gravity.CENTER_VERTICAL);

				return view;
			}

			@Override
			public long getItemId(int position) {
				return position + 1;
			}

			@Override
			public Object getItem(int position) {
				return null;
			}

			@Override
			public int getCount() {
				return mTestItemFragmanets.length;
			}
		};

		public void updateData() {
			if (mListView != null) {
				mListView.postInvalidate();
			}
		}

		@Override
		protected int getNameResource() {
			return R.string.app_name;
		}

		@Override
		protected int getLayoutResource() {
			return R.layout.test_result;
		}

		@Override
		public boolean doInitialize() {
			mListView = (ListView) findViewById(R.id.listViewTestResult);
			mListView.setAdapter(mAdapter);
			mListView.setOnItemClickListener(this);

			return true;
		}

		@Override
		public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
			mAutoTestEnable = false;
			setTestItem(position);
		}
	}

	public abstract class TestItemFragment extends BaseTestFragment {

		private int mIndex;

		public void finishTest() {}

		public TestItemFragment(int index) {
			mIndex = index;
		}

		public int getTestResult() {
			if (mTestResult == null) {
				return -1;
			}

			return mTestResult.getResult(mIndex);
		}

		public boolean setTestResult(int result) {
			if (mTestResult == null) {
				return false;
			}

			mTestResult.setResult(mIndex, result);

			if (mBleToy == null) {
				return false;
			}

			return mBleToy.writeTestResult(mTestResult);
		}

		public Drawable getIcon() {
			int result = getTestResult();

			if (result < 0) {
				return mDrawableNoTest;
			} else if (result > 0) {
				return mDrawablePass;
			} else {
				return mDrawableFail;
			}
		}
	}

	public class ButtonTestFragment extends TestItemFragment {

		private JwaooKeyTestView[] mButtons;

		public ButtonTestFragment(int index) {
			super(index);
		}

		@Override
		protected int getNameResource() {
			return R.string.test_item_button;
		}

		@Override
		protected int getLayoutResource() {
			return R.layout.button_test;
		}

		@Override
		protected boolean doInitialize() {
			mButtons = new JwaooKeyTestView[] {
				(JwaooKeyTestView) findViewById(R.id.buttonKey0),
				(JwaooKeyTestView) findViewById(R.id.buttonKey1),
				(JwaooKeyTestView) findViewById(R.id.buttonKey2),
				(JwaooKeyTestView) findViewById(R.id.buttonKey3),
			};

			CharSequence[] texts = getResources().getTextArray(R.array.text_keys);

			for (int i = 0; i < mButtons.length && i < texts.length; i++) {
				mButtons[i].setTextRaw(texts[i].toString());
			}

			return true;
		}

		private boolean isTestPass() {
			for (JwaooKeyTestView button : mButtons) {
				if (button.isTestFail()) {
					return false;
				}
			}

			return true;
		}

		@Override
		protected void handleMessage(Message msg) {
			if (msg.what == MSG_KEY_STATE) {
				int code = msg.arg1;
				int value = msg.arg2;

				CavanAndroid.eLog("code = " + code + ", value = " + value);

				if (code >= 0 && code < mButtons.length) {
					mButtons[code].setPressState(value > 0);

					if (isTestPass()) {
						setTestComplete(true);
					}
				}
			}
		}
	}

	public abstract class SensorTestFragment extends TestItemFragment {

		protected JwaooSensorTestView[] mTestViews;

		protected abstract void onSensorDataReceived(JwaooToySensor sensor);

		public SensorTestFragment(int index) {
			super(index);
		}

		public boolean isPassed() {
			for (int i = mTestViews.length - 1; i >= 0; i--) {
				if (!mTestViews[i].isPassed()) {
					return false;
				}
			}

			return true;
		}

		public void setDifferenceMin(double min) {
			for (int i = mTestViews.length - 1; i >= 0; i--) {
				mTestViews[i].setDifferenceMin(min);
			}
		}

		@Override
		protected boolean doInitialize() {
			return mBleToy.setSensorEnable(true, 30);
		}

		@Override
		public void onStop() {
			mBleToy.setSensorEnable(false);
			super.onStop();
		}

		@Override
		protected void handleMessage(Message msg) {
			if (msg.what == MSG_SENSOR_DATA) {
				onSensorDataReceived((JwaooToySensor) msg.obj);

				if (isPassed()) {
					setTestComplete(true);
				}
			}
		}
	}

	public class DepthSensorTestFragment extends SensorTestFragment {

		public DepthSensorTestFragment(int index) {
			super(index);
		}

		@Override
		protected int getNameResource() {
			return R.string.test_item_depth_sensor;
		}

		@Override
		protected int getLayoutResource() {
			if (mBleToy.getDeviveId() == JwaooBleToy.DEVICE_ID_K101) {
				return R.layout.depth_sensor_test_k101;
			}

			return R.layout.depth_sensor_test_k100;
		}

		@Override
		protected boolean doInitialize() {
			if (mBleToy.getDeviveId() == JwaooBleToy.DEVICE_ID_K101) {
				mTestViews = new JwaooSensorTestView[] {
					(JwaooSensorTestView) findViewById(R.id.capacityView1),
					(JwaooSensorTestView) findViewById(R.id.capacityView2),
					(JwaooSensorTestView) findViewById(R.id.capacityView3),
				};
			} else {
				mTestViews = new JwaooSensorTestView[] {
					(JwaooSensorTestView) findViewById(R.id.capacityView1),
					(JwaooSensorTestView) findViewById(R.id.capacityView2),
					(JwaooSensorTestView) findViewById(R.id.capacityView3),
					(JwaooSensorTestView) findViewById(R.id.capacityView4),
				};
			}

			setDifferenceMin(30);

			return super.doInitialize();
		}

		@Override
		protected void onSensorDataReceived(JwaooToySensor sensor) {
			double[] capacitys = sensor.getCapacitys();

			for (int i = mTestViews.length - 1; i >= 0; i--) {
				mTestViews[i].putValueValue(capacitys[i]);
			}
		}
	}

	public class GsensorTestFragment extends SensorTestFragment {

		public GsensorTestFragment(int index) {
			super(index);
		}

		@Override
		protected int getNameResource() {
			return R.string.test_item_accel_sensor;
		}

		@Override
		protected int getLayoutResource() {
			return R.layout.gsensor_test;
		}

		@Override
		protected boolean doInitialize() {
			mTestViews = new JwaooSensorTestView[] {
				(JwaooSensorTestView) findViewById(R.id.gsensorView1),
				(JwaooSensorTestView) findViewById(R.id.gsensorView2),
				(JwaooSensorTestView) findViewById(R.id.gsensorView3),
			};

			setDifferenceMin(18);

			return super.doInitialize();
		}

		@Override
		protected void onSensorDataReceived(JwaooToySensor sensor) {
			mTestViews[0].putValueValue(sensor.getAxisX());
			mTestViews[1].putValueValue(sensor.getAxisY());
			mTestViews[2].putValueValue(sensor.getAxisZ());
		}
	}

	public class ChargeTestFragment extends TestItemFragment {

		private String[] mBatteryStates;

		private TextView mTextViewBatteryState;
		private TextView mTextViewBatteryVoltage;
		private TextView mTextViewBatteryCapacity;

		public ChargeTestFragment(int index) {
			super(index);
		}

		@Override
		protected int getNameResource() {
			return R.string.test_item_charge;
		}

		@Override
		protected int getLayoutResource() {
			return R.layout.charge_test;
		}

		@Override
		protected boolean doInitialize() {
			mBatteryStates = MainActivity.this.getResources().getStringArray(R.array.battery_states);

			mTextViewBatteryState = (TextView) findViewById(R.id.textViewBatteryState);
			mTextViewBatteryVoltage = (TextView) findViewById(R.id.textViewBatteryVoltage);
			mTextViewBatteryCapacity = (TextView) findViewById(R.id.textViewBatteryCapacity);

			return mBleToy.setBatteryEventEnable(true);
		}

		@Override
		public void onStop() {
			mBleToy.setBatteryEventEnable(false);
			super.onStop();
		}

		@Override
		protected void handleMessage(Message msg) {
			if (msg.what == MSG_BATTERY_STATE) {
				int state = msg.arg1;
				double voltage = (double) msg.obj;

				if (state >= 0 && state < mBatteryStates.length) {
					mTextViewBatteryState.setText(mBatteryStates[state]);
				} else {
					mTextViewBatteryState.setText(Integer.toString(msg.arg1));
				}

				mTextViewBatteryVoltage.setText(voltage + " (v)");
				mTextViewBatteryCapacity.setText(msg.arg2 + "%");

				if (voltage > 2.5 && voltage < 4.5) {
					mTextViewBatteryVoltage.setTextColor(Color.BLACK);

					if ((state & 2) != 0) {
						setPassEnable();
					}
				} else {
					mTextViewBatteryVoltage.setTextColor(Color.RED);
				}
			}
		}
	}

	public class LedTestFragment extends TestItemFragment implements OnCheckedChangeListener {

		private int mLedCountBt;
		private int mLedCountBatt;
		private CheckBox mCheckBoxLedBt;
		private CheckBox mCheckBoxLedBatt;

		public LedTestFragment(int index) {
			super(index);
		}

		@Override
		protected int getNameResource() {
			return R.string.test_item_led;
		}

		@Override
		protected int getLayoutResource() {
			return R.layout.led_test;
		}

		@Override
		protected boolean doInitialize() {
			mCheckBoxLedBatt = (CheckBox) findViewById(R.id.checkBoxLedBattery);
			mCheckBoxLedBatt.setOnCheckedChangeListener(this);

			mCheckBoxLedBt = (CheckBox) findViewById(R.id.checkBoxLedBluetooth);
			mCheckBoxLedBt.setOnCheckedChangeListener(this);

			return true;
		}

		@Override
		public void onStart() {
			mLedCountBt = 0;
			mLedCountBatt = 0;
			super.onStart();
		}

		@Override
		public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
			switch (buttonView.getId()) {
			case R.id.checkBoxLedBattery:
				if (mBleToy.setLedEnable(1, isChecked)) {
					mLedCountBatt++;
				}
				break;

			case R.id.checkBoxLedBluetooth:
				if (mBleToy.setLedEnable(2, isChecked)) {
					mLedCountBt++;
				}
				break;
			}

			if (mLedCountBatt > 1 && mLedCountBt > 1) {
				setPassEnable();
			}
		}

		@Override
		public void onStop() {
			mBleToy.setLedEnable(1, false);
			mBleToy.setLedEnable(2, true);

			super.onStop();
		}
	}

	public class MotoTestFragment extends TestItemFragment implements OnSeekBarChangeListener {

		private int mLevel;
		private SeekBar mSeekBar;
		private Runnable mRunnableSetLevel = new Runnable() {

			@Override
			public void run() {
				if (mBleToy.setMotoMode(0, mLevel)) {
					if (mLevel > mSeekBar.getMax() / 2) {
						setPassEnable();
					}
				}
			}
		};

		public MotoTestFragment(int index) {
			super(index);
		}

		@Override
		protected int getNameResource() {
			return R.string.test_item_moto;
		}

		@Override
		protected int getLayoutResource() {
			return R.layout.moto_test;
		}

		@Override
		protected boolean doInitialize() {
			mSeekBar = (SeekBar) findViewById(R.id.seekBarMotoLevel);
			mSeekBar.setOnSeekBarChangeListener(this);

			return true;
		}

		@Override
		public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
			mLevel = progress;
			mHandler.post(mRunnableSetLevel);
		}

		@Override
		public void onStartTrackingTouch(SeekBar seekBar) {
		}

		@Override
		public void onStopTrackingTouch(SeekBar seekBar) {
		}

		@Override
		public void onStop() {
			mHandler.removeCallbacks(mRunnableSetLevel);
			mBleToy.setMotoMode(0, 0);

			super.onStop();
		}
	}
}
