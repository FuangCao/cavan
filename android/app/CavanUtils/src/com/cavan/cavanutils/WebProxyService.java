package com.cavan.cavanutils;

public class WebProxyService extends CavanService {

	public static final String NAME = "WEB_PROXY";

	@Override
	public String getServiceName() {
		return NAME;
	}

	@Override
	protected void mainServiceLoop(int port) {
		CavanUtils.doWebProxy("-p", Integer.toString(port));
	}

	@Override
	public int getDefaultPort() {
		return 9090;
	}

	@Override
	public boolean stopService() {
		return CavanUtils.kill("web_proxy");
	}
}