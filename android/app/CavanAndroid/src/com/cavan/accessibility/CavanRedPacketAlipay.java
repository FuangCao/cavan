package com.cavan.accessibility;

import java.util.Date;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;

import com.cavan.android.CavanAndroid;

public class CavanRedPacketAlipay extends CavanRedPacket {

	private static long LIFE_TIME = 86400000;
	private static long RECENT_OVERTIME = 1800000;
	private static int RECENT_SIZE = 12;

	private static final Pattern sRedPacketCodePattern = Pattern.compile("[\\w￥]+");
	private static LinkedList<CavanRedPacketAlipay> mRecentPackets = new LinkedList<CavanRedPacketAlipay>();
	private static HashMap<String, CavanRedPacketAlipay> sPackets = new HashMap<String, CavanRedPacketAlipay>();

	public static String filtration(String text) {
		StringBuilder builder = new StringBuilder();
		Matcher matcher = sRedPacketCodePattern.matcher(text);
		while (matcher.find()) {
			builder.append(matcher.group());
		}

		return builder.toString();
	}

	public static CavanRedPacketAlipay update(String code, long time, boolean ignore) {
		CavanRedPacketAlipay packet = get(code, false);
		if (packet == null) {
			return null;
		}

		packet.setUnpackTime(time);
		packet.setIgnored(ignore);
		packet.setRepeatable();
		packet.setNetworkEnable(true, true);

		CavanAccessibilityService service = CavanAccessibilityService.instance;
		if (service == null) {
			return packet;
		}

		AlarmManager manager = (AlarmManager) CavanAndroid.getSystemServiceCached(service, Context.ALARM_SERVICE);
		if (manager == null) {
			return packet;
		}

		Intent intent = new Intent(service, service.getBroadcastReceiverClass()).setAction(CavanAccessibilityService.ACTION_CODE_RECEIVED).putExtra("code", code);
		PendingIntent operation = PendingIntent.getBroadcast(service, code.hashCode(), intent, PendingIntent.FLAG_UPDATE_CURRENT);

		manager.cancel(operation);

		if (!ignore) {
			long timeNow = System.currentTimeMillis();
			long timeAlarm = time - 40000;

			if (timeAlarm > timeNow) {
				CavanAndroid.dLog("timeAlarm = " + sDateFormat.format(new Date(timeAlarm)));
				CavanAndroid.setAlarm(manager, timeAlarm, operation);
			}
		}

		return packet;
	}

	public static List<CavanRedPacketAlipay> getRecentPackets() {
		synchronized (mRecentPackets) {
			updateRecentPackets();
			return mRecentPackets;
		}
	}

	public static int updateRecentPackets() {
		int count = 0;

		synchronized (mRecentPackets) {
			while (mRecentPackets.size() > RECENT_SIZE && mRecentPackets.getLast().getUnpackOver() > RECENT_OVERTIME) {
				mRecentPackets.removeLast();
				count++;
			}
		}

		return count;
	}

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
		CavanAndroid.dLog("put: " + code + " -> " + packet);

		return packet;
	}

	public static synchronized CavanRedPacketAlipay get(String code, boolean create, boolean report) {
		CavanRedPacketAlipay packet = sPackets.get(code);
		CavanAndroid.dLog("get: " + code + " <- " + packet);

		if (packet == null) {
			if (!create) {
				return null;
			}

			packet = create(code);

			if (report) {
				CavanAccessibilityAlipay alipay = CavanAccessibilityAlipay.instance;
				if (alipay != null) {
					alipay.onPacketCreated(packet);
				}
			}
		}

		synchronized (mRecentPackets) {
			updateRecentPackets();
			mRecentPackets.remove(packet);
			mRecentPackets.addFirst(packet);
		}

		return packet;
	}

	public static synchronized CavanRedPacketAlipay get(String code, boolean create) {
		return get(code, create, true);
	}

	public static synchronized CavanRedPacketAlipay get(String code) {
		return get(code, false, false);
	}

	private String mCode;
	private int mPostTimes;
	private int mCommitTimes;
	private long mRepeatTime;
	private boolean mInvalid;
	private boolean mIgnored;
	private boolean mRepeatable;
	private boolean mSendEnabled = true;
	private boolean mRecvEnabled = true;

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

	public synchronized void setRepeatable() {
		mRepeatable = true;
	}

	public synchronized boolean isRepeatable() {
		return mRepeatable;
	}

	public synchronized void updateUnpackTime() {
		long timeNow = System.currentTimeMillis();
		mUnpackTime = ((timeNow - 5000) / 60000 + 1) * 60000;
	}

	public synchronized void setSendEnable(boolean enable) {
		mSendEnabled = enable;
	}

	public synchronized boolean isSendEnabled() {
		return mSendEnabled;
	}

	public synchronized void setRecvEnable(boolean enable) {
		mRecvEnabled = enable;
	}

	public synchronized boolean isRecvEnabled() {
		return mRecvEnabled;
	}

	public synchronized void setNetworkEnable(boolean sendEnable, boolean recvEnable) {
		mSendEnabled = sendEnable;
		mRecvEnabled = recvEnable;
	}

	@Override
	public String getPacketName() {
		return "支付宝口令: " + mCode;
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
	protected List<String> getOptions(List<String> options) {
		if (mInvalid) {
			options.add("invalid");
		}

		if (mRepeatable) {
			if (mRepeatTime == 0) {
				options.add("repeatable");
			} else {
				options.add("repeat=" + mRepeatTime);
			}
		}

		if (mIgnored) {
			options.add("ignored");
		}

		return super.getOptions(options);
	}
}
