package com.cavan.cavanjni;

interface ICavanService {
	void start(int port);
	boolean stop();
	int getState();
	boolean isEnabled();
	int getPort();
	String getAction();
}
