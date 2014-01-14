package com.cavan.eventrepeater;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;

public class MainActivity extends Activity implements OnCheckedChangeListener {
	private static final String TAG = "Cavan";

	private CheckBox mCheckBoxStartService;
	private BroadcastReceiver mReceiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context arg0, Intent arg1) {
			String action = arg1.getAction();
			Log.d(TAG, "action = " + action);

			int state = arg1.getIntExtra("state", -1);
			switch (state) {
			case EventRepeater.STATE_ERROR:
				Log.d(TAG, "EventRepeater.STATE_ERROR");
				break;

			case EventRepeater.STATE_RUNNING:
				Log.d(TAG, "EventRepeater.STATE_RUNNING");
				break;

			case EventRepeater.STATE_START:
				Log.d(TAG, "EventRepeater.STATE_START");
				break;

			case EventRepeater.STATE_STOPPED:
				Log.d(TAG, "EventRepeater.STATE_STOPPED");
				break;

			case EventRepeater.STATE_STOPPING:
				Log.d(TAG, "EventRepeater.STATE_STOPPING");
				break;
			}
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		mCheckBoxStartService = (CheckBox) findViewById(R.id.checkBoxStartService);
		mCheckBoxStartService.setOnCheckedChangeListener(this);
		mCheckBoxStartService.setChecked(true);
	}

	@Override
	protected void onPause() {
		unregisterReceiver(mReceiver);
		super.onPause();
	}

	@Override
	protected void onResume() {
		IntentFilter filter = new IntentFilter(EventRepeater.ACTION_STATE_CHANGED);
		registerReceiver(mReceiver, filter);
		super.onResume();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate(R.menu.activity_main, menu);
		return true;
	}

	@Override
	public void onCheckedChanged(CompoundButton arg0, boolean arg1) {
		Intent intent = new Intent(getApplicationContext(), EventRepeater.class);
		if (arg1) {
			startService(intent);
		} else {
			stopService(intent);
		}
	}
}
