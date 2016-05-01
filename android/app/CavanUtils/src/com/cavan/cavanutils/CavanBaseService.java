package com.cavan.cavanutils;

import android.content.Context;
import android.content.Intent;

public abstract class CavanBaseService {

	private int mPort;
	private String mName;
	private String mAction;
	private Context mContext;
	private boolean mStopped = true;

	public abstract void runDeamon(int port);

	public CavanBaseService(Context context, String name, String action, int port) {
		mPort = port;
		mName = name;
		mAction = action;
		mContext = context;
	}

	public int getPort() {
		return mPort;
	}

	public void setPort(int port) {
		mPort = port;
	}

	class MyThread extends Thread {

		@Override
		public void run() {
			if (mStopped) {
				CavanUtils.logE("Enter: service " + mName);
				mStopped = false;
				sendStateBroadcast(true);
				runDeamon(mPort);
				mStopped = true;
				sendStateBroadcast(false);
				CavanUtils.logE("Exit: service " + mName);
			} else {
				sendStateBroadcast(true);
			}
		}
	}

	private void sendStateBroadcast(boolean state) {
		Intent intent = new Intent(mAction);
		intent.putExtra("state", state);
		mContext.sendBroadcast(intent);
	}

	public void start() {
		new MyThread().start();
	}

	public boolean getState() {
		return !mStopped;
	}
}
