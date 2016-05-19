package com.cavan.cavanjni;

public class FtpService extends CavanService {

	public static final String NAME = "FTP";

	@Override
	public String getServiceName() {
		return NAME;
	}

	@Override
	protected void mainServiceLoop(int port) {
		CavanJni.doFtpServer("-p", Integer.toString(port));
	}

	@Override
	public int getDefaultPort() {
		return 2121;
	}

	@Override
	public boolean stopService() {
		return CavanJni.kill("ftp_server");
	}
}