package com.cavan.java;

public class CavanCountedNode<E> {

	private E mValue;
	private int mCount;

	public CavanCountedNode(E value) {
		mCount = 0;
		mValue = value;
	}

	public E getValue() {
		return mValue;
	}

	public void setValue(E value) {
		mValue = value;
		mCount = 1;
	}

	public int getCount() {
		return mCount;
	}

	public void setCount(int count) {
		mCount = count;
	}

	public int add(int count, int max) {
		mCount += count;
		if (mCount > max) {
			mCount = max;
		}

		return mCount;
	}

	public int sub(int count) {
		if (mCount > count) {
			mCount -= count;
		} else {
			mCount = 0;
		}

		return mCount;
	}

	public int increment(int max) {
		if (mCount < max) {
			mCount++;
		}

		return mCount;
	}

	public int decrement() {
		if (mCount > 0) {
			return --mCount;
		}

		return 0;
	}

	public boolean isLessThen(CavanCountedNode<E> node) {
		return mCount == 0 || mCount < node.getCount();
	}

	public boolean isGreaterThen(CavanCountedNode<E> node) {
		return mCount > node.getCount();
	}
}
