package com.cavan.cavanjni;

interface ICavanService {
	void start(int port);
	boolean stop();
	boolean getState();
	boolean isEnabled();
	int getPort();
	String getAction();
}
