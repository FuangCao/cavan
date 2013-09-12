package com.cavan.touchscreen;

import android.os.Build;

public class TouchscreenFT5216 extends TouchscreenDevice {
	@Override
	public String getDevPath() {
		return "/dev/HUA-FT5216";
	}

	@Override
	public String getDevName() {
		if (Build.BOARD.equals("h2") || Build.BOARD.equals("h4")) {
			return "FT6306";
		}

		return "FT5216";
	}

	@Override
	public DeviceID readDevID() {
		return readDevID("/sys/devices/platform/sc8810-i2c.2/i2c-2/2-0038/firmware_id");
	}

	@Override
	public String getFwName() {
		return getDevName() + ".bin";
	}
}
