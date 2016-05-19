package com.cavan.cavanjni;

public class TcpDdService extends CavanService {

	public static final String NAME = "TCP_DD";

	@Override
	public String getServiceName() {
		return NAME;
	}

	@Override
	protected void mainServiceLoop(int port) {
		CavanJni.doTcpDdServer("-p", Integer.toString(port), "-s", "0");
	}

	@Override
	public int getDefaultPort() {
		return 8888;
	}

	@Override
	public boolean stopService() {
		return CavanJni.kill("tcp_dd_server");
	}
}