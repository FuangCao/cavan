package com.cavan.accessibility;

import java.util.ArrayList;
import java.util.List;

import com.cavan.android.CavanAndroid;

public class CavanRedPacket {

	protected CavanAccessibilityPackage mPackage;
	protected boolean mCompleted;
	protected boolean mGotoIdle;
	protected boolean mPending;
	protected long mUnpackTime;
	protected int mSendTimes;

	protected CavanRedPacket prev;
	protected CavanRedPacket next;

	public void onAdded() {
		mPackage.onPacketAdded(this);
	}

	public void onRemoved() {
		mPackage.onPacketRemoved(this);
	}

	public CavanRedPacket() {
		clear();
	}

	public void addPrev(CavanAccessibilityPackage pkg, CavanRedPacket node) {
		node.setPackage(pkg);
		node.prev = prev;
		node.next = this;
		prev.next = node;
		prev = node;
	}

	public void addNext(CavanAccessibilityPackage pkg, CavanRedPacket node) {
		node.setPackage(pkg);
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

	protected String getPacketName() {
		return "dummy";
	}

	protected List<String> getOptions(List<String> options) {
		if (mCompleted) {
			options.add("completed");
		}

		if (mPending) {
			options.add("pending");
		}

		if (mGotoIdle) {
			options.add("idle");
		}

		if (mUnpackTime != 0) {
			options.add("unpack=" + mUnpackTime);
		}

		return options;
	}

	@Override
	public String toString() {
		StringBuilder builder = new StringBuilder(getPacketName());

		builder.append('@').append(Integer.toHexString(hashCode()));
		builder.append(": [");

		int index = 0;

		for (String option : getOptions(new ArrayList<String>())) {
			if (index > 0) {
				builder.append(',');
			}

			builder.append(option);
			index++;
		}

		builder.append(']');

		return builder.toString();
	}
}