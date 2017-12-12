package com.cavan.service;

import com.cavan.service.ICavanServiceCallback;

interface ICavanService {
	void start(int port);
	void stop();
	int getState();
	boolean isEnabled();
	int getPort();
	boolean addCallback(ICavanServiceCallback callback);
	boolean removeCallback(ICavanServiceCallback callback);
}
