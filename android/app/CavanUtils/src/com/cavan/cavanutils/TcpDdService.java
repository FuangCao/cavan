package com.cavan.cavanutils;


public class TcpDdService extends CavanService {

	public static final String NAME = "TCP_DD";

	@Override
	public String getServiceName() {
		return NAME;
	}

	@Override
	protected void mainServiceLoop(int port) {
		CavanUtils.doTcpDdServer("-p", Integer.toString(port), "-s", "0", "--discovery");
	}

	@Override
	public int getDefaultPort() {
		return 8888;
	}
}
