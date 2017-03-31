package com.cavan.cavanmain;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import android.annotation.SuppressLint;
import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;

import com.cavan.android.CavanAndroid;

@SuppressLint("SimpleDateFormat")
public class RedPacketCode implements Comparable<RedPacketCode> {

	private static int LAST_CODE_SIZE = 9;
	private static long LAST_CODE_OVERTIME = 1800000;
	private static long CODE_OVERTIME = 28800000;
	private static long REPEAT_TIME_ALIGN = 60000;
	private static long TIME_MISTAKE = 10000;

	private static final Pattern sRedPacketCodePattern = Pattern.compile("[\\w￥]+");
	private static final SimpleDateFormat sDateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");

	private static HashMap<String, RedPacketCode> mCodeMap = new HashMap<String, RedPacketCode>();
	private static LinkedList<RedPacketCode> mLastCodes = new LinkedList<RedPacketCode>();

	private int mPostCount;
	private int mCommitCount;
	private boolean mPostPending;

	private long mTime;
	private long mExactTime;
	private long mRepeatTime;

	private String mCode;
	private int mPriority;
	private boolean mValid;
	private boolean mInvalid;
	private boolean mIgnored;
	private boolean mSendDisable;
	private boolean mRecvDisable;
	private boolean mSendPending;
	private boolean mTestOnly;
	private boolean mCompleted;
	private boolean mRepeatable;
	private boolean mMaybeInvalid;

	public static String filtration(String text) {
		StringBuilder builder = new StringBuilder();
		Matcher matcher = sRedPacketCodePattern.matcher(text);
		while (matcher.find()) {
			builder.append(matcher.group());
		}

		return builder.toString();
	}

	public static RedPacketCode getInstence(String code, int priority, boolean create, boolean test, boolean send) {
		synchronized (mCodeMap) {
			Iterator<RedPacketCode> iterator = mCodeMap.values().iterator();
			while (iterator.hasNext()) {
				RedPacketCode node = iterator.next();
				if (node.getTimeout() > CODE_OVERTIME) {
					iterator.remove();
				}
			}

			RedPacketCode node = mCodeMap.get(code);
			if (node == null) {
				if (create) {
					node = new RedPacketCode(code, priority);
					node.setSendPending(send);
					mCodeMap.put(code, node);
				} else {
					return null;
				}
			} else if (send) {
				synchronized (mLastCodes) {
					if (mLastCodes.contains(node)) {
						return node;
					}
				}
			}

			if (create) {
				if (test) {
					node.setTestOnly();
				} else {
					synchronized (mLastCodes) {
						mLastCodes.remove(node);
						mLastCodes.addFirst(node);
						updateLastCodes();
					}
				}
			}

			return node;
		}
	}

	public static RedPacketCode getInstence(String code) {
		return getInstence(code, 0, false, false, false);
	}

	public static RedPacketCode getInstence(Intent intent) {
		String code = intent.getStringExtra("code");
		if (code == null) {
			return null;
		}

		return getInstence(code, 0, false, false, false);
	}

	public static List<RedPacketCode> getLastCodes() {
		synchronized (mLastCodes) {
			updateLastCodes();
			return mLastCodes;
		}
	}

	public static int updateLastCodes() {
		int count = 0;

		synchronized (mLastCodes) {
			while (mLastCodes.size() > LAST_CODE_SIZE && mLastCodes.getLast().getTimeout() > LAST_CODE_OVERTIME) {
				mLastCodes.removeLast();
				count++;
			}
		}

		return count;
	}

	public static RedPacketCode update(Context context, String code, long time, boolean ignore) {
		RedPacketCode node = getInstence(code, 0, true, false, false);
		if (node == null) {
			return null;
		}

		node.setExactTime(time);
		node.setIgnored(ignore);
		node.setRepeatable();

		AlarmManager manager = (AlarmManager) CavanAndroid.getCachedSystemService(context, Context.ALARM_SERVICE);
		if (manager != null) {
			Intent intent = new Intent(context, RedPacketAlarmReceiver.class).putExtra("code", code);
			PendingIntent operation = PendingIntent.getBroadcast(context, code.hashCode(), intent, PendingIntent.FLAG_UPDATE_CURRENT);

			if (ignore) {
				manager.cancel(operation);
			} else {
				long timeNow = System.currentTimeMillis();
				if (time < timeNow) {
					manager.cancel(operation);
				} else {
					manager.set(AlarmManager.RTC_WAKEUP, time - 20000, operation);
				}
			}
		}

		return node;
	}

	private RedPacketCode(String code, int priority) {
		mCode = code;
		mPriority = priority;
		mTime = System.currentTimeMillis();
	}

	synchronized public int getPriority() {
		return mPriority;
	}

	synchronized public int setPriority(int priority) {
		return mPriority;
	}

	synchronized public long getTime() {
		return mTime;
	}

	synchronized public void setTime(long time) {
		mTime = time;
	}

	synchronized public long updateTime() {
		long time = System.currentTimeMillis();

		if (mTime > time) {
			return mTime - time;
		}

		mTime = time;

		return 0;
	}

	synchronized public String getCode() {
		return mCode;
	}

	synchronized public void setCode(String code) {
		mCode = code;
	}

	synchronized public void setTestOnly() {
		mTestOnly = true;
	}

	synchronized public boolean isTestOnly() {
		return mTestOnly;
	}

	synchronized void setSendPending(boolean pending) {
		mSendPending = pending;
	}

	synchronized boolean isSendPending() {
		return mSendPending;
	}

	synchronized public void setDelay(long delay) {
		mTime = System.currentTimeMillis() + delay;
	}

	synchronized public long getDelay() {
		long time = System.currentTimeMillis();
		if (mTime > time) {
			return mTime - time;
		}

		return 0;
	}

	synchronized public long getTimeout() {
		long time = System.currentTimeMillis();
		if (mTime < time) {
			return time - mTime;
		}

		return 0;
	}

	synchronized public void setValid() {
		mValid = true;
		mInvalid = false;
		mMaybeInvalid = false;
	}

	synchronized public boolean isValid() {
		return mValid;
	}

	synchronized public void setInvalid() {
		mInvalid = true;
	}

	synchronized public boolean isInvalid() {
		if (mValid) {
			return false;
		}

		return mInvalid;
	}

	synchronized public void setSendDisable() {
		mSendDisable = true;
	}

	synchronized public boolean isSendDisabled() {
		return mSendDisable;
	}

	synchronized public boolean isSendEnabled() {
		if (mRepeatable) {
			return true;
		}

		return !mSendDisable;
	}

	synchronized public void setRecvDisable() {
		mRecvDisable = true;
	}

	synchronized public boolean isRecvDisabled() {
		return mRecvDisable;
	}

	synchronized public boolean isRecvEnabled() {
		if (mRepeatable) {
			return true;
		}

		return !mRecvDisable;
	}

	synchronized public boolean isRepeatable() {
		return mRepeatable;
	}

	synchronized public void setRepeatable() {
		mRepeatable = true;
	}

	synchronized public void updateRepeatTime(Context context) {
		mRepeatable = true;
		mRepeatTime = System.currentTimeMillis() + REPEAT_TIME_ALIGN + TIME_MISTAKE;
		mRepeatTime -= mRepeatTime % REPEAT_TIME_ALIGN;
		mTime = mRepeatTime - MainActivity.getCommitAhead(context);
	}

	synchronized public long getRepeatTime() {
		if (mRepeatable) {
			return mRepeatTime;
		}

		return 0;
	}

	synchronized public long getRepeatTimeout() {
		return System.currentTimeMillis() - mRepeatTime;
	}

	synchronized public void setCompleted() {
		mCompleted = true;
	}

	synchronized public boolean isCompleted() {
		if (mInvalid) {
			return true;
		}

		if (mRepeatTime > 0) {
			return false;
		}

		return mCompleted;
	}

	synchronized public int getCommitCount() {
		if (mValid) {
			return 0;
		}

		return mCommitCount;
	}

	synchronized public void setCommitCount(int count) {
		mPostPending = false;
		mMaybeInvalid = false;
		mCommitCount = count;
	}

	synchronized public boolean canRemove() {
		return mPostCount > 0;
	}

	synchronized public int getPostCount() {
		return mPostCount;
	}

	synchronized public void setPostPending(boolean pending) {
		mPostPending = pending;
		mMaybeInvalid = false;
	}

	synchronized public boolean maybeInvalid() {
		if (mValid) {
			return false;
		}

		return mMaybeInvalid;
	}

	synchronized public boolean setPostComplete() {
		if (!mPostPending) {
			return false;
		}

		if (mPriority > 0 && mCommitCount > 0) {
			mPriority--;
		}

		setCommitCount(mCommitCount + 1);
		mMaybeInvalid = true;
		mPostCount++;

		return true;
	}

	synchronized void setIgnored(boolean ignored) {
		mIgnored = ignored;
	}

	synchronized boolean isIgnored() {
		return mIgnored;
	}

	synchronized void setExactTime(long time) {
		mExactTime = time;
	}

	synchronized long getExactTime() {
		return mExactTime;
	}

	@Override
	public boolean equals(Object obj) {
		if (mCode == obj) {
			return true;
		}

		if (obj instanceof RedPacketCode) {
			obj = ((RedPacketCode) obj).getCode();
		} else if (obj != null) {
			obj = obj.toString();
		} else {
			return false;
		}

		return mCode.equals(obj);
	}

	@Override
	public int hashCode() {
		return mCode.hashCode();
	}

	@Override
	public String toString() {
		StringBuilder builder = new StringBuilder(mCode);

		if (mPriority > 0) {
			builder.append(", Priority:" + mPriority);
		}

		if (mTime > 0) {
			builder.append(", ");
			builder.append(sDateFormat.format(new Date(mTime)));
		}

		if (mSendDisable) {
			builder.append(", SendDisabled");
		}

		if (mRecvDisable) {
			builder.append(", RecvDisabled");
		}

		if (mSendPending) {
			builder.append(", SendPending");
		}

		if (mCompleted) {
			builder.append(", Completed");
		}

		if (mRepeatTime > 0) {
			builder.append(", Repeatable:");
			builder.append(sDateFormat.format(new Date(mRepeatTime)));
		} else if (mValid) {
			builder.append(", Valid");
		} else if (mInvalid) {
			builder.append(", Invalid");
		} else if (mMaybeInvalid) {
			builder.append(", MaybeInvalid");
		}

		return builder.toString();
	}

	@Override
	public int compareTo(RedPacketCode another) {
		if (another == null) {
			return 1;
		}

		if (mPriority != another.getPriority()) {
			return mPriority - another.getPriority();
		}

		return (int) (another.getTime() - mTime);
	}

	public boolean needSkip() {
		if (mInvalid) {
			CavanAndroid.dLog("skip invalid code: " + mCode);
			return true;
		}

		if (mIgnored) {
			CavanAndroid.dLog("skip ignored code: " + mCode);
			return true;
		}

		if (mCompleted) {
			CavanAndroid.dLog("skip completed code: " + mCode);
			return true;
		}

		CavanAndroid.dLog("mExactTime = " + mExactTime);

		if (mExactTime != 0) {
			long timeNow = System.currentTimeMillis();

			if (timeNow > (mExactTime + 60000)) {
				CavanAndroid.dLog("skip overtime code: " + mCode);
				return true;
			}

			if (timeNow < (mExactTime - 60000)) {
				CavanAndroid.dLog("skip early code: " + mCode);
				return true;
			}
		}

		return false;
	}
}
