package com.cavan.location;

import java.util.HashMap;
import java.util.List;

import android.app.Activity;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ListView;

import com.cavan.android.CavanAndroid;
import com.cavan.cavanmain.CavanMessageView;
import com.cavan.cavanmain.R;

public class ShowLocationActivity extends Activity implements LocationListener {

	private LocationManager mManager;
	private ListView mListViewLocations;
	
	private String[] mProviders = new String[0];
	private HashMap<String, Location> mLocationMap = new HashMap<String, Location>();

	private BaseAdapter mAdapter = new BaseAdapter() {

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			CavanMessageView view;

			if (convertView != null) {
				view = (CavanMessageView) convertView;
			} else {
				view = CavanMessageView.getInstance(ShowLocationActivity.this);
			}

			CharSequence title;
			CharSequence content;
			String provider = mProviders[position];
			Location location = mLocationMap.get(provider);

			if (location == null) {
				title = provider;

				if (mManager.isProviderEnabled(provider)) {
					content = "unknown";
				} else {
					content = "disabled";
				}
			} else {
				StringBuilder builder = new StringBuilder();
				
				builder.append(provider).append(": ");
				builder.append(location.getTime());

				if (location.isFromMockProvider()) {
					builder.append(" mock");
				}

				title = builder.toString();

				builder = new StringBuilder();

				builder.append("Latitude = ").append(location.getLatitude());
				builder.append("\nLongitude = ").append(location.getLongitude());

				if (location.hasAltitude()) {
					builder.append("\nAltitude = ").append(location.getAltitude());
				}

				if (location.hasSpeed()) {
					builder.append("\nAltitude = ").append(location.getAltitude());
				}

				if (location.hasBearing()) {
					builder.append("\nBearing = ").append(location.getBearing());
				}

				if (location.hasAccuracy()) {
					builder.append("\nAccuracy = ").append(location.getAccuracy());
				}

				content = builder.toString();
			}

			view.setTitle(title);
			view.setContent(content, getPackageName(), null);

			return view;
		}

		@Override
		public long getItemId(int position) {
			return position;
		}

		@Override
		public Object getItem(int position) {
			return mProviders[position];
		}

		@Override
		public int getCount() {
			return mProviders.length;
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_show_location);

		mManager = (LocationManager) getSystemService(LOCATION_SERVICE);

		mListViewLocations = (ListView) findViewById(R.id.listViewLocations);
		mListViewLocations.setAdapter(mAdapter);
	}

	@Override
	protected void onResume() {
		super.onResume();

		List<String> providers = mManager.getAllProviders();

		mProviders = new String[providers.size()];
		mLocationMap.clear();

		int i = 0;

		for (String provider : providers) {
			mLocationMap.put(provider, mManager.getLastKnownLocation(provider));
			mManager.requestLocationUpdates(provider, 0, 0, this);
			mProviders[i++] = provider;
		}

		mAdapter.notifyDataSetChanged();
	}

	@Override
	protected void onPause() {
		super.onPause();

		mManager.removeUpdates(this);
	}

	@Override
	public void onLocationChanged(Location location) {
		CavanAndroid.dLog("onLocationChanged: " + location);

		String provider = location.getProvider();
		mLocationMap.put(provider, location);
		mAdapter.notifyDataSetChanged();
	}

	@Override
	public void onStatusChanged(String provider, int status, Bundle extras) {
		CavanAndroid.dLog("onStatusChanged: " + provider + ", status = " + status);
	}

	@Override
	public void onProviderEnabled(String provider) {
		CavanAndroid.dLog("onProviderEnabled: " + provider);
	}

	@Override
	public void onProviderDisabled(String provider) {
		CavanAndroid.dLog("onProviderDisabled: " + provider);
	}
}
