package com.cavan.accessibility;

import com.cavan.android.CavanAndroid;

public class CavanRedPacket {

	protected CavanAccessibilityPackage mPackage;
	protected boolean mCompleted;
	protected boolean mGotoIdle;
	protected boolean mPending;
	protected long mUnpackTime;
	protected int mSendTimes;

	public CavanRedPacket prev;
	public CavanRedPacket next;

	public void onAdded() {
		mPackage.onPacketAdded(this);
	}

	public void onRemoved() {
		mPackage.onPacketRemoved(this);
	}

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

	public synchronized boolean isCompleted() {
		return mCompleted;
	}

	public synchronized void setCompleted() {
		CavanAndroid.dLog("complete: " + toString());
		mCompleted = true;
		mPending = false;
	}

	public synchronized boolean isPending() {
		return mPending;
	}

	public synchronized void setPending() {
		mPending = true;
	}

	public synchronized boolean needGotoIdle() {
		return mGotoIdle;
	}

	public synchronized void setGotoIdle(boolean enable) {
		mGotoIdle = enable;
	}

	public synchronized void setUnpackDelay(long delay) {
		setUnpackTime(System.currentTimeMillis() + delay);
	}

	public synchronized long getUnpackDelay(long delay) {
		long timeNow = System.currentTimeMillis();
		if (mUnpackTime < timeNow) {
			return delay;
		}

		return mUnpackTime - timeNow;
	}

	public synchronized long getUnpackOver(long time) {
		if (time > mUnpackTime) {
			return time - mUnpackTime;
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
}