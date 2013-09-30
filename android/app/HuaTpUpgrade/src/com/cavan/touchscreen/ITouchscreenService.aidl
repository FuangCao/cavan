package com.cavan.touchscreen;

import com.cavan.touchscreen.DeviceID;

interface ITouchscreenService {
	String getDevName();
	String getDevPath();
	DeviceID readDevID();
	String getFwName();
	List<String> findFirmware();
	void upgradeFirmware(String pathname);
}