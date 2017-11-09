package com.cavan.cavanjni;

public class FtpService extends CavanNativeService {

	public static final String NAME = "FTP";

	@Override
	public String getServiceName() {
		return NAME;
	}

	@Override
	protected void doMainLoop(int port) {
		CavanJni.doFtpServer("-p", Integer.toString(port));
	}

	@Override
	public int getDefaultPort() {
		return 2121;
	}

	@Override
	public boolean doStopService() {
		return CavanJni.kill("ftp_server");
	}
}