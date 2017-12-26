package com.cavan.cavanjni;

import com.cavan.service.CavanService;
import com.cavan.service.CavanServiceState;


public abstract class CavanNativeService extends CavanService {

	protected int mPort;
	protected boolean mEnabled;

	class MyThread extends Thread {

		@Override
		public void run() {
			setServiceState(CavanServiceState.PREPARE);

			while (mEnabled) {
				setServiceState(CavanServiceState.RUNNING);
				doMainLoop(mPort);

				setServiceState(CavanServiceState.WAITING);

				try {
					for (int i = 0; i < 10 && mEnabled; i++) {
						Thread.sleep(200);
					}
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}

			setServiceState(CavanServiceState.STOPPED);
		}
	}

	public abstract int getDefaultPort();
	public abstract boolean doStopService();
	protected abstract void doMainLoop(int port);

	public CavanNativeService() {
		super();
		mPort = getDefaultPort();
	}

	@Override
	public void onCreate() {
		CavanJni.setupEnv(this);
		super.onCreate();
	}

	@Override
	public void start(int port) {
		mEnabled = true;

		if (mState == CavanServiceState.STOPPED) {
			mPort = port;
			new MyThread().start();
		}
	}

	@Override
	public int getPort() {
		return mPort;
	}

	@Override
	public void stop() {
		mEnabled = false;
		doStopService();
	}

	@Override
	public boolean isEnabled() {
		return mEnabled;
	}
}
