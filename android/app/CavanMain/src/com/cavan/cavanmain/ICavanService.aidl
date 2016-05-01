package com.cavan.cavanmain;

interface ICavanService {
	void startTcpDdService();
	int getTcpDdServicePort();
	void setTcpDdServicePort(int port);
	boolean getTcpDdServiceState();

	void startWebProxyService();
	int getWebProxyServicePort();
	void setWebProxyServicePort(int port);
	boolean getWebProxyServiceState();

	void startFtpService();
	int getFtpServicePort();
	void setFtpServicePort(int port);
	boolean getFtpServiceState();
}
