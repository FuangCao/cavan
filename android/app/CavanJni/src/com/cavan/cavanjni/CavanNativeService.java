package com.cavan.cavanjni;


public abstract class CavanNativeService extends CavanService {

	protected int mPort;
	protected boolean mEnabled;

	class MyThread extends Thread {

		@Override
		public void run() {
			setServiceState(STATE_PREPARE);

			while (mEnabled) {
				setServiceState(STATE_RUNNING);
				doMainLoop(mPort);

				setServiceState(STATE_WAITING);

				try {
					for (int i = 0; i < 10 && mEnabled; i++) {
						Thread.sleep(200);
					}
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}

			setServiceState(STATE_STOPPED);
		}
	}

	public abstract int getDefaultPort();
	public abstract boolean doStopService();
	protected abstract void doMainLoop(int port);

	public CavanNativeService() {
		super();
		mPort = getDefaultPort();
	}

	public String getServiceAction() {
		return "cavan.intent.action." + getServiceName();
	}

	@Override
	public void onCreate() {
		CavanJni.setupEnv(this);
		super.onCreate();
	}

	@Override
	public void start(int port) {
		mEnabled = true;

		if (mState != STATE_STOPPED) {
			sendStateBroadcast(mState);
		} else {
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
