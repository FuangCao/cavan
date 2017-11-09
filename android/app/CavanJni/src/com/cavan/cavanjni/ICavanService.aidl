package com.cavan.cavanjni;

interface ICavanService {
	void start(int port);
	void stop();
	int getState();
	boolean isEnabled();
	int getPort();
	String getAction();
}
