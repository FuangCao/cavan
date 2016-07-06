package com.cavan.java;

public class CavanPeakValleyValue {

	public static final int TYPE_FALLING = 0;
	public static final int TYPE_RISING = 1;

	protected int mType;
	protected long mTime;
	protected double mPeakValue;
	protected double mValleyValue;

	public CavanPeakValleyValue(double peak, double valley, int type, long time) {
		super();

		mType = type;
		mTime = time;
		mPeakValue = peak;
		mValleyValue = valley;
	}

	public CavanPeakValleyValue(double peak, double valley, int type) {
		this(peak, valley, type, System.currentTimeMillis());
	}

	public CavanPeakValleyValue(double peak, double valley) {
		this(peak, valley, TYPE_FALLING);
	}

	public CavanPeakValleyValue(double value) {
		this(value, value);
	}

	public double getPeakValue() {
		return mPeakValue;
	}

	public void setPeakValue(double peak) {
		mPeakValue = peak;
	}

	public double getValleyValue() {
		return mValleyValue;
	}

	public void setValleyValue(double valley) {
		mValleyValue = valley;
	}

	public int getType() {
		return mType;
	}

	public void setType(int type) {
		mType = type;
	}

	public long getTime() {
		return mTime;
	}

	public long getTimeEarly(long time) {
		return time - mTime;
	}

	public long getTimeEarly(CavanPeakValleyValue value) {
		return getTimeEarly(value.getTime());
	}

	public long getTimeLate(long time) {
		return mTime - time;
	}

	public long getTimeLate(CavanPeakValleyValue value) {
		return getTimeLate(value.getTime());
	}

	public long getTimeDiff(long time) {
		if (mTime > time) {
			return mTime - time;
		} else {
			return time - mTime;
		}
	}

	public long getTimeDiff(CavanPeakValleyValue value) {
		return getTimeDiff(value.getTime());
	}

	public boolean isFalling() {
		return mType == TYPE_FALLING;
	}

	public boolean isRising() {
		return mType == TYPE_RISING;
	}

	public double getDiff() {
		return mPeakValue - mValleyValue;
	}

	public void extend(CavanPeakValleyValue value) {
		if (value.getPeakValue() > mPeakValue) {
			mPeakValue = value.getPeakValue();
		}

		if (value.getValleyValue() < mValleyValue) {
			mValleyValue = value.getValleyValue();
		}
	}
}
