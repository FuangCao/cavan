package com.cavan.cavanutils;

import android.content.Context;
import android.content.Intent;

public abstract class CavanBaseService {

	private String mName;
	private String mAction;
	private Context mContext;
	private boolean mStopped = true;

	public abstract void runDeamon();

	public CavanBaseService(Context context, String name, String action) {
		mName = name;
		mAction = action;
		mContext = context;
	}

	class MyThread extends Thread {

		@Override
		public void run() {
			if (mStopped) {
				CavanUtils.logE("Enter: service " + mName);
				mStopped = false;
				sendStateBroadcast(true);
				runDeamon();
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
