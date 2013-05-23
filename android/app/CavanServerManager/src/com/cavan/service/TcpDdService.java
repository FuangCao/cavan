package com.cavan.service;

import android.content.Intent;

public class TcpDdService extends CavanService {
	public static final String ACTION_SERVICE_STATE_CHANGE = "com.cavan.service.TCP_DD_STATE_CHANGE";

	public TcpDdService() {
		super();
		mPort = 8888;
	}

	@Override
	public boolean start(String pathname, int port) {
		return super.start(pathname + " tcp_dd_server -s 0 -p %d", port);
	}

	protected void notifyStateChanged() {
		Intent intent = new Intent(ACTION_SERVICE_STATE_CHANGE);
		intent.putExtra("state", mState);
		sendBroadcast(intent);
	}
}