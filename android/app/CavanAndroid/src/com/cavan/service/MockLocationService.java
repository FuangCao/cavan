package com.cavan.service;

import java.net.ServerSocket;
import java.util.Random;

import android.location.Criteria;
import android.location.Location;
import android.location.LocationManager;
import android.location.LocationProvider;
import android.os.SystemClock;

import com.cavan.android.CavanAndroid;
import com.cavan.java.CavanTcpPacketService;

public class MockLocationService extends CavanService {

	private static final String NAME = "MOCK_LOCATION";
	private static final long UPDATE_LOCATION_DELAY = 1000;
	private static final String PROVIDER = LocationManager.GPS_PROVIDER;

	private LocationManager mLocationManager;
	private Location mLocation = new Location(PROVIDER);
	private Random mRandom = new Random();
	private boolean mMockEnabled;

	private Runnable mRunnableUpdataLocation = new Runnable() {

		@Override
		public void run() {
			updateLocation();
		}
	};

	private CavanTcpPacketService mTcpPacketService = new CavanTcpPacketService(9099) {

		@Override
		protected void onServerOpened(ServerSocket socket) {
			setServiceState(CavanServiceState.RUNNING);
		}

		@Override
		protected void onServerClosed() {
			if (isEnabled()) {
				setServiceState(CavanServiceState.WAITING);
			} else {
				setServiceState(CavanServiceState.STOPPED);
			}
		}

		@Override
		protected void onClientDisconnected(Client client) {
			if (getClientCount() <= 0) {
				setEnable(false);
			}
		}

		@Override
		protected boolean onPacketReceived(Client client, byte[] bytes, int length) {
			String text = new String(bytes, 0, length);
			CavanAndroid.dLog("onPacketReceived: " + text);
			String[] args = text.trim().split("\\s+");
			return onCommandReceived(args);
		}

		private synchronized boolean onCommandReceived(String args[]) {
			String command = args[0];

			if (command.equals("set_enable")) {
				setEnable(args.length > 1 && Boolean.parseBoolean(args[1]));
			} else if (command.equals("set_location")) {
				if (args.length > 2) {
					mLocation.setLatitude(Double.parseDouble(args[1]));
					mLocation.setLongitude(Double.parseDouble(args[2]));
					setEnable(true);
					mMessageQueue.postReplace(mRunnableUpdataLocation);
				}
			}

			return true;
		}
	};

	public synchronized boolean setEnable(boolean enabled) {
		if (enabled == mMockEnabled) {
			return true;
		}

		CavanAndroid.dLog("setEnable: " + enabled);

		try {
			if (enabled) {
				LocationProvider provider = mLocationManager.getProvider(PROVIDER);

				boolean requiresNetwork;
				boolean requiresSatellite;
				boolean requiresCell;
				boolean hasMonetaryCost;
				boolean supportsAltitude;
				boolean supportsSpeed;
				boolean supportsBearing;
				int powerRequirement;
				int accuracy;

				if (provider != null) {
					requiresNetwork = provider.requiresNetwork();
					requiresSatellite = provider.requiresSatellite();
					requiresCell = provider.requiresCell();
					hasMonetaryCost = provider.hasMonetaryCost();
					supportsAltitude = provider.supportsAltitude();
					supportsSpeed = provider.supportsSpeed();
					supportsBearing = provider.supportsBearing();
					powerRequirement = provider.getPowerRequirement();
					accuracy = provider.getAccuracy();
				} else {
					requiresNetwork = false;
					requiresSatellite = false;
					requiresCell = false;
					hasMonetaryCost = false;
					supportsAltitude = false;
					supportsSpeed = false;
					supportsBearing = false;
					powerRequirement = Criteria.POWER_HIGH;
					accuracy = Criteria.ACCURACY_FINE;
				}

				mLocationManager.addTestProvider(PROVIDER, requiresNetwork, requiresSatellite, requiresCell, hasMonetaryCost, supportsAltitude, supportsSpeed, supportsBearing, powerRequirement, accuracy);
				mLocationManager.setTestProviderStatus(PROVIDER, LocationProvider.AVAILABLE, null, System.currentTimeMillis());
				mLocationManager.setTestProviderEnabled(PROVIDER, true);
			} else {
				mLocationManager.removeTestProvider(PROVIDER);
			}

			mMockEnabled = enabled;
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			mMockEnabled = false;
		}

		return false;
	}

	public synchronized boolean updateLocation() {
		CavanAndroid.dLog("setLocation: Latitude = " + mLocation.getLatitude() + ", Longitude = " + mLocation.getLongitude());

		if (!mMockEnabled) {
			return false;
		}

		try {
			float accuracy = mRandom.nextFloat() * 10;
			if (accuracy < 3) {
				mLocation.setAccuracy(3);
			} else {
				mLocation.setAccuracy(accuracy);
			}

			mLocation.setTime(System.currentTimeMillis());

			if (CavanAndroid.SDK_VERSION > 17) {
				mLocation.setElapsedRealtimeNanos(SystemClock.elapsedRealtimeNanos());
			}

			mLocationManager.setTestProviderLocation(PROVIDER, mLocation);

			mMessageQueue.postUniqDelayed(mRunnableUpdataLocation, UPDATE_LOCATION_DELAY);
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}

		return true;
	}

	@Override
	public void onCreate() {
		super.onCreate();
		mLocationManager = (LocationManager) getSystemService(LOCATION_SERVICE);
	}

	@Override
	public String getServiceName() {
		return NAME;
	}

	@Override
	public void start(int port) {
		setServiceState(CavanServiceState.PREPARE);
		mTcpPacketService.open();
	}

	@Override
	public int getPort() {
		return mTcpPacketService.getPort();
	}

	@Override
	public void stop() {
		mTcpPacketService.close();
		setServiceState(CavanServiceState.STOPPED);
	}

	@Override
	public boolean isEnabled() {
		return mTcpPacketService.isEnabled();
	}
}
