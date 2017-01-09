package com.cavan.cavanmain;

public class CavanChatNode {

	private static final long OVERTIME = 7200000;

	private long mTime;
	private String mName;

	public CavanChatNode(String name) {
		setName(name);
	}

	public void setName(String name) {
		mName = name;
		updateTime();
	}

	public String getName() {
		return mName;
	}

	public void updateTime() {
		mTime = System.currentTimeMillis();
	}

	public long getTime() {
		return mTime;
	}

	public boolean isOvertime(long time) {
		return time - mTime > OVERTIME;
	}

	public boolean isMatch(String name) {
		return isMatch(mName, name);
	}

	public static boolean isMatch(String name1, String name2) {
		return name1.contains(name2) || name2.contains(name1);
	}

	@Override
	public String toString() {
		return mName;
	}
}
