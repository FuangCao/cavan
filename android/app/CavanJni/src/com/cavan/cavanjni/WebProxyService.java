package com.cavan.cavanjni;

public class WebProxyService extends CavanService {

	public static final String NAME = "WEB_PROXY";

	@Override
	public String getServiceName() {
		return NAME;
	}

	@Override
	protected void mainServiceLoop(int port) {
		CavanJni.doWebProxy("-p", Integer.toString(port));
	}

	@Override
	public int getDefaultPort() {
		return 9090;
	}

	@Override
	public boolean stopService() {
		return CavanJni.kill("web_proxy");
	}
}