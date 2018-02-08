package com.cavan.cavanjni;

import com.cavan.android.CavanDaemonThread;
import com.cavan.service.CavanService;
import com.cavan.service.CavanServiceState;


public abstract class CavanNativeService extends CavanService {

	private int mPort;
	private CavanNativeCommand mCommand;
	private CavanDaemonThread mDaemonThread = new CavanDaemonThread() {

		@Override
		protected void onDaemonStarted() {
			setServiceState(CavanServiceState.PREPARE);
		}

		@Override
		protected void onDaemonStopped() {
			setServiceState(CavanServiceState.STOPPED);
		}

		@Override
		protected void onDaemonWaiting() {
			setServiceState(CavanServiceState.WAITING);
		}

		@Override
		protected void onDaemonRuning() {
			setServiceState(CavanServiceState.RUNNING);
		}

		@Override
		protected void onDaemonStopping() {
			mCommand.kill();
		}

		@Override
		protected void mainLoop() {
			mCommand.run(getCommandArgs(mPort));
		}
	};

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
		if (mPort != port) {
			mPort = port;
			mDaemonThread.restart();
		} else {
			mDaemonThread.start();
		}
	}

	@Override
	public int getPort() {
		return mPort;
	}

	@Override
	public void stop() {
		mDaemonThread.stop();
	}

	@Override
	public boolean isEnabled() {
		return mDaemonThread.isEnabled();
	}
}
