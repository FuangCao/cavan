package com.cavan.accessibility;

public class CavanRedPacket {

	protected CavanAccessibilityPackage mPackage;
	protected long mUnpackTime;

	public CavanRedPacket(CavanAccessibilityPackage pkg) {
		mPackage = pkg;
	}

	public CavanAccessibilityPackage getPackage() {
		return mPackage;
	}

	public synchronized long getUnpackTime() {
		return mUnpackTime;
	}

	public synchronized void setUnpackTime(long time) {
		mUnpackTime = time;
	}

	public synchronized void setUnpackDelay(long delay) {
		setUnpackDelay(System.currentTimeMillis() + delay);
	}

	public boolean send() {
		return false;
	}

}