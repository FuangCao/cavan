package com.cavan.cavanjni;


public class TcpDdService extends CavanService {

	public static final String NAME = "TCP_DD";

	@Override
	public String getServiceName() {
		return NAME;
	}

	@Override
	protected void doMainLoop(int port) {
		CavanJni.doTcpDdServer("-p", Integer.toString(port), "-s", "0");
	}

	@Override
	public int getDefaultPort() {
		return 9898;
	}

	@Override
	public boolean doStopService() {
		return CavanJni.kill("tcp_dd_server");
	}
}