package com.cavan.cavanmain;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Build;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.support.v7.app.ActionBarActivity;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

import com.cavan.cavanutils.CavanUtils;

@SuppressLint("HandlerLeak") @SuppressWarnings("deprecation")
@TargetApi(Build.VERSION_CODES.HONEYCOMB) public class MainActivity extends ActionBarActivity implements OnClickListener {

	public static final String TAG = "Cavan";

	private Button mButtonTcpDdService;

	private BroadcastReceiver mReceiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {
			String action = intent.getAction();
			CavanUtils.logE("action = " + action);

			if (action.equals(CavanService.ACTION_TCP_DD_CHANGED)) {
				if (intent.getBooleanExtra("state", false)) {
					mButtonTcpDdService.setText(R.string.text_stop);
				} else {
					mButtonTcpDdService.setText(R.string.text_start);
				}
			} else if (action.equals(CavanService.ACTION_FTP_CHANGED)) {
			} else if (action.equals(CavanService.ACTION_WEB_PROXY_CHANGED)) {
			}
		}

	};

	private ICavanService mService;
	private ServiceConnection mConnection = new ServiceConnection() {

		@Override
		public void onServiceDisconnected(ComponentName arg0) {
			CavanUtils.logE("onServiceDisconnected");
			mService = null;
		}

		@Override
		public void onServiceConnected(ComponentName arg0, IBinder arg1) {
			CavanUtils.logE("onServiceConnected");
			mService = ICavanService.Stub.asInterface(arg1);
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		mButtonTcpDdService = (Button) findViewById(R.id.buttonTcpDdService);
		mButtonTcpDdService.setOnClickListener(this);

		Intent service = new Intent(this, CavanService.class);
		startService(service);
		bindService(service, mConnection, BIND_AUTO_CREATE);
	}

	@Override
	protected void onDestroy() {
		unbindService(mConnection);
		super.onDestroy();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		int id = item.getItemId();
		if (id == R.id.action_settings) {
			return true;
		}
		return super.onOptionsItemSelected(item);
	}

	@Override
	protected void onPause() {
		unregisterReceiver(mReceiver);
		super.onPause();
	}

	@Override
	protected void onResume() {
		IntentFilter filter = new IntentFilter();
		filter.addAction(CavanService.ACTION_FTP_CHANGED);
		filter.addAction(CavanService.ACTION_TCP_DD_CHANGED);
		filter.addAction(CavanService.ACTION_WEB_PROXY_CHANGED);

		registerReceiver(mReceiver, filter);

		super.onResume();
	}

	@Override
	public void onClick(View arg0) {
		if (mService == null) {
			return;
		}

		try {
			switch (arg0.getId()) {
			case R.id.buttonTcpDdService:
				mService.startTcpDdService();
				break;
			}
		} catch (RemoteException e) {
			e.printStackTrace();
		}
	}
}
