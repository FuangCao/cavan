package com.cavan.remotecontrol;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.InetAddress;
import java.net.MulticastSocket;
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

@SuppressLint("DefaultLocale")
public class DiscoveryService extends Service {

	private static final String TAG = "Cavan";

	public static final String ACTION_SCAN_RESULT_CHANGED = "cavan.discovery.intent.action.SCAN_RESULT_CHANGED";

	private InetAddress mAddress;
	private MulticastSocket mSocket;
	private List<ScanResult> mScanResults = new ArrayList<ScanResult>();
	private Pattern mPattern = Pattern.compile("TCP_DD:\\s*port\\s*=\\s*([0-9]+),\\s*hostname\\s*=\\s*(.*)");

	@Override
	public IBinder onBind(Intent intent) {
		return mBinder;
	}

	@Override
	public void onCreate() {

		try {
			mSocket = new MulticastSocket();
			mAddress = InetAddress.getByName("224.0.0.1");
		} catch (IOException e) {
			e.printStackTrace();
		}

		super.onCreate();
	}

	@Override
	public void onDestroy() {
		if (mSocket != null) {
			mSocket.close();
		}

		super.onDestroy();
	}

	private IDiscoveryService.Stub mBinder = new IDiscoveryService.Stub() {

		@Override
		public boolean startDiscovery(int port) throws RemoteException {
			if (mSocket == null || mAddress == null) {
				return false;
			}

			mScanResults.clear();

			DiscoveryThread thread = new DiscoveryThread(port);
			thread.start();

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
			byte[] bytes = "cavan-discovery".getBytes();
			DatagramPacket packet = new DatagramPacket(bytes, bytes.length, mAddress, mPort);
			try {
				mSocket.send(packet);
			} catch (IOException e1) {
				e1.printStackTrace();
				return;
			}

			if (mThreadStopped) {
				mThreadStopped = false;

				bytes = new byte[1024];
				packet.setData(bytes);

				while (mSocket != null) {
					try {
						mSocket.receive(packet);
						String text = new String(packet.getData(), 0, packet.getLength());
						onDiscovery(packet.getAddress(), text);
					} catch (IOException e) {
						e.printStackTrace();
						break;
					}
				}

				mThreadStopped = true;
			}
		}
	}
}
