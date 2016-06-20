package com.cavan.android;


public class RemoteCtrlClient extends TcpInputClient {

	private ScanResult mScanResult;

	public RemoteCtrlClient(ScanResult result) {
		super(result.getAddress(), result.getPort(), TCP_REMOTE_CTRL);
		mScanResult = result;
	}

	public boolean isConnectedTo(ScanResult result) {
		return isConnected() && mScanResult.equals(result);
	}

	@Override
	public boolean sendKeyEvent(int code, int value) {
		return sendValue16((short) (code | (value << 15)));
	}
}
