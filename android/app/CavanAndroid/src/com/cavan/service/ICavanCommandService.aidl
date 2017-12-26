package com.cavan.service;

import com.cavan.service.ICavanCommandServiceCallback;

interface ICavanCommandService {
	void start(String command);
	void stop();
	int getState();
	String getCommand();
	boolean isEnabled();
	boolean addCallback(ICavanCommandServiceCallback callback);
	boolean removeCallback(ICavanCommandServiceCallback callback);
}
