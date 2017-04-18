package com.cavan.jwaootoyfactorytest;

import android.app.Fragment;
import android.app.FragmentTransaction;
import android.bluetooth.BluetoothDevice;
import android.graphics.Color;
import android.graphics.drawable.Drawable;
import android.net.Uri;
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
import com.cavan.java.CavanMacAddress;
import com.cavan.resource.JwaooToyActivity;
import com.jwaoo.android.JwaooBleToy;
import com.jwaoo.android.JwaooToySensor;

public class MainActivity extends JwaooToyActivity implements OnClickListener {

	private static final int MSG_SET_TEST_ITEM = 100;
	private static final int MSG_SENSOR_DATA = 101;
	private static final int MSG_KEY_STATE = 102;
	private static final int MSG_BATTERY_STATE = 103;
	private static final int MSG_CONNECT_STATE_CHANGED = 104;

	private static final int TEST_ITEM_BUTTON = 0;
	private static final int TEST_ITEM_DEPTH = 1;
	private static final int TEST_ITEM_GSENSOR = 2;
	private static final int TEST_ITEM_LED = 3;
	private static final int TEST_ITEM_MOTO = 4;
	private static final int TEST_ITEM_CHARGE = 5;
	private static final int TEST_ITEM_SUSPEND = 7;

	private TestItemFragment[] mTestItemFragmanetsEmpty = new TestItemFragment[0];

	private TestItemFragment[] mTestItemFragmanetsModel06 = {
		new ButtonTestFragment(),
		new GsensorTestFragment(),
		new LedTestFragment(),
		new MotoTestFragment(),
		new ChargeTestFragment(),
		new SuspendTestFragment(),
		// new DepthSensorTestFragment(),
	};

	private TestItemFragment[] mTestItemFragmanetsModel10 = {
		new ButtonTestFragment(),
		new GsensorTestFragment(),
		new LedTestFragment(),
		new ChargeTestFragment(),
		new SuspendTestFragment(),
	};

	private Button mButtonPass;
	private Button mButtonFail;
	private Button mButtonStart;
	private TextView mTextViewInfo;

	private Drawable mDrawablePass;
	private Drawable mDrawableFail;
	private Drawable mDrawableNoTest;

	private int mCurrentItem;
	private boolean mAutoTestEnable;
	private boolean mSuspendSuccess;

	private TestResult mTestResult;
	private TestResultFragment mTestResultFragment = new TestResultFragment();

	private TestItemFragment mCurrentFragmanet;
	private TestItemFragment[] mTestItemFragmanets = mTestItemFragmanetsEmpty;

	public void setTestItem(int item) {
		BaseTestFragment fragment;

		mButtonPass.setVisibility(View.INVISIBLE);

		if (item < 0 || item >= mTestItemFragmanets.length) {
			item = -1;
			mCurrentFragmanet = null;
			fragment = mTestResultFragment;

			mButtonFail.setVisibility(View.INVISIBLE);
			mButtonStart.setVisibility(View.VISIBLE);
		} else {
			if (mBleToy == null || mBleToy.isConnected() == false) {
				return;
			}

			mCurrentFragmanet = mTestItemFragmanets[item];
			fragment = mCurrentFragmanet;

			mButtonFail.setVisibility(View.VISIBLE);
			mButtonStart.setVisibility(View.INVISIBLE);
		}

		FragmentTransaction transaction = getFragmentManager().beginTransaction();
		transaction.replace(R.id.frame_content, fragment);
		transaction.commitAllowingStateLoss();

		setTitle(fragment.getTestName());

		mCurrentItem = item;
	}

	public void gotoNextTest(boolean pass) {
		if (mCurrentFragmanet != null) {
			mCurrentFragmanet.setTestResult(pass);
		}

		if (mSuspendSuccess) {
			showScanActivity();
		} else {
			setTestItem(mAutoTestEnable ? (mCurrentItem + 1) : -1);
		}
	}

	public void setPassEnable() {
		mButtonPass.setVisibility(View.VISIBLE);
		mButtonPass.setEnabled(true);
	}

	public void setFailEnable() {
		mButtonFail.setVisibility(View.VISIBLE);
		mButtonFail.setEnabled(true);
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
	protected void onCreateBle(Bundle savedInstanceState) {
		setContentView(R.layout.activity_main);

		CavanAndroid.acquireWakeLock(this);

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

		mTextViewInfo = (TextView) findViewById(R.id.textViewInfo);

		showScanActivity();
	}

	@Override
	protected void onDestroy() {
		CavanAndroid.releaseWakeLock();

		try {
			mBleToy.setFactoryModeEnable(false);
		} catch (Exception e) {
			e.printStackTrace();
		}

		super.onDestroy();
	}

	@Override
	public boolean handleMessage(Message msg) {
		if (super.handleMessage(msg)) {
			return true;
		}

		switch (msg.what) {
		case MSG_SET_TEST_ITEM:
			setTestItem(msg.arg1);
			break;

		case MSG_CONNECT_STATE_CHANGED:
			if ((Boolean) msg.obj) {
				mTextViewInfo.setText(getResources().getString(R.string.device_connected, mBleToy.getAddress() + " - " + mBleToy.getDeviceName()));

				if (mBleToy.getDeviveId() == JwaooBleToy.DEVICE_ID_MODEL10) {
					mTestItemFragmanets = mTestItemFragmanetsModel10;
				} else {
					mTestItemFragmanets = mTestItemFragmanetsModel06;
				}
			} else {
				if (mSuspendSuccess && mCurrentFragmanet instanceof SuspendTestFragment) {
					((SuspendTestFragment) mCurrentFragmanet).setSuspendComplete();
				}

				mTextViewInfo.setText(R.string.device_disconnected);
				mTestItemFragmanets = mTestItemFragmanetsEmpty;
			}

			mTestResultFragment.notifyDataSetChanged();
			break;

		default:
			if (mCurrentItem < 0) {
				break;
			}

			TestItemFragment fragment = mTestItemFragmanets[mCurrentItem];
			if (fragment.isInitialized()) {
				fragment.handleMessage(msg);
			}
		}

		return true;
	}

	@Override
	public void showScanActivity() {
		if (mSuspendSuccess && mBleToy != null) {
			String address = mBleToy.getAddress();
			CavanMacAddress mac = new CavanMacAddress().fromString(address);

			mac.increase();
			setAddresses2(mac.toString(), address);
		}

		super.showScanActivity();
	}

	@Override
	protected void onScanComplete(BluetoothDevice device) {
		mTextViewInfo.setText(R.string.device_not_connect);
		mTestResult = new TestResult(this, device.getAddress());
	}

	@Override
	public void onConnectionStateChanged(boolean connected) {
		CavanAndroid.dLog("onConnectionStateChanged: connected = " + connected);

		updateUI(connected);

		if (connected) {
			showProgressDialog(false);
			mSuspendSuccess = false;
			setAddresses(null);
		} else if (mSuspendSuccess) {
			mBleToy.disconnect();
		} else {
			showProgressDialog(true);
		}

		mHandler.obtainMessage(MSG_CONNECT_STATE_CHANGED, connected).sendToTarget();
	}

	@Override
	public boolean onInitialize() {
		if (!super.onInitialize()) {
			return false;
		}

		try {
			if (!mBleToy.setFactoryModeEnable(true)) {
				CavanAndroid.dLog("Failed to setFactoryModeEnable");
				return false;
			}
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}

		mHandler.obtainMessage(MSG_SET_TEST_ITEM, -1, 0).sendToTarget();

		return true;
	}

	@Override
	public void onSensorDataReceived(JwaooToySensor sensor, byte[] data) {
		mHandler.obtainMessage(MSG_SENSOR_DATA, sensor).sendToTarget();
	}

	@Override
	public void onKeyStateChanged(int code, int state) {
		mHandler.obtainMessage(MSG_KEY_STATE, code, state).sendToTarget();;
	}

	@Override
	public void onBatteryStateChanged(int state, int level, double voltage) {
		mHandler.obtainMessage(MSG_BATTERY_STATE, state, level, voltage).sendToTarget();
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

		public void notifyDataSetChanged() {
			mAdapter.notifyDataSetChanged();
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

			return mTestResult.get(mIndex);
		}

		public boolean setTestResult(boolean result) {
			if (mTestResult == null) {
				return false;
			}

			Uri uri = mTestResult.put(MainActivity.this, mIndex, result);
			if (uri == null) {
				return false;
			}

			return true;
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

		public ButtonTestFragment() {
			super(TEST_ITEM_BUTTON);
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
			if (mBleToy.getDeviveId() == JwaooBleToy.DEVICE_ID_MODEL10) {
				findViewById(R.id.buttonKey0).setVisibility(View.INVISIBLE);
				findViewById(R.id.buttonKey1).setVisibility(View.INVISIBLE);
				findViewById(R.id.buttonKey3).setVisibility(View.INVISIBLE);

				mButtons = new JwaooKeyTestView[] {
					(JwaooKeyTestView) findViewById(R.id.buttonKey2),
				};

				mButtons[0].setTextRaw(R.string.text_key);
			} else {
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

				CavanAndroid.dLog("code = " + code + ", value = " + value);

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
			try {
				return mBleToy.setSensorEnable(true, 30);
			} catch (Exception e) {
				e.printStackTrace();
				return false;
			}
		}

		@Override
		public void onStop() {
			try {
				mBleToy.setSensorEnable(false);
			} catch (Exception e) {
				e.printStackTrace();
			}

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

		public DepthSensorTestFragment() {
			super(TEST_ITEM_DEPTH);
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

		public GsensorTestFragment() {
			super(TEST_ITEM_GSENSOR);
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

			setDifferenceMin(4);

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

		public ChargeTestFragment() {
			super(TEST_ITEM_CHARGE);
		}

		@Override
		protected int getNameResource() {
			if (mBleToy.getDeviveId() == JwaooBleToy.DEVICE_ID_MODEL10) {
				return R.string.test_item_battery;
			}

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

			try {
				return mBleToy.setBatteryEventEnable(true);
			} catch (Exception e) {
				e.printStackTrace();
				return false;
			}
		}

		@Override
		public void onStop() {
			try {
				mBleToy.setBatteryEventEnable(false);
			} catch (Exception e) {
				e.printStackTrace();
			}

			super.onStop();
		}

		@Override
		protected void handleMessage(Message msg) {
			if (msg.what == MSG_BATTERY_STATE) {
				int state = msg.arg1;
				double voltage = (Double) msg.obj;

				if (state >= 0 && state < mBatteryStates.length) {
					mTextViewBatteryState.setText(mBatteryStates[state]);
				} else {
					mTextViewBatteryState.setText(Integer.toString(msg.arg1));
				}

				mTextViewBatteryVoltage.setText(voltage + " (v)");
				mTextViewBatteryCapacity.setText(msg.arg2 + "%");

				if (mBleToy.getDeviveId() == JwaooBleToy.DEVICE_ID_MODEL10) {
					if (voltage > 1.8 && voltage < 3.5) {
						mTextViewBatteryVoltage.setTextColor(Color.BLACK);
						setPassEnable();
					} else {
						mTextViewBatteryVoltage.setTextColor(Color.RED);
					}
				} else if (voltage > 2.5 && voltage < 4.5) {
					mTextViewBatteryVoltage.setTextColor(Color.BLACK);

					if (state == JwaooBleToy.BATTERY_STATE_CHARGING) {
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

		public LedTestFragment() {
			super(TEST_ITEM_LED);
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

			if (mBleToy.getDeviveId() == JwaooBleToy.DEVICE_ID_MODEL10) {
				mCheckBoxLedBatt.setVisibility(View.INVISIBLE);
			} else {
				mCheckBoxLedBatt.setChecked(false);
				mCheckBoxLedBatt.setOnCheckedChangeListener(this);
			}

			mCheckBoxLedBt = (CheckBox) findViewById(R.id.checkBoxLedBluetooth);
			mCheckBoxLedBt.setChecked(true);
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
				try {
					if (mBleToy.setLedEnable(JwaooBleToy.LED_BATT, isChecked)) {
						mLedCountBatt++;
					}
				} catch (Exception e) {
					e.printStackTrace();
				}
				break;

			case R.id.checkBoxLedBluetooth:
				try {
					if (mBleToy.setLedEnable(JwaooBleToy.LED_BT, isChecked)) {
						mLedCountBt++;
					}
				} catch (Exception e) {
					e.printStackTrace();
				}
				break;
			}

			if (mCheckBoxLedBatt.getVisibility() == View.VISIBLE) {
				if (mLedCountBatt > 1 && mLedCountBt > 1) {
					setPassEnable();
				}
			} else if (mLedCountBt > 1) {
				setPassEnable();
			}
		}

		@Override
		public void onStop() {
			try {
				mBleToy.setLedEnable(JwaooBleToy.LED_BATT, false);
				mBleToy.setLedEnable(JwaooBleToy.LED_BT, true);
			} catch (Exception e) {
				e.printStackTrace();
			}

			super.onStop();
		}
	}

	public class MotoTestFragment extends TestItemFragment implements OnSeekBarChangeListener {

		private int mLevel;
		private SeekBar mSeekBar;
		private Runnable mRunnableSetLevel = new Runnable() {

			@Override
			public void run() {
				try {
					if (mBleToy.setMotoMode(JwaooBleToy.MOTO_MODE_LINE, mLevel)) {
						if (mLevel > 0) {
							setPassEnable();
						}
					}
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		};

		public MotoTestFragment() {
			super(TEST_ITEM_MOTO);
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
			mSeekBar.setProgress(0);
			mSeekBar.setOnSeekBarChangeListener(this);

			return true;
		}

		@Override
		public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
			if (progress > 0) {
				mLevel = progress + 5;
			} else {
				mLevel = 0;
			}

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

			try {
				mBleToy.setMotoMode(JwaooBleToy.MOTO_MODE_IDLE, 0);
			} catch (Exception e) {
				e.printStackTrace();
			}

			super.onStop();
		}
	}

	public class SuspendTestFragment extends TestItemFragment implements OnClickListener {

		private Button mButtonIntoSuspend;

		public SuspendTestFragment() {
			super(TEST_ITEM_SUSPEND);
		}

		public void setSuspendComplete() {
			mButtonIntoSuspend.setText(R.string.device_suspend_propmpt);
			mButtonIntoSuspend.setEnabled(false);
			setPassEnable();
			setFailEnable();
		}

		@Override
		protected int getNameResource() {
			return R.string.test_item_suspend;
		}

		@Override
		protected int getLayoutResource() {
			return R.layout.suspend_test;
		}

		@Override
		protected boolean doInitialize() {
			mButtonIntoSuspend = (Button) findViewById(R.id.buttonIntoSuspend);
			mButtonIntoSuspend.setText(R.string.into_suspend);
			mButtonIntoSuspend.setOnClickListener(this);

			if (mAutoTestEnable) {
				mButtonIntoSuspend.performClick();
			}

			return true;
		}

		@Override
		public void onClick(View v) {
			try {
				if (mBleToy.setFactoryModeEnable(false) && mBleToy.doShutdown()) {
					mSuspendSuccess = true;
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}
}
