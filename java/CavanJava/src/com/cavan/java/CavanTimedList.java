package com.cavan.java;

import java.util.ArrayList;

@SuppressWarnings("serial")
public class CavanTimedList<E> extends ArrayList<CavanTimedNode<E>> {

	protected long mTime;
	protected CavanTimedNode<E> mLastNode;

	private long mOverTime;

	public CavanTimedList(long overtime) {
		mOverTime = overtime;
	}

	public void setOverTime(long time) {
		mOverTime = time;
	}

	protected CavanTimedNode<E> removeTimedNode(int index) {
		return remove(index);
	}

	protected void addTimedNode(CavanTimedNode<E> node) {
		mLastNode = node;
		mTime = node.getTime();

		while (true) {
			CavanTimedNode<E> first;

			try {
				first = get(0);
			} catch (IndexOutOfBoundsException e) {
				break;
			}

			if (mTime - first.getTime() < mOverTime) {
				break;
			}

			removeTimedNode(0);
		}

		add(node);
	}

	protected double calculateFreq() {
		int last = size() - 1;

		if (last > 0) {
			long time = get(last).getTime() - get(0).getTime();

			try {
				return 1000.0 * last / time;
			} catch (Exception e) {
				return 0;
			}
		}

		return 0;
	}
}