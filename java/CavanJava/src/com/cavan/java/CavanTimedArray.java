package com.cavan.java;

import java.util.ArrayList;

@SuppressWarnings("serial")
public class CavanTimedArray<E> extends ArrayList<CavanTimedNode<E>> {

	protected double mFreq;
	protected long mCycle;
	protected CavanTimedNode<E> mLastNode;

	private long mOverTime;

	public CavanTimedArray(long overtime) {
		mOverTime = overtime;
	}

	public void setOverTime(long time) {
		mOverTime = time;
	}

	public double getFreq() {
		return mFreq;
	}

	public long getCycle() {
		return mCycle;
	}

	protected CavanTimedNode<E> removeTimedNode(int index) {
		return remove(index);
	}


	public void addTimedNode(CavanTimedNode<E> node) {
		mLastNode = node;

		removeOvertimeNodes(node.getTime());
		add(node);
	}

	public void addTimedValue(E value) {
		addTimedNode(new CavanTimedNode<E>(value));
	}

	public E getTimedValue(int index) {
		return get(index).getValue();
	}

	public boolean hasTimedValue(E value) {
		for (CavanTimedNode<E> node : this) {
			if (node.getValue().equals(value)) {
				return true;
			}
		}

		return false;
	}

	protected int removeTimedNodes(long timeNow, long overtime) {
		int count = 0;

		while (true) {
			CavanTimedNode<E> first;

			try {
				first = get(0);
			} catch (IndexOutOfBoundsException e) {
				break;
			}

			if (timeNow - first.getTime() < overtime) {
				break;
			}

			removeTimedNode(0);
			count++;
		}

		return count;
	}

	protected int removeOvertimeNodes(long timeNow) {
		return removeTimedNodes(timeNow, mOverTime);
	}

	protected int removeOvertimeNodes() {
		return removeTimedNodes(System.currentTimeMillis(), mOverTime);
	}

	protected double updateFreq() {
		int size = size();
		if (size > 1) {
			int count = size - 1;
			long timeFirst = get(0).getTime();
			long timeLast = get(count).getTime();
			long timeNow = System.currentTimeMillis();

			mCycle = (timeLast - timeFirst) / count;

			if (timeNow - timeLast > mCycle) {
				mFreq = 1000.0 * size / (timeNow - timeFirst);
			} else {
				mFreq = 1000.0 / mCycle;
			}
		} else if (size < 1) {
			mFreq = 0;
		}

		return mFreq;
	}
}