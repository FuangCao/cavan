package com.cavan.cavanmain;

import android.content.Context;
import android.content.Intent;

import com.cavan.android.CavanAndroid;
import com.cavan.java.CavanTcpClient;
import com.cavan.java.CavanTcpPacketClient;
import com.cavan.service.CavanPowerStateListener;
import com.cavan.service.CavanTcpConnService;

public class CavanInputProxyConnService extends CavanTcpConnService implements CavanPowerStateListener {

	public static CavanInputProxyConnService instance;

	private CavanTcpPacketClient mTcpPacketClient = new CavanTcpPacketClient() {

		@Override
		protected int onGetConnOvertime() {
			return 2000;
		}

		@Override
		protected boolean onPacketReceived(byte[] bytes, int length) {
			CavanAndroid.dLog("onPacketReceived: " + length);
			return true;
		}

		@Override
		public long onTcpConnFailed(int times) {
			if (times > 3) {
				return -1;
			}

			return super.onTcpConnFailed(times);
		}
	};

	private CavanTcpClient.SendThread mSendThread = mTcpPacketClient.newSendThread();

	public static Intent getIntent(Context context) {
		return new Intent(context, CavanInputProxyConnService.class);
	}

	public static void startService(Context context) {
		context.startService(getIntent(context));
	}

	public static boolean send(String message) {
		CavanInputProxyConnService service = instance;
		if (service == null) {
			return false;
		}

		return service.getSendThread().send(message);
	}

	public CavanTcpClient.SendThread getSendThread() {
		return mSendThread;
	}

	@Override
	protected CavanTcpClient doCreateTcpClient() {
		mTcpPacketClient.setTcpClientListener(this);
		return mTcpPacketClient;
	}

	@Override
	public void onCreate() {
		super.onCreate();

		instance = this;
		CavanMainApplication.addPowerStateListener(this);
	}

	@Override
	public void onDestroy() {
		instance = null;
		super.onDestroy();
	}

	@Override
	protected int getDefaultPort() {
		return 9981;
	}

	@Override
	public void onScreenOn() {
		mTcpPacketClient.connect();
	}

	@Override
	public void onScreenOff() {}

	@Override
	public void onUserPresent() {}

	@Override
	public void onCloseSystemDialogs(String reason) {}

	@Override
	public void onKeepAliveFailed() {}
}
