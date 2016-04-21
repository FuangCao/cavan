package com.cavan.cavanutils;

public class SuClient extends TcpExecClient {

	public SuClient() {
		super("/dev/cavan/network/socket");
	}
}