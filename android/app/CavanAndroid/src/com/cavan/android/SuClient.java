package com.cavan.android;

public class SuClient extends TcpExecClient {

	public SuClient() {
		super("/dev/cavan/network/socket");
	}
}