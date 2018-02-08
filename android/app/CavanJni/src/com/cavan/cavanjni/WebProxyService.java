package com.cavan.cavanjni;

public class WebProxyService extends CavanNativeService {

	public static final String NAME = "WEB_PROXY";

	public WebProxyService() {
		super(new CavanCommandWebProxy());
	}

	@Override
	public String getServiceName() {
		return NAME;
	}

	@Override
	protected String[] getCommandArgs(int port) {
		return new String[] { "-p", Integer.toString(port) };
	}

	@Override
	public int getDefaultPort() {
		return 9090;
	}
}