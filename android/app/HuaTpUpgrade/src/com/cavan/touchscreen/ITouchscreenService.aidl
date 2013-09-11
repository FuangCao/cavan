package com.cavan.touchscreen;

interface ITouchscreenService {
	String getDevName();
	String getDevPath();
	int readFwID();
	String getVendorName(int id);
	void upgradeFirmware(String pathname);
}