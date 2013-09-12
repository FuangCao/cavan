package com.cavan.touchscreen;


public class TouchscreenCY8C242 extends TouchscreenDevice {
	@Override
	public String getDevPath() {
		return "/dev/HUA-CY8C242";
	}

	@Override
	public String getDevName() {
		return "CY8C242";
	}

	@Override
	public DeviceID readDevID() {
		return readDevID("/sys/devices/platform/sc8810-i2c.2/i2c-2/2-0024/firmware_id");
	}

	@Override
	public String getFwName() {
		return getDevName().toLowerCase() + ".iic";
	}
}
