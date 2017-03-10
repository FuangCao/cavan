package com.cavan.cavanjni;

public class HttpService extends CavanService {

	public static final String NAME = "HTTP";

	@Override
	public String getServiceName() {
		return NAME;
	}

	@Override
	protected void mainServiceLoop(int port) {
		CavanJni.doHttpService("-p", Integer.toString(port));
	}

	@Override
	public int getDefaultPort() {
		return 8021;
	}

	@Override
	public boolean stopService() {
		return CavanJni.kill("http_service");
	}
}