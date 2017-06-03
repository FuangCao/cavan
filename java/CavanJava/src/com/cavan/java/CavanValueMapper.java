package com.cavan.java;

public class CavanValueMapper {

	public static class Entry {

		private double mValue0;
		private double mValue1;

		public Entry(double v0, double v1) {
			mValue0 = v0;
			mValue1 = v1;
		}

		public double getValue0() {
			return mValue0;
		}

		public void setValue0(double v0) {
			mValue0 = v0;
		}

		public double getValue1() {
			return mValue1;
		}

		public void setValue1(double v1) {
			mValue1 = v1;
		}

		public double sub01() {
			return mValue0 - mValue1;
		}

		public double sub10() {
			return mValue1 - mValue0;
		}

		public double sub0(Entry entry) {
			return mValue0 - entry.getValue0();
		}

		public double sub1(Entry entry) {
			return mValue1 - entry.getValue1();
		}
	}

	public static void main(String[] args) {
		Entry[] entries = {
			new Entry(3.4, 0),
			new Entry(4.2, 100),
		};

		CavanValueMapper mapper = new CavanValueMapper(entries);
		mapper.setOverflowUpEnable();

		CavanJava.dLog("v1 = " + mapper.getDouble1(3.4));
		CavanJava.dLog("v1 = " + mapper.getDouble1(3.6));
		CavanJava.dLog("v1 = " + mapper.getDouble1(4.2));
		CavanJava.dLog("v1 = " + mapper.getDouble1(4.3));

		CavanJava.dLog("v0 = " + mapper.getDouble0(0));
		CavanJava.dLog("v0 = " + mapper.getDouble0(10));
		CavanJava.dLog("v0 = " + mapper.getDouble0(20));
		CavanJava.dLog("v0 = " + mapper.getDouble0(25));
		CavanJava.dLog("v0 = " + mapper.getDouble0(100));
	}

	private static Entry[] sDefaultEntries = {
		new Entry(0, 0), new Entry(1, 1)
	};

	private int mIndex0;
	private int mIndex1;
	private boolean mOverflowUpEnabled;
	private boolean mOverflowDownEnabled;
	private Entry[] mEntries = sDefaultEntries;

	public CavanValueMapper(Entry[] entries) {
		setEntries(entries);
	}

	public CavanValueMapper(double v0, double v1) {
		setEntries(v0, v1);
	}

	public int getIndex0() {
		return mIndex0;
	}

	public void setIndex0(int index0) {
		mIndex0 = index0;
	}

	public int getIndex1() {
		return mIndex1;
	}

	public void setIndex1(int index1) {
		mIndex1 = index1;
	}

	public Entry[] getEntries() {
		return mEntries;
	}

	public void setEntries(Entry[] entries) {
		if (entries.length < 2) {
			if (entries.length > 0) {
				mEntries = new Entry[] {
					sDefaultEntries[0], entries[0]
				};
			} else {
				mEntries = sDefaultEntries;
			}
		} else {
			mEntries = entries;
		}

		mIndex0 = 0;
		mIndex1 = 1;
	}

	public void setEntries(double v0, double v1) {
		Entry[] entries = new Entry[] {
			sDefaultEntries[0],
			new Entry(v0, v1)
		};

		setEntries(entries);
	}

	public Entry getEntry(int index) {
		return mEntries[index];
	}

	public void setEntry(int index, Entry entry) {
		mEntries[index] = entry;
	}

	public boolean isOverflowUpEnabled() {
		return mOverflowUpEnabled;
	}

	public void setOverflowUpEnable(boolean enable) {
		mOverflowUpEnabled = enable;
	}

	public void setOverflowUpEnable() {
		mOverflowUpEnabled = true;
	}

	public boolean isOverflowDownEnabled() {
		return mOverflowDownEnabled;
	}

	public void setOverflowDownEnable(boolean enable) {
		mOverflowDownEnabled = enable;
	}

	public void setOverflowDownEnable() {
		mOverflowDownEnabled = true;
	}

	public Entry getMinEntry() {
		return mEntries[mIndex0];
	}

	public Entry getMaxEntry() {
		return mEntries[mIndex1];
	}

	public double getMinValue0() {
		return mEntries[mIndex0].getValue0();
	}

	public double getMaxValue0() {
		return mEntries[mIndex1].getValue0();
	}

	public double getMinValue1() {
		return mEntries[mIndex0].getValue1();
	}

	public double getMaxValue1() {
		return mEntries[mIndex1].getValue1();
	}

	public double getRange0() {
		return mEntries[mIndex1].getValue0() - mEntries[mIndex0].getValue0();
	}

	public double getRange1() {
		return mEntries[mIndex1].getValue1() - mEntries[mIndex0].getValue1();
	}

	public double getDouble0(double v1) {
		if (v1 < mEntries[mIndex0].getValue1()) {
			do {
				if (mIndex0 > 0) {
					mIndex1 = mIndex0--;
				} else {
					if (mOverflowDownEnabled) {
						break;
					}

					return mEntries[mIndex0].getValue0();
				}
			} while (v1 < mEntries[mIndex0].getValue1());
		} else {
			while (v1 > mEntries[mIndex1].getValue1()) {
				int index = mIndex1 + 1;
				if (index < mEntries.length) {
					mIndex0 = mIndex1;
					mIndex1 = index;
				} else {
					if (mOverflowUpEnabled) {
						break;
					}

					return mEntries[mIndex1].getValue0();
				}
			}
		}

		return getMinValue0() + (v1 - getMinValue1()) * getRange0() / getRange1();
	}

	public double getDouble1(double v0) {
		if (v0 < mEntries[mIndex0].getValue0()) {
			do {
				if (mIndex0 > 0) {
					mIndex1 = mIndex0--;
				} else {
					if (mOverflowDownEnabled) {
						break;
					}

					return mEntries[mIndex0].getValue1();
				}
			} while (v0 < mEntries[mIndex0].getValue0());
		} else {
			while (v0 > mEntries[mIndex1].getValue0()) {
				int index = mIndex1 + 1;
				if (index < mEntries.length) {
					mIndex0 = mIndex1;
					mIndex1 = index;
				} else {
					if (mOverflowUpEnabled) {
						break;
					}

					return mEntries[mIndex1].getValue1();
				}
			}
		}

		return getMinValue1() + (v0 - getMinValue0()) * getRange1() / getRange0();
	}

	public long getLong0(double v1) {
		return Math.round(getDouble0(v1));
	}

	public long getLong1(double v0) {
		return Math.round(getDouble1(v0));
	}
}
