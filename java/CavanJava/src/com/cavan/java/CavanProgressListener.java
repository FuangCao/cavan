package com.cavan.java;

public abstract class CavanProgressListener {
	private int mProgress;
	private int mProgressMin;
	private int mProgressMax;
	private int mProggressRange;
	private long mValue;
	private long mValueMin;
	private long mValueMax;
	private long mValueRange;

	protected abstract void onProgressUpdated(int progress);

	public CavanProgressListener(int min, int max) {
		setProgressRange(min, max);
	}

	public CavanProgressListener(int max) {
		this(0, max);
	}

	public CavanProgressListener() {
		this(0, 100);
	}

	public void setProgressRange(int min, int max) {
		mProgress = min;
		mProgressMin = min;
		mProgressMax = max;
		mProggressRange = max - min;
	}

	public void setProgressMin(int min) {
		setProgressRange(min, mProgressMax);
	}

	public void setProgressMax(int max) {
		setProgressRange(mProgressMin, max);
	}

	public void setProgress(int progress) {
		if (progress > mProgressMax) {
			progress = mProgressMax;
		} else if (progress < mProgressMin) {
			progress = mProgressMin;
		}

		mProgress = progress;
		onProgressUpdated(progress);
	}

	public void addProgress(int value) {
		setProgress(mProgress + value);
	}

	public void addProgress() {
		addProgress(1);
	}

	public void startProgress() {
		setProgress(mProgressMin);
	}

	public void finishProgress() {
		setProgress(mProgressMax);
	}

	public void setValueRange(long min, long max) {
		setProgressMin(mProgress);

		mValue = min;
		mValueMin = min;
		mValueMax = max;
		mValueRange = max - min;
	}

	public void setValueRange(long max) {
		setValueRange(0, max);
	}

	public void setValue(long value) {
		mValue = value;
		setProgress((int) (mProgressMin + value * mProggressRange / mValueRange));
	}

	public void setValuePersent(int persent) {
		setValue(mValueMin + persent * mValueRange / 100);
	}

	public void addValue(long value) {
		setValue(mValue + value);
	}

	public void startValue() {
		setValue(mValueMin);
	}

	public void finishValue() {
		setValue(mValueMax);
	}
}
