package com.cavan.cavanutils;

public class FtpService extends CavanService {

	public static final String NAME = "FTP";

	@Override
	public String getServiceName() {
		return NAME;
	}

	@Override
	protected void mainServiceLoop(int port) {
		CavanUtils.doFtpServer("-p", Integer.toString(port));
	}

	@Override
	public int getDefaultPort() {
		return 2121;
	}
}
