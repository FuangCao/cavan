package com.cavan.remotecontrol;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import android.annotation.SuppressLint;
import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;
import android.widget.Toast;

import com.cavan.cavanutils.CavanNetworkClient;

@SuppressLint("DefaultLocale")
public class DiscoveryService extends Service {

	private static final String TAG = MainActivity.TAG;

	public static final String ACTION_SCAN_RESULT_CHANGED = "cavan.discovery.intent.action.SCAN_RESULT_CHANGED";

	private CavanNetworkClient mClient;
	private List<ScanResult> mScanResults = new ArrayList<ScanResult>();
	private Pattern mPattern = Pattern.compile("TCP_DD:\\s*port\\s*=\\s*([0-9]+),\\s*hostname\\s*=\\s*(.*)");

	@Override
	public IBinder onBind(Intent intent) {
		return mBinder;
	}

	@Override
	public void onCreate() {

		try {
			mClient = new CavanNetworkClient(InetAddress.getByName("224.0.0.1"), 8888, true);
		} catch (UnknownHostException e) {
			e.printStackTrace();
		}

		super.onCreate();
	}

	@Override
	public void onDestroy() {
		if (mClient != null) {
			mClient.disconnect();
		}

		super.onDestroy();
	}

	private IDiscoveryService.Stub mBinder = new IDiscoveryService.Stub() {

		@Override
		public boolean startDiscovery(int port) throws RemoteException {
			if (mClient == null) {
				return false;
			}

			mScanResults.clear();

			DiscoveryThread thread = new DiscoveryThread(port);
			thread.start();
			Toast.makeText(getApplicationContext(), R.string.text_scanning, Toast.LENGTH_SHORT).show();

			return false;
		}

		@Override
		public List<ScanResult> getScanResult() throws RemoteException {
			return mScanResults;
		}
	};

	private void onDiscovery(InetAddress address, String text) {
		Matcher matcher = mPattern.matcher(text);
		if (matcher.find()) {
			String port = matcher.group(1);
			String hostname = matcher.group(2);

			ScanResult result = new ScanResult(Integer.valueOf(port), hostname, address);
			Log.e(TAG, "result = " + result);

			mScanResults.add(result);

			Intent intent = new Intent(ACTION_SCAN_RESULT_CHANGED);
			sendBroadcast(intent);
		}
	}

	private boolean mThreadStopped = true;

	class DiscoveryThread extends Thread {

		int mPort = 8888;

		public DiscoveryThread(int port) {
			super();

			if (port > 0) {
				mPort = port;
			}
		}

		@Override
		public void run() {
			if (!mClient.sendData("cavan-discovery".getBytes())) {
				return;
			}

			if (mThreadStopped) {
				mThreadStopped = false;

				byte[] bytes = new byte[1024];

				while (mClient != null) {
					int length = mClient.recvData(bytes);
					if (length <= 0) {
						break;
					}

					String text = new String(bytes, 0, length);
					onDiscovery(mClient.getRemoteAddress(), text);
				}

				mThreadStopped = true;
			}
		}
	}
}
