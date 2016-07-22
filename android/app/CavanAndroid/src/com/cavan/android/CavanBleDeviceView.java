package com.cavan.android;

import android.content.Context;
import android.widget.Button;

public class CavanBleDeviceView extends Button {

	private CavanBleDevice mDevice;

	public CavanBleDeviceView(Context context, CavanBleDevice device, int index) {
		super(context);
		setDevice(device, index);
	}

	public void updateText() {
		setText(mDevice.toString());
	}

	public void setDevice(CavanBleDevice device, int index) {
		device.setIndex(index);
		mDevice = device;
		updateText();
	}

	public CavanBleDevice getDevice() {
		return mDevice;
	}
}