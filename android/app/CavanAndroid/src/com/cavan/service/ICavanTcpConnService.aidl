package com.cavan.service;

import com.cavan.service.ICavanTcpConnCallback;

interface ICavanTcpConnService {
	boolean addCallback(ICavanTcpConnCallback callback);
	void removeCallback(ICavanTcpConnCallback callback);
	void setAddresses(in List<String> addresses);
	String getCurrentAddress();
	int getState();
}
