package com.cavan.accessibility;

public class CavanRedPacket {

	protected CavanAccessibilityPackage mPackage;
	protected long mUnpackTime;
	protected int mSendTimes;

	public CavanRedPacket prev;
	public CavanRedPacket next;

	public CavanRedPacket() {
		clear();
	}

	public void addPrev(CavanRedPacket node) {
		node.prev = prev;
		node.next = this;
		prev.next = node;
		prev = node;
	}

	public void addNext(CavanRedPacket node) {
		node.prev = this;
		node.next = next;
		next.prev = node;
		next = node;
	}

	public synchronized void clear() {
		prev = next = this;
	}

	public void remove() {
		prev.next = next;
		next.prev = prev;
		clear();
	}

	public synchronized CavanAccessibilityPackage getPackage() {
		return mPackage;
	}

	public synchronized void setPackage(CavanAccessibilityPackage pkg) {
		mPackage = pkg;
	}

	public synchronized long getUnpackTime() {
		return mUnpackTime;
	}

	public synchronized void setUnpackTime(long time) {
		mUnpackTime = time;
	}

	public synchronized void setUnpackDelay(long delay) {
		setUnpackTime(System.currentTimeMillis() + delay);
	}

	public synchronized long getUnpackRemain() {
		if (mUnpackTime > 0) {
			long timeNow = System.currentTimeMillis();
			if (mUnpackTime > timeNow) {
				return mUnpackTime - timeNow;
			}
		}

		return 0;
	}

	public synchronized boolean launch() {
		if (mPackage.launch()) {
			mSendTimes++;
			return true;
		}

		return false;
	}

	public boolean needGotoIdle() {
		return false;
	}

}