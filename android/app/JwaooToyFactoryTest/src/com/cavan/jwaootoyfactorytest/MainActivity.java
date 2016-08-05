package com.cavan.jwaootoyfactorytest;

import android.bluetooth.BluetoothDevice;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Message;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.GridView;
import android.widget.TextView;

import com.cavan.android.CavanAndroid;
import com.cavan.resource.JwaooToyActivity;
import com.jwaoo.android.JwaooBleToy;
import com.jwaoo.android.JwaooToySensor;

public class MainActivity extends JwaooToyActivity implements OnCheckedChangeListener {

	private static final int MSG_SENSOR_DATA = 10;
	private static final int MSG_KEY_STATE = 11;

	private GridView mGridViewKey;
	private GridView mGridViewDepth;
	private GridView mGridViewResult;

	private CheckBox mCheckBoxMoto;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		CavanAndroid.setSuspendEnable(this, false);

		mGridViewKey = (GridView) findViewById(R.id.gridViewKey);
		mGridViewKey.setAdapter(new MyGridViewAdapter(mGridViewKey, R.array.text_keys));

		mGridViewDepth = (GridView) findViewById(R.id.gridViewDepth);
		mGridViewDepth.setAdapter(new MyGridViewAdapter(mGridViewDepth, R.array.text_depths));

		mGridViewResult = (GridView) findViewById(R.id.gridViewResult);
		mGridViewResult.setAdapter(new MyGridViewAdapter(mGridViewResult, R.array.text_items));

		mCheckBoxMoto = (CheckBox) findViewById(R.id.checkBoxMoto);
		mCheckBoxMoto.setOnCheckedChangeListener(this);

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
		case MSG_SENSOR_DATA:
			JwaooToySensor sensor = (JwaooToySensor) msg.obj;
			double[] capacitys = sensor.getCapacitys();

			for (int i = capacitys.length - 1; i >= 0; i--) {
				int color;
				TextView view = (TextView) mGridViewDepth.getChildAt(i);

				if (capacitys[i] < JwaooToySensor.CAPACITY_MAX) {
					color = Color.GRAY;
				} else {
					color = Color.GREEN;
				}

				view.setBackgroundColor(color);
			}
			break;

		case MSG_KEY_STATE:
			mGridViewKey.getChildAt(msg.arg1).setBackgroundColor(msg.arg2 > 0 ? Color.GREEN : Color.GRAY);
			break;
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
					CavanAndroid.logE("Failed to setFactoryModeEnable");
					return false;
				}

				if (!mBleToy.setSensorEnable(true, 200)) {
					CavanAndroid.logE("Failed to setSensorEnable");
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
	public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
		mBleToy.setMotoMode(isChecked ? 1 : 0, 5);
	}
}
