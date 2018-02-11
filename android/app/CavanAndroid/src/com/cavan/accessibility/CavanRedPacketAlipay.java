package com.cavan.accessibility;

import java.util.HashMap;
import java.util.Iterator;

public class CavanRedPacketAlipay extends CavanRedPacket {

	private static long LIFE_TIME = 86400000;

	private static HashMap<String, CavanRedPacketAlipay> sPackets = new HashMap<String, CavanRedPacketAlipay>();

	private static synchronized CavanRedPacketAlipay create(String code) {
		Iterator<CavanRedPacketAlipay> iterator = sPackets.values().iterator();
		long timeNow = System.currentTimeMillis();

		while (iterator.hasNext()) {
			CavanRedPacketAlipay node = iterator.next();
			if (node.getUnpackOver(timeNow) > LIFE_TIME) {
				iterator.remove();
			}
		}

		CavanRedPacketAlipay packet = new CavanRedPacketAlipay(code, timeNow);
		sPackets.put(code, packet);

		CavanAccessibilityService service = CavanAccessibilityService.instance;
		if (service != null) {
			service.onPacketCreated(packet);
		}

		return packet;
	}

	public static synchronized CavanRedPacketAlipay get(String code, boolean create) {
		CavanRedPacketAlipay packet = sPackets.get(code);
		if (packet == null && create) {
			return create(code);
		}

		return packet;
	}

	private String mCode;
	private int mPostTimes;
	private int mCommitTimes;
	private long mRepeatTime;
	private boolean mInvalid;
	private boolean mIgnored;
	private boolean mRepeatable;

	private CavanRedPacketAlipay(String code, long time) {
		mUnpackTime = time;
		mCode = code;
	}

	public String getCode() {
		return mCode;
	}

	public void setCode(String code) {
		mCode = code;
	}

	public synchronized int getPostTimes() {
		return mPostTimes;
	}

	public synchronized int addPostTimes() {
		return ++mPostTimes;
	}

	public synchronized int getCommitTimes() {
		return mCommitTimes;
	}

	public synchronized int addCommitTimes(boolean repeatable) {
		if (repeatable) {
			if (!mRepeatable) {
				updateUnpackTime();
			}
		} else {
			mPending = false;
		}

		mRepeatable = repeatable;
		mInvalid = false;

		return ++mCommitTimes;
	}

	public synchronized long getRepeatTime() {
		return mRepeatTime;
	}

	public synchronized void setRepeatTime(long time) {
		mRepeatTime = time;
	}

	public synchronized boolean isInvalid() {
		return mInvalid;
	}

	public void setInvalid() {
		mInvalid = true;
	}

	public synchronized boolean isIgnored() {
		return mIgnored;
	}

	public synchronized void setIgnored(boolean ignored) {
		mIgnored = ignored;
	}

	public synchronized boolean isRepeatable() {
		return mRepeatable;
	}

	public synchronized void updateUnpackTime() {
		long timeNow = System.currentTimeMillis();
		mUnpackTime = ((timeNow - 5000) / 60000 + 1) * 60000;
	}

	@Override
	public void onAdded() {
		if (mRepeatable) {
			updateUnpackTime();
		}

		mPostTimes = 0;
		mCommitTimes = 0;

		super.onAdded();
	}

	@Override
	public synchronized boolean isCompleted() {
		if (mInvalid || mIgnored) {
			return true;
		}

		if (mRepeatable) {
			if (mRepeatTime == 0) {
				return false;
			}

			long timeNow = System.currentTimeMillis();
			if (mRepeatTime > timeNow) {
				return ((mRepeatTime - timeNow) > 60000);
			}

			return ((timeNow - mRepeatTime) > 20000);
		}

		return super.isCompleted();
	}

	@Override
	public synchronized boolean isPending() {
		if (mInvalid || mIgnored) {
			return false;
		}

		if (mRepeatable) {
			long timeNow = System.currentTimeMillis();
			return ((timeNow - mUnpackTime) < 20000);
		}

		return super.isPending();
	}

	@Override
	public boolean equals(Object o) {
		if (super.equals(o)) {
			return true;
		}

		if (o instanceof CavanRedPacketAlipay) {
			return mCode.equals(((CavanRedPacketAlipay) o).getCode());
		}

		if (o instanceof String) {
			return mCode.equals(o);
		}

		return false;
	}

	@Override
	public String toString() {
		return mCode;
	}
}
