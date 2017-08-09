package com.cavan.bluetoothinterphone;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.UUID;

import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothServerSocket;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.os.IBinder;
import android.os.RemoteException;

import com.cavan.android.CavanAndroid;
import com.cavan.java.CavanDaemonThread;

public class InterphoneService extends Service {

	public static final UUID SERVICE_UUID = UUID.fromString("00002888-0000-1000-8000-00805f9b34fb");

	private BluetoothAdapter mAdapter = BluetoothAdapter.getDefaultAdapter();

	private BluetoothSocket mSocket;
	private InputStream mInputStream;
	private OutputStream mOutputStream;
	private BluetoothServerSocket mServerSocket;

	public synchronized InputStream getInputStream() {
		return mInputStream;
	}

	public synchronized OutputStream getOutputStream() {
		return mOutputStream;
	}

	public synchronized BluetoothServerSocket getServerSocket() {
		return mServerSocket;
	}

	private CavanDaemonThread mDaemonThread = new CavanDaemonThread() {

		@Override
		public synchronized void onDaemonStop() {
			if (mServerSocket != null) {
				try {
					mServerSocket.close();
				} catch (IOException e) {
					e.printStackTrace();
				}

				mServerSocket = null;
			}

			if (mSocket != null) {
				try {
					mSocket.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}

		@Override
		protected boolean runDaemon() {
			BluetoothServerSocket ssocket = mServerSocket;

			if (ssocket == null) {
				return true;
			}

			BluetoothSocket socket;

			try {
				CavanAndroid.dLog("Start: accept");
				socket = ssocket.accept();
				CavanAndroid.dLog("End: accept");
			} catch (IOException e) {
				CavanAndroid.dLog("Exit: accept");
				e.printStackTrace();
				return true;
			}

			InputStream istream = null;
			OutputStream ostream = null;

			try {
				istream = socket.getInputStream();
				ostream = socket.getOutputStream();

				synchronized (this) {
					mSocket = socket;
					mInputStream = istream;
					mOutputStream = ostream;
				}

				while (true) {
					byte[] bytes = new byte[4096];
					int length = istream.read(bytes);

					if (length < 0) {
						break;
					}

					ostream.write(bytes, 0, length);
				}
			} catch (IOException e) {
				e.printStackTrace();
			} finally {
				synchronized (this) {
					mSocket = null;
					mInputStream = null;
					mOutputStream = null;
				}

				if (istream != null) {
					try {
						istream.close();
					} catch (IOException e) {
						e.printStackTrace();
					}
				}

				if (ostream != null) {
					try {
						ostream.close();
					} catch (IOException e) {
						e.printStackTrace();
					}
				}

				try {
					socket.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}

			return false;
		}
	};

	private IInterphoneService.Stub mBinder = new IInterphoneService.Stub() {

		@Override
		public synchronized boolean start() throws RemoteException {
			if (mServerSocket == null) {
				try {
					mServerSocket = mAdapter.listenUsingInsecureRfcommWithServiceRecord("BluetoothInterphone", SERVICE_UUID);
				} catch (IOException e) {
					e.printStackTrace();
					return false;
				}
			}

			mDaemonThread.startDaemon();

			return true;
		}

		@Override
		public synchronized void stop() throws RemoteException {
			mDaemonThread.stopDaemon();
		}
	};

	@Override
	public void onCreate() {
		super.onCreate();
		CavanAndroid.pLog();
	}

	@Override
	public void onDestroy() {
		CavanAndroid.pLog();
		super.onDestroy();
	}

	@Override
	public IBinder onBind(Intent intent) {
		CavanAndroid.pLog();
		return mBinder;
	}
}
