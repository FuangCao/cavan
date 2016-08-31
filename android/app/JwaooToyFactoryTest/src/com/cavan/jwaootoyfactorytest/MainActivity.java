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
import com.jwaoo.android.JwaooToySensor;

public class MainActivity extends JwaooToyActivity implements OnClickListener {

	private static final int MSG_SENSOR_DATA = 10;
	private static final int MSG_KEY_STATE = 11;
	private static final int MSG_BATTERY_STATE = 12;

	private Button mButtonPass;
	private Button mButtonFail;
	private Button mButtonStart;

	private Drawable mDrawablePass;
	private Drawable mDrawableFail;
	private Drawable mDrawableNoTest;

	private int mTestItem;
	private boolean mAutoTestEnable;

	private BaseTestFragment[] mTestFragmanets = {
		new TestResultFragment(),
		new ButtonTestFragment(),
		new CapacityTestFragment(),
		new ChargeTestFragment(),
		new LedTestFragment(),
		new MotoTestFragment(),
	};

	public void setTestItem(int item) {
		if (item < 0 || item >= mTestFragmanets.length) {
			item = 0;
		}

		if (item > 0) {
			if (mBleToy == null || mBleToy.isConnected() == false) {
				return;
			}

			mButtonFail.setVisibility(View.VISIBLE);
			mButtonPass.setVisibility(View.VISIBLE);
			mButtonStart.setVisibility(View.INVISIBLE);
		} else {
			mButtonFail.setVisibility(View.INVISIBLE);
			mButtonPass.setVisibility(View.INVISIBLE);
			mButtonStart.setVisibility(View.VISIBLE);
		}

		BaseTestFragment fragment = mTestFragmanets[item];

		FragmentTransaction transaction = getFragmentManager().beginTransaction();
		transaction.replace(R.id.frame_content, fragment);
		transaction.commit();

		setTitle(fragment.getTestName());



		mTestItem = item;
	}

	public void gotoNextTest(boolean pass) {
		if (mTestItem > 0) {
			((TestItemFragment) mTestFragmanets[mTestItem]).setTestResult(CavanJava.getBoolValueInt(pass));
		}

		setTestItem(mAutoTestEnable ? (mTestItem + 1) : 0);
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
		mButtonPass.setOnClickListener(this);

		mButtonFail = (Button) findViewById(R.id.buttonFail);
		mButtonFail.setOnClickListener(this);

		mButtonStart = (Button) findViewById(R.id.buttonStart);
		mButtonStart.setOnClickListener(this);

		showScanActivity();

		setTestItem(0);
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
		BaseTestFragment fragment = mTestFragmanets[mTestItem];
		if (fragment.isInitialized()) {
			fragment.handleMessage(msg);
		}
	}

	@Override
	protected JwaooBleToy createJwaooBleToy(BluetoothDevice device) {
		return new JwaooBleToy(device) {

			@Override
			protected void onConnectionStateChange(boolean connected) {
				if (!connected) {
					showScanActivity();
				} else {
					updateUI(true);
				}
			}

			@Override
			protected boolean onInitialize() {
				if (!mBleToy.setFactoryModeEnable(true)) {
					CavanAndroid.eLog("Failed to setFactoryModeEnable");
					return false;
				}

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
			setTestItem(1);
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
			if (mRootView != null && doInitialize()) {
				mInitilized = true;
				return mRootView;
			}

			return null;
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

				TestItemFragment fragment = (TestItemFragment) mTestFragmanets[position + 1];

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
				return mTestFragmanets.length - 1;
			}
		};

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
			setTestItem(position + 1);
		}
	}

	public abstract class TestItemFragment extends BaseTestFragment {

		private int mTestResult = -1;

		public void finishTest() {}

		public int getTestResult() {
			return mTestResult;
		}

		public void setTestResult(int result) {
			mTestResult = result;
		}

		public Drawable getIcon() {
			if (mTestResult < 0) {
				return mDrawableNoTest;
			} else if (mTestResult > 0) {
				return mDrawablePass;
			} else {
				return mDrawableFail;
			}
		}
	}

	public class ButtonTestFragment extends TestItemFragment {

		private Button[] mKeyViews;

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
			mKeyViews = new Button[] {
				(Button) findViewById(R.id.buttonKeyUp),
				(Button) findViewById(R.id.buttonKeyO),
				(Button) findViewById(R.id.buttonKeyDown),
				(Button) findViewById(R.id.buttonKeyMax),
			};

			return true;
		}

		@Override
		protected void handleMessage(Message msg) {
			if (msg.what == MSG_KEY_STATE) {
				int code = msg.arg1;
				int value = msg.arg2;

				CavanAndroid.eLog("code = " + code + ", value = " + value);

				if (code >= 0 && code < mKeyViews.length) {
					mKeyViews[code].setBackgroundColor(value > 0 ? Color.GREEN : Color.GRAY);
				}
			}
		}
	}

	public class CapacityTestFragment extends TestItemFragment {

		private TextView[] mCapacityViews;

		@Override
		protected int getNameResource() {
			return R.string.test_item_capacity_sensor;
		}

		@Override
		protected int getLayoutResource() {
			return R.layout.capacity_test;
		}

		@Override
		protected boolean doInitialize() {
			mCapacityViews = new TextView[] {
				(TextView) findViewById(R.id.textViewCapacity1),
				(TextView) findViewById(R.id.textViewCapacity2),
				(TextView) findViewById(R.id.textViewCapacity3),
				(TextView) findViewById(R.id.textViewCapacity4),
			};

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
				JwaooToySensor sensor = (JwaooToySensor) msg.obj;
				double[] capacitys = sensor.getCapacitys();

				for (int i = capacitys.length - 1; i >= 0; i--) {
					int color;

					if (capacitys[i] < JwaooToySensor.CAPACITY_MAX * 2 / 3) {
						color = Color.GRAY;
					} else {
						color = Color.GREEN;
					}

					mCapacityViews[i].setText(String.format("%7.2f", capacitys[i]));
					mCapacityViews[i].setBackgroundColor(color);
				}
			}
		}
	}

	public class ChargeTestFragment extends TestItemFragment {

		private String[] mBatteryStates;

		private TextView mTextViewBatteryState;
		private TextView mTextViewBatteryVoltage;
		private TextView mTextViewBatteryCapacity;

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

				if (state >= 0 && state < mBatteryStates.length) {
					mTextViewBatteryState.setText(mBatteryStates[state]);
				} else {
					mTextViewBatteryState.setText(Integer.toString(msg.arg1));
				}

				mTextViewBatteryVoltage.setText(msg.obj + " (v)");
				mTextViewBatteryCapacity.setText(msg.arg2 + "%");
			}
		}
	}

	public class LedTestFragment extends TestItemFragment implements OnCheckedChangeListener {

		private CheckBox mCheckBoxLedBattery;
		private CheckBox mCheckBoxLedBluetooth;

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
			mCheckBoxLedBattery = (CheckBox) findViewById(R.id.checkBoxLedBattery);
			mCheckBoxLedBattery.setOnCheckedChangeListener(this);

			mCheckBoxLedBluetooth = (CheckBox) findViewById(R.id.checkBoxLedBluetooth);
			mCheckBoxLedBluetooth.setOnCheckedChangeListener(this);

			return true;
		}

		@Override
		public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
			switch (buttonView.getId()) {
			case R.id.checkBoxLedBattery:
				mBleToy.setLedEnable(1, isChecked);
				break;

			case R.id.checkBoxLedBluetooth:
				mBleToy.setLedEnable(2, isChecked);
				break;
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
				mBleToy.setMotoMode(0, mLevel);
			}
		};

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
