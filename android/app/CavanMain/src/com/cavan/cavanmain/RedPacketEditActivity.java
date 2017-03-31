package com.cavan.cavanmain;

import java.util.Date;
import java.util.List;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.graphics.Color;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.text.Editable;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.TimePicker;

@SuppressWarnings("deprecation")
public class RedPacketEditActivity extends Activity implements OnClickListener, OnItemClickListener {

	private Button mButtonDelete;
	private Button mButtonUpdate;
	private TimePicker mTimePicker;
	private EditText mEditTextCode;
	private ListView mListViewCodes;
	private CheckBox mCheckBoxSync;
	private CheckBox mCheckBoxIgnore;

	private IFloatMessageService mFloatMessageService;
	private ServiceConnection mFloatMessageConnection = new ServiceConnection() {

		@Override
		public void onServiceDisconnected(ComponentName arg0) {
			mFloatMessageService = null;
		}

		@Override
		public void onServiceConnected(ComponentName component, IBinder binder) {
			mFloatMessageService = IFloatMessageService.Stub.asInterface(binder);
		}
	};

	private BaseAdapter mAdapter = new BaseAdapter() {

		private RedPacketCode[] mCodes = buildCodeArray();

		public RedPacketCode[] buildCodeArray() {
			List<RedPacketCode> list = RedPacketCode.getLastCodes();
			RedPacketCode[] codes = new RedPacketCode[list.size()];
			list.toArray(codes);
			return codes;
		}

		@Override
		public void notifyDataSetChanged() {
			mCodes = buildCodeArray();
			super.notifyDataSetChanged();
		}

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			TextView view;

			if (convertView != null) {
				view = (TextView) convertView;
			} else {
				view = new TextView(RedPacketEditActivity.this);
			}

			RedPacketCode node = mCodes[position];

			view.setText(node.getCode());
			view.setTextSize(20);

			if (node.isInvalid()) {
				view.setTextColor(Color.RED);
			} else if (node.isIgnored()) {
				view.setTextColor(Color.BLUE);
			} else {
				view.setTextColor(Color.BLACK);
			}

			return view;
		}

		@Override
		public long getItemId(int position) {
			return position;
		}

		@Override
		public Object getItem(int position) {
			return mCodes[position];
		}

		@Override
		public int getCount() {
			return mCodes.length;
		}
	};

	public static Intent getIntent(Context context) {
		return new Intent(context, RedPacketEditActivity.class);
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.red_packet_editor);

		mEditTextCode = (EditText) findViewById(R.id.editTextCode);
		mCheckBoxSync = (CheckBox) findViewById(R.id.checkBoxSync);
		mCheckBoxIgnore = (CheckBox) findViewById(R.id.checkBoxIgnore);

		mTimePicker = (TimePicker) findViewById(R.id.timePicker);
		mTimePicker.setIs24HourView(true);

		mButtonDelete = (Button) findViewById(R.id.buttonReload);
		mButtonDelete.setOnClickListener(this);

		mButtonUpdate = (Button) findViewById(R.id.buttonUpdate);
		mButtonUpdate.setOnClickListener(this);

		mListViewCodes = (ListView) findViewById(R.id.listViewCodes);
		mListViewCodes.setAdapter(mAdapter);
		mListViewCodes.setOnItemClickListener(this);

		bindService(FloatMessageService.buildIntent(this), mFloatMessageConnection, 0);
	}

	@Override
	protected void onDestroy() {
		unbindService(mFloatMessageConnection);
		super.onDestroy();
	}

	@Override
	public void onClick(View v) {
		switch (v.getId()) {
		case R.id.buttonReload:
			mAdapter.notifyDataSetChanged();
			break;

		case R.id.buttonUpdate:
			Editable text = mEditTextCode.getText();
			String code = text.toString().trim();

			text.clear();

			if (code.isEmpty()) {
				break;
			}

			Date date = new Date();
			date.setHours(mTimePicker.getCurrentHour());
			date.setMinutes(mTimePicker.getCurrentMinute());

			long time = date.getTime() / 60000 * 60000;

			RedPacketCode node = RedPacketCode.update(this, code, time, mCheckBoxIgnore.isChecked());
			if (node == null) {
				break;
			}

			if (mCheckBoxSync.isChecked() && mFloatMessageService != null) {
				StringBuilder builder = new StringBuilder();
				builder.append(FloatMessageService.NET_CMD_UPDATE);
				builder.append(node.getCode());
				builder.append('|').append(node.getExactTime());
				builder.append('|').append(node.isIgnored());

				try {
					mFloatMessageService.sendTcpCommand(builder.toString());
				} catch (RemoteException e) {
					e.printStackTrace();
				}
			}

			mAdapter.notifyDataSetChanged();
			break;
		}
	}

	@Override
	public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
		RedPacketCode node = (RedPacketCode) mAdapter.getItem(position);

		mEditTextCode.setText(node.getCode());
		mCheckBoxIgnore.setChecked(node.isIgnored());

		Date date = new Date(node.getExactTime());
		mTimePicker.setCurrentHour(date.getHours());
		mTimePicker.setCurrentMinute(date.getMinutes());
	}
}
