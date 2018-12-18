package com.cavan.service;

public interface CavanPowerStateListener {
	public void onScreenOn();
	public void onScreenOff();
	public void onUserPresent();
	public void onCloseSystemDialogs(String reason);
}
