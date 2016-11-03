package com.cavan.android;

import java.net.InetAddress;
import java.util.ArrayList;
import java.util.List;

import android.annotation.SuppressLint;
import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.RemoteException;

@SuppressLint("DefaultLocale")
public abstract class DiscoveryService extends Service {

	public static final String ACTION_SCAN_RESULT_CHANGED = "cavan.discovery.intent.action.SCAN_RESULT_CHANGED";

	private List<ScanResult> mScanResults = new ArrayList<ScanResult>();

	protected abstract boolean startScan(int port);

	@Override
	public IBinder onBind(Intent intent) {
		return mBinder;
	}

	private IDiscoveryService.Stub mBinder = new IDiscoveryService.Stub() {

		@Override
		public boolean scan(int port) throws RemoteException {
			mScanResults.clear();

			if (port <= 0) {
				port = 8888;
			}

			if (!startScan(port)) {
				return false;
			}

			// Toast.makeText(getApplicationContext(), R.string.text_scanning, Toast.LENGTH_SHORT).show();

			return true;
		}

		@Override
		public List<ScanResult> getScanResult() throws RemoteException {
			return mScanResults;
		}
	};

	synchronized public void addScanResult(ScanResult result) {
		CavanAndroid.dLog("result = " + result);

		mScanResults.add(result);

		Intent intent = new Intent(ACTION_SCAN_RESULT_CHANGED);
		sendBroadcast(intent);
	}

	public void addScanResult(int port, String hostname, InetAddress address) {
		ScanResult result = new ScanResult(port, hostname, address);
		addScanResult(result);
	}
}
