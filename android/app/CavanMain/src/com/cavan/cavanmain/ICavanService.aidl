package com.cavan.cavanmain;

interface ICavanService {
	void startTcpDdService();
	boolean getTcpDdServiceState();
	void startWebProxyService();
	boolean getWebProxyServiceState();
	void startFtpService();
	boolean getFtpServiceState();
}
