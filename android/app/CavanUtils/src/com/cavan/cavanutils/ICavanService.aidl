package com.cavan.cavanutils;

interface ICavanService {
	void start(int port);
	boolean getState();
	int getPort();
	String getAction();
}
