package com.cavan.cavanjni;

public class FtpService extends CavanNativeService {

	public static final String NAME = "FTP";

	public FtpService() {
		super(new CavanCommandFtpServer());
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
		return 2121;
	}
}