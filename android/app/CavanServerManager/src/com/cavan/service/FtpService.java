package com.cavan.service;

import android.content.Intent;

public class FtpService extends CavanService {
	public static final String ACTION_SERVICE_STATE_CHANGE = "com.cavan.service.FTP_STATE_CHANGE";

	public FtpService() {
		super();
		mPort = 6666;
	}

	@Override
	public boolean start(String pathname, int port) {
		return super.start(pathname + " ftp_server -p %d", port);
	}

	protected void notifyStateChanged() {
		Intent intent = new Intent(ACTION_SERVICE_STATE_CHANGE);
		intent.putExtra("state", mState);
		sendBroadcast(intent);
	}
}