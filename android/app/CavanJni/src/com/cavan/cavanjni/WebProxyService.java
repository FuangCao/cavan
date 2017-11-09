package com.cavan.cavanjni;

public class WebProxyService extends CavanNativeService {

	public static final String NAME = "WEB_PROXY";

	@Override
	public String getServiceName() {
		return NAME;
	}

	@Override
	protected void doMainLoop(int port) {
		CavanJni.doWebProxy("-p", Integer.toString(port));
	}

	@Override
	public int getDefaultPort() {
		return 9090;
	}

	@Override
	public boolean doStopService() {
		return CavanJni.kill("web_proxy");
	}
}