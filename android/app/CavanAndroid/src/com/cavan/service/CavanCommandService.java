package com.cavan.service;

import java.io.File;
import java.io.IOException;
import java.util.HashSet;

import com.cavan.android.CavanAndroid;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.RemoteException;

public class CavanCommandService extends Service implements Runnable {

	private HashSet<ICavanCommandServiceCallback> mCallbacks = new HashSet<ICavanCommandServiceCallback>();
	private Thread mDaemonThread = new Thread(this);
	private Process mProcess;
	private boolean mEnabled;
	private String mCommand;
	private int mState;

	private ICavanCommandService.Stub mBinder = new ICavanCommandService.Stub() {

		@Override
		public void stop() throws RemoteException {
			stopDaemonThread();
		}

		@Override
		public void start(String command) throws RemoteException {
			startDaemonThread(command);
		}

		@Override
		public boolean removeCallback(ICavanCommandServiceCallback callback) throws RemoteException {
			synchronized (mCallbacks) {
				return mCallbacks.remove(callback);
			}
		}

		@Override
		public boolean isEnabled() throws RemoteException {
			return mEnabled;
		}

		@Override
		public int getState() throws RemoteException {
			return mState;
		}

		@Override
		public boolean addCallback(final ICavanCommandServiceCallback callback) throws RemoteException {
			synchronized (mCallbacks) {
				if (!mCallbacks.add(callback)) {
					return false;
				}

				callback.asBinder().linkToDeath(new DeathRecipient() {

					@Override
					public void binderDied() {
						synchronized (mCallbacks) {
							mCallbacks.remove(callback);
						}
					}
				}, 0);

				callback.onServiceStateChanged(mState);
			}

			return true;
		}

		@Override
		public String getCommand() throws RemoteException {
			return mCommand;
		}
	};

	public synchronized void setServiceEnable(boolean enable) {
		mEnabled = enable;
	}

	public synchronized boolean isServiceEnabled() {
		return mEnabled;
	}

	public synchronized void setServiceState(int state) {
		if (mState == state) {
			return;
		}

		mState = state;

		synchronized (mCallbacks) {
			for (ICavanCommandServiceCallback callback : mCallbacks) {
				try {
					callback.onServiceStateChanged(state);
				} catch (RemoteException e) {
					e.printStackTrace();
				}
			}
		}
	}

	public synchronized int getServiceState() {
		return mState;
	}

	public synchronized void startDaemonThread(String command) {
		mEnabled = true;

		synchronized (mDaemonThread) {
			mCommand = command;

			if (mDaemonThread.isAlive()) {
				mDaemonThread.notify();
			} else {
				mDaemonThread.start();
			}
		}
	}

	public synchronized void stopDaemonThread() {
		mEnabled = false;

		doCommandStop();

		synchronized (mDaemonThread) {
			if (mDaemonThread.isAlive()) {
				mDaemonThread.notify();
			}
		}
	}

	@Override
	public IBinder onBind(Intent intent) {
		return mBinder;
	}

	@Override
	public void run() {
		while (true) {
			while (isServiceEnabled()) {
				setServiceState(CavanServiceState.RUNNING);
				doCommandMain(mCommand);
				setServiceState(CavanServiceState.WAITING);

				if (!isServiceEnabled()) {
					break;
				}

				synchronized (mDaemonThread) {
					try {
						mDaemonThread.wait(2000);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
			}

			setServiceState(CavanServiceState.STOPPED);

			synchronized (mDaemonThread) {
				try {
					mDaemonThread.wait();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
		}
	}

	private String[] buildEnvs() {
		File dir = getDir("bin", MODE_PRIVATE);
		if (dir == null) {
			return null;
		}

		String path = System.getenv("PATH");
		if (path == null) {
			path = dir.getAbsolutePath();
		} else {
			path += ":" + dir.getAbsolutePath();
		}

		CavanAndroid.dLog("path = " + path);

		return new String[] { "PATH=" + path };
	}

	protected int doCommandMain(String command) {
		CavanAndroid.dLog("command = " + command);

		if (command == null || command.isEmpty()) {
			return -1;
		}

		Process process = null;

		try {
			String[] args = { "/system/bin/sh", "-c", command };

			process = Runtime.getRuntime().exec(args, buildEnvs());

			synchronized (mDaemonThread) {
				mProcess = process;
			}

			process.waitFor();

			return process.exitValue();
		} catch (IOException e) {
			e.printStackTrace();
		} catch (InterruptedException e) {
			e.printStackTrace();
		} finally {
			synchronized (mDaemonThread) {
				mProcess = null;
			}

			if (process != null) {
				process.destroy();
			}
		}

		return -1;
	}

	protected void doCommandStop() {
		synchronized (mDaemonThread) {
			if (mProcess != null) {
				mProcess.destroy();
			}
		}
	}
}
