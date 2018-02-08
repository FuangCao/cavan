package com.cavan.cavanjni;

public class TcpRepeater extends CavanNativeService {

	public static final String NAME = "TCP_REPEATER";

	public TcpRepeater() {
		super(new CavanCommandTcpRepeater());
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
		return 8864;
	}
}