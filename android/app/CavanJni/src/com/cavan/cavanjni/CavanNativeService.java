package com.cavan.cavanjni;

import com.cavan.service.CavanService;
import com.cavan.service.CavanServiceState;


public abstract class CavanNativeService extends CavanService {

	protected int mPort;
	protected boolean mEnabled;
	protected CavanNativeCommand mCommand;

	class DaemonThread extends Thread {

		@Override
		public void run() {
			setServiceState(CavanServiceState.PREPARE);

			while (mEnabled) {
				setServiceState(CavanServiceState.RUNNING);
				mCommand.run(getCommandArgs(mPort));
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
	protected abstract String[] getCommandArgs(int port);

	public CavanNativeService(CavanNativeCommand command) {
		mCommand = command;
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
			new DaemonThread().start();
		}
	}

	@Override
	public int getPort() {
		return mPort;
	}

	@Override
	public void stop() {
		mEnabled = false;
		mCommand.kill();
	}

	@Override
	public boolean isEnabled() {
		return mEnabled;
	}
}
