package com.cavan.eventrepeater;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;

import org.apache.http.entity.InputStreamEntity;

import android.app.Service;
import android.content.Intent;
import android.media.MediaRecorder.OutputFormat;
import android.os.IBinder;
import android.util.Log;

public class EventRepeater extends Service {
	private static final String TAG = "Cavan";
	private static final int MIN_DAEMON_COUNT = 10;
	public static final String ACTION_STATE_CHANGED = "com.cavan.eventrepeater.STATE_CHANGED";

	public static final int STATE_ERROR = -1;
	public static final int STATE_STOPPED = 0;
	public static final int STATE_STOPPING = 1;
	public static final int STATE_START = 2;
	public static final int STATE_RUNNING = 3;

	private ServerSocket mServerSocket;
	private int mDaemonCount;
	private int mDaemonUsed;

	@Override
	public IBinder onBind(Intent arg0) {
		return null;
	}

	@Override
	public void onCreate() {
		Log.d(TAG, "onCreate()");
		super.onCreate();
	}

	@Override
	public void onDestroy() {
		Log.d(TAG, "onDestroy()");

		if (mServerSocket != null) {
			try {
				mServerSocket.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}

		super.onDestroy();
	}

	private void notifyStateChanged(int state) {
		Intent intent = new Intent(ACTION_STATE_CHANGED);
		intent.putExtra("state", state);
		sendStickyBroadcast(intent);
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		Log.d(TAG, "onStartCommand()");

		notifyStateChanged(STATE_START);

		try {
			mServerSocket = new ServerSocket(3333);
		} catch (IOException e) {
			e.printStackTrace();
			notifyStateChanged(STATE_ERROR);
			return START_NOT_STICKY;
		}

		DaemonThread thread = new DaemonThread();
		thread.start();

		notifyStateChanged(STATE_RUNNING);

		return super.onStartCommand(intent, flags, startId);
	}

	private boolean run(InputStream inputStream, OutputStream outputStream) throws IOException {
		while (true) {
			int lb = inputStream.read();
			if (lb < 0) {
				break;
			}

			int hb = inputStream.read();
			if (hb < 0) {
				break;
			}

			int length = hb << 8 | lb;
			Log.d(TAG, "length = " + length);

			byte[] buff = new byte[length];
			length = inputStream.read(buff);
			if (length < 0) {
				break;
			}

			Log.d(TAG, new String(buff));
		}
		
		return true;
	}

	private boolean run(Socket client) {
		InputStream inputStream;
		try {
			inputStream = client.getInputStream();
		} catch (IOException e) {
			e.printStackTrace();
			return false;
		}

		OutputStream outputStream;
		try {
			outputStream = client.getOutputStream();
		} catch (IOException e) {
			e.printStackTrace();
			try {
				inputStream.close();
			} catch (IOException e1) {
				e1.printStackTrace();
			}
			return false;
		}

		boolean res;
		try {
			res = run(inputStream, outputStream);
		} catch (IOException e1) {
			e1.printStackTrace();
			res = false;
		}

		try {
			outputStream.close();
		} catch (IOException e) {
			e.printStackTrace();
		}

		try {
			inputStream.close();
		} catch (IOException e) {
			e.printStackTrace();
		}

		return res;
	}

	private void mainLoop() {
		int index;

		synchronized (this) {
			index = mDaemonCount++;
			Log.d(TAG, "Daemon[" + index + "] Entry (" + mDaemonUsed + " / " + mDaemonCount + ")");
		}

		while (true) {
			Socket client;

			Log.d(TAG, "Daemon[" + index + "] Listening (" + mDaemonUsed + " / " + mDaemonCount + ")");
			try {
				client = mServerSocket.accept();
				if (client == null) {
					break;
				}
			} catch (IOException e) {
				e.printStackTrace();
				break;
			}

			synchronized (this) {
				mDaemonUsed++;
				Log.d(TAG, "Daemon[" + index + "] Running (" + mDaemonUsed + " / " + mDaemonCount + ")");

				if (mDaemonUsed >= mDaemonCount) {
					DaemonThread thread = new DaemonThread();
					thread.start();
				}
			}

			run(client);

			try {
				client.close();
			} catch (IOException e) {
				e.printStackTrace();
			}

			synchronized (this) {
				mDaemonUsed--;
				Log.d(TAG, "Daemon[" + index + "] Stopped (" + mDaemonUsed + " / " + mDaemonCount + ")");

				if (mDaemonCount - mDaemonUsed > MIN_DAEMON_COUNT) {
					break;
				}
			}
		}

		synchronized (this) {
			mDaemonCount--;
			if (mDaemonCount <= 0) {
				try {
					mServerSocket.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
				mServerSocket = null;
				notifyStateChanged(STATE_STOPPED);
			}
			Log.d(TAG, "Daemon[" + index + "] Exit (" + mDaemonUsed + " / " + mDaemonCount + ")");
		}
	}

	class DaemonThread extends Thread {
		@Override
		public void run() {
			mainLoop();
		}
	}
}