package com.cavan.cavanmain;

import android.annotation.SuppressLint;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.v7.app.ActionBarActivity;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

import com.cavan.cavanutils.CavanNative;
import com.cavan.cavanutils.CavanUtils;

@SuppressWarnings("deprecation")
@SuppressLint({ "NewApi", "HandlerLeak" }) public class MainActivity extends ActionBarActivity {

	public static final String TAG = "Cavan";

	private static final int EVENT_TCP_DD_SERVICE_STATE_CHANGED = 1;

	private TcpDdThread mTcpDdThread;
	private Button mButtonTcpDdService;

	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case EVENT_TCP_DD_SERVICE_STATE_CHANGED:
				if (mTcpDdThread == null) {
					mButtonTcpDdService.setText(R.string.text_start);
				} else {
					mButtonTcpDdService.setText(R.string.text_stop);
				}
				break;
			}
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		mButtonTcpDdService = (Button) findViewById(R.id.buttonTcpDdService);
		mButtonTcpDdService.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				if (mTcpDdThread == null) {
					mTcpDdThread = new TcpDdThread();
					mTcpDdThread.start();
				}
			}
		});
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

	class TcpDdThread extends Thread {
		@Override
		public void run() {
			CavanUtils.logD("doTcpDdServer running");
			mHandler.sendEmptyMessage(EVENT_TCP_DD_SERVICE_STATE_CHANGED);
			CavanNative.doTcpDdServer("-p", "9999", "-s", "0");
			mTcpDdThread = null;
			mHandler.sendEmptyMessage(EVENT_TCP_DD_SERVICE_STATE_CHANGED);
			CavanUtils.logD("doTcpDdServer stopped");
		}
	}
}
