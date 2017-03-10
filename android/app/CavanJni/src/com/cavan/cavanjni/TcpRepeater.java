package com.cavan.cavanjni;

public class TcpRepeater extends CavanService {

	public static final String NAME = "TCP_REPEATER";

	@Override
	public String getServiceName() {
		return NAME;
	}

	@Override
	protected void doMainLoop(int port) {
		CavanJni.doTcpRepeater("-p", Integer.toString(port));
	}

	@Override
	public int getDefaultPort() {
		return 8864;
	}

	@Override
	public boolean doStopService() {
		return CavanJni.kill("tcp_repeater");
	}
}