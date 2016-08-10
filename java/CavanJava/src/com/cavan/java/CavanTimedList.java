package com.cavan.java;

import java.util.ArrayList;

@SuppressWarnings("serial")
public class CavanTimedList<E> extends ArrayList<CavanTimedNode<E>> {

	protected double mFreq;
	protected CavanTimedNode<E> mLastNode;

	private long mOverTime;

	public CavanTimedList(long overtime) {
		mOverTime = overtime;
	}

	public void setOverTime(long time) {
		mOverTime = time;
	}

	public double getFreq() {
		return mFreq;
	}

	protected CavanTimedNode<E> removeTimedNode(int index) {
		return remove(index);
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

	protected void addTimedNode(CavanTimedNode<E> node) {
		mLastNode = node;

		removeOvertimeNodes(node.getTime());
		add(node);
	}

	protected double updateFreq() {
		int size = size();
		if (size > 1) {
			int last = size - 1;
			long time = get(last).getTime() - get(0).getTime();

			try {
				mFreq = 1000.0 * last / time;
			} catch (Exception e) {
				mFreq = 0;
			}
		} else if (size < 1) {
			mFreq = 0;
		}

		return mFreq;
	}
}