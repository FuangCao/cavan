package com.cavan.wifi;

import java.util.Arrays;

import android.Manifest;
import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Color;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.TextView;

import com.cavan.android.CavanAndroid;
import com.cavan.cavanmain.R;

public class WifiDetectorActivity extends Activity {
	
	private static final int MSG_START_SCAN = 1;
	private static final int MSG_SCAN_COMPLETE = 2;

	private static final String[] PERMISSIONS = {
		Manifest.permission.ACCESS_COARSE_LOCATION,
	};

	private CavanAccessPoint mAccessPoint;
	private CavanAccessPoint[] mAccessPoints = new CavanAccessPoint[0];

	public class CavanAccessPoint implements Comparable<CavanAccessPoint> {

		public String SSID;
		public String BSSID;
		public int RSSI;
		public int MAX;
		public int COUNT;

		public CavanAccessPoint(String ssid) {
			SSID = ssid;
		}

		@Override
		public String toString() {
			StringBuilder builder = new StringBuilder();

			if (BSSID != null && BSSID.length() > 0) {
				builder.append(BSSID.toUpperCase());
			}

			builder.append('@').append(COUNT).append(" - ").append(SSID);
			builder.append(' ').append(MAX).append(' ').append(RSSI);

			return builder.toString();
		}

		@Override
		public int compareTo(CavanAccessPoint another) {
			if (this == mAccessPoint) {
				return -1;
			}

			if (another == mAccessPoint) {
				return 1;
			}

			return another.RSSI - RSSI;
		}
	}

	public class CavanAccessPointAdapter extends BaseAdapter implements OnItemClickListener {

		public CavanAccessPoint getAccessPoint(String ssid) {
			for (CavanAccessPoint point : mAccessPoints) {
				if (ssid.equals(point.SSID)) {
					return point;
				}
			}

			CavanAccessPoint[] points = new CavanAccessPoint[mAccessPoints.length + 1];
			System.arraycopy(mAccessPoints, 0, points, 0, mAccessPoints.length);

			CavanAccessPoint point = new CavanAccessPoint(ssid);
			points[mAccessPoints.length] = point;
			mAccessPoints = points;

			return point;
		}

		@Override
		public void notifyDataSetChanged() {
			for (ScanResult result : mWifiManager.getScanResults()) {
				if (result.SSID != null && result.SSID.length() > 0) {
					CavanAccessPoint point = getAccessPoint(result.SSID);
					point.BSSID = result.BSSID;
					point.RSSI = result.level;

					if (point.COUNT == 0) {
						point.MAX = point.RSSI;
					} else if (point.RSSI > point.MAX) {
						point.MAX = point.RSSI;
					}

					point.COUNT++;
				}
			}

			Arrays.sort(mAccessPoints);

			super.notifyDataSetChanged();
		}

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			TextView view;

			if (convertView == null) {
				view = new TextView(WifiDetectorActivity.this);
				view.setPadding(0, 30, 0, 30);
			} else {
				view = (TextView) convertView;
			}

			CavanAccessPoint point = mAccessPoints[position];
			view.setText(point.toString());

			if (point == mAccessPoint) {
				view.setTextColor(Color.WHITE);
				view.setBackgroundColor(Color.BLUE);
			} else {
				view.setTextColor(Color.BLACK);
				view.setBackgroundColor(Color.WHITE);
			}

			return view;
		}

		@Override
		public long getItemId(int position) {
			return position;
		}

		@Override
		public Object getItem(int position) {
			return mAccessPoints[position];
		}

		@Override
		public int getCount() {
			return mAccessPoints.length;
		}

		@Override
		public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
			mAccessPoint = mAccessPoints[position];
			notifyDataSetChanged();
		}
	};

	private boolean mPaused;
	private WifiManager mWifiManager;
	private ListView mListViewAccessPoints;
	private CavanAccessPointAdapter mAdapter = new CavanAccessPointAdapter();

	private Handler mHandler = new Handler() {

		@Override
		public void handleMessage(Message msg) {
			removeMessages(msg.what);

			if (mPaused) {
				return;
			}

			switch (msg.what) {
			case MSG_START_SCAN:
				CavanAndroid.dLog("MSG_START_SCAN");
				mWifiManager.startScan();
				sendEmptyMessageDelayed(MSG_START_SCAN, 5000);
				break;

			case MSG_SCAN_COMPLETE:
				CavanAndroid.dLog("MSG_SCAN_COMPLETE");
				sendEmptyMessage(MSG_START_SCAN);
				mAdapter.notifyDataSetChanged();
				break;

			default:
				CavanAndroid.eLog("Invalid message: " + msg.what);
			}
		}
	};

	private BroadcastReceiver mReceiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {
			CavanAndroid.dLog("action = " + intent.getAction());

			mHandler.sendEmptyMessage(MSG_SCAN_COMPLETE);
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_wifi_detector);

		mWifiManager = (WifiManager) getSystemService(WIFI_SERVICE);

		mListViewAccessPoints = (ListView) findViewById(R.id.listViewAccessPoints);
		mListViewAccessPoints.setOnItemClickListener(mAdapter);
		mListViewAccessPoints.setAdapter(mAdapter);

		CavanAndroid.checkAndRequestPermissions(this, 100, PERMISSIONS);
	}

	@Override
	protected void onResume() {
		super.onResume();
		mPaused = false;

		IntentFilter filter = new IntentFilter(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION);
		registerReceiver(mReceiver, filter);

		mHandler.sendEmptyMessage(MSG_SCAN_COMPLETE);
	}

	@Override
	protected void onPause() {
		mPaused = true;
		unregisterReceiver(mReceiver);
		super.onPause();
	}
}
