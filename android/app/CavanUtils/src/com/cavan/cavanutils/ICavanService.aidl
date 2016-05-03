package com.cavan.cavanutils;

interface ICavanService {
	void start(int port);
	boolean stop();
	boolean getState();
	int getPort();
	String getAction();
}
