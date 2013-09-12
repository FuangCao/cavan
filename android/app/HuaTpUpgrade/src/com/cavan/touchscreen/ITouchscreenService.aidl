package com.cavan.touchscreen;

import com.cavan.touchscreen.DeviceID;

interface ITouchscreenService {
	String getDevName();
	String getDevPath();
	DeviceID readDevID();
	String getFwName();
	void upgradeFirmware(String pathname);
}